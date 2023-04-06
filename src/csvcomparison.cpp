#include "csvcomparison.h"

#include <float.h>

CsvComparison::CsvComparison()
{
    csvDataA = new CsvData();
    csvDataB = new CsvData();
}

CsvComparison::~CsvComparison()
{
    delete csvDataA;
    delete csvDataB;
}

void CsvComparison::loadCsv(QString filepathA, QString filepathB, char delimiter)
{
    csvDataA->setup(filepathA, delimiter);
    csvDataB->setup(filepathB, delimiter);

    QList<QStringList> dataA = csvDataA->getCsv();
    QList<QStringList> dataB = csvDataB->getCsv();

    emit displayHeaders(csvDataA->getHeaders(), csvDataB->getHeaders());
    emit displayCsv(dataA, dataB);

    // Store the diference
    diffValues.clear();
    int maxRows = std::max(dataA.size(), dataB.size());

    for (int row = 0; row < maxRows; ++row)
    {
        diffValues.append(QList<double>());
        QStringList rowA = dataA.value(row);
        QStringList rowB = dataB.value(row);

        int maxCols = std::max(rowA.size(), rowB.size());

        for (int col = 0; col < maxCols; ++col)
        {
            QString valA = rowA.value(col);
            QString valB = rowB.value(col);
            double diff = 0.0;

            if (valA == valB) // strings equal
                diff = 0.0;
            else if (valA == "" || valB == "") // different number of rows/columns
                diff = -1.0;
            else // different values
            {
                bool isNumericA = false;
                bool isNumericB = false;
                double numA = valA.toDouble(&isNumericA);
                double numB = valB.toDouble(&isNumericB);
                if (isNumericA && isNumericB) // Get numeric difference
                    diff = std::abs(numA-numB);
                else // different strings || one cannot be cast to double
                    diff = -1.0;
            }
            diffValues[row].append(diff);
        }
    }
}

void CsvComparison::updateDiff(double threshold, QList<int> headerIdxs)
{
    // Check which differences are ok
    diffPoints.clear();

    for (int row = 0; row < diffValues.size(); ++row)
    {
        for (int col = 0; col < diffValues.at(row).size(); ++col)
        {
            double val = diffValues.at(row).at(col);

            if (val < 0) // Strings !eq and could not do numeric comparison
                diffPoints.append(QPoint(col, row));
            else if (!headerIdxs.contains(col) && val != 0.0) // Different and column expects exact match
                diffPoints.append(QPoint(col, row));
            else if (headerIdxs.contains(col) && (val - FLT_EPSILON) > threshold) // Column allows diff vals but diff is too much
                diffPoints.append(QPoint(col, row));
            // else exact or tolerable match
        }
    }

    emit displayDiff(diffPoints);
}
