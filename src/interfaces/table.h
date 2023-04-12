#ifndef TABLE_H
#define TABLE_H

#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSpinBox>
#include <QList>

namespace
{
// CellWidget
constexpr int cellWidgetWidth = 100;
constexpr int cellWidgetHeight = 20;

// Table
constexpr int tableRowHeight = cellWidgetHeight;
constexpr int tableLayoutHeightReserve = tableRowHeight;
constexpr int tableMinimalHeight = 3 * tableRowHeight + tableLayoutHeightReserve;
constexpr int tableHeaderHeight = cellWidgetHeight;
constexpr int tableHeaderNumberWidth = 95;
constexpr int tableHeaderTimeWidth = 95;
constexpr int tableHeaderDurationWidth = 73;
constexpr int tableHeaderStateWidth = 55;
} // namespace

class HeaderWidget : public QWidget
{
public:
    HeaderWidget(QWidget *parent = nullptr);

private:
    QScopedPointer<QWidget> _blank;
    QScopedPointer<QLabel> _time;
    QScopedPointer<QLabel> _duration;
    QScopedPointer<QLabel> _state;
};

class CellWidget : public QWidget
{
public:
    CellWidget(const QString &time = "0", QWidget *parent = nullptr);

    void setText(const QString &text);

private:
    QScopedPointer<QLabel> _number;
    QScopedPointer<QLabel> _time;
    QScopedPointer<QSpinBox> _duration;
    QScopedPointer<QPushButton> _state;
};

class Table : public QWidget
{
public:
    Table(size_t size, QWidget *parent = nullptr);

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    void initScrollBar();
    void initHeader();
    void initWidgetRows(int height);
    void onScrollTableUpdate(int min, int max);
    void onTableResize(int newMin, int newMax);
    void updateScrollBar(int height);

    QScopedPointer<QScrollBar> _scrollBar;
    QScopedPointer<HeaderWidget> _header;
    QList<CellWidget*> _widgets;
    //std::vector<std::unique_ptr<CellWidget>> _widgets;
    QVBoxLayout *_innerVLayout = nullptr;
    QGridLayout *_mainGLayout = nullptr;
    bool isInitialized = false;
    size_t _size = 0;
};

#endif // TABLE_H
