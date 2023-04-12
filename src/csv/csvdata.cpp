#include "csvdata.h"

CsvData::CsvData()
{

}

void CsvData::setup(QString filepath, char delimiter)
{
    QFile file(filepath);

    QString errMsg;

    if (!file.exists())
        errMsg = QString("CSV file '%1' doesn't exist.").arg(file.fileName());

    if (!file.open(QIODevice::ReadOnly))
        errMsg = QString("CSV file '%1' cannot be opened.").arg(file.fileName());

    if (!errMsg.isEmpty())
    {
        qDebug(errMsg.toUtf8());
        return;
    }

    // Read csv headers, trimming all whitespace
    headers = QString(file.readLine()).split(delimiter);
    for (QString& val : headers)
        val = val.trimmed();

    data.clear();
    dataTransposed.clear();

    while (!file.atEnd())
    {
        // Read line and trim whitespace
        QStringList line = QString(file.readLine()).split(delimiter);
        for (QString& val : line)
            val = val.trimmed();

        // Add to data && data'
        data.append(line);
        for (int i = 0; i < line.size(); ++i)
        {
            if (i >= dataTransposed.size())
                dataTransposed.append(QStringList());
            dataTransposed[i].append(line.at(i));
        }
    }

    file.close();
}

int CsvData::getNumCols(int row) {
    if (row > -1 && row < getNumRows())
        return data.at(row).size();
    else
        return 0;
}

QStringList CsvData::getRow(int row)
{
    if (row > -1 && row < getNumRows())
        return data.at(row);
    else
        return QStringList();
}

QStringList CsvData::getCol(int col)
{
    if (col > -1 && col < getNumCols())
        return dataTransposed.at(col);
    else
        return QStringList();
}

QString CsvData::getVal(int row, int col)
{
    if ((row > -1 && row < getNumRows()) && (col > -1 && col < getNumCols()))
        return data.at(row).at(col);
    else
        return QString();
}

QString CsvData::getHeader(int col)
{
    if (col > -1 && col < getNumCols())
        return headers.at(col);
    else
        return QString();
}
