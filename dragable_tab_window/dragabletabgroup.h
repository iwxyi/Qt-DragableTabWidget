#ifndef DRAGABLETABGROUP_H
#define DRAGABLETABGROUP_H

#include <QObject>
#include <QTabWidget>
#include <QMimeData>
#include <QDrag>
#include <QScreen>
#include <QApplication>
#include <QHBoxLayout>
#include <QTimer>
#include "dragabletabbar.h"

#define DRAGABLE_TAB_WINDOW_MIME_KEY "DRAGABLE_TAB_WINDOW_MIME_KEY"
#define DRAGABLE_TAB_WIDGET_MIME_KEY "DRAGABLE_TAB_WIGET_MIME_KEY"
#define DRAGABLE_TAB_LABEL_MIME_KEY "DRAGABLE_TAB_LABEL_MIME_KEY"

class DragableTabGroup : public QTabWidget
{
    Q_OBJECT
public:
    DragableTabGroup(QWidget* parent = nullptr);

    void split(QBoxLayout::Direction direction, bool copy = true);

    bool isFocusing();
    bool hasTab(QWidget* widget);
    void deleteIfEmpty();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

public slots:
    void slotStartDrag(int index);
    DragableTabGroup *createDraggedNewWindow();
    bool mergeDroppedLabel(QDropEvent* event);

signals:
    void signalNewTabWindowCreated(DragableTabGroup* window);
    void signalWidgetFocused(QWidget* widget);
    void signalSplitCurrentTab(QBoxLayout::Direction direction, bool copy);

protected:
    DragableTabBar* tab_bar;
    int dragging_index;
    QWidget* dragging_widget;
    QPoint dragging_point_delta; // 拖拽的 鼠标-子窗口左上角

    bool _is_main; // 是不是主窗口
    static bool _drag_merged;
};

#endif // DRAGABLETABGROUP_H
