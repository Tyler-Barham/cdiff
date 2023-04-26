#include "table.h"

Table::Table(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    setMinimumHeight(tableMinimalHeight);

    mainGLayout = new QGridLayout();
    mainGLayout->setAlignment(Qt::AlignTop);

    innerVLayout = new QVBoxLayout();
    innerVLayout->setSpacing(0);
    innerVLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainGLayout->addLayout(innerVLayout, 0, 0);

    initScrollBar();
    initHeader();

    setLayout(mainGLayout);
}

Table::~Table()
{
    for (int i = 0; i < dataRows.size(); ++i)
        delete dataRows[i];
    dataRows.clear();

    delete headerRow;
    delete scrollBar;
    delete innerVLayout;
    delete mainGLayout;
}

void Table::onScrollTableUpdate(int min, int max)
{
    // Round bottom border
    max -= max % tableRowHeight;

    // Add widgets to layout
    for (int rowIdx = 0, pos = min; pos < max - tableReservedHeight && rowIdx < dataRows.size();
         ++rowIdx, pos += tableRowHeight)
    {
        const int dataIdx = pos / tableRowHeight;
        if (dataIdx >= csvData.size())
            break;

        dataRows[rowIdx]->updateRowData(csvData.value(dataIdx));
    }
}

void Table::resizeEvent(QResizeEvent *event)
{
    const int dataHeight = event->size().height() - tableHeaderHeight;

    QWidget::resizeEvent(event);
    updateScrollBar(dataHeight);

    if (dataRows.isEmpty())
        initWidgetRows(dataHeight);
    else
        onTableResize(scrollBar->value(), scrollBar->value() + dataHeight);
}

void Table::wheelEvent(QWheelEvent *event)
{
    scrollBar->event(event);
}

void Table::setHeaders(QStringList headers)
{
    headerRow->updateRowData(headers);
}

void Table::setHeaderState(int state, int idx)
{
    headerRow->setCheckState(state, idx);
}

void Table::setData(QList<QStringList> data)
{
    csvData = data;

    const int dataHeight = this->height() - tableHeaderHeight;
    updateScrollBar(dataHeight);
    onTableResize(scrollBar->value(), scrollBar->value() + dataHeight);
}

void Table::initScrollBar()
{
    scrollBar = new QScrollBar(this);
    scrollBar->setSingleStep(tableRowHeight);
    scrollBar->setPageStep(tableRowHeight);

    mainGLayout->addWidget(scrollBar, 0, 1);

    connect(scrollBar, &QScrollBar::valueChanged, this,
            [&](int pos) { return onScrollTableUpdate(pos, pos + this->height()); });

    updateScrollBar(this->height());
}

void Table::initHeader()
{
    headerRow = new TableRowOfCheckboxs(tableCellWidth, tableHeaderHeight, this);
    innerVLayout->addWidget(headerRow);
    connect(headerRow, &TableRowOfCheckboxs::stateChanged, this, &Table::checkboxStateChanged);
}

void Table::initWidgetRows(int height)
{
    // Calculate max rows depending on screen
    const auto screens = QGuiApplication::screens();
    int _maxWidgetNumber = 0;
    for (int screenIdx = 0; screenIdx < screens.size(); ++screenIdx)
    {
        const int temp = screens[screenIdx]->geometry().height() / tableRowHeight;
        if (temp > _maxWidgetNumber)
            _maxWidgetNumber = temp;
    }

    // Round bottom border
    height -= height % tableRowHeight;
    // Add new widgets
    for (int rowIdx = 0, pos = 0; rowIdx < _maxWidgetNumber; ++rowIdx, pos += tableRowHeight)
    {
        TableRowOfLabels *tRow = new TableRowOfLabels(tableCellWidth, tableRowHeight, this);
        tRow->updateRowData(QStringList());
        dataRows.append(tRow);
        dataRows.back()->setMinimumHeight(1);
        innerVLayout->addWidget(dataRows.back());
        if (pos >= height - tableReservedHeight || rowIdx >= csvData.size())
            dataRows.back()->hide();
    }
}

void Table::onTableResize(int newMin, int newMax)
{
    blockSignals(true);

    // Remove widgets from layout
    for (int rowIdx = 0; rowIdx < dataRows.size(); ++rowIdx)
    {
        dataRows[rowIdx]->hide();
    }

    // Round bottom border
    newMax -= newMax % tableRowHeight;
    //    Add widgets to layout
    for (int rowIdx = 0, pos = newMin; rowIdx < dataRows.size() && pos < newMax - tableReservedHeight;
         ++rowIdx, pos += tableRowHeight)
    {
        const int dataIdx = pos / tableRowHeight;
        if (dataIdx >= csvData.size())
            break;

        dataRows[rowIdx]->updateRowData(csvData.at(dataIdx));
        dataRows[rowIdx]->show();
    }

    blockSignals(false);
}

void Table::updateScrollBar(int dataHeight)
{
    int scrollBarMax = (csvData.size() * tableRowHeight) - dataHeight + tableReservedHeight;

    //  Hide when there are too little widgets
    if (scrollBarMax <= 0)
    {
        scrollBar->setMinimum(0);
        scrollBar->setMaximum(0);
        scrollBar->hide();
        return;
    }

    // Round up
    if (scrollBarMax % tableRowHeight != 0)
        scrollBarMax += tableRowHeight - scrollBarMax % tableRowHeight;
    scrollBar->setMaximum(scrollBarMax);
    scrollBar->show();
}
