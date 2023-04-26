#ifndef TABLE_H
#define TABLE_H

#include <QGuiApplication>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>

#include "tablerow.h"

namespace
{
constexpr int tableCellWidth = 100;
constexpr int tableRowHeight = 20;
constexpr int tableHeaderHeight = 30;
constexpr int tableReservedHeight = tableRowHeight;
constexpr int tableMinimalHeight = tableHeaderHeight + (3 * tableRowHeight) + tableReservedHeight;
} // namespace

class Table : public QWidget
{
    Q_OBJECT

public:
    Table(QWidget *parent = nullptr);
    ~Table();

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    void setHeaders(QStringList headers);
    void setHeaderState(int state, int idx);
    void setData(QList<QStringList> data);

private:
    void initScrollBar();
    void initHeader();
    void initWidgetRows(int height);
    void onScrollTableUpdate(int min, int max);
    void onTableResize(int newMin, int newMax);
    void updateScrollBar(int height);

    QScrollBar *_scrollBar = nullptr;
    TableRowOfCheckboxs *_header = nullptr;
    QList<TableRowOfLabels*> _rows;
    QVBoxLayout *_innerVLayout = nullptr;
    QGridLayout *_mainGLayout = nullptr;
    bool isInitialized = false;

    QList<QStringList> _data;

signals:
    void checkboxStateChanged(int state, int idx);
};

#endif // TABLE_H
