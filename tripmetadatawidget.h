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

#ifndef TRIPMETADATAWIDGET_H
#define TRIPMETADATAWIDGET_H

#include "tripmetadata.h"

#include <QWidget>

class QLabel;

class TripMetadataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TripMetadataWidget(QWidget *parent = nullptr);

    void setMetadata(const TripMetaData &metadata);

private:
    QLabel *createLabel();

    QLabel *m_serialLabel = nullptr;
    QLabel *m_employeeLabel = nullptr;
    QLabel *m_employeeIdLabel = nullptr;
    QLabel *m_fileNameLabel = nullptr;
};

#endif
