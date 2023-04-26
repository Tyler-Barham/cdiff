#include "tablerow.h"

void TableRowOfCheckboxs::setChecked(bool isChecked)
{
    for(QCheckBox *cBox : columns)
        cBox->setChecked(isChecked);
}
