#include "table.h"

TableRowWidget::TableRowWidget(QWidget *parent) : QWidget(parent)
{
    // Setup layout
    _rowLayout = new QHBoxLayout(this);
    _rowLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    _rowLayout->setContentsMargins(QMargins());
    _rowLayout->setSpacing(0);
    setLayout(_rowLayout);
}

void TableRowWidget::updateRowData(QStringList data)
{
    // Ensure at least one element
    if (data.isEmpty()) data.append("");

    int maxCols = std::max(data.size(), _columns.size());

    for (int col = 0; col < maxCols; ++col)
    {
        QLabel *lbl = Q_NULLPTR;

        // Get the label to be updated
        if (col < _columns.size())
        {
            lbl = _columns[col];
        }
        // Create a new label
        else
        {
            lbl = new QLabel(this);
            lbl->setAlignment(Qt::AlignLeft);
            lbl->setFixedSize(tableHeaderTimeWidth, cellWidgetHeight);
            lbl->setStyleSheet(
                "border-width: 1px;"
                "border-style: solid;"
                "border-color: black;");

            _columns.append(lbl);
            _rowLayout->addWidget(lbl);
        }

        // Update
        lbl->setText(data.value(col));
    }
}

Table::Table(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    setMinimumHeight(tableMinimalHeight);

    _mainGLayout = new QGridLayout();
    _mainGLayout->setAlignment(Qt::AlignTop);

    _innerVLayout = new QVBoxLayout();
    _innerVLayout->setSpacing(0);
    _innerVLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _mainGLayout->addLayout(_innerVLayout, 0, 0);

    initScrollBar();
    initHeader();

    setLayout(_mainGLayout);
}

void Table::onScrollTableUpdate(int min, int max)
{
    // Round bottom border
    max -= max % tableRowHeight;

    // Add widgets to layout
    for (int rowIdx = 0, pos = min; pos < max - tableLayoutHeightReserve && rowIdx < _rows.size();
         ++rowIdx, pos += tableRowHeight)
    {
        const int dataIdx = pos / tableRowHeight;
        if (dataIdx >= _data.size())
            break;

        _rows[rowIdx]->updateRowData(_data.value(dataIdx));
    }
}

void Table::resizeEvent(QResizeEvent *event)
{
    const int widgetHeight = event->size().height() - tableHeaderHeight;

    QWidget::resizeEvent(event);
    updateScrollBar(widgetHeight);

    if (!isInitialized)
        initWidgetRows(widgetHeight);
    else
        onTableResize(_scrollBar->value(), _scrollBar->value() + widgetHeight);
}

void Table::wheelEvent(QWheelEvent *event)
{
    _scrollBar->event(event);
}

void Table::setHeaders(QStringList headers)
{
    _header->updateRowData(headers);
}

void Table::setData(QList<QStringList> data)
{
    _data = data;

    const int widgetHeight = this->height() - tableHeaderHeight;
    updateScrollBar(widgetHeight);
    onTableResize(_scrollBar->value(), _scrollBar->value() + widgetHeight);
}

void Table::initScrollBar()
{
    _scrollBar = new QScrollBar(this);
    _scrollBar->setSingleStep(tableRowHeight);
    _scrollBar->setPageStep(tableRowHeight);

    _mainGLayout->addWidget(_scrollBar, 0, 1);

    connect(_scrollBar, &QScrollBar::valueChanged, this,
            [&](int pos) { return onScrollTableUpdate(pos, pos + this->height()); });

    updateScrollBar(this->height());
}

void Table::initHeader()
{
    _header = new TableRowWidget(this);
    _innerVLayout->addWidget(_header);
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
        TableRowWidget *tRow = new TableRowWidget(this);
        tRow->updateRowData(QStringList());
        _rows.append(tRow);
        _rows.back()->setMinimumHeight(1);
        _innerVLayout->addWidget(_rows.back());
        if (pos >= height - tableLayoutHeightReserve || rowIdx >= _data.size())
            _rows.back()->hide();
    }

    isInitialized = true;
}

void Table::onTableResize(int newMin, int newMax)
{
    blockSignals(true);

    // Remove widgets from layout
    for (int rowIdx = 0; rowIdx < _rows.size(); ++rowIdx)
    {
        _rows[rowIdx]->hide();
    }

    // Round bottom border
    newMax -= newMax % tableRowHeight;
    //    Add widgets to layout
    for (int rowIdx = 0, pos = newMin; rowIdx < _rows.size() && pos < newMax - tableLayoutHeightReserve;
         ++rowIdx, pos += tableRowHeight)
    {
        const int dataIdx = pos / tableRowHeight;
        if (dataIdx >= _data.size())
            break;

        _rows[rowIdx]->updateRowData(_data.at(dataIdx));
        _rows[rowIdx]->show();
    }

    blockSignals(false);
}

void Table::updateScrollBar(int height)
{
    int scrollBarMax = _data.size() * tableRowHeight - height + tableLayoutHeightReserve;

    //  Hide when there are too little widgets
    if (scrollBarMax <= 0)
    {
        _scrollBar->setMinimum(0);
        _scrollBar->setMaximum(0);
        _scrollBar->hide();
        return;
    }

    // Round up
    if (scrollBarMax % tableRowHeight != 0)
        scrollBarMax += tableRowHeight - scrollBarMax % tableRowHeight;
    _scrollBar->setMaximum(scrollBarMax);
    _scrollBar->show();
}
