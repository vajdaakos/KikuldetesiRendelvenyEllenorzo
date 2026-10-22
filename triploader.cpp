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

#include "triploader.h"

#include <poppler-document.h>
#include <poppler-page.h>

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace {

struct Word
{
    double x = 0;
    double y = 0;
    std::string text;
};

struct Line
{
    double y = 0;
    std::vector<Word> words;
};

enum class Column
{
    Id,
    Begin,
    End,
    Purpose,
    Route,
    Length,
    FuelPrice,
    FuelCost,
    Factor1,
    Factor2,
    Total
};

enum class RoutePart
{
    None,
    From,
    To
};

constexpr double kLineTolerance = 3.0;

Column columnOf(double x)
{
    if (x < 52)
        return Column::Id;
    if (x < 100)
        return Column::Begin;
    if (x < 148)
        return Column::End;
    if (x < 200)
        return Column::Purpose;
    if (x < 310)
        return Column::Route;
    if (x < 345)
        return Column::Length;
    if (x < 390)
        return Column::FuelPrice;
    if (x < 430)
        return Column::FuelCost;
    if (x < 460)
        return Column::Factor1;
    if (x < 490)
        return Column::Factor2;
    return Column::Total;
}

std::string utf8(const poppler::ustring &text)
{
    const poppler::byte_array bytes = text.to_utf8();
    return std::string(bytes.data(), bytes.size());
}

std::string trim(std::string text)
{
    const auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
    const auto begin = std::find_if_not(text.begin(), text.end(), isSpace);
    const auto end = std::find_if_not(text.rbegin(), text.rend(), isSpace).base();
    if (begin >= end)
        return {};
    return std::string(begin, end);
}

void appendField(QString &field, const std::string &word)
{
    if (word.empty())
        return;
    const bool punctuation = word == "," || word == "." || word == ";" || word == ":";
    if (!field.isEmpty() && !punctuation)
        field.append(QLatin1Char(' '));
    field.append(QString::fromStdString(word));
}

std::optional<quint32> parseId(const std::string &text)
{
    if (text.size() < 2 || text.back() != '.')
        return std::nullopt;
    for (std::size_t i = 0; i + 1 < text.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(text[i])) == 0)
            return std::nullopt;
    }
    return static_cast<quint32>(std::stoul(text));
}

quint32 parseAmount(const Line &line, Column column)
{
    std::string digits;
    for (const Word &word : line.words) {
        if (columnOf(word.x) != column)
            continue;
        for (unsigned char ch : word.text) {
            if (std::isdigit(ch) != 0)
                digits.push_back(static_cast<char>(ch));
        }
    }
    if (digits.empty())
        return 0;
    return static_cast<quint32>(std::stoul(digits));
}

std::string stripMarker(const std::string &word, const std::string &marker)
{
    if (word == marker || word == marker + ":")
        return {};
    const std::string prefixed = marker + ":";
    if (word.starts_with(prefixed))
        return trim(word.substr(prefixed.size()));
    if (word.starts_with(marker))
        return trim(word.substr(marker.size()));
    return word;
}

bool isMarker(const std::string &word, const std::string &marker)
{
    return word == marker || word.starts_with(marker + ":") || word.starts_with(marker + " ");
}

void consumeRoute(Trip &trip, RoutePart &part, const std::string &word)
{
    if (isMarker(word, "Innen")) {
        part = RoutePart::From;
        appendField(trip.m_from, stripMarker(word, "Innen"));
        return;
    }
    if (isMarker(word, "Ide")) {
        part = RoutePart::To;
        appendField(trip.m_to, stripMarker(word, "Ide"));
        return;
    }
    if (part == RoutePart::From)
        appendField(trip.m_from, word);
    else if (part == RoutePart::To)
        appendField(trip.m_to, word);
}

void fillText(Trip &trip, RoutePart &part, const Line &line)
{
    for (const Word &word : line.words) {
        switch (columnOf(word.x)) {
        case Column::Begin:
            appendField(trip.m_trip_date_begin, word.text);
            break;
        case Column::End:
            appendField(trip.m_trip_date_end, word.text);
            break;
        case Column::Purpose:
            appendField(trip.m_purpose, word.text);
            break;
        case Column::Route:
            consumeRoute(trip, part, word.text);
            break;
        default:
            break;
        }
    }
}

void fillAmounts(Trip &trip, const Line &line)
{
    trip.m_length = parseAmount(line, Column::Length);
    trip.m_fuel_price = parseAmount(line, Column::FuelPrice);
    trip.m_fuel_cost = parseAmount(line, Column::FuelCost);
    trip.m_maintenance_cost_factor1 = parseAmount(line, Column::Factor1);
    trip.m_maintenance_cost_factor2 = parseAmount(line, Column::Factor2);
    trip.m_total_cost = parseAmount(line, Column::Total);
}

std::vector<Line> groupLines(std::vector<Word> words)
{
    std::sort(words.begin(), words.end(), [](const Word &left, const Word &right) {
        return left.y < right.y;
    });

    std::vector<Line> lines;
    for (Word &word : words) {
        if (lines.empty() || std::abs(lines.back().y - word.y) > kLineTolerance)
            lines.push_back(Line{word.y, {}});
        lines.back().words.push_back(std::move(word));
    }
    for (Line &line : lines) {
        std::sort(line.words.begin(), line.words.end(), [](const Word &left, const Word &right) {
            return left.x < right.x;
        });
    }
    return lines;
}

std::optional<quint32> lineId(const Line &line)
{
    for (const Word &word : line.words) {
        if (columnOf(word.x) != Column::Id)
            continue;
        if (const std::optional<quint32> id = parseId(word.text))
            return id;
    }
    return std::nullopt;
}

std::vector<Word> pageWords(const poppler::page &page)
{
    std::vector<Word> words;
    for (const poppler::text_box &box : page.text_list()) {
        const std::string text = trim(utf8(box.text()));
        if (text.empty())
            continue;
        const poppler::rectf bounds = box.bbox();
        words.push_back(Word{
            bounds.left(),
            (bounds.top() + bounds.bottom()) / 2.0,
            text});
    }
    return words;
}

bool sameLabel(const std::string &word, const std::string &label)
{
    return word == label || word == label + ":";
}

QString textRightOf(const Line &line, double minX)
{
    QString text;
    for (const Word &word : line.words) {
        if (word.x < minX)
            continue;
        appendField(text, word.text);
    }
    return text;
}

quint32 parseEmployeeId(const QString &text)
{
    std::string digits;
    for (const QChar character : text) {
        if (character.isDigit())
            digits.push_back(character.toLatin1());
    }
    if (digits.empty())
        return 0;
    return static_cast<quint32>(std::stoul(digits));
}

TripMetaData readMetadata(const std::vector<Line> &lines, const QString &fileName)
{
    TripMetaData metadata;
    metadata.m_filename = fileName;

    for (std::size_t index = 0; index < lines.size(); ++index) {
        bool serialLabel = false;
        for (const Word &word : lines[index].words) {
            if (word.x > 450 && sameLabel(word.text, "sorszám"))
                serialLabel = true;
        }
        if (!serialLabel)
            continue;
        for (std::size_t next = index + 1; next < lines.size(); ++next) {
            const QString serialNumber = textRightOf(lines[next], 450);
            if (serialNumber.isEmpty())
                continue;
            metadata.m_serial_number = serialNumber;
            break;
        }
        break;
    }

    for (const Line &line : lines) {
        double labelX = -1;
        for (const Word &word : line.words) {
            if (word.x > 200 && word.x < 330 && sameLabel(word.text, "neve")) {
                labelX = word.x;
                break;
            }
        }
        if (labelX < 0)
            continue;
        metadata.m_employee = textRightOf(line, labelX + 40);
        break;
    }

    for (const Line &line : lines) {
        double labelX = -1;
        for (const Word &word : line.words) {
            if (word.text.starts_with("törzsszáma")) {
                labelX = word.x;
                break;
            }
        }
        if (labelX < 0)
            continue;
        metadata.m_employee_id = parseEmployeeId(textRightOf(line, labelX + 40));
        break;
    }

    return metadata;
}

}

TripLoader::TripLoader(QObject *parent)
    : QObject(parent)
{
}

bool TripLoader::loadDirectory(const QString &directoryPath)
{
    const QDir directory(directoryPath);
    const QFileInfoList files = directory.entryInfoList(
        QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase);

    QVector<QVector<Trip>> trips;
    QVector<TripMetaData> metadata;
    QStringList errors;
    bool foundPdf = false;

    for (const QFileInfo &file : files) {
        if (file.suffix().compare(QStringLiteral("pdf"), Qt::CaseInsensitive) != 0)
            continue;

        foundPdf = true;
        try {
            TripMetaData fileMetadata;
            trips.append(loadTrips(file.absoluteFilePath(), fileMetadata));
            metadata.append(fileMetadata);
        } catch (const std::exception &error) {
            errors.append(tr("%1: %2").arg(file.fileName(), QString::fromUtf8(error.what())));
        }
    }

    if (!foundPdf)
        return false;

    emit tripsLoaded(trips, metadata, QFileInfo(directoryPath).absoluteFilePath());
    if (!errors.isEmpty())
        emit loadFailed(errors);
    return true;
}

QVector<Trip> TripLoader::loadTrips(const QString &path, TripMetaData &metadata) const
{
    const std::string filePath = path.toStdString();
    const std::unique_ptr<poppler::document> document(poppler::document::load_from_file(filePath));
    if (!document)
        throw std::runtime_error("A PDF nem nyitható meg: " + filePath);

    QVector<Trip> trips;
    trips.reserve(document->pages());
    metadata = {};
    metadata.m_filename = QFileInfo(path).fileName();

    for (int pageIndex = 0; pageIndex < document->pages(); ++pageIndex) {
        const std::unique_ptr<poppler::page> page(document->create_page(pageIndex));
        if (!page)
            throw std::runtime_error("A PDF oldala nem olvasható: " + std::to_string(pageIndex + 1));

        const std::vector<Line> lines = groupLines(pageWords(*page));
        if (pageIndex == 0) {
            const QString fileName = metadata.m_filename;
            metadata = readMetadata(lines, fileName);
        }

        std::optional<qsizetype> current;
        RoutePart routePart = RoutePart::None;
        for (const Line &line : lines) {
            if (const std::optional<quint32> id = lineId(line)) {
                trips.append(Trip{});
                current = trips.size() - 1;
                routePart = RoutePart::None;
                trips[*current].id = *id;
                trips[*current].m_sourcefile = path;
                fillAmounts(trips[*current], line);
                fillText(trips[*current], routePart, line);
            } else if (current) {
                fillText(trips[*current], routePart, line);
            }
        }
    }

    return trips;
}
