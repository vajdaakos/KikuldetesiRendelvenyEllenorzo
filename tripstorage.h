#ifndef TRIPSTORAGE_H
#define TRIPSTORAGE_H

#include "trip.h"
#include "tripmetadata.h"

#include <QRangeModel>
#include <QString>
#include <QVector>

class TripStorage : public QRangeModel
{
    Q_OBJECT

public:
    explicit TripStorage(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setTrips(const QVector<QVector<Trip>> &trips, const QVector<TripMetaData> &metadata);
    bool showPreviousFile();
    bool showNextFile();
    bool showFileAt(qsizetype index);

    const QVector<QVector<Trip>> &trips() const;
    const QVector<TripMetaData> &fileMetadata() const;

    qsizetype currentFile() const;
    const TripMetaData &currentMetadata() const;
    TripMetaData metadataAt(qsizetype index) const;
    QString currentFileName() const;
    QString fileNameAt(qsizetype index) const;
    bool hasPrevious() const;
    bool hasNext() const;

private:
    void showFile(qsizetype index);
    void applyCurrentMetadata();

    QVector<QVector<Trip>> m_trips;
    QVector<Trip> m_rows;
    QVector<TripMetaData> m_file_metadata;
    TripMetaData m_trip_meta_data;
    qsizetype m_currentFile = -1;
};

#endif
