#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QEvent>
#include <QLineEdit>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QPushButton>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextLayout>

#include <QtMath>

#include <algorithm>
#include <array>

namespace {

constexpr int kGridThickness = 2;

class CenteredComboDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignCenter;
    }
};

class ComboPopupOnClick : public QObject
{
public:
    explicit ComboPopupOnClick(QComboBox *combo)
        : QObject(combo)
        , m_combo(combo)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress && m_combo->isEnabled()) {
            m_combo->showPopup();
            return true;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QComboBox *m_combo;
};

void centerFileCombo(QComboBox *combo)
{
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    combo->setItemDelegate(new CenteredComboDelegate(combo));
    QLineEdit *edit = combo->lineEdit();
    edit->setReadOnly(true);
    edit->setAlignment(Qt::AlignCenter);
    edit->setFrame(false);
    edit->installEventFilter(new ComboPopupOnClick(combo));
}

class WrappingItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QString text = opt.text;
        opt.text.clear();

        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        if (!text.isEmpty()) {
            const QRect textRect = textArea(style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget));
            QTextLayout layout(text, opt.font);
            layoutText(layout, textRect.width());
            const qreal yOffset = qMax(0.0, (textRect.height() - layout.boundingRect().height()) / 2.0);

            painter->save();
            painter->setClipRect(textRect);
            painter->setPen(opt.palette.color(opt.state & QStyle::State_Selected
                                                   ? QPalette::HighlightedText
                                                   : QPalette::Text));
            layout.draw(painter, textRect.topLeft() + QPointF(0, yOffset));
            painter->restore();
        }

        const QColor lineColor = opt.palette.color(QPalette::Mid);
        painter->fillRect(QRect(option.rect.right() - kGridThickness + 1, option.rect.top(),
                                 kGridThickness, option.rect.height()),
                          lineColor);
        painter->fillRect(QRect(option.rect.left(), option.rect.bottom() - kGridThickness + 1,
                                 option.rect.width(), kGridThickness),
                          lineColor);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QSize base = QStyledItemDelegate::sizeHint(option, index);
        if (opt.text.isEmpty())
            return base;

        QTextLayout layout(opt.text, opt.font);
        layoutText(layout, textWidthFor(opt));
        return QSize(base.width(), qMax(base.height(), qCeil(layout.boundingRect().height()) + 8 + kGridThickness));
    }

private:
    static int textWidthFor(const QStyleOptionViewItem &option)
    {
        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        int textWidth = style->subElementRect(QStyle::SE_ItemViewItemText, &option, widget).width();
        if (textWidth <= 1)
            textWidth = option.rect.width();
        return qMax(1, textWidth - kGridThickness);
    }

    static void layoutText(QTextLayout &layout, int width)
    {
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textOption.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        layout.setTextOption(textOption);
        layout.beginLayout();
        qreal y = 0;
        while (true) {
            QTextLine line = layout.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(qMax(1, width));
            line.setPosition(QPointF(0, y));
            y += line.height();
        }
        layout.endLayout();
    }

    static QRect textArea(QRect rect)
    {
        return rect.adjusted(0, 0, -kGridThickness, -kGridThickness);
    }
};

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_triploader(this)
    , m_tripmanager(this)
{
    ui->setupUi(this);
    ui->leftPane->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    ui->rightPane->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    ui->mainSplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(1, 1);
    ui->mainSplitter->handle(1)->setEnabled(false);
    ui->leftLayout->setStretch(ui->leftLayout->indexOf(ui->tripTable), 1);
    ui->rightLayout->setStretch(ui->rightLayout->indexOf(ui->errorTable), 1);
    ui->tripTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->errorTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tripTable->setShowGrid(false);
    ui->errorTable->setShowGrid(false);
    ui->tripTable->setModel(m_tripmanager.storage());
    ui->errorTable->setModel(m_tripmanager.errorStorage());
    ui->reportTable->setModel(m_tripmanager.reporter());
    ui->reportTable->setWordWrap(true);
    ui->reportTable->setTextElideMode(Qt::TextElideMode::ElideNone);
    ui->reportTabLayout->setStretch(0, 1);
    configureTripView(ui->tripTable, ui->tripTable->horizontalHeader(), false);
    configureTripView(ui->errorTable, ui->errorTable->horizontalHeader(), true);
    configureTripView(ui->reportTable, ui->reportTable->horizontalHeader(), true);
    ui->tripTable->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tripTable->verticalHeader()->setStyleSheet(ui->tripTable->horizontalHeader()->styleSheet());
    ui->tripTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->errorTable->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->errorTable->verticalHeader()->setStyleSheet(ui->errorTable->horizontalHeader()->styleSheet());
    ui->errorTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->reportTable->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->reportTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    centerFileCombo(ui->currentFileCombo);
    centerFileCombo(ui->currentErrorFileCombo);
    showTripColumns(ui->tripTable);
    showTripColumns(ui->errorTable);
    fitReportColumns();
    connect(m_tripmanager.storage(), &QAbstractItemModel::modelReset, this, [this] {
        showTripColumns(ui->tripTable);
    });
    connect(m_tripmanager.errorStorage(), &QAbstractItemModel::modelReset, this, [this] {
        showTripColumns(ui->errorTable);
    });
    connect(m_tripmanager.reporter(), &QAbstractItemModel::modelReset, this, [this] {
        fitReportColumns();
    });
    connect(ui->actionOpenDirectory, &QAction::triggered,
            this, &MainWindow::openDirectory);
    connect(ui->actionHelp, &QAction::triggered,
            this, &MainWindow::showHelp);
    connect(ui->actionAbout, &QAction::triggered,
            this, &MainWindow::showAbout);
    connect(&m_triploader, &TripLoader::tripsLoaded,
            &m_tripmanager, &TripManager::setTrips);
    connect(&m_triploader, &TripLoader::tripsLoaded,
            this, &MainWindow::showLoadResult);
    connect(&m_triploader, &TripLoader::loadFailed,
            this, &MainWindow::showLoadErrors);
    connect(ui->checkLoadedButton, &QPushButton::clicked,
            this, &MainWindow::checkLoaded);
    connect(ui->reportErrorsButton, &QPushButton::clicked,
            &m_tripmanager, &TripManager::exportErrors);
    connect(ui->previousFileButton, &QPushButton::clicked,
            &m_tripmanager, &TripManager::showPreviousFile);
    connect(ui->nextFileButton, &QPushButton::clicked,
            &m_tripmanager, &TripManager::showNextFile);
    connect(&m_tripmanager, &TripManager::currentFileChanged,
            this, &MainWindow::showCurrentFile);
    connect(ui->previousErrorButton, &QPushButton::clicked,
            &m_tripmanager, &TripManager::showPreviousErrorFile);
    connect(ui->nextErrorButton, &QPushButton::clicked,
            &m_tripmanager, &TripManager::showNextErrorFile);
    connect(ui->currentFileCombo, &QComboBox::activated,
            this, &MainWindow::showFileFromCombo);
    connect(ui->currentErrorFileCombo, &QComboBox::activated,
            this, &MainWindow::showErrorFileFromCombo);
    connect(&m_tripmanager, &TripManager::currentErrorFileChanged,
            this, &MainWindow::showCurrentErrorFile);
    connect(ui->errorTable, &QTableView::clicked, this, [this](const QModelIndex &index) {
        showSelectedError(index, {});
    });
    connect(ui->errorTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::showSelectedError);
    connect(ui->mainTabs, &QTabWidget::currentChanged, this, [this] {
        equalizePanes();
    });
    updateActionButtons();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    equalizePanes();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    equalizePanes();
}

void MainWindow::equalizePanes()
{
    const int available = ui->mainSplitter->width() - ui->mainSplitter->handleWidth();
    if (available <= 0)
        return;

    const int left = available / 2;
    const int right = available - left;
    const QList<int> sizes = ui->mainSplitter->sizes();
    if (sizes.size() == 2 && sizes.at(0) == left && sizes.at(1) == right)
        return;
    ui->mainSplitter->setSizes({left, right});
}

void MainWindow::configureTripView(QAbstractItemView *view, QHeaderView *header, bool boldHeader)
{
    view->setItemDelegate(new WrappingItemDelegate(view));
    const QString headerStyle = QStringLiteral(
        "QHeaderView::section {"
        "  border: none;"
        "  border-right: %1px solid palette(mid);"
        "  border-bottom: %1px solid palette(mid);"
        "  padding: 4px;"
        "%2"
        "}").arg(kGridThickness).arg(boldHeader ? QStringLiteral("  font-weight: bold;") : QString());
    if (boldHeader) {
        QFont font = header->font();
        font.setBold(true);
        header->setFont(font);
    }
    header->setDefaultAlignment(Qt::AlignCenter);
    header->setStyleSheet(headerStyle);
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(true);
    connect(header, &QHeaderView::sectionResized, view, [view] {
        if (auto *table = qobject_cast<QTableView *>(view))
            table->resizeRowsToContents();
    });
}

namespace {

template <typename View>
void applyVisibleColumns(View *view, QHeaderView *header)
{
    static constexpr std::array kVisibleColumns{0, 5, 6, 7, 12};
    if (!view->model())
        return;

    const int columns = view->model()->columnCount();
    for (int column = 0; column < columns; ++column) {
        const bool visible = std::find(kVisibleColumns.begin(), kVisibleColumns.end(), column)
                             != kVisibleColumns.end();
        view->setColumnHidden(column, !visible);
        if (!visible)
            continue;
        view->resizeColumnToContents(column);
        const int minimum = header->sectionSizeHint(column) + 12;
        const int maximum = qMax(minimum, 280);
        view->setColumnWidth(column, qBound(minimum, view->columnWidth(column), maximum));
    }
}

}

void MainWindow::showTripColumns(QTableView *view)
{
    applyVisibleColumns(view, view->horizontalHeader());
    view->resizeRowsToContents();
}

void MainWindow::fitReportColumns()
{
    QHeaderView *header = ui->reportTable->horizontalHeader();
    const int columns = ui->reportTable->model()->columnCount();
    for (int column = 0; column < columns; ++column) {
        ui->reportTable->resizeColumnToContents(column);
        const int minimum = header->sectionSizeHint(column) + 12;
        const int maximum = qMax(minimum, 280);
        ui->reportTable->setColumnWidth(column, qBound(minimum, ui->reportTable->columnWidth(column), maximum));
    }
    ui->reportTable->resizeRowsToContents();
}

void MainWindow::showHelp()
{
    if (!m_helpWindow) {
        auto *dialog = new QDialog(this);
        dialog->setWindowTitle(tr("Súgó"));
        dialog->resize(760, 640);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        auto *layout = new QVBoxLayout(dialog);
        auto *browser = new QTextBrowser(dialog);
        browser->setOpenExternalLinks(true);
        QFile helpFile(QStringLiteral(":/help/readme.md"));
        if (helpFile.open(QIODevice::ReadOnly))
            browser->setMarkdown(QString::fromUtf8(helpFile.readAll()));
        else
            browser->setPlainText(tr("A súgó nem tölthető be."));
        layout->addWidget(browser);
        connect(dialog, &QObject::destroyed, this, [this] { m_helpWindow = nullptr; });
        m_helpWindow = dialog;
    }
    m_helpWindow->show();
    m_helpWindow->raise();
    m_helpWindow->activateWindow();
}

void MainWindow::showAbout()
{
    QMessageBox box(QMessageBox::NoIcon, tr("Névjegy"), QString(), QMessageBox::Ok, this);
    box.setTextFormat(Qt::RichText);
    box.setText(
        tr("<p align=\"center\"><b>Kiküldetési Rendelvény Ellenőrző</b></p>"
           "<p align=\"center\">v %1.%2.%3</p>"
           "<p align=\"center\">Vajda Ákos Péter<br>vajdaakos@alu-fon.hu</p>")
            .arg(MAJOR)
            .arg(MINOR)
            .arg(PATCH));
    const QPixmap icon = windowIcon().pixmap(64, 64);
    if (!icon.isNull())
        box.setIconPixmap(icon);
    for (QLabel *label : box.findChildren<QLabel *>())
        label->setAlignment(Qt::AlignCenter);
    box.exec();
}

void MainWindow::openDirectory()
{
    QFileDialog dialog(this, tr("PDF könyvtár megnyitása"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, false);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setNameFilter(tr("PDF fájlok (*.pdf)"));
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString directoryPath = dialog.selectedFiles().value(0);
    if (directoryPath.isEmpty())
        return;

    if (!m_triploader.loadDirectory(directoryPath)) {
        QMessageBox::information(
            this, tr("PDF fájlok megnyitása"),
            tr("A kiválasztott könyvtár nem tartalmaz PDF fájlt."));
    }
}

void MainWindow::showLoadResult(const QVector<QVector<Trip>> &trips)
{
    qsizetype tripCount = 0;
    for (const QVector<Trip> &fileTrips : trips)
        tripCount += fileTrips.size();

    statusBar()->showMessage(
        tr("%1 PDF feldolgozva, %2 utazás betöltve")
            .arg(trips.size())
            .arg(tripCount));
}

void MainWindow::showCurrentFile(const QString &fileName, bool hasPrevious, bool hasNext)
{
    Q_UNUSED(fileName);
    QComboBox *combo = ui->currentFileCombo;
    const QSignalBlocker blocker(combo);
    combo->clear();
    const TripStorage *storage = m_tripmanager.storage();
    const QVector<QVector<Trip>> &trips = storage->trips();
    const QVector<TripMetaData> &metadata = storage->fileMetadata();
    for (qsizetype file = 0; file < trips.size(); ++file) {
        QString name = storage->fileNameAt(file);
        if (name.isEmpty() && file < metadata.size())
            name = metadata.at(file).m_filename;
        combo->addItem(name, int(file));
        combo->setItemData(combo->count() - 1, int(Qt::AlignCenter), Qt::TextAlignmentRole);
    }
    combo->setCurrentIndex(combo->findData(int(storage->currentFile())));
    if (QLineEdit *edit = combo->lineEdit())
        edit->setAlignment(Qt::AlignCenter);
    combo->setEnabled(combo->count() > 0);
    ui->previousFileButton->setEnabled(hasPrevious);
    ui->nextFileButton->setEnabled(hasNext);
    ui->leftMetadataWidget->setMetadata(storage->currentMetadata());
    updateActionButtons();
}

void MainWindow::showCurrentErrorFile(const QString &fileName, bool hasPrevious, bool hasNext)
{
    Q_UNUSED(fileName);
    QComboBox *combo = ui->currentErrorFileCombo;
    const QSignalBlocker blocker(combo);
    combo->clear();
    const TripStorage *storage = m_tripmanager.storage();
    const ErrorStorage *errors = m_tripmanager.errorStorage();
    const QVector<QVector<Trip>> &trips = storage->trips();
    const QVector<TripMetaData> &metadata = storage->fileMetadata();
    for (qsizetype file = 0; file < trips.size(); ++file) {
        if (!errors->hasErrorAt(file))
            continue;
        QString name = storage->fileNameAt(file);
        if (name.isEmpty() && file < metadata.size())
            name = metadata.at(file).m_filename;
        combo->addItem(name, int(file));
        combo->setItemData(combo->count() - 1, int(Qt::AlignCenter), Qt::TextAlignmentRole);
    }
    combo->setCurrentIndex(combo->findData(int(errors->currentFile())));
    if (QLineEdit *edit = combo->lineEdit())
        edit->setAlignment(Qt::AlignCenter);
    combo->setEnabled(combo->count() > 0);
    ui->previousErrorButton->setEnabled(hasPrevious);
    ui->nextErrorButton->setEnabled(hasNext);
    ui->rightMetadataWidget->setMetadata(storage->metadataAt(errors->currentFile()));
    updateActionButtons();
}

void MainWindow::showFileFromCombo(int index)
{
    if (index < 0)
        return;
    m_tripmanager.showFile(ui->currentFileCombo->itemData(index).toInt());
}

void MainWindow::showErrorFileFromCombo(int index)
{
    if (index < 0)
        return;
    m_tripmanager.showErrorFile(ui->currentErrorFileCombo->itemData(index).toInt());
}

void MainWindow::updateActionButtons()
{
    const QVector<QVector<Trip>> &trips = m_tripmanager.storage()->trips();
    const bool hasTrips = std::any_of(trips.cbegin(), trips.cend(),
                                      [](const QVector<Trip> &fileTrips) { return !fileTrips.isEmpty(); });
    ui->checkLoadedButton->setEnabled(hasTrips);
    ui->reportErrorsButton->setEnabled(m_tripmanager.errorStorage()->hasErrors());

    const QString leftName = m_tripmanager.storage()->currentFileName();
    const QString rightName = m_tripmanager.storage()->fileNameAt(
        m_tripmanager.errorStorage()->currentFile());
    const QString centered = QStringLiteral(
        "QComboBox QLineEdit { qproperty-alignment: AlignCenter; background: transparent; border: none; }");
    ui->currentFileCombo->setStyleSheet(leftName == rightName
                                            ? centered
                                            : QStringLiteral("QComboBox, QComboBox QLineEdit { color: red; font-weight: bold; } ")
                                                  + centered);
    if (QLineEdit *edit = ui->currentFileCombo->lineEdit())
        edit->setAlignment(Qt::AlignCenter);
}

void MainWindow::checkLoaded()
{
    m_tripmanager.checkLoaded();
    if (!m_tripmanager.errorStorage()->hasErrors()) {
        QMessageBox::information(
            this, tr("Betöltöttek ellenőrzése"),
            tr("Nincs hiba a betöltött utak között."));
        return;
    }

    const QModelIndex firstError = ui->errorTable->model()->index(0, 0);
    if (!firstError.isValid())
        return;
    ui->errorTable->selectionModel()->setCurrentIndex(
        firstError, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    ui->errorTable->scrollTo(firstError, QAbstractItemView::ScrollHint::PositionAtCenter);
}

void MainWindow::showSelectedError(const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid())
        return;

    const Trip *source = m_tripmanager.errorStorage()->sourceAt(current.row());
    const int row = m_tripmanager.focusTrip(source);
    if (row < 0)
        return;

    ui->tripTable->selectRow(row);
    const QModelIndex tripIndex = ui->tripTable->model()->index(row, 0);
    ui->tripTable->scrollTo(tripIndex, QAbstractItemView::ScrollHint::PositionAtCenter);
}

void MainWindow::showLoadErrors(const QStringList &errors)
{
    QMessageBox::warning(
        this, tr("Néhány PDF nem nyitható meg"),
        tr("%1 fájl feldolgozása sikertelen:\n\n%2")
            .arg(errors.size())
            .arg(errors.join(QLatin1Char('\n'))));
}
