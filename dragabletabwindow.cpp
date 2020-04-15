#include "dragabletabwindow.h"

DragableTabWindow::DragableTabWindow(QWidget *parent) : QTabWidget(parent)
{
    setAcceptDrops(true);

    tab_bar = new DragableTabBar(this);
    setTabBar(tab_bar);
}

void DragableTabWindow::dragEnterEvent(QDragEnterEvent *event)
{
    return QTabWidget::dragEnterEvent(event);
}

void DragableTabWindow::dragMoveEvent(QDragMoveEvent *event)
{
    return QTabWidget::dragMoveEvent(event);
}

void DragableTabWindow::dropEvent(QDropEvent *event)
{
    return QTabWidget::dropEvent(event);
}

void DragableTabWindow::startDrag(int index)
{

}

void DragableTabWindow::endDrag()
{

}
