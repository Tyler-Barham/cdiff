#ifndef TABLEROW_H
#define TABLEROW_H

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QList>

template <typename WidgetType>
class TableRow : public QWidget
{
public:
    TableRow(int _cellWidth, int _rowHeight, QWidget *parent=nullptr)
        : QWidget(parent)
    {
        cellWidth = _cellWidth;
        rowHeight = _rowHeight;

        defaultStyle = QString("border-width: 1px;"
                                "border-style: solid;"
                                "border-color: black;");

        // Setup layout
        rowLayout = new QHBoxLayout(this);
        rowLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        rowLayout->setContentsMargins(QMargins());
        rowLayout->setSpacing(0);
        setLayout(rowLayout);
    }

    ~TableRow()
    {
        for (int i = 0; i < columns.size(); ++i)
            delete columns[i];
        columns.clear();

        delete rowLayout;
    }

    virtual void updateRowData(QStringList data)
    {
        // convert data to map of [col]=isDiff??
        //QString currStyle = defaultStyle;
        //currStyle.append("background-color : rgba(0,0,255,75);");

        // Ensure at least one element
        if (data.isEmpty())
            data.append("");

        int maxCols = std::max(data.size(), columns.size());

        for (int col = 0; col < maxCols; ++col)
        {
            WidgetType *widget = Q_NULLPTR;

            // Get the label to be updated
            if (col < columns.size())
            {
                widget = columns[col];
            }
            // Create a new label
            else
            {
                widget = new WidgetType(this);
                widget->setFixedSize(cellWidth, rowHeight);
                widget->setStyleSheet(defaultStyle);

                columns.append(widget);
                rowLayout->addWidget(widget);
            }

            // Update
            widget->setText(data.value(col));
        }
    }

protected:
    QHBoxLayout *rowLayout;
    QList<WidgetType*> columns;

    QString defaultStyle;

    int cellWidth;
    int rowHeight;
};

class TableRowOfLabels : public TableRow<QLabel>
{
public:
    TableRowOfLabels(int _cellWidth, int _rowHeight, QWidget *parent=nullptr)
        : TableRow(_cellWidth, _rowHeight, parent)
    {}
};

class TableRowOfCheckboxs : public TableRow<QCheckBox>
{
    Q_OBJECT

public:
    TableRowOfCheckboxs(int _cellWidth, int _rowHeight, QWidget *parent=nullptr)
        : TableRow(_cellWidth, _rowHeight, parent)
    {}

    virtual void updateRowData(QStringList data) override;

    void setCheckState(int state, int idx);
    QList<int> getCheckedStates();

public slots:
    void onCheckboxStateChanged(int state);

signals:
    void stateChanged(int state, int idx);
};

#endif // TABLEROW_H
