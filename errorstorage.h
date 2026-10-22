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

#ifndef ERRORSTORAGE_H
#define ERRORSTORAGE_H

#include "trip.h"
#include "tripmetadata.h"

#include <QRangeModel>
#include <QVector>

class ErrorStorage : public QRangeModel
{
    Q_OBJECT

public:
    explicit ErrorStorage(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QVector<QVector<const Trip*>> &errors();
    const QVector<const TripMetaData *> &fileMetadata() const;
    void setFileMetadata(const QVector<TripMetaData> &metadata);
    void clear();
    void beginUpdate();
    void endUpdate();

    bool showPreviousFile();
    bool showNextFile();
    bool showFileAt(qsizetype index);
    bool hasErrorAt(qsizetype index) const;
    bool isError(qsizetype file, const Trip *trip) const;
    bool hasErrors() const;
    qsizetype currentFile() const;
    const Trip *sourceAt(int row) const;
    bool hasPrevious() const;
    bool hasNext() const;

private:
    void showFile(qsizetype index);
    void rebuildRows();
    bool fileHasError(qsizetype index) const;
    qsizetype previousErrorFile() const;
    qsizetype nextErrorFile() const;

    QVector<QVector<const Trip*>> m_triperrors;
    QVector<const TripMetaData*>  m_file_metadata;
    QVector<Trip> m_rows;
    qsizetype m_currentFile = -1;
};

#endif
