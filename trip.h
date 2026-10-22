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

#ifndef TRIP_H
#define TRIP_H

#include <QString>
#include <QtTypes>

#include <tuple>
#include <type_traits>
#include <utility>

class Trip
{
public:
    quint32 id = 0;
    QString m_sourcefile;
    QString m_trip_date_begin;
    QString m_trip_date_end;
    QString m_purpose;
    QString m_from;
    QString m_to;
    quint32 m_length = 0;
    quint32 m_fuel_price = 0;
    quint32 m_fuel_cost = 0;
    quint32 m_maintenance_cost_factor1 = 0;
    quint32 m_maintenance_cost_factor2 = 0;
    quint32 m_total_cost = 0;

    template <std::size_t I, typename T>
        requires ((I <= 12) && std::is_same_v<std::remove_cvref_t<T>, Trip>)
    friend inline decltype(auto) get(T &&trip)
    {
        if constexpr (I == 0)
            return (std::as_const(trip).id);
        else if constexpr (I == 1)
            return (std::as_const(trip).m_sourcefile);
        else if constexpr (I == 2)
            return (std::as_const(trip).m_trip_date_begin);
        else if constexpr (I == 3)
            return (std::as_const(trip).m_trip_date_end);
        else if constexpr (I == 4)
            return (std::as_const(trip).m_purpose);
        else if constexpr (I == 5)
            return (std::as_const(trip).m_from);
        else if constexpr (I == 6)
            return (std::as_const(trip).m_to);
        else if constexpr (I == 7)
            return (std::as_const(trip).m_length);
        else if constexpr (I == 8)
            return (std::as_const(trip).m_fuel_price);
        else if constexpr (I == 9)
            return (std::as_const(trip).m_fuel_cost);
        else if constexpr (I == 10)
            return (std::as_const(trip).m_maintenance_cost_factor1);
        else if constexpr (I == 11)
            return (std::as_const(trip).m_maintenance_cost_factor2);
        else
            return (std::as_const(trip).m_total_cost);
    }
};

namespace std {
template <>
struct tuple_size<Trip> : std::integral_constant<std::size_t, 13> {};

template <std::size_t I>
struct tuple_element<I, Trip>
{
    using type = decltype(get<I>(std::declval<Trip>()));
};
}

#endif
