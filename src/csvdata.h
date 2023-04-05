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
    CsvData(char delimiter=',');
    CsvData(QString filepath, char delimiter=',');
    CsvData(QFile file, char delimiter=',');

    int getNumRows() { return data.size(); }
    int getNumCols() { return dataTransposed.size(); }

    QStringList getHeaders() { return headers; }
    QStringList getRow(int row);
    QStringList getCol(int col);

    QString getVal(int row, int col);
    QString getHeader(int col);

private:
    const char delim;
    QStringList headers;
    QList<QStringList> data;
    QList<QStringList> dataTransposed;

    void init(QFile& file);
};

#endif // CSV_H
