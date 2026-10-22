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
