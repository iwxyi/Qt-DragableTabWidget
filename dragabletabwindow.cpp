#include "dragabletabwindow.h"

DragableTabWindow::DragableTabWindow(QWidget *parent)
    : QTabWidget(parent), tab_bar(new DragableTabBar(this)),
      dragging_index(0), dragging_widget(nullptr), _is_main(false)
{
    setAcceptDrops(true);
    setTabBar(tab_bar);
    setMovable(true);

    connect(tab_bar, SIGNAL(signalStartDrag(int)), this, SLOT(slotStartDrag(int)));
    connect(tab_bar, SIGNAL(signalEndDrag()), this, SLOT(slotEndDrag()));
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

void DragableTabWindow::slotStartDrag(int index)
{
    qDebug() << "start drag";
    dragging_index = index;
    dragging_widget = this->widget(index);
    dragging_point_delta = QCursor::pos() - dragging_widget->mapToGlobal(dragging_widget->pos());

    QPixmap pixmap(this->size());
    pixmap.fill(Qt::transparent);
    dragging_widget->render(&pixmap, dragging_widget->mapToGlobal(pos()) - this->mapToGlobal(pos()));

    QMimeData* mime = new QMimeData;
    mime->setData(DRAGABLE_TAB_WINDOW_MIME_KEY, QString::number(reinterpret_cast<int>(dragging_widget)).toUtf8());
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(pixmap);
    drag->setHotSpot(dragging_point_delta);
    connect(drag, &QDrag::destroyed, this, [=](QObject*){
        // 拖拽结束
        slotEndDrag();
    });
    drag->exec();
}

void DragableTabWindow::slotEndDrag()
{
    qDebug() << "slot end drag";
    DragableTabWindow* window = new DragableTabWindow(nullptr/*_is_main ? this : this->parentWidget()*/);
    window->resize(this->size());
    window->move(QCursor::pos()-dragging_point_delta-QPoint(0,tab_bar->height()));
    window->show();
    QString label = tab_bar->tabText(dragging_index);
    removeTab(dragging_index);
    window->addTab(dragging_widget, label);
    emit signalTabWindowCreated(window);
}
