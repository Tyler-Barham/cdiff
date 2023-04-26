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

    // Scrollable UI elements
    tableA = new Table(this);
    ui->layoutComparison->addWidget(tableA);
    tableB = new Table(this);
    ui->layoutComparison->addWidget(tableB);

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

    delete tableA;
    delete tableB;

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
    // TODO: Setup for table class
}

void MainWindow::displayHeaders(QStringList headersA, QStringList headersB)
{
    tableA->setHeaders(headersA);
    tableB->setHeaders(headersB);

    connect(tableA, &Table::checkboxStateChanged, this, &MainWindow::onCheckboxStateChanged);
    connect(tableB, &Table::checkboxStateChanged, this, &MainWindow::onCheckboxStateChanged);
}

void MainWindow::displayCsv(QList<QStringList> csvDataA, QList<QStringList> csvDataB)
{
    // Remove old data
    clearCsvGrids();
    // Setup headers again
    displayHeaders(csvDataA.takeFirst(), csvDataB.takeFirst());

    tableA->setData(csvDataA);
    tableB->setData(csvDataB);

    // Trigger a diff
    triggerUpdate();
}

void MainWindow::displayDiff(QList<QPoint> diffPoints)
{
    lastDiffPoints.clear();
    lastDiffPoints.append(diffPoints);

    // TODO: Setup for table class
}

void MainWindow::triggerUpdate()
{
    // Read the threshhold
    double thresh = ui->inputTolerance->value();

    // Get columns that are checked
    QList<int> columnIndexes;

    // TODO: Setup for table class
    /*
    foreach columnHeader
        if (colHead->isChecked())
            columnIndexes.append(col);
    // Set the selectAll checkbox state depending on the number of columns checked
    */

    // Trigger a diff update
    emit updateDiff(thresh, columnIndexes);

    // Reset higlighting while the diff thread is working
    // The DisplayDiff slot may be triggered before we finish highlighting, but cannot execute until after
}

void MainWindow::on_inputTolerance_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    triggerUpdate();
}

void MainWindow::updateCheckboxes(int state, int idx)
{
    tableA->blockSignals(true);
    tableB->blockSignals(true);

    tableA->setHeaderState(state, idx);
    tableB->setHeaderState(state, idx);

    tableA->blockSignals(false);
    tableB->blockSignals(false);

    triggerUpdate();
}

void MainWindow::onCheckboxStateChanged(int state, int idx)
{
    updateCheckboxes(state, idx);
}

void MainWindow::on_checkBoxAllCols_stateChanged(int arg1)
{
    // -1 == all
    updateCheckboxes(arg1, -1);
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

    ui->checkBoxAllCols->setCheckState(Qt::CheckState::Unchecked);

    // Trigger a reload of data
    emit loadCsv(pathA, pathB, delim);
}
