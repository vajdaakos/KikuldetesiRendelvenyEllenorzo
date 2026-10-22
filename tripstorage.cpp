#include "tripstorage.h"
#include "columnheaders.h"
#include "errorstorage.h"

#include <QBrush>
#include <QFileInfo>

#include <functional>

TripStorage::TripStorage(QObject *parent)
    : QRangeModel(std::ref(m_rows), parent)
{
}

QVariant TripStorage::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && isErrorRow(index.row())) {
        if (role == Qt::BackgroundRole)
            return QBrush(QColor(255, 120, 120));
        if (role == Qt::ForegroundRole)
            return QBrush(Qt::black);
    }
    return QRangeModel::data(index, role);
}

void TripStorage::multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const
{
    QRangeModel::multiData(index, roleDataSpan);
    if (!index.isValid() || !isErrorRow(index.row()))
        return;
    for (QModelRoleData &roleData : roleDataSpan) {
        if (roleData.role() == Qt::BackgroundRole)
            roleData.setData(QBrush(QColor(255, 120, 120)));
        else if (roleData.role() == Qt::ForegroundRole)
            roleData.setData(QBrush(Qt::black));
    }
}

void TripStorage::setErrorStorage(const ErrorStorage *errors)
{
    m_errors = errors;
}

void TripStorage::refreshErrorMarks()
{
    if (m_rows.isEmpty())
        return;
    emit dataChanged(index(0, 0), index(int(m_rows.size()) - 1, columnCount() - 1),
                     {Qt::BackgroundRole, Qt::ForegroundRole});
}

bool TripStorage::isErrorRow(int row) const
{
    if (!m_errors || m_currentFile < 0 || m_currentFile >= m_trips.size())
        return false;
    const QVector<Trip> &fileTrips = m_trips.at(m_currentFile);
    if (row < 0 || row >= fileTrips.size())
        return false;
    return m_errors->isError(m_currentFile, &fileTrips.at(row));
}

QVariant TripStorage::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        const QStringList headers = tripColumnHeaders();
        if (section >= 0 && section < headers.size())
            return headers.at(section);
    }
    return QRangeModel::headerData(section, orientation, role);
}

void TripStorage::setTrips(const QVector<QVector<Trip>> &trips, const QVector<TripMetaData> &metadata)
{
    beginResetModel();
    m_trips = trips;
    m_file_metadata = metadata;
    m_currentFile = nextTripFile(-1);
    m_rows = m_currentFile < 0 ? QVector<Trip>{} : m_trips.at(m_currentFile);
    applyCurrentMetadata();
    endResetModel();
}

bool TripStorage::showPreviousFile()
{
    const qsizetype file = previousTripFile();
    if (file < 0)
        return false;
    showFile(file);
    return true;
}

bool TripStorage::showNextFile()
{
    const qsizetype file = nextTripFile(m_currentFile);
    if (file < 0)
        return false;
    showFile(file);
    return true;
}

bool TripStorage::showFileAt(qsizetype index)
{
    if (!hasTripsAt(index))
        return false;
    if (index != m_currentFile)
        showFile(index);
    return true;
}

const QVector<QVector<Trip>> &TripStorage::trips() const
{
    return m_trips;
}

const QVector<TripMetaData> &TripStorage::fileMetadata() const
{
    return m_file_metadata;
}

qsizetype TripStorage::currentFile() const
{
    return m_currentFile;
}

const TripMetaData &TripStorage::currentMetadata() const
{
    return m_trip_meta_data;
}

TripMetaData TripStorage::metadataAt(qsizetype index) const
{
    if (index < 0 || index >= m_file_metadata.size())
        return {};
    return m_file_metadata.at(index);
}

QString TripStorage::currentFileName() const
{
    return fileNameAt(m_currentFile);
}

QString TripStorage::fileNameAt(qsizetype index) const
{
    if (index < 0 || index >= m_trips.size() || m_trips.at(index).isEmpty())
        return {};
    return QFileInfo(m_trips.at(index).constFirst().m_sourcefile).fileName();
}

bool TripStorage::hasTripsAt(qsizetype index) const
{
    return index >= 0 && index < m_trips.size() && !m_trips.at(index).isEmpty();
}

bool TripStorage::hasPrevious() const
{
    return previousTripFile() >= 0;
}

bool TripStorage::hasNext() const
{
    return m_currentFile >= 0 && nextTripFile(m_currentFile) >= 0;
}

qsizetype TripStorage::previousTripFile() const
{
    for (qsizetype file = m_currentFile - 1; file >= 0; --file) {
        if (hasTripsAt(file))
            return file;
    }
    return -1;
}

qsizetype TripStorage::nextTripFile(qsizetype after) const
{
    for (qsizetype file = after + 1; file < m_trips.size(); ++file) {
        if (hasTripsAt(file))
            return file;
    }
    return -1;
}

void TripStorage::showFile(qsizetype index)
{
    beginResetModel();
    m_currentFile = index;
    m_rows = m_trips.at(index);
    applyCurrentMetadata();
    endResetModel();
}

void TripStorage::applyCurrentMetadata()
{
    if (m_currentFile >= 0 && m_currentFile < m_file_metadata.size())
        m_trip_meta_data = m_file_metadata.at(m_currentFile);
    else
        m_trip_meta_data = {};
}
