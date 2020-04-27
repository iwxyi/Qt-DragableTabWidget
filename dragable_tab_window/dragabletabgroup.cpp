#include "dragabletabgroup.h"

bool DragableTabGroup::_drag_merged = false;

DragableTabGroup::DragableTabGroup(QWidget *parent)
    : QTabWidget(parent), tab_bar(new DragableTabBar(this)),
      dragging_index(0), dragging_widget(nullptr), _is_main(false)
{
    setAcceptDrops(true);
    setTabBar(tab_bar);
    setMovable(true);
//    setTabsClosable(true);
//    tab_bar->setAutoHide(true);

    connect(tab_bar, SIGNAL(signalStartDrag(int)), this, SLOT(slotStartDrag(int)));
    connect(tab_bar, SIGNAL(signalEndDrag()), this, SLOT(createDraggedNewWindow()));

    connect(qApp, &QApplication::focusChanged, this, [=](QWidget*old, QWidget* now){
        if (isFocusing())
            emit signalWidgetFocused(now);
    });
}

/**
 * 分割标签组
 * @param direction 方向
 * @param copy      是否复制（默认复制，保留旧tab）
 */
void DragableTabGroup::split(QBoxLayout::Direction direction, bool copy)
{
    emit signalSplitCurrentTab(direction, copy);
}

/**
 * 是否拥有焦点
 */
bool DragableTabGroup::isFocusing()
{
    QWidget* widget = QApplication::focusWidget();
    return widget != nullptr && hasTab(widget);
}

/**
 * 是否包含某一widget
 */
bool DragableTabGroup::hasTab(QWidget *widget)
{
    for (int i = 0; i < count(); i++)
    {
        if (this->widget(i) == widget)
            return true;
    }
    return false;
}

void DragableTabGroup::deleteIfEmpty()
{
    if (!_is_main && count() == 0)
        deleteLater();
}

void DragableTabGroup::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mime = event->mimeData();
    if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // Tab拖拽
    {
        /*QPoint pos = event->pos();
        if (pos.y() >= tab_bar->geometry().top() - qMax(tab_bar->height(), 32) && pos.y() <= tab_bar->geometry().bottom()+qMax(tab_bar->height(), 32)) // 只有 tabBar 的位置可拖拽
            event->accept();
        else
            event->ignore();*/
        event->accept();
    }

    return QTabWidget::dragEnterEvent(event);
}

void DragableTabGroup::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData* mime = event->mimeData();
    if (count() == 0)
    {
        event->accept();
    }
    else if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // 整行拖拽
    {
        QPoint pos = event->pos();
        if (pos.y() >= tab_bar->geometry().top() - qMax(tab_bar->height(), 32) && pos.y() <= tab_bar->geometry().bottom()+qMax(tab_bar->height(), 32)) // 只有 tabBar 的位置可拖拽
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        return QTabWidget::dragMoveEvent(event);
    }
}

void DragableTabGroup::dropEvent(QDropEvent *event)
{
    if (mergeDroppedLabel(event))
        event->accept();

    return QTabWidget::dropEvent(event);
}

/**
 * 开始标签拖拽
 */
void DragableTabGroup::slotStartDrag(int index)
{
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
    mime->setData(DRAGABLE_TAB_LABEL_MIME_KEY, tab_bar->tabText(index).toLocal8Bit());
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(pixmap);
    drag->setHotSpot(dragging_point_delta);
    connect(drag, &QDrag::destroyed, this, [=](QObject*){
        // 顺序：先触发 dropEvent，在 drag::destroyed

        // 判断有没有被合并到窗口
        if (_drag_merged)
        {
            return ;
        }

        // 没有合并到其他窗口，则创建一个新窗口
        createDraggedNewWindow();
    });
    drag->exec();
}

/**
 * 自己的标签拖出到新窗口
 */
DragableTabGroup* DragableTabGroup::createDraggedNewWindow()
{
    if (count() == 1) // 只有一个标签，直接移动窗口
    {
        // 会导致没有 update，第一次按下无法操作，已取消
        // move(QCursor::pos()-dragging_point_delta-QPoint(0,tab_bar->height()));
        // return ;
    }

    DragableTabGroup* window = new DragableTabGroup(nullptr/*_is_main ? this : this->parentWidget()*/);
    window->resize(this->size());
    window->move(QCursor::pos()-dragging_point_delta-QPoint(0,tab_bar->height()));
    window->show();
    QString label = tab_bar->tabText(dragging_index);
    removeTab(dragging_index);
    window->addTab(dragging_widget, label);
    emit signalNewTabWindowCreated(window);
    if (!_is_main && count() == 0) // 标签拖完了
        deleteLater();
    window->setFocus();
    QTimer::singleShot(0, window, [=]{
        dragging_widget->setFocus();
    });
    return window;
}

/**
 * 另一个窗口拖拽本窗口的tabbar，合并标签
 */
bool DragableTabGroup::mergeDroppedLabel(QDropEvent *event)
{
    _drag_merged = true;
    int insert_index = count();
    // 根据鼠标的位置判断插入的位置
    for (int i = count()-1; i >= 0; i--)
    {
        if (tab_bar->tabRect(i).center().x() + tab_bar->pos().x() >= event->pos().x())
            insert_index = i;
    }

    // 被拖拽的信息
    const QMimeData* mime = event->mimeData();
    DragableTabGroup* window = reinterpret_cast<DragableTabGroup*>(mime->data(DRAGABLE_TAB_WINDOW_MIME_KEY).toInt());
    QWidget* widget = reinterpret_cast<QWidget*>(mime->data(DRAGABLE_TAB_WIDGET_MIME_KEY).toInt());
    QString label = mime->data(DRAGABLE_TAB_LABEL_MIME_KEY);
    if (window == this) // 被拖拽的就是自己
    {
        if (insert_index == currentIndex()) // 根本就没有被拖动
            return false;
        // 交换标签顺序
        removeTab(currentIndex());
        insertTab(insert_index, widget, label);
        setCurrentIndex(insert_index);
        return false;
    }

    // 移除旧的
    window->removeTab(window->currentIndex());
    window->deleteIfEmpty(); // 标签拖完了

    // 插入新的
    if (insert_index >= count()) // 加到末尾
        addTab(widget, label);
    else
        insertTab(insert_index, widget, label);

    setCurrentIndex(insert_index);
//    this->raise(); // 如果用frameless，raise会一直生效，导致界面会被挡住……
    this->setFocus();
    QTimer::singleShot(0, this, [=]{
        widget->setFocus();
    });
    return true;
}
