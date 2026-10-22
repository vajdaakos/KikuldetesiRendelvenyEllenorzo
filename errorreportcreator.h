#ifndef ERRORREPORTCREATOR_H
#define ERRORREPORTCREATOR_H

#include "settingsmanager.h"
#include "trip.h"
#include "tripmetadata.h"
#include "columnheaders.h"

#include <QRangeModel>
#include <QStringList>
#include <QVector>

class ErrorReportCreator : public QRangeModel
{
    Q_OBJECT

public:
    explicit ErrorReportCreator(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void create(const QVector<QVector<Trip>> &trips,
                const QVector<QVector<const Trip*>> &errors,
                const QVector<const TripMetaData *> &metadata,
                const QString &directoryPath);
    SettingsManager* getSettingsManager(){return &m_settings;}

public slots:
    void applyLoadedSettings(const HeaderSettings &columnHeaders,
                             const HeaderSettings &metadataHeaders);

private:

    QString sourceFileName(const TripMetaData *metadata, const QVector<Trip> &trips);
    QString csvFileName(const QString &employee, const QString &sourceFileName);
    bool writeCsv(const QString &filePath, const QStringList &header, const QVector<QStringList> &rows, QString *error);
    void writeRow(QTextStream &out, const QStringList &fields);
    QStringList reportRow(const TripMetaData *fileMetadata, const Trip &trip);
    QStringList metadataFields(const TripMetaData *fileMetadata) const;
    QStringList selectedMetadataHeaders() const;
    QStringList selectedColumnHeaders() const;
    QString csvField(const QString &value);
    QStringList tripFields(const Trip &trip);
    QVector<QStringList> m_rows;
    QStringList m_reportHeaders;
    bool m_reportReady = false;
    SettingsManager m_settings;
    HeaderSettings m_columnHeaders = kDefaultColumnHeaders;
    HeaderSettings m_metadataHeaders = kDefaultMetadataHeaders;
};

#endif
