#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "csvcomparison.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void setupCsv();
    void triggerUpdate();
    void resetHighlighting();

    CsvComparison *csvComparison;
    QThread *csvThread;

    int fDataRow = 1;

private slots:
    void displayHeaders(QStringList headersA, QStringList headersB);
    void displayCsv(QList<QStringList> csvDataA, QList<QStringList> csvDataB);
    void displayDiff(QList<QPoint> diffPoints);

    // Connected to UI signals (manually)
    void onCheckboxStateChanged(int state);

    // Connected to UI signals (automatically)
    void on_inputTolerance_valueChanged(double arg1);
    void on_checkBoxAllCols_stateChanged(int arg1);

signals:
    void loadCsv(QString filepathA, QString filepathB, char delimiter);
    void updateDiff(double threshold, QList<int> columnIndexes);
};

#endif // MAINWINDOW_H
