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
#include <QStandardPaths>
#include <QWidget>

namespace {

QString bundledSettingsFilePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("settings.json"));
}

QString settingsFilePath()
{
#if defined(Q_OS_MACOS)
    // A .app bundle (és az App Translocation) gyakran csak olvasható; a mentés
    // a felhasználói Application Support mappába kerül.
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return QDir(dir).filePath(QStringLiteral("settings.json"));
#else
    return bundledSettingsFilePath();
#endif
}

QString settingsFilePathForReading()
{
    const QString path = settingsFilePath();
    if (QFileInfo::exists(path))
        return path;
#if defined(Q_OS_MACOS)
    const QString bundled = bundledSettingsFilePath();
    if (QFileInfo::exists(bundled))
        return bundled;
#endif
    return path;
}

QJsonArray headerArray(const HeaderSettings &headers)
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

QJsonObject settingsSection(const HeaderSettings &metadataHeaders,
                            const HeaderSettings &columnHeaders)
{
    QJsonObject section;
    section.insert(QStringLiteral("metadataheaders"), headerArray(metadataHeaders));
    section.insert(QStringLiteral("columnheaders"), headerArray(columnHeaders));
    return section;
}

void readHeaders(const QJsonObject &root, const QString &key,
                 HeaderSettings &headers)
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
    QFile file(settingsFilePathForReading());
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
    const QJsonValue exportSettings = root.value(QStringLiteral("Exportálási beállítások"));
    if (exportSettings.isObject()) {
        const QJsonObject section = exportSettings.toObject();
        readHeaders(section, QStringLiteral("columnheaders"), m_columnHeaders);
        readHeaders(section, QStringLiteral("metadataheaders"), m_metadataHeaders);
    } else {
        readHeaders(root, QStringLiteral("columnheaders"), m_columnHeaders);
        readHeaders(root, QStringLiteral("metadataheaders"), m_metadataHeaders);
    }
    const QJsonValue displaySettings = root.value(QStringLiteral("Kiküldetési rendelvény megjelenítése"));
    if (displaySettings.isObject()) {
        const QJsonObject section = displaySettings.toObject();
        readHeaders(section, QStringLiteral("columnheaders"), m_displayColumnHeaders);
        readHeaders(section, QStringLiteral("metadataheaders"), m_displayMetadataHeaders);
    }
    const QJsonValue errorSettings = root.value(QStringLiteral("Hibák megjelenítése"));
    if (errorSettings.isObject()) {
        const QJsonObject section = errorSettings.toObject();
        readHeaders(section, QStringLiteral("columnheaders"), m_errorColumnHeaders);
        readHeaders(section, QStringLiteral("metadataheaders"), m_errorMetadataHeaders);
    }
    emit settingsLoaded(m_columnHeaders, m_metadataHeaders);
}

void SettingsManager::writeSettingsFile()
{
    QJsonObject root;
    root.insert(QStringLiteral("Exportálási beállítások"),
                settingsSection(m_metadataHeaders, m_columnHeaders));
    root.insert(QStringLiteral("Kiküldetési rendelvény megjelenítése"),
                settingsSection(m_displayMetadataHeaders, m_displayColumnHeaders));
    root.insert(QStringLiteral("Hibák megjelenítése"),
                settingsSection(m_errorMetadataHeaders, m_errorColumnHeaders));
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    const bool previousFileRemains = QFileInfo::exists(settingsFilePath());

    QSaveFile file(settingsFilePath());
    const bool opened = file.open(QIODevice::WriteOnly);
    const bool written = opened && file.write(bytes) == bytes.size() && file.error() == QFile::NoError;
    if (!written || !file.commit())
        warnSaveFailed(this, previousFileRemains);
}

void SettingsManager::applySettings(const HeaderSettings &columnHeaders,
                                    const HeaderSettings &metadataHeaders)
{
    m_columnHeaders = columnHeaders;
    m_metadataHeaders = metadataHeaders;
    writeSettingsFile();
}

void SettingsManager::applyDisplaySettings(const HeaderSettings &columnHeaders,
                                           const HeaderSettings &metadataHeaders,
                                           const HeaderSettings &errorColumnHeaders,
                                           const HeaderSettings &errorMetadataHeaders)
{
    m_displayColumnHeaders = columnHeaders;
    m_displayMetadataHeaders = metadataHeaders;
    m_errorColumnHeaders = errorColumnHeaders;
    m_errorMetadataHeaders = errorMetadataHeaders;
    writeSettingsFile();
}
