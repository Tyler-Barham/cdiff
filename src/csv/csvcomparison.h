#ifndef CSVCOMPARISON_H
#define CSVCOMPARISON_H

#include <QObject>

#include "csvdata.h"

struct Point {
    int row, col;
    Point(int r, int c) { row = r; col = c; }
};

class CsvComparison : public QObject
{
    Q_OBJECT

public:
    CsvComparison();
    ~CsvComparison();

private:
    CsvData *csvDataA;
    CsvData *csvDataB;
    QList<QList<double>> diffValues;
    QList<QPoint> diffPoints;

public slots:
    void loadCsv(QString filepathA, QString filepathB, char delimiter=',');
    void updateDiff(double threshold, QList<int> headerIdxs);

signals:
    void displayCsv(QList<QStringList> csvDataA, QList<QStringList> csvDataB);
    void displayDiff(QList<QPoint> diffPoints);
};

#endif // CSVCOMPARISON_H
