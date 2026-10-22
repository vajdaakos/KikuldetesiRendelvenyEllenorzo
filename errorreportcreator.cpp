#include "errorreportcreator.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringConverter>
#include <QTextStream>
#include <QWidget>

#include <functional>

QString ErrorReportCreator::csvField(const QString &value)
{
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

void ErrorReportCreator::writeRow(QTextStream &out, const QStringList &fields)
{
    QStringList escaped;
    escaped.reserve(fields.size());
    for (const QString &field : fields)
        escaped.append(csvField(field));
    out << escaped.join(QLatin1Char(',')) << '\n';
}


QStringList ErrorReportCreator::metadataFields(const TripMetaData *fileMetadata) const
{
    const QStringList values{
        fileMetadata ? fileMetadata->m_filename : QString{},
        fileMetadata ? fileMetadata->m_serial_number : QString{},
        fileMetadata ? fileMetadata->m_employee : QString{},
        fileMetadata ? QString::number(fileMetadata->m_employee_id) : QString{},
    };
    QStringList fields;
    const qsizetype count = qMin(m_metadataHeaders.size(), values.size());
    for (qsizetype index = 0; index < count; ++index) {
        if (m_metadataHeaders.at(index).second)
            fields.append(values.at(index));
    }
    return fields;
}

QStringList ErrorReportCreator::selectedMetadataHeaders() const
{
    QStringList headers;
    for (const auto &header : m_metadataHeaders) {
        if (header.second)
            headers.append(QCoreApplication::translate(kColumnHeaderContext, header.first.toUtf8().constData()));
    }
    return headers;
}

QStringList ErrorReportCreator::selectedColumnHeaders() const
{
    QStringList headers;
    for (const auto &header : m_columnHeaders) {
        if (header.second)
            headers.append(QCoreApplication::translate(kColumnHeaderContext, header.first.toUtf8().constData()));
    }
    return headers;
}

QStringList ErrorReportCreator::reportRow(const TripMetaData *fileMetadata, const Trip &trip)
{
    QStringList row = metadataFields(fileMetadata);
    row += tripFields(trip);
    return row;
}

QString ErrorReportCreator::csvFileName(const QString &employee, const QString &sourceFileName)
{
    QString employeePart = employee;
    QString sourcePart = sourceFileName;
    employeePart.replace(QLatin1Char(' '), QLatin1Char('_'));
    sourcePart.replace(QLatin1Char(' '), QLatin1Char('_'));
    return QStringLiteral("%1-%2.csv").arg(employeePart, sourcePart);
}

bool ErrorReportCreator::writeCsv(const QString &filePath, const QStringList &header, const QVector<QStringList> &rows, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);
    writeRow(out, header);
    for (const QStringList &row : rows)
        writeRow(out, row);
    out.flush();
    if (out.status() != QTextStream::Ok || file.error() != QFile::NoError) {
        if (error)
            *error = file.errorString();
        return false;
    }
    return true;
}

QString ErrorReportCreator::sourceFileName(const TripMetaData *metadata, const QVector<Trip> &trips)
{
    QString fileName;
    if (metadata && !metadata->m_filename.isEmpty())
        fileName = metadata->m_filename;
    else if (!trips.isEmpty())
        fileName = QFileInfo(trips.constFirst().m_sourcefile).fileName();
    return QFileInfo(fileName).completeBaseName();
}



ErrorReportCreator::ErrorReportCreator(QObject *parent)
    : QRangeModel(std::ref(m_rows), parent)
    , m_settings(*this)
{
    connect(&m_settings, &SettingsManager::settingsLoaded,
            this, &ErrorReportCreator::applyLoadedSettings);
}

void ErrorReportCreator::applyLoadedSettings(const HeaderSettings &columnHeaders,
                                             const HeaderSettings &metadataHeaders)
{
    m_columnHeaders = columnHeaders;
    m_metadataHeaders = metadataHeaders;
}

QVariant ErrorReportCreator::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_reportReady) {
        if (section >= 0 && section < m_reportHeaders.size())
            return m_reportHeaders.at(section);
    }
    return QRangeModel::headerData(section, orientation, role);
}

void ErrorReportCreator::create(const QVector<QVector<Trip>> &trips,
                                const QVector<QVector<const Trip*>> &errors,
                                const QVector<const TripMetaData *> &metadata,
                                const QString &directoryPath)
{
    QVector<QStringList> rows;
    for (qsizetype fileIndex = 0; fileIndex < errors.size(); ++fileIndex) {
        const TripMetaData *fileMetadata = fileIndex < metadata.size() ? metadata.at(fileIndex) : nullptr;
        for (const Trip *trip : errors.at(fileIndex)) {
            if (!trip)
                continue;
            rows.append(reportRow(fileMetadata, *trip));
        }
    }

    QStringList reportHeaders = selectedMetadataHeaders();
    reportHeaders += selectedColumnHeaders();

    beginResetModel();
    m_rows = std::move(rows);
    m_reportHeaders = reportHeaders;
    m_reportReady = true;
    endResetModel();

    QWidget *parentWidget = nullptr;
    for (QObject *ancestor = QObject::parent(); ancestor != nullptr; ancestor = ancestor->parent()) {
        if (auto *widget = qobject_cast<QWidget *>(ancestor)) {
            parentWidget = widget;
            break;
        }
    }

    const QString reportDirectory = QDir(directoryPath).filePath(QStringLiteral("report"));
    if (!QDir().mkpath(reportDirectory)) {
        QMessageBox::warning(
            parentWidget, tr("Hibariport"),
            tr("A %1 könyvtár nem hozható létre.").arg(QDir::toNativeSeparators(reportDirectory)));
        return;
    }

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy_MM_dd_HH-mm"));
    const QString filePath = QDir(reportDirectory).filePath(QStringLiteral("report_%1.csv").arg(stamp));

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(
            parentWidget, tr("Hibariport"),
            tr("A %1 fájl nem írható: %2")
                .arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);
    writeRow(out, m_reportHeaders);
    for (const QStringList &row : m_rows)
        writeRow(out, row);
    out.flush();
    if (out.status() != QTextStream::Ok || file.error() != QFile::NoError) {
        const QString detail = file.errorString().isEmpty()
                                   ? tr("Az írás közben hiba történt.")
                                   : file.errorString();
        QMessageBox::warning(
            parentWidget, tr("Hibariport"),
            tr("A %1 fájl nem írható: %2")
                .arg(QDir::toNativeSeparators(filePath), detail));
        return;
    }
    file.close();

    QStringList fileExportErrors;
    for (qsizetype fileIndex = 0; fileIndex < errors.size(); ++fileIndex) {
        QVector<QStringList> exportedRows;
        exportedRows.reserve(errors.at(fileIndex).size());
        const TripMetaData *fileMetadata = fileIndex < metadata.size() ? metadata.at(fileIndex) : nullptr;
        for (const Trip *trip : errors.at(fileIndex)) {
            if (!trip)
                continue;
            QStringList row = metadataFields(fileMetadata);
            row += tripFields(*trip);
            exportedRows.append(row);
        }
        if (exportedRows.isEmpty())
            continue;

        const QVector<Trip> fileTrips = fileIndex < trips.size() ? trips.at(fileIndex) : QVector<Trip>{};
        const QString exportedName = csvFileName(
            fileMetadata ? fileMetadata->m_employee : QString{},
            sourceFileName(fileMetadata, fileTrips));
        const QString exportedPath = QDir(reportDirectory).filePath(exportedName);

        QString writeError;
        if (!writeCsv(exportedPath, m_reportHeaders, exportedRows, &writeError))
            fileExportErrors.append(tr("%1: %2").arg(QDir::toNativeSeparators(exportedPath), writeError));
    }

    if (fileExportErrors.isEmpty()) {
        QMessageBox::information(
            parentWidget, tr("Hibariport"),
            tr("A hibariport elkészült a %1 fájlba és a hibariport fülre. A hibát tartalmazó PDF-ek CSV-je a hibás utakról a report mappába került.")
                .arg(QDir::toNativeSeparators(filePath)));
    } else {
        QMessageBox::warning(
            parentWidget, tr("Hibariport"),
            tr("A hibariport elkészült a %1 fájlba és a hibariport fülre, de néhány PDF CSV-exportja sikertelen:\n\n%2")
                .arg(QDir::toNativeSeparators(filePath), fileExportErrors.join(QLatin1Char('\n'))));
    }
}

QStringList ErrorReportCreator::tripFields(const Trip &trip)
{
    QStringList fields;
    if(m_columnHeaders.at(0).second)
    {
        fields.append(QString::number(trip.id));
    }
    if(m_columnHeaders.at(1).second)
    {
        fields.append(trip.m_sourcefile);
    }
    if(m_columnHeaders.at(2).second)
    {
        fields.append(trip.m_trip_date_begin);
    }
    if(m_columnHeaders.at(3).second)
    {
        fields.append(trip.m_trip_date_end);
    }
    if(m_columnHeaders.at(4).second)
    {
        fields.append(trip.m_purpose);
    }
    if(m_columnHeaders.at(5).second)
    {
        fields.append(trip.m_from);
    }
    if(m_columnHeaders.at(6).second)
    {
        fields.append(trip.m_to);
    }
    if(m_columnHeaders.at(7).second)
    {
        fields.append(QString::number(trip.m_length));
    }
    if(m_columnHeaders.at(8).second)
    {
        fields.append(QString::number(trip.m_fuel_price));
    }
    if(m_columnHeaders.at(9).second)
    {
        fields.append(QString::number(trip.m_fuel_cost));
    }
    if(m_columnHeaders.at(10).second)
    {
        fields.append(QString::number(trip.m_maintenance_cost_factor1));
    }
    if(m_columnHeaders.at(11).second)
    {
        fields.append(QString::number(trip.m_maintenance_cost_factor2));
    }
    if(m_columnHeaders.at(12).second)
    {
        fields.append(QString::number(trip.m_total_cost));
    }
    return fields;
}

