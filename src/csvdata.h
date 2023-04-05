#ifndef CSV_H
#define CSV_H

#include <QObject>

#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>

class CsvData
{
public:
    CsvData();

    void setup(QString filename, char delim);

    int getNumRows() { return data.size(); }
    int getNumCols(int row=0);

    QStringList getHeaders() { return headers; }
    QList<QStringList> getCsv() { return data; }
    QStringList getRow(int row);
    QStringList getCol(int col);

    QString getVal(int row, int col);
    QString getHeader(int col);

private:
    QStringList headers;
    QList<QStringList> data;
    QList<QStringList> dataTransposed;
};

#endif // CSV_H
