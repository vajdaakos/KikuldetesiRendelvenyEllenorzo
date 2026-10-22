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

#include "settingsmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QWidget>

namespace {

QString settingsFilePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("settings.json"));
}

QJsonArray headerArray(const QVector<std::pair<QString, bool>> &headers)
{
    QJsonArray array;
    for (const std::pair<QString, bool> &header : headers) {
        QJsonObject entry;
        entry.insert(header.first, header.second);
        array.append(entry);
    }
    return array;
}

QWidget *widgetParent(QObject *object)
{
    for (QObject *ancestor = object->parent(); ancestor != nullptr; ancestor = ancestor->parent()) {
        if (auto *widget = qobject_cast<QWidget *>(ancestor))
            return widget;
    }
    return nullptr;
}

void showCenteredWarning(QObject *object, const QString &text)
{
    QMessageBox box(QMessageBox::Warning, SettingsManager::tr("Beállítások"), QString(),
                    QMessageBox::Ok, widgetParent(object));
    box.setWindowModality(Qt::ApplicationModal);
    box.setTextFormat(Qt::RichText);
    box.setText(text);
    for (QLabel *label : box.findChildren<QLabel *>())
        label->setAlignment(Qt::AlignCenter);
    box.exec();
}

void warnLoadFailed(QObject *object)
{
    showCenteredWarning(object, SettingsManager::tr(
        "<p align=\"center\">A beállító fájl betöltése sikertelen, "
        "a program az alapértelmezett beállításokkal indul!</p>"));
}

void warnSaveFailed(QObject *object, bool previousFileRemains)
{
    const QString text = previousFileRemains
        ? SettingsManager::tr(
            "<p align=\"center\">A beállítások mentése sikertelen! "
            "A korábbi beállítófájl változatlan marad, a következő induláskor abból tölt a program.</p>")
        : SettingsManager::tr(
            "<p align=\"center\">A beállítások mentése sikertelen! "
            "A következő induláskor a program az alapértelmezett beállításokkal fog indulni!</p>");
    showCenteredWarning(object, text);
}

void readHeaders(const QJsonObject &root, const QString &key,
                 QVector<std::pair<QString, bool>> &headers)
{
    const QJsonValue value = root.value(key);
    if (!value.isArray())
        return;

    const QJsonArray entries = value.toArray();
    for (qsizetype index = 0; index < headers.size(); ++index) {
        for (const QJsonValue &item : entries) {
            const QJsonObject entry = item.toObject();
            const auto it = entry.constFind(headers.at(index).first);
            if (it == entry.constEnd())
            {
                continue; // header not found
            }
            headers[index].second = it.value().toBool();
            break;
        }
    }
}

}

SettingsManager::SettingsManager(QObject &parent)
    : QObject(&parent)
{
}

void SettingsManager::readSettings()
{
    QFile file(settingsFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        warnLoadFailed(this);
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        warnLoadFailed(this);
        return;
    }
    const QJsonObject root = document.object();
    readHeaders(root, QStringLiteral("columnheaders"), m_columnHeaders);
    readHeaders(root, QStringLiteral("metadataheaders"), m_metadataHeaders);
    emit settingsLoaded(m_columnHeaders, m_metadataHeaders);
}

void SettingsManager::applySettings(const HeaderSettings &columnHeaders,
                                    const HeaderSettings &metadataHeaders)
{
    m_columnHeaders = columnHeaders;
    m_metadataHeaders = metadataHeaders;

    QJsonObject root;
    root.insert(QStringLiteral("metadataheaders"), headerArray(metadataHeaders));
    root.insert(QStringLiteral("columnheaders"), headerArray(columnHeaders));
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    const bool previousFileRemains = QFileInfo::exists(settingsFilePath());

    QSaveFile file(settingsFilePath());
    const bool opened = file.open(QIODevice::WriteOnly);
    const bool written = opened && file.write(bytes) == bytes.size() && file.error() == QFile::NoError;
    if (!written || !file.commit())
        warnSaveFailed(this, previousFileRemains);
}
