#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QCheckBox>
#include <QLabel>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QChar delim = ',';

    QFile fileA("testA.csv");
    QFile fileB("testB.csv");

    if (fileA.open(QIODevice::ReadOnly) && fileB.open(QIODevice::ReadOnly))
    {
        QStringList headers;
        QList<QStringList> valuesA;
        QList<QStringList> valuesB;

        QString line1 = fileA.readLine().trimmed();
        if (line1 == fileB.readLine().trimmed())
        {
            headers = line1.split(delim);
        }
        else
        {
            // TODO: Proper handling
            qDebug("Header mismatch!");
            exit(1);
        }


        while (!fileA.atEnd())
        {
            QString line = fileA.readLine().trimmed();
            valuesA.append(line.split(','));
        }
        while (!fileB.atEnd())
        {
            QString line = fileB.readLine().trimmed();
            valuesB.append(line.split(','));
        }

        for (int col = 0; col < headers.size(); ++col)
        {
            ui->gridComparisonA->addWidget(new QCheckBox(headers.at(col)), 0, col);
            ui->gridComparisonB->addWidget(new QCheckBox(headers.at(col)), 0, col);
        }

        for (int r = 0; r < valuesA.size(); ++r)
        {
            QStringList line = valuesA.at(r);

            for (int c = 0; c < line.size(); ++c)
            {
                ui->gridComparisonA->addWidget(new QLabel(line.at(c)), r+1, c);
            }
        }
        for (int r = 0; r < valuesB.size(); ++r)
        {
            QStringList line = valuesB.at(r);

            for (int c = 0; c < line.size(); ++c)
            {
                ui->gridComparisonB->addWidget(new QLabel(line.at(c)), r+1, c);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
