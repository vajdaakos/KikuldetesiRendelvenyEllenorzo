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
