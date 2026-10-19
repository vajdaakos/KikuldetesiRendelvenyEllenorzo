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
