#include "dragabletabwindow.h"

bool DragableTabWindow::_drag_merged = false;

DragableTabWindow::DragableTabWindow(QWidget *parent)
    : QTabWidget(parent), tab_bar(new DragableTabBar(this)),
      dragging_index(0), dragging_widget(nullptr), _is_main(false)
{
    setAcceptDrops(true);
    setTabBar(tab_bar);
    setMovable(true);

    connect(tab_bar, SIGNAL(signalStartDrag(int)), this, SLOT(slotStartDrag(int)));
    connect(tab_bar, SIGNAL(signalEndDrag()), this, SLOT(slotDragToNewWindow()));
}

void DragableTabWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mime = event->mimeData();
    if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // 整行拖拽
    {
        event->accept();
    }

    return QTabWidget::dragEnterEvent(event);
}

void DragableTabWindow::dragMoveEvent(QDragMoveEvent *event)
{
    return QTabWidget::dragMoveEvent(event);
}

/**
 * 拖拽到另一种窗口，合并标签
 */
void DragableTabWindow::dropEvent(QDropEvent *event)
{
    qDebug() << "Window::dropEvent";
    const QMimeData* mime = event->mimeData();
    DragableTabWindow* window = reinterpret_cast<DragableTabWindow*>(mime->data(DRAGABLE_TAB_WINDOW_MIME_KEY).toInt());
    QWidget* widget = reinterpret_cast<QWidget*>(mime->data(DRAGABLE_TAB_WIDGET_MIME_KEY).toInt());
    QString label = mime->data(DRAGABLE_TAB_LABEL_MIME_KEY);
    window->removeTab(window->currentIndex());
    if (!window->_is_main && window->count() == 0) // 标签拖完了
        window->deleteLater();
    this->addTab(widget, label);
    _drag_merged = true;
    this->raise();
    setCurrentIndex(count()-1);

    return QTabWidget::dropEvent(event);
}

void DragableTabWindow::slotStartDrag(int index)
{
    qDebug() << "start drag";
    dragging_index = index;
    dragging_widget = this->widget(index);
    dragging_point_delta = QCursor::pos() - dragging_widget->mapToGlobal(dragging_widget->pos());
    _drag_merged = false;

    QPixmap pixmap(this->size());
    pixmap.fill(Qt::transparent);
    dragging_widget->render(&pixmap, dragging_widget->mapToGlobal(pos()) - this->mapToGlobal(pos()));

    QMimeData* mime = new QMimeData;
    mime->setData(DRAGABLE_TAB_WINDOW_MIME_KEY, QString::number(reinterpret_cast<int>(this)).toUtf8());
    mime->setData(DRAGABLE_TAB_WIDGET_MIME_KEY, QString::number(reinterpret_cast<int>(dragging_widget)).toUtf8());
    mime->setData(DRAGABLE_TAB_LABEL_MIME_KEY, tab_bar->tabText(tab_bar->currentIndex()).toLocal8Bit());
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(pixmap);
    drag->setHotSpot(dragging_point_delta);
    connect(drag, &QDrag::destroyed, this, [=](QObject*){
        // 顺序：先触发 dropEvent，在 drag::destroyed

        qDebug() << "QDrag::destroyed";
        // 判断有没有被合并到窗口
        if (_drag_merged)
        {
            return ;
        }

        // 没有合并到其他窗口，则创建一个新窗口
        slotDragToNewWindow();
    });
    drag->exec();
}

/**
 * 自己的标签拖出到新窗口
 */
void DragableTabWindow::slotDragToNewWindow()
{
    qDebug() << "slotDragToNewWindow";
    DragableTabWindow* window = new DragableTabWindow(nullptr/*_is_main ? this : this->parentWidget()*/);
    window->resize(this->size());
    window->move(QCursor::pos()-dragging_point_delta-QPoint(0,tab_bar->height()));
    window->show();
    QString label = tab_bar->tabText(dragging_index);
    removeTab(dragging_index);
    window->addTab(dragging_widget, label);
    emit signalTabWindowCreated(window);
    if (!_is_main && count() == 0) // 标签拖完了
        deleteLater();
}
