#include "tablerow.h"

void TableRowOfCheckboxs::setCheckState(int state, int idx)
{
    Qt::CheckState checkState = static_cast<Qt::CheckState>(state);

    if(idx == -1)
    {
        for(QCheckBox *cBox : columns)
        {
            cBox->setCheckState(checkState);
        }
    }
    else if (idx >= columns.size() || idx < 0)
    {
        qDebug("setChecked() with invalid idx. Skipping...");
    }
    else
    {
        columns[idx]->setCheckState(checkState);
    }
}

void TableRowOfCheckboxs::updateRowData(QStringList data)
{
    TableRow::updateRowData(data);

    for(QCheckBox *cBox : columns)
    {
        connect(cBox, &QCheckBox::stateChanged, this, &TableRowOfCheckboxs::onCheckboxStateChanged);
    }
}

void TableRowOfCheckboxs::onCheckboxStateChanged(int state)
{
    QCheckBox *signalSenger = qobject_cast<QCheckBox*>(sender());

    for(int i = 0; i < columns.size(); ++i)
    {
        if(signalSenger == columns.at(i))
        {
            emit stateChanged(state, i);
            return;
        }
    }
}
