#include "tripmanager.h"

TripManager::TripManager(QObject *parent)
    : QObject(parent)
    , m_storage(this)
    , m_errorstorage(this)
    , m_reporter(this)
{
}

TripStorage *TripManager::storage()
{
    return &m_storage;
}

ErrorStorage *TripManager::errorStorage()
{
    return &m_errorstorage;
}

ErrorReportCreator *TripManager::reporter()
{
    return &m_reporter;
}

void TripManager::setTrips(const QVector<QVector<Trip>> &trips, const QVector<TripMetaData> &metadata,
                          const QString &directoryPath)
{
    m_sourceDirectory = directoryPath;
    m_errorstorage.clear();
    m_storage.setTrips(trips, metadata);
    m_errorstorage.setFileMetadata(m_storage.fileMetadata());
    announceCurrentFile();
    announceCurrentErrorFile();
}

void TripManager::showPreviousFile()
{
    if (m_storage.showPreviousFile())
        announceCurrentFile();
}

void TripManager::showNextFile()
{
    if (m_storage.showNextFile())
        announceCurrentFile();
}

void TripManager::showPreviousErrorFile()
{
    if (m_errorstorage.showPreviousFile())
        announceCurrentErrorFile();
}

void TripManager::showNextErrorFile()
{
    if (m_errorstorage.showNextFile())
        announceCurrentErrorFile();
}

void TripManager::showFile(int index)
{
    if (m_storage.showFileAt(index))
        announceCurrentFile();
}

void TripManager::showErrorFile(int index)
{
    if (m_errorstorage.showFileAt(index))
        announceCurrentErrorFile();
}

int TripManager::focusTrip(const Trip *source)
{
    if (!source)
        return -1;

    const QVector<QVector<Trip>> &files = m_storage.trips();
    for (qsizetype file = 0; file < files.size(); ++file) {
        const QVector<Trip> &fileTrips = files.at(file);
        for (qsizetype row = 0; row < fileTrips.size(); ++row) {
            if (&fileTrips.at(row) != source)
                continue;
            const bool fileChanged = file != m_storage.currentFile();
            if (!m_storage.showFileAt(file))
                return -1;
            if (fileChanged)
                announceCurrentFile();
            return int(row);
        }
    }
    return -1;
}

void TripManager::checkLoaded()
{
    m_errorstorage.beginUpdate();
    m_tripchecker.check(m_storage.trips(), m_errorstorage.errors());
    m_errorstorage.endUpdate();
    announceCurrentErrorFile();
}

void TripManager::exportErrors()
{
    m_reporter.create(m_storage.trips(), m_errorstorage.errors(),
                      m_errorstorage.fileMetadata(), m_sourceDirectory);
}

void TripManager::announceCurrentFile()
{
    emit currentFileChanged(m_storage.currentFileName(),
                            m_storage.hasPrevious(),
                            m_storage.hasNext());
}

void TripManager::announceCurrentErrorFile()
{
    emit currentErrorFileChanged(m_storage.fileNameAt(m_errorstorage.currentFile()),
                                 m_errorstorage.hasPrevious(),
                                 m_errorstorage.hasNext());
}
