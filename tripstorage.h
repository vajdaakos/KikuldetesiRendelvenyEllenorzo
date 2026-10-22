/*
 * Kiküldetési Rendelvény Ellenőrző — kiküldetési rendelvények ellenőrzése.
 * Copyright (C) 2026 Vajda Ákos Péter <vajdaakos@alu-fon.hu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
