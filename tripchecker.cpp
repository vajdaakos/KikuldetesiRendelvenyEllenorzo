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

#include "tripchecker.h"

void TripChecker::check(const QVector<QVector<Trip>> &trips, QVector<QVector<const Trip*>> &errors)
{
    errors = QVector<QVector<const Trip*>>(trips.size());
    for (qsizetype fileIndex = 0; fileIndex < trips.size(); ++fileIndex) {
        if (trips.at(fileIndex).isEmpty())
            continue;
        for (qsizetype rowindex = 0; rowindex < trips.at(fileIndex).size(); ++rowindex) {
            const Trip &record = trips.at(fileIndex).at(rowindex);
            if (record.m_from == record.m_to && record.m_total_cost > 0)
                errors[fileIndex].append(&trips.at(fileIndex).at(rowindex));
        }
    }
}
