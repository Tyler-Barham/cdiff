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
        if (headA != headB)
        {
            headA = QString("%1 | %2").arg(headA).arg(headB);
            headB = headA;
        }

        QCheckBox *colHeadA = new QCheckBox(headA);
        QCheckBox *colHeadB = new QCheckBox(headB);
        connect(colHeadA, &QCheckBox::stateChanged, this, &MainWindow::onCheckboxStateChanged);
        connect(colHeadB, &QCheckBox::stateChanged, this, &MainWindow::onCheckboxStateChanged);

        ui->gridComparisonA->addWidget(colHeadA, 0, col);
        ui->gridComparisonB->addWidget(colHeadB, 0, col);
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

            ui->gridComparisonA->addWidget(new QLabel(valA), fDataRow+row, col);
            ui->gridComparisonB->addWidget(new QLabel(valB), fDataRow+row, col);
        }
    }

    triggerUpdate();
}

void MainWindow::displayDiff(QList<QPoint> diffPoints)
{
    for (QPoint p : diffPoints)
    {
        QLayoutItem* layoutItem;

        layoutItem = ui->gridComparisonA->itemAtPosition(fDataRow+p.y(), p.x());
        if (layoutItem != nullptr && layoutItem->widget() != nullptr)
            layoutItem->widget()->setStyleSheet("QWidget { background-color : rgba(0,0,255,75); }");

        layoutItem = ui->gridComparisonB->itemAtPosition(fDataRow+p.y(), p.x());
        if (layoutItem != nullptr && layoutItem->widget() != nullptr)
            layoutItem->widget()->setStyleSheet("QWidget { background-color : rgba(0,0,255,75); }");
    }
}

void MainWindow::triggerUpdate()
{
    double thresh = ui->inputTolerance->value();
    QList<int> columnIndexes;
    for (int col = 0; col < ui->gridComparisonA->columnCount(); ++col)
    {
        QCheckBox *colHead = qobject_cast<QCheckBox*>(ui->gridComparisonA->itemAtPosition(0, col)->widget());

        if (colHead->isChecked())
            columnIndexes.append(col);
    }

    if (columnIndexes.size() == ui->gridComparisonA->columnCount())
        ui->checkBoxAllCols->setCheckState(Qt::CheckState::Checked);
    else if (columnIndexes.size() == 0)
        ui->checkBoxAllCols->setCheckState(Qt::CheckState::Unchecked);
    else
        ui->checkBoxAllCols->setCheckState(Qt::CheckState::PartiallyChecked);

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

    for (int row = fDataRow; row < ui->gridComparisonA->rowCount(); ++row)
    {
        for (int col = 0; col < ui->gridComparisonA->columnCount(); ++col)
        {
            ui->gridComparisonA->itemAtPosition(row, col)->widget()->setStyleSheet("");
            ui->gridComparisonB->itemAtPosition(row, col)->widget()->setStyleSheet("");
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

    QWidget *header = qobject_cast<QWidget*>(sender());
    int idxA = ui->gridComparisonA->indexOf(header);
    int idxB = ui->gridComparisonB->indexOf(header);

    if (idxA > -1)
    {
        QCheckBox *colHead = qobject_cast<QCheckBox*>(ui->gridComparisonB->itemAtPosition(0, idxA)->widget());
        colHead->setCheckState(static_cast<Qt::CheckState>(state));
    }
    else if (idxB > -1)
    {
        QCheckBox *colHead = qobject_cast<QCheckBox*>(ui->gridComparisonA->itemAtPosition(0, idxB)->widget());
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

    // This checkBox was clicked
    for (int col = 0; col < ui->gridComparisonA->columnCount(); ++col)
    {
        QCheckBox *colHeadA = qobject_cast<QCheckBox*>(ui->gridComparisonA->itemAtPosition(0, col)->widget());
        QCheckBox *colHeadB = qobject_cast<QCheckBox*>(ui->gridComparisonB->itemAtPosition(0, col)->widget());

        colHeadA->blockSignals(true);
        colHeadB->blockSignals(true);

        colHeadA->setCheckState(state);
        colHeadB->setCheckState(state);

        colHeadA->blockSignals(false);
        colHeadB->blockSignals(false);
    }

    triggerUpdate();
}
