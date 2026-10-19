#include "errorreportcreator.h"
#include "columnheaders.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringConverter>
#include <QTextStream>
#include <QWidget>

#include <functional>

namespace {

QString csvField(const QString &value)
{
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

void writeRow(QTextStream &out, const QStringList &fields)
{
    QStringList escaped;
    escaped.reserve(fields.size());
    for (const QString &field : fields)
        escaped.append(csvField(field));
    out << escaped.join(QLatin1Char(',')) << '\n';
}

QStringList tripFields(const Trip &trip)
{
    return {
        QString::number(trip.id),
        trip.m_sourcefile,
        trip.m_trip_date_begin,
        trip.m_trip_date_end,
        trip.m_purpose,
        trip.m_from,
        trip.m_to,
        QString::number(trip.m_length),
        QString::number(trip.m_fuel_price),
        QString::number(trip.m_fuel_cost),
        QString::number(trip.m_maintenance_cost_factor1),
        QString::number(trip.m_maintenance_cost_factor2),
        QString::number(trip.m_total_cost),
    };
}

QStringList reportRow(const TripMetaData *fileMetadata, const Trip &trip)
{
    QStringList row{
        fileMetadata ? fileMetadata->m_filename : QString{},
        fileMetadata ? fileMetadata->m_serial_number : QString{},
        fileMetadata ? fileMetadata->m_employee : QString{},
        fileMetadata ? QString::number(fileMetadata->m_employee_id) : QString{},
    };
    row += tripFields(trip);
    return row;
}

QString csvFileName(const QString &employee, const QString &sourceFileName)
{
    QString employeePart = employee;
    QString sourcePart = sourceFileName;
    employeePart.replace(QLatin1Char(' '), QLatin1Char('_'));
    sourcePart.replace(QLatin1Char(' '), QLatin1Char('_'));
    return QStringLiteral("%1-%2.csv").arg(employeePart, sourcePart);
}

bool writeCsv(const QString &filePath, const QStringList &header, const QVector<QStringList> &rows, QString *error)
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

QString sourceFileName(const TripMetaData *metadata, const QVector<Trip> &trips)
{
    QString fileName;
    if (metadata && !metadata->m_filename.isEmpty())
        fileName = metadata->m_filename;
    else if (!trips.isEmpty())
        fileName = QFileInfo(trips.constFirst().m_sourcefile).fileName();
    return QFileInfo(fileName).completeBaseName();
}

}

ErrorReportCreator::ErrorReportCreator(QObject *parent)
    : QRangeModel(std::ref(m_rows), parent)
{
}

QVariant ErrorReportCreator::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        const QStringList headers = reportColumnHeaders();
        if (section >= 0 && section < headers.size())
            return headers.at(section);
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

    beginResetModel();
    m_rows = std::move(rows);
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
    writeRow(out, reportColumnHeaders());
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
        for (const Trip *trip : errors.at(fileIndex)) {
            if (!trip)
                continue;
            exportedRows.append(tripFields(*trip));
        }
        if (exportedRows.isEmpty())
            continue;

        const TripMetaData *fileMetadata = fileIndex < metadata.size() ? metadata.at(fileIndex) : nullptr;
        const QVector<Trip> fileTrips = fileIndex < trips.size() ? trips.at(fileIndex) : QVector<Trip>{};
        const QString exportedName = csvFileName(
            fileMetadata ? fileMetadata->m_employee : QString{},
            sourceFileName(fileMetadata, fileTrips));
        const QString exportedPath = QDir(reportDirectory).filePath(exportedName);

        QString writeError;
        if (!writeCsv(exportedPath, tripColumnHeaders(), exportedRows, &writeError))
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
