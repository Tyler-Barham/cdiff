#include "csvcomparison.h"

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

    emit displayHeaders(csvDataA->getHeaders(), csvDataB->getHeaders());
    emit displayCsv(csvDataA->getCsv(), csvDataB->getCsv());
}

void CsvComparison::updateDiff(double threshold, QList<int> headerIdxs)
{
    diffPoints.clear();

    // TODO: Handle different number of rows/cols between files
    int minRows = std::min(csvDataA->getNumRows(), csvDataB->getNumRows());
    int maxRows = std::max(csvDataA->getNumRows(), csvDataB->getNumRows());

    for (int row = 0; row < minRows; ++row)
    {
        int minCols = std::min(csvDataA->getNumCols(row), csvDataB->getNumCols(row));
        int maxCols = std::max(csvDataA->getNumCols(row), csvDataB->getNumCols(row));

        for (int col = 0; col < minCols; ++col)
        {
            QString valA = csvDataA->getVal(row, col);
            QString valB = csvDataB->getVal(row, col);

            // str comparison passed
            if (valA == valB)
                continue;

            // Track if the diff is less than the threshold
            bool isAcceptable = false;

            if (headerIdxs.contains(col))
            {
                bool isNumericA = false;
                bool isNumericB = false;
                double numA = valA.toDouble(&isNumericA);
                double numB = valB.toDouble(&isNumericB);
                if (isNumericA && isNumericB && (std::abs(numA-numB) <= threshold))
                {
                    isAcceptable = true;
                }
            }

            if (!isAcceptable)
            {
                diffPoints.append(QPoint(col, row));
            }
        }
    }
    emit displayDiff(diffPoints);
}
