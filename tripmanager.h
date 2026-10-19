#ifndef TRIPMANAGER_H
#define TRIPMANAGER_H

#include "errorreportcreator.h"
#include "errorstorage.h"
#include "trip.h"
#include "tripchecker.h"
#include "tripstorage.h"

#include <QObject>
#include <QString>
#include <QVector>

class TripManager : public QObject
{
    Q_OBJECT

public:
    explicit TripManager(QObject *parent = nullptr);

    TripStorage *storage();
    ErrorStorage *errorStorage();
    ErrorReportCreator *reporter();

public slots:
    void setTrips(const QVector<QVector<Trip>> &trips, const QVector<TripMetaData> &metadata,
                  const QString &directoryPath);
    void showPreviousFile();
    void showNextFile();
    void showPreviousErrorFile();
    void showNextErrorFile();
    void showFile(int index);
    void showErrorFile(int index);
    void checkLoaded();
    void exportErrors();
    int focusTrip(const Trip *source);

signals:
    void currentFileChanged(const QString &fileName, bool hasPrevious, bool hasNext);
    void currentErrorFileChanged(const QString &fileName, bool hasPrevious, bool hasNext);

private:
    void announceCurrentFile();
    void announceCurrentErrorFile();

    TripStorage m_storage;
    ErrorStorage m_errorstorage;
    TripChecker m_tripchecker;
    ErrorReportCreator m_reporter;
    QString m_sourceDirectory;
};

#endif
