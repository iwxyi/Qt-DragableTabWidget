#include "dragabletabbar.h"

DragableTabBar::DragableTabBar(QWidget *parent) : QTabBar(parent), dragging(false)
{
    setMovable(true);
    setMouseTracking(true);
}

void DragableTabBar::mousePressEvent(QMouseEvent *e)
{
    QTabBar::mousePressEvent(e);

    if (e->button() == Qt::LeftButton)
    {
        dragging = true;
        press_pos = e->pos();
    }
}

void DragableTabBar::mouseMoveEvent(QMouseEvent *e)
{
    QTabBar::mouseMoveEvent(e);

    // 高度超过标签栏高度 或 标签数量=1 时，开启拖拽
    if (dragging && (e->buttons() & Qt::LeftButton) && (count() == 1 || !contentsRect().contains(e->pos())))
    {
        int index = this->currentIndex();
        if (index == -1)
            return ;

        if (count() == 1 || !tabRect(index).contains(e->pos())) // 拖拽到外面来了
        {
            emit signalStartDrag(index);
        }
    }
}

void DragableTabBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (dragging && e->button() == Qt::LeftButton && !contentsRect().contains(e->pos()))
    {
        dragging = false;
        // emit signalEndDrag();
    }
    return QTabBar::mouseReleaseEvent(e);
}

void DragableTabBar::focusOutEvent(QFocusEvent *e)
{
    if (dragging)
    {
        dragging = false;
    }

    return QTabBar::focusOutEvent(e);
}

void DragableTabBar::contextMenuEvent(QContextMenuEvent *event)
{
    int index = tabAt(event->pos());
    emit signalTabMenu(event->pos(), index);
}
