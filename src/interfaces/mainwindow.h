#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QThread>

#include "table.h"
#include "csv/csvcomparison.h"

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
    Table *tableA;
    Table *tableB;

    void resizeEvent(QResizeEvent *event) override;
    void setupCsv();
    void triggerUpdate();
    void updateCheckboxes(int state, int idx);

    CsvComparison *csvComparison;
    QThread *csvThread;

    QString lastPath;
    char delim;

private slots:
    void displayCsv(QList<QStringList> csvDataA, QList<QStringList> csvDataB);
    void displayDiff(QList<QPoint> diffPoints);

    // Connected to UI signals (manually)
    void onCheckboxStateChanged(int state, int idx);

    // Connected to UI signals (automatically)
    void on_inputTolerance_valueChanged(double arg1);
    void on_checkBoxAllCols_stateChanged(int arg1);
    void on_btnSelectFiles_clicked();

signals:
    void loadCsv(QString filepathA, QString filepathB, char delimiter);
    void updateDiff(double threshold, QList<int> columnIndexes);
};

#endif // MAINWINDOW_H
