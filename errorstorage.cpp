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

#include "errorstorage.h"
#include "columnheaders.h"

#include <QStringList>

#include <functional>

ErrorStorage::ErrorStorage(QObject *parent)
    : QRangeModel(std::ref(m_rows), parent)
{
}

QVariant ErrorStorage::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        const QStringList headers = tripColumnHeaders();
        if (section >= 0 && section < headers.size())
            return headers.at(section);
    }
    return QRangeModel::headerData(section, orientation, role);
}

QVector<QVector<const Trip*>> &ErrorStorage::errors()
{
    return m_triperrors;
}

const QVector<const TripMetaData *> &ErrorStorage::fileMetadata() const
{
    return m_file_metadata;
}

void ErrorStorage::setFileMetadata(const QVector<TripMetaData> &metadata)
{
    m_file_metadata.resize(metadata.size());
    for (qsizetype index = 0; index < metadata.size(); ++index)
        m_file_metadata[index] = &metadata.at(index);
}

void ErrorStorage::clear()
{
    beginResetModel();
    m_triperrors.clear();
    m_file_metadata.clear();
    m_rows.clear();
    m_currentFile = -1;
    endResetModel();
}

void ErrorStorage::beginUpdate()
{
    beginResetModel();
}

void ErrorStorage::endUpdate()
{
    m_currentFile = -1;
    for (qsizetype file = 0; file < m_triperrors.size(); ++file) {
        if (!fileHasError(file))
            continue;
        m_currentFile = file;
        break;
    }
    rebuildRows();
    endResetModel();
}

bool ErrorStorage::hasErrors() const
{
    for (qsizetype file = 0; file < m_triperrors.size(); ++file) {
        if (fileHasError(file))
            return true;
    }
    return false;
}

bool ErrorStorage::showPreviousFile()
{
    const qsizetype file = previousErrorFile();
    if (file < 0)
        return false;
    showFile(file);
    return true;
}

bool ErrorStorage::showNextFile()
{
    const qsizetype file = nextErrorFile();
    if (file < 0)
        return false;
    showFile(file);
    return true;
}

bool ErrorStorage::showFileAt(qsizetype index)
{
    if (!fileHasError(index))
        return false;
    if (index != m_currentFile)
        showFile(index);
    return true;
}

bool ErrorStorage::hasErrorAt(qsizetype index) const
{
    return fileHasError(index);
}

bool ErrorStorage::isError(qsizetype file, const Trip *trip) const
{
    if (!trip || file < 0 || file >= m_triperrors.size())
        return false;
    return m_triperrors.at(file).contains(trip);
}

qsizetype ErrorStorage::currentFile() const
{
    return m_currentFile;
}

const Trip *ErrorStorage::sourceAt(int row) const
{
    if (row < 0 || m_currentFile < 0 || m_currentFile >= m_triperrors.size())
        return nullptr;

    int visibleRow = 0;
    for (const Trip *trip : m_triperrors.at(m_currentFile)) {
        if (!trip)
            continue;
        if (visibleRow == row)
            return trip;
        ++visibleRow;
    }
    return nullptr;
}

bool ErrorStorage::hasPrevious() const
{
    return previousErrorFile() >= 0;
}

bool ErrorStorage::hasNext() const
{
    return nextErrorFile() >= 0;
}

bool ErrorStorage::fileHasError(qsizetype index) const
{
    if (index < 0 || index >= m_triperrors.size())
        return false;
    for (const Trip *trip : m_triperrors.at(index)) {
        if (trip)
            return true;
    }
    return false;
}

qsizetype ErrorStorage::previousErrorFile() const
{
    for (qsizetype file = m_currentFile - 1; file >= 0; --file) {
        if (fileHasError(file))
            return file;
    }
    return -1;
}

qsizetype ErrorStorage::nextErrorFile() const
{
    for (qsizetype file = m_currentFile + 1; file < m_triperrors.size(); ++file) {
        if (fileHasError(file))
            return file;
    }
    return -1;
}

void ErrorStorage::showFile(qsizetype index)
{
    beginResetModel();
    m_currentFile = index;
    rebuildRows();
    endResetModel();
}

void ErrorStorage::rebuildRows()
{
    m_rows.clear();
    if (m_currentFile < 0 || m_currentFile >= m_triperrors.size())
        return;
    for (const Trip *trip : m_triperrors.at(m_currentFile)) {
        if (trip)
            m_rows.append(*trip);
    }
}
