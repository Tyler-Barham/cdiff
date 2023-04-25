#ifndef TABLE_H
#define TABLE_H

#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QCheckBox>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
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

template <typename WidgetType>
class TableRowWidget : public QWidget
{
public:
    TableRowWidget(QWidget *parent = nullptr);
    ~TableRowWidget();

    void updateRowData(QStringList data);

private:
    QHBoxLayout *_rowLayout;
    QList<WidgetType*> _columns;

    QString defaultStyle;
};

class Table : public QWidget
{
public:
    Table(QWidget *parent = nullptr);
    ~Table();

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    void setHeaders(QStringList headers);
    void setData(QList<QStringList> data);

private:
    void initScrollBar();
    void initHeader();
    void initWidgetRows(int height);
    void onScrollTableUpdate(int min, int max);
    void onTableResize(int newMin, int newMax);
    void updateScrollBar(int height);

    QScrollBar *_scrollBar = nullptr;
    TableRowWidget<QCheckBox> *_header = nullptr;
    QList<TableRowWidget<QLabel>*> _rows;
    QVBoxLayout *_innerVLayout = nullptr;
    QGridLayout *_mainGLayout = nullptr;
    bool isInitialized = false;

    QList<QStringList> _data;
};

#endif // TABLE_H
