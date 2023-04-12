#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Add gridLayouts to the scroll areas
    gridA = new QGridLayout(ui->scrollContentsA);
    gridB = new QGridLayout(ui->scrollContentsB);
    ui->scrollContentsA->setLayout(gridA);
    ui->scrollContentsB->setLayout(gridB);

    // Setup file icons
    QPixmap csvPixmap = QPixmap(":/icons/csv.png").scaledToHeight(16, Qt::SmoothTransformation);
    ui->iconFileA->setPixmap(csvPixmap);
    ui->iconFileB->setPixmap(csvPixmap);

    delim = ',';
    lastPath = QString(".");

    setupCsv();
}

MainWindow::~MainWindow()
{
    // Stop the thread
    csvThread->quit();
    csvThread->wait();

    delete csvComparison;
    delete csvThread;

    delete gridA;
    delete gridB;

    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // TODO: Resizing smaller than initial size work. Resizing larger expands some inner containers but not all?
    QRect rectCentral = ui->centralWidget->geometry();
    rectCentral.setWidth(event->size().width());
    rectCentral.setHeight(event->size().height());
    ui->centralWidget->setGeometry(rectCentral);
    ui->layoutMain->setGeometry(rectCentral);

    QRect rectComp = ui->layoutComparison->geometry();
    int newCompWidth = event->size().width() - (rectComp.x()*2); // Assumes this item is centered horizontally
    int newCompHeight = event->size().height() - rectComp.y(); // Assumes this item is the bottom-most
    rectComp.setWidth(newCompWidth);
    rectComp.setHeight(newCompHeight);
    ui->layoutComparison->setGeometry(rectComp);
}

void MainWindow::setupCsv()
{
    csvComparison = new CsvComparison();

    // Move the csv processing to another thread so we don't block the UI
    csvThread = new QThread();
    csvComparison->moveToThread(csvThread);

    // Register QLists that we want to use in signals/slots
    qRegisterMetaType<QList<QStringList>>("QList<QStringList>");
    qRegisterMetaType<QList<QPoint>>("QList<QPoint>");

    // Connect loading and displaying
    connect(this, &MainWindow::loadCsv, csvComparison, &CsvComparison::loadCsv);
    connect(this, &MainWindow::updateDiff, csvComparison, &CsvComparison::updateDiff);
    connect(csvComparison, &CsvComparison::displayCsv, this, &MainWindow::displayCsv);
    connect(csvComparison, &CsvComparison::displayDiff, this, &MainWindow::displayDiff);

    // Start thread to begin catch signals
    csvThread->start();
}

void MainWindow::clearCsvGrids()
{
    // Clears everything from the grids

    QLayoutItem *child;
    while ((child = gridA->takeAt(0)) != 0)
    {
        child->widget()->hide();
        gridA->removeItem(child);
        delete child;
    }
    while ((child = gridB->takeAt(0)) != 0)
    {
        child->widget()->hide();
        gridB->removeItem(child);
        delete child;
    }
}

void MainWindow::displayHeaders(QStringList headersA, QStringList headersB)
{
    // Adds checkboxes for header columns

    // Get initial state of checkboxes
    bool selectAll = ui->checkBoxAllCols->isChecked();

    int maxCols = std::max(headersA.size(), headersB.size());

    for (int col = 0; col < maxCols; ++col)
    {
        QString headA = headersA.value(col);
        QString headB = headersB.value(col);
        // If different number of columns, one will be empty string
        if (headA == "") headA = headB;
        if (headB == "") headB = headA;
        // If headers are different, show the same for both
        if (headA != headB)
        {
            headA = QString("%1 | %2").arg(headA).arg(headB);
            headB = headA;
        }

        // Populate UI items
        QCheckBox *colHeadA = new QCheckBox(headA);
        QCheckBox *colHeadB = new QCheckBox(headB);
        colHeadA->setChecked(selectAll);
        colHeadB->setChecked(selectAll);
        connect(colHeadA, &QCheckBox::stateChanged, this, &MainWindow::onCheckboxStateChanged);
        connect(colHeadB, &QCheckBox::stateChanged, this, &MainWindow::onCheckboxStateChanged);

        gridA->addWidget(colHeadA, 0, col);
        gridB->addWidget(colHeadB, 0, col);
    }
}

void MainWindow::displayCsv(QList<QStringList> csvDataA, QList<QStringList> csvDataB)
{
    qDebug("Display update started.");

    // Remove old data
    clearCsvGrids();
    // Setup headers again
    displayHeaders(csvDataA.takeFirst(), csvDataB.takeFirst());

    // Now display data
    int maxRows = std::max(csvDataA.size(), csvDataB.size());

    for (int row = 0; row < maxRows; ++row)
    {
        // .value() will return and empty QStringList() if index out-of-bounds
        QStringList rowA = csvDataA.value(row);
        QStringList rowB = csvDataB.value(row);

        int maxCols = std::max(rowA.size(), rowB.size());

        for (int col = 0; col < maxCols; ++col)
        {
            // .value() will return and empty QString() if index out-of-bounds
            QString valA = rowA.value(col);
            QString valB = rowB.value(col);

            gridA->addWidget(new QLabel(valA), row+1, col); // +1 as we popped the first row as headers
            gridB->addWidget(new QLabel(valB), row+1, col);
        }
    }
    qDebug("Display update finished");

    // Trigger a diff
    triggerUpdate();
}

void MainWindow::displayDiff(QList<QPoint> diffPoints)
{
    // Go through each different item and highlight
    for (QPoint p : diffPoints)
    {
        QLayoutItem* layoutItem;

        layoutItem = gridA->itemAtPosition(p.y(), p.x());
        if (layoutItem != nullptr && layoutItem->widget() != nullptr)
            layoutItem->widget()->setStyleSheet("QWidget { background-color : rgba(0,0,255,75); }");

        layoutItem = gridB->itemAtPosition(p.y(), p.x());
        if (layoutItem != nullptr && layoutItem->widget() != nullptr)
            layoutItem->widget()->setStyleSheet("QWidget { background-color : rgba(0,0,255,75); }");
    }
}

void MainWindow::triggerUpdate()
{
    // Read the threshhold
    double thresh = ui->inputTolerance->value();

    // Get columns that are checked
    QList<int> columnIndexes;
    for (int col = 0; col < gridA->columnCount(); ++col)
    {
        QCheckBox *colHead = qobject_cast<QCheckBox*>(gridA->itemAtPosition(0, col)->widget());

        if (colHead->isChecked())
            columnIndexes.append(col);
    }

    // Set the selectAll checkbox state depending on the number of columns checked
    if (columnIndexes.size() == gridA->columnCount())
        ui->checkBoxAllCols->setCheckState(Qt::CheckState::Checked);
    else if (columnIndexes.size() == 0)
        ui->checkBoxAllCols->setCheckState(Qt::CheckState::Unchecked);
    else
        ui->checkBoxAllCols->setCheckState(Qt::CheckState::PartiallyChecked);

    // Trigger a diff update
    emit updateDiff(thresh, columnIndexes);

    // Reset higlighting while the diff thread is working
    // The DisplayDiff slot may be triggered before we finish highlighting, but cannot execute until after
    resetHighlighting();
}

void MainWindow::resetHighlighting()
{
    // Even if data is different, the grids should be the same size
    if (gridA->rowCount() != gridB->rowCount() ||
        gridA->columnCount() != gridB->columnCount())
        qFatal("Inconsistent grid sizes!");

    // Reset the stylesheets
    for (int row = 0; row < gridA->rowCount(); ++row)
    {
        for (int col = 0; col < gridA->columnCount(); ++col)
        {
            gridA->itemAtPosition(row, col)->widget()->setStyleSheet("");
            gridB->itemAtPosition(row, col)->widget()->setStyleSheet("");
        }
    }
}

void MainWindow::on_inputTolerance_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    triggerUpdate();
}

void MainWindow::onCheckboxStateChanged(int state)
{
    Q_UNUSED(state);

    // Find which checkbox triggered this slot
    QWidget *header = qobject_cast<QWidget*>(sender());
    int idxA = gridA->indexOf(header);
    int idxB = gridB->indexOf(header);

    // Get the checkbox from the other grid and toggle it so that both sides have the same state
    if (idxA > -1)
    {
        QCheckBox *colHead = qobject_cast<QCheckBox*>(gridB->itemAtPosition(0, idxA)->widget());
        colHead->setCheckState(static_cast<Qt::CheckState>(state));
    }
    else if (idxB > -1)
    {
        QCheckBox *colHead = qobject_cast<QCheckBox*>(gridA->itemAtPosition(0, idxB)->widget());
        colHead->setCheckState(static_cast<Qt::CheckState>(state));
    }
    else
    {
        qFatal("Both sides of comparison should have same number of columns but don't!");
    }

    triggerUpdate();
}

void MainWindow::on_checkBoxAllCols_stateChanged(int arg1)
{
    Qt::CheckState state = static_cast<Qt::CheckState>(arg1);

    // A column header was clicked
    if (state == Qt::CheckState::PartiallyChecked) return;

    // This checkBox was clicked, change the state of all other checkboxes
    for (int col = 0; col < gridA->columnCount(); ++col)
    {
        QCheckBox *colHeadA = qobject_cast<QCheckBox*>(gridA->itemAtPosition(0, col)->widget());
        QCheckBox *colHeadB = qobject_cast<QCheckBox*>(gridB->itemAtPosition(0, col)->widget());

        colHeadA->blockSignals(true);
        colHeadB->blockSignals(true);

        colHeadA->setCheckState(state);
        colHeadB->setCheckState(state);

        colHeadA->blockSignals(false);
        colHeadB->blockSignals(false);
    }

    triggerUpdate();
}

void MainWindow::on_btnSelectFiles_clicked()
{
    // Read in user-selected files
    QString pathA = QFileDialog::getOpenFileName(this, tr("Select fileA"), lastPath, "CSV (*.csv)");
    if (pathA.isEmpty())
        return;
    lastPath = pathA;

    QString pathB = QFileDialog::getOpenFileName(this, tr("Select fileB"), lastPath, "CSV (*.csv)");
    if (pathB.isEmpty())
        return;
    lastPath = pathB;

    // Update the labels with filenames
    ui->labelFileA->setText(pathA.split('/').last());
    ui->labelFileB->setText(pathB.split('/').last());

    // Trigger a reload of data
    emit loadCsv(pathA, pathB, delim);
}
