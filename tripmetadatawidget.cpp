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

#include "tripmetadatawidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>

TripMetadataWidget::TripMetadataWidget(QWidget *parent)
    : QWidget(parent)
    , m_serialLabel(createLabel())
    , m_employeeLabel(createLabel())
    , m_employeeIdLabel(createLabel())
    , m_fileNameLabel(createLabel())
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_serialLabel);
    layout->addWidget(m_employeeLabel);
    layout->addWidget(m_employeeIdLabel);
    layout->addWidget(m_fileNameLabel);
    setMetadata({});
}

void TripMetadataWidget::setMetadata(const TripMetaData &metadata)
{
    const bool loaded = !metadata.m_filename.isEmpty();
    m_serialLabel->setText(tr("Sorszám: %1").arg(metadata.m_serial_number));
    m_employeeLabel->setText(tr("Munkavállaló: %1").arg(metadata.m_employee));
    m_employeeIdLabel->setText(tr("Törzsszám: %1").arg(loaded ? QString::number(metadata.m_employee_id) : QString()));
    m_fileNameLabel->setText(tr("Fájlnév: %1").arg(metadata.m_filename));
}

QLabel *TripMetadataWidget::createLabel()
{
    auto *label = new QLabel(this);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    return label;
}
