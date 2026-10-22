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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "trip.h"
#include "triploader.h"
#include "tripmanager.h"

#include <QMainWindow>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStringList>
#include <QVector>

#include <utility>

class QAbstractItemView;
class QDialog;
class QHeaderView;
class QModelIndex;
class QTableView;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void exportSettingsAccepted(const HeaderSettings &columnHeaders,
                                const HeaderSettings &metadataHeaders);

private slots:
    void openDirectory();
    void showLoadResult(const QVector<QVector<Trip>> &trips);
    void showLoadErrors(const QStringList &errors);
    void showCurrentFile(const QString &fileName, bool hasPrevious, bool hasNext);
    void showCurrentErrorFile(const QString &fileName, bool hasPrevious, bool hasNext);
    void showSelectedError(const QModelIndex &current, const QModelIndex &previous);
    void checkLoaded();
    void showFileFromCombo(int index);
    void showErrorFileFromCombo(int index);
    void showHelp();
    void showAbout();
    void showExportSettings();

private:
    void showTripColumns(QTableView *view);
    void fitReportColumns();
    void configureTripView(QAbstractItemView *view, QHeaderView *header, bool boldHeader);
    void equalizePanes();
    void updateActionButtons();
    Ui::MainWindow *ui;
    QDialog *m_helpWindow = nullptr;
    TripLoader m_triploader;
    TripManager m_tripmanager;
};
#endif // MAINWINDOW_H
