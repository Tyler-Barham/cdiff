#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupCsv();
}

MainWindow::~MainWindow()
{
    csvThread->quit();
    csvThread->wait();

    delete csvComparison;
    delete csvThread;

    delete ui;
}

void MainWindow::setupCsv()
{
    char delim = ',';

    csvComparison = new CsvComparison();

    // Move the csv processing to another thread so we don't block the UI
    csvThread = new QThread();
    csvComparison->moveToThread(csvThread);

    // Register cv::Mat so that it can be used in signals/slots
    qRegisterMetaType<QList<QStringList>>("QList<QStringList>");
    qRegisterMetaType<QList<QPoint>>("QList<QPoint>");

    // General loading and displaying
    connect(this, &MainWindow::loadCsv, csvComparison, &CsvComparison::loadCsv);
    connect(this, &MainWindow::updateDiff, csvComparison, &CsvComparison::updateDiff);
    connect(csvComparison, &CsvComparison::displayHeaders, this, &MainWindow::displayHeaders);
    connect(csvComparison, &CsvComparison::displayCsv, this, &MainWindow::displayCsv);
    connect(csvComparison, &CsvComparison::displayDiff, this, &MainWindow::displayDiff);

    // Start thread to begin catch signals
    csvThread->start();

    emit loadCsv("testA.csv", "testB.csv", delim);
}

void MainWindow::displayHeaders(QStringList headersA, QStringList headersB)
{
    int maxCols = std::max(headersA.size(), headersB.size());

    for (int col = 0; col < maxCols; ++col)
    {
        QString headA = headersA.value(col);
        QString headB = headersB.value(col);
        if (headA == "") headA = headB;
        if (headB == "") headB = headA;

        ui->gridComparisonA->addWidget(new QCheckBox(headA), 0, col);
        ui->gridComparisonB->addWidget(new QCheckBox(headB), 0, col);
    }
}

void MainWindow::displayCsv(QList<QStringList> csvDataA, QList<QStringList> csvDataB)
{
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

            ui->gridComparisonA->addWidget(new QLabel(valA), row+1, col);
            ui->gridComparisonB->addWidget(new QLabel(valB), row+1, col);
        }
    }

    triggerUpdate();
}

void MainWindow::displayDiff(QList<QPoint> diffPoints)
{
    for (QPoint p : diffPoints)
    {
        QLayoutItem* layoutItem;

        layoutItem = ui->gridComparisonA->itemAtPosition(p.y()+1, p.x());
        if (layoutItem != nullptr && layoutItem->widget() != nullptr)
            layoutItem->widget()->setStyleSheet("QWidget { background-color : rgba(0,0,255,75); }");

        layoutItem = ui->gridComparisonB->itemAtPosition(p.y()+1, p.x());
        if (layoutItem != nullptr && layoutItem->widget() != nullptr)
            layoutItem->widget()->setStyleSheet("QWidget { background-color : rgba(0,0,255,75); }");
    }
}

void MainWindow::triggerUpdate()
{
    double thresh = ui->inputThreshold->value();
    QList<int> columnIndexes({0, 1, 2}); // TODO: pull from the UI

    // Trigger a diff update
    emit updateDiff(thresh, columnIndexes);

    // Reset higlighting while the diff thread is working
    resetHighlighting();
}

void MainWindow::resetHighlighting()
{
    // Even if data is different, the grids should be the same size
    if (ui->gridComparisonA->rowCount() != ui->gridComparisonB->rowCount() ||
        ui->gridComparisonA->columnCount() != ui->gridComparisonB->columnCount())
        return;

    for (int row = 1; row < ui->gridComparisonA->rowCount(); ++row)
    {
        for (int col = 0; col < ui->gridComparisonA->columnCount(); ++col)
        {
            ui->gridComparisonA->itemAtPosition(row, col)->widget()->setStyleSheet("");
            ui->gridComparisonB->itemAtPosition(row, col)->widget()->setStyleSheet("");
        }
    }
}

void MainWindow::on_inputThreshold_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    triggerUpdate();
}
