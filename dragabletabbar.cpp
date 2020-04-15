#include "dragabletabbar.h"

DragableTabBar::DragableTabBar(QWidget *parent) : QTabBar(parent), dragging(false)
{

}

void DragableTabBar::mousePressEvent(QMouseEvent *e)
{
    QTabBar::mousePressEvent(e);

    if (e->button() == Qt::LeftButton)
    {
        dragging = true;
        press_pos = e->pos();
        this->setFocus();
    }
}

void DragableTabBar::mouseMoveEvent(QMouseEvent *e)
{
    QTabBar::mouseMoveEvent(e);

    if (dragging)
    {
        int index = this->currentIndex();
        if (index == -1)
            return ;

        QPoint point = e->pos();
        // 拖拽到外面来了
        if (!tabRect(index).contains(point))
        {
            dragging = false;
            emit signalStartDrag(index);
        }
    }
}

void DragableTabBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (dragging && e->button() == Qt::LeftButton)
    {
        dragging = false;
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
