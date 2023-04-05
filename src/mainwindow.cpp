#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QLabel>

#include "csvdata.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupCsvValues();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupCsvValues()
{
    char delim = ',';

    CsvData csvA = CsvData("testA.csv", delim);
    CsvData csvB = CsvData("testB.csv", delim);

    for (int c = 0; c < csvA.getNumCols(); ++c)
    {
        ui->gridComparisonA->addWidget(new QCheckBox(csvA.getHeader(c)), 0, c);
        ui->gridComparisonB->addWidget(new QCheckBox(csvB.getHeader(c)), 0, c);
    }

    for (int r = 0; r < csvA.getNumRows(); ++r)
    {
        for (int c = 0; c < csvA.getNumCols(); ++c)
        {
            ui->gridComparisonA->addWidget(new QLabel(csvA.getVal(r, c)), r+1, c);
        }
    }

    for (int r = 0; r < csvB.getNumRows(); ++r)
    {
        for (int c = 0; c < csvB.getNumCols(); ++c)
        {
            ui->gridComparisonB->addWidget(new QLabel(csvB.getVal(r, c)), r+1, c);
        }
    }
}

void MainWindow::updateDiff()
{
    // TODO: get [[row,col]] that are different
    // TODO: pass tolerance (global)
    // TODO: pass headers (localized tolerance)
}
