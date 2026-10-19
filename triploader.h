#ifndef TRIPLOADER_H
#define TRIPLOADER_H

#include "trip.h"
#include "tripmetadata.h"

#include <QObject>
#include <QStringList>
#include <QVector>

class TripLoader : public QObject
{
    Q_OBJECT

public:
    explicit TripLoader(QObject *parent = nullptr);

    bool loadDirectory(const QString &directoryPath);

signals:
    void tripsLoaded(const QVector<QVector<Trip>> &trips, const QVector<TripMetaData> &metadata,
                     const QString &directoryPath);
    void loadFailed(const QStringList &errors);

private:
    QVector<Trip> loadTrips(const QString &path, TripMetaData &metadata) const;
};

#endif
