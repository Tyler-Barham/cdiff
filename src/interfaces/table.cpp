#include "table.h"

HeaderWidget::HeaderWidget(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout->setContentsMargins({});
    layout->setSpacing(0);

    // Blank
    _blank.reset(new QWidget(this));
    _blank->setFixedSize(tableHeaderNumberWidth, cellWidgetHeight);
    _blank->setStyleSheet("background-color: grey;");
    layout->addWidget(_blank.data());

    // Time
    _time.reset(new QLabel(this));
    _time->setText("Time");
    //    _time->setFixedHeight(buttonHeight);
    _time->setFixedSize(tableHeaderTimeWidth, cellWidgetHeight);
    _time->setStyleSheet(
        "border-width: 1px;"
        "border-style: solid;"
        "border-bottom-style: none;"
        "border-color: black;");
    layout->addWidget(_time.data());

    //     Duration
    _duration.reset(new QLabel(this));
    _duration->setText("Duration");
    //    _time->setFixedHeight(buttonHeight);
    _duration->setFixedSize(tableHeaderDurationWidth, cellWidgetHeight);
    _duration->setStyleSheet(
        "border-width: 1px;"
        "border-style: solid;"
        "border-bottom-style: none;"
        "border-color: black;");
    layout->addWidget(_duration.data());

    // State
    _state.reset(new QLabel(this));
    _state->setText("State");
    //    _time->setFixedHeight(buttonHeight);
    _state->setFixedSize(tableHeaderStateWidth, cellWidgetHeight);
    _state->setStyleSheet(
        "border-width: 1px;"
        "border-style: solid;"
        "border-bottom-style: none;"
        "border-color: black;");
    layout->addWidget(_state.data());

    setLayout(layout);
}

CellWidget::CellWidget(const QString &time, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    // Number
    _number.reset(new QLabel(this));
    _number->setText(time);
    _number->setAlignment(Qt::AlignCenter);
    _number->setFixedSize(tableHeaderNumberWidth, cellWidgetHeight);
    _number->setStyleSheet(
        "border-width: 1px;"
        "border-style: solid;"
        "border-right-style: none;"
        "border-bottom-style: none;"
        "border-color: black;");
    layout->addWidget(_number.data());

    // Time
    _time.reset(new QLabel(this));
    _time->setText(time);
    _time->setAlignment(Qt::AlignLeft);

    //    _time->setFrameShape(QFrame::StyledPanel);

    //    _time->setFixedHeight(buttonHeight);
    _time->setFixedSize(tableHeaderTimeWidth, cellWidgetHeight);
    _time->setStyleSheet(
        "border-width: 1px;"
        "border-style: solid;"
        "border-right-style: none;"
        "border-bottom-style: none;"
        "border-color: black;");
    layout->addWidget(_time.data());

    //     Duration
    _duration.reset(new QSpinBox(this));
    _duration->setMinimum(0);
    _duration->setMaximum(1000);
    _duration->setValue(100);
    _duration->setFixedSize(tableHeaderDurationWidth, cellWidgetHeight);
    layout->addWidget(_duration.data());

    // State
    _state.reset(new QPushButton(this));
    _state->setText("State");
    _state->setFixedSize(tableHeaderStateWidth, cellWidgetHeight);
    layout->addWidget(_state.data());

    setLayout(layout);
}

void CellWidget::setText(const QString &text) { _time->setText(text); }

Table::Table(size_t size, QWidget *parent) : QWidget(parent), _size(size)
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
    for (int i = 0, pos = min; pos < max - tableLayoutHeightReserve && i < _widgets.size();
         ++i, pos += tableRowHeight)
    {
        const int index = pos / tableRowHeight;
        if (index >= _size)
            break;

        _widgets[i]->setText(QString::number(index));
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

void Table::wheelEvent(QWheelEvent *event) { _scrollBar->event(event); }

void Table::initScrollBar()
{
    _scrollBar.reset(new QScrollBar(this));
    _scrollBar->setSingleStep(tableRowHeight);
    _scrollBar->setPageStep(tableRowHeight);

    _mainGLayout->addWidget(_scrollBar.data(), 0, 1);

    connect(_scrollBar.data(), &QScrollBar::valueChanged, this,
            [&](int pos) { return onScrollTableUpdate(pos, pos + this->height()); });

    updateScrollBar(this->height());
}

void Table::initHeader()
{
    _header.reset(new HeaderWidget(this));
    _innerVLayout->addWidget(_header.data());
}

void Table::initWidgetRows(int height)
{
    // Calculate max buttons number depending on screen
    const auto screens = QGuiApplication::screens();
    int _maxWidgetNumber = 0;
    for (int i = 0; i < screens.size(); ++i)
    {
        const int temp = screens[i]->geometry().height() / tableRowHeight;
        if (temp > _maxWidgetNumber)
            _maxWidgetNumber = temp;
    }

    // Round bottom border
    height -= height % tableRowHeight;
    // Add new widgets
    for (int i = 0, pos = 0; i < _maxWidgetNumber; ++i, pos += tableRowHeight)
    {
        //_widgets.push_back(std::make_unique<CellWidget>(QString::number(i), this));
        _widgets.push_back(new CellWidget(QString::number(i), this));
        //
        //        _widgets.back()->setFixedSize(cellWidgetWidth, tableRowHeight);
        //
        _widgets.back()->setMinimumHeight(1);
        _innerVLayout->addWidget(_widgets.back());
        if (pos >= height - tableLayoutHeightReserve || i >= _size)
            _widgets[i]->hide();
    }

    isInitialized = true;
}

void Table::onTableResize(int newMin, int newMax)
{
    blockSignals(true);

    // Remove widgets from layout
    for (int i = 0; i < _widgets.size(); ++i)
    {
        _widgets[i]->hide();
    }

    // Round bottom border
    newMax -= newMax % tableRowHeight;
    //    Add widgets to layout
    for (int i = 0, pos = newMin; i < _widgets.size() && pos < newMax - tableLayoutHeightReserve;
         ++i, pos += tableRowHeight)
    {
        const int index = pos / tableRowHeight;
        if (index >= _size)
            break;

        _widgets[i]->setText(QString::number(index));
        _widgets[i]->show();
    }

    blockSignals(false);
}

void Table::updateScrollBar(int height)
{
    int scrollBarMax = _size * tableRowHeight - height + tableLayoutHeightReserve;

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
