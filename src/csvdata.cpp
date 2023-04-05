#include "csvdata.h"

CsvData::CsvData(char delimiter)
    : delim(delimiter)
{

}

CsvData::CsvData(QString filepath, char delimiter)
    : delim(delimiter)
{
    QFile file(filepath);
    init(file);
}

CsvData::CsvData(QFile file, char delimiter)
    : delim(delimiter)
{
    init(file);
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

void CsvData::init(QFile& file)
{
    QString errMsg;

    if (!file.exists())
        errMsg = QString("CSV file '%1' doesn't exist.").arg(file.fileName());

    if (!file.isOpen())
        if (!file.open(QIODevice::ReadOnly))
            errMsg = QString("CSV file '%1' cannot be opened.").arg(file.fileName());

    if (!errMsg.isEmpty())
    {
        qDebug(errMsg.toUtf8());
        return;
    }

    // Read csv headers, trimming all whitespace
    headers = QString(file.readLine()).split(delim);
    for (QString& val : headers)
        val = val.trimmed();

    while (!file.atEnd())
    {
        // Read line and trim whitespace
        QStringList line = QString(file.readLine()).split(delim);
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
