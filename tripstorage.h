#ifndef TRIPSTORAGE_H
#define TRIPSTORAGE_H

#include "trip.h"
#include "tripmetadata.h"

#include <QRangeModel>
#include <QString>
#include <QVector>

class ErrorStorage;

class TripStorage : public QRangeModel
{
    Q_OBJECT

public:
    explicit TripStorage(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setErrorStorage(const ErrorStorage *errors);
    void refreshErrorMarks();

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
    bool hasTripsAt(qsizetype index) const;
    bool hasPrevious() const;
    bool hasNext() const;

private:
    qsizetype previousTripFile() const;
    qsizetype nextTripFile(qsizetype after) const;
    void showFile(qsizetype index);
    void applyCurrentMetadata();
    bool isErrorRow(int row) const;

    const ErrorStorage *m_errors = nullptr;
    QVector<QVector<Trip>> m_trips;
    QVector<Trip> m_rows;
    QVector<TripMetaData> m_file_metadata;
    TripMetaData m_trip_meta_data;
    qsizetype m_currentFile = -1;
};

#endif
