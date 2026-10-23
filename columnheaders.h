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

#ifndef COLUMNHEADERS_H
#define COLUMNHEADERS_H

#include <QCoreApplication>
#include <QStringList>
#include <QVector>

#include <utility>

using HeaderSettings = QVector<std::pair<QString,bool>>;

inline constexpr char kColumnHeaderContext[] = "ColumnHeaders";

inline const HeaderSettings kDefaultColumnHeaders{
    std::make_pair("Sorszám", true),
    std::make_pair("Forrásfájl", false),
    std::make_pair("Kezdés", true),
    std::make_pair("Befejezés", false),
    std::make_pair("Cél", true),
    std::make_pair("Honnan", true),
    std::make_pair("Hova", false),
    std::make_pair("Távolság", true),
    std::make_pair("Üzemanyagár", false),
    std::make_pair("Üzemanyagköltség", false),
    std::make_pair("Fenntartási szorzó 1", false),
    std::make_pair("Fenntartási szorzó 2", false),
    std::make_pair("Összesen", false)
};

inline const HeaderSettings kDefaultMetadataHeaders{
    std::make_pair("Fájlnév", false),
    std::make_pair("Sorszám", false),
    std::make_pair("Munkavállaló", true),
    std::make_pair("Törzsszám", false)
};

inline const HeaderSettings kDefaultErrorColumnHeaders{
    std::make_pair("Sorszám", true),
    std::make_pair("Forrásfájl", false),
    std::make_pair("Kezdés", false),
    std::make_pair("Befejezés", false),
    std::make_pair("Cél", false),
    std::make_pair("Honnan", true),
    std::make_pair("Hova", true),
    std::make_pair("Távolság", true),
    std::make_pair("Üzemanyagár", false),
    std::make_pair("Üzemanyagköltség", false),
    std::make_pair("Fenntartási szorzó 1", false),
    std::make_pair("Fenntartási szorzó 2", false),
    std::make_pair("Összesen", true)
};

inline QStringList tripColumnHeaders()
{
    QStringList headers;
    for (const auto &header : kDefaultColumnHeaders) {
        headers.append(QCoreApplication::translate(kColumnHeaderContext, header.first.toUtf8().data()));
    }
    return headers;
}

#endif
