#ifndef DRAGABLETABWINDOW_H
#define DRAGABLETABWINDOW_H

#include <QObject>
#include <QTabWidget>
#include <QMimeData>
#include <QDrag>
#include <QScreen>
#include <QApplication>
#include "dragabletabbar.h"

#define DRAGABLE_TAB_WINDOW_MIME_KEY "DRAGABLE_TAB_WINDOW_MIME_KEY"

class DragableTabWindow : public QTabWidget
{
    Q_OBJECT
public:
    DragableTabWindow(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

public slots:
    void slotStartDrag(int index);
    void slotEndDrag();

signals:
    void signalTabWindowCreated(DragableTabWindow* window);


protected:
    DragableTabBar* tab_bar;
    int dragging_index;
    QWidget* dragging_widget;

    bool _is_main; // 是不是主窗口
};

#endif // DRAGABLETABWINDOW_H
