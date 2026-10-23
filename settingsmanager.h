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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include "columnheaders.h"

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject &parent);

    const HeaderSettings &columnHeaders() const { return m_columnHeaders; }
    const HeaderSettings &metadataHeaders() const { return m_metadataHeaders; }
    const HeaderSettings &displayColumnHeaders() const { return m_displayColumnHeaders; }
    const HeaderSettings &displayMetadataHeaders() const { return m_displayMetadataHeaders; }
    const HeaderSettings &errorColumnHeaders() const { return m_errorColumnHeaders; }
    const HeaderSettings &errorMetadataHeaders() const { return m_errorMetadataHeaders; }

signals:
    void settingsLoaded(const HeaderSettings &columnHeaders,
                        const HeaderSettings &metadataHeaders);

public slots:
    void readSettings();
    void applySettings(const HeaderSettings &columnHeaders,
                       const HeaderSettings &metadataHeaders);
    void applyDisplaySettings(const HeaderSettings &columnHeaders,
                              const HeaderSettings &metadataHeaders,
                              const HeaderSettings &errorColumnHeaders,
                              const HeaderSettings &errorMetadataHeaders);

private:
    void writeSettingsFile();

    HeaderSettings m_columnHeaders = kDefaultColumnHeaders;
    HeaderSettings m_metadataHeaders = kDefaultMetadataHeaders;
    HeaderSettings m_displayColumnHeaders = kDefaultColumnHeaders;
    HeaderSettings m_displayMetadataHeaders = kDefaultMetadataHeaders;
    HeaderSettings m_errorColumnHeaders = kDefaultErrorColumnHeaders;
    HeaderSettings m_errorMetadataHeaders = kDefaultMetadataHeaders;
};

#endif
