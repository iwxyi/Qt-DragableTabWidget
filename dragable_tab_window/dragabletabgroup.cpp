#include "dragabletabgroup.h"

bool DragableTabGroup::_drag_merged = false;
QString DragableTabGroup::dragging_label;
QIcon DragableTabGroup::dragging_icon;
int DragableTabGroup::closed_stack_max = 20;
QList<TabPageBean> DragableTabGroup::closed_stack;

DragableTabGroup::DragableTabGroup(QWidget *parent)
    : QTabWidget(parent), tab_bar(newTabBar(this)),
      dragging_index(0), dragging_widget(nullptr), _is_main(false)
{
    setAcceptDrops(true);
    setTabBar(tab_bar);
    setMovable(true);
    //    setTabsClosable(true);
    //    tab_bar->setAutoHide(true);

    // 无窗口，看起来更像浏览器的
    if (frameless)
    {
        setWindowFlag(Qt::FramelessWindowHint, true);
    }

    connect(tab_bar, SIGNAL(signalStartDrag(int)), this, SLOT(slotStartDrag(int)));
    connect(tab_bar, SIGNAL(signalEndDrag()), this, SLOT(createDraggedNewWindow()));

    connect(qApp, &QApplication::focusChanged, this, [=](QWidget *, QWidget *now) {
        int index = indexOf(now);
        if (index > -1)
        {
            tabWidgetFocused(index, now);
        }
    });
}

void DragableTabGroup::tabInserted(int index)
{
    auto widget = QTabWidget::widget(index);
    if (widget)
        tabWidgetInserted(index, widget);
}

/**
 * 替换自带的removeTab方法
 * 所有的移除都可以在这里监听到
 */
void DragableTabGroup::removeTab(int index)
{
    if (index < 0 || index >= count())
        return;

    if (isRestoreTabEnabled())
    {
        closed_stack.append(TabPageBean(
            tabIcon(index),
            tabText(index),
            widget(index)));

        if (closed_stack.count() > closed_stack_max)
        {
            closed_stack.takeFirst().widget->deleteLater();
        }
    }

    tabWidgetRemoved(index, this->widget(index));
    QTabWidget::removeTab(index);
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
    QWidget *widget = QApplication::focusWidget();
    return widget != nullptr && hasTab(widget);
}

/**
 * 是否包含某一widget
 */
bool DragableTabGroup::hasTab(QWidget *widget)
{
    return indexOf(widget) > -1;
}

void DragableTabGroup::deleteTab(int index)
{
    QWidget *widget = this->widget(index);
    removeTab(index);
    widget->deleteLater();
}

/**
 * 移除tab，并删除所有控件
 */
void DragableTabGroup::deleteAllWidget()
{
    while (this->count())
    {
        auto widget = this->widget(0);
        widget->deleteLater();
        removeTab(0);
    }
}

/**
 * 移除或删除多个标签页
 */
void DragableTabGroup::removeTabs(QList<int> indexes, bool del)
{
    // 排好序
    std::sort(indexes.begin(), indexes.end(), [=](int a, int b) {
        return a < b;
    });

    for (int i = indexes.size() - 1; i >= 0; i--)
    {
        if (del)
            removeTab(i);
        else
            deleteTab(i);
    }
}

/**
 * 移除或者删除指定范围内的标签页
 */
void DragableTabGroup::removeTabs(int start, int end, bool del)
{
    QList<int> indexes;
    if (start == -1)
        start = 0;
    else if (end == -1)
        end = this->count() - 1;

    for (int i = start; i <= end; i++)
    {
        indexes << i;
    }
    removeTabs(indexes, del);
}

/**
 * 判断自己的数量是不是空的
 * 如果是空窗口，则主动删除
 */
void DragableTabGroup::deleteIfEmptyWindow()
{
    if (!_is_main && count() == 0)
        deleteLater();
}

/**
 * 判断标签组是否在主窗口中
 */
bool DragableTabGroup::isInMain()
{
    return parentWidget() != nullptr;
}

bool DragableTabGroup::isRestoreTabEnabled()
{
    return closed_stack_max > 0;
}

/**
 * 能否恢复已关闭的标签页
 */
bool DragableTabGroup::canRestoreTab()
{
    return closed_stack.size() > 0;
}

QJsonObject DragableTabGroup::toJson()
{
    QJsonObject object;
    object.insert("width", this->width());
    object.insert("height", this->height());

    QJsonArray array;
    for (int i = 0; i < this->count(); i++)
    {
        QWidget *widget = this->widget(i);
        QJsonObject obj = widgetToJson(widget);
        array.append(obj);
    }

    object.insert("tabs", array);
    object.insert("current", this->currentIndex());
    return object;
}

QJsonObject DragableTabGroup::widgetToJson(QWidget *widget)
{
    Q_UNUSED(widget)
    return QJsonObject();
}

/**
 * 从JSON恢复标签组的控件
 * 有两种方式：
 * - TabGroup中继承，添加widget
 * - TabsArea中在group中添加widget
 */
void DragableTabGroup::jsonToWidget(QJsonObject object)
{
    emit signalJsonToWidget(object);
}

void DragableTabGroup::closeLeftTabs(int index)
{
    if (index == -1)
        index = currentIndex();
    removeTabs(-1, index - 1);
}

void DragableTabGroup::closeRightTabs(int index)
{
    if (index == -1)
        index = currentIndex();
    removeTabs(index + 1, -1);
}

void DragableTabGroup::closeOtherTabs(int index)
{
    if (index == -1)
        index = currentIndex();

    QList<int> indexes;
    for (int i = 0; i < this->count(); i++)
        indexes << i;
    indexes.removeOne(index);
    removeTabs(indexes);
}

void DragableTabGroup::closeAllTabs()
{
    QList<int> indexes;
    for (int i = 0; i < this->count(); i++)
        indexes << i;
    removeTabs(indexes);
    deleteIfEmptyWindow();
}

void DragableTabGroup::restoreClosedTab(int index)
{
    if (!closed_stack.size())
        return;

    TabPageBean page = closed_stack.takeLast();
    if (index == -1)
        addTab(page.widget, page.icon, page.label);
    else
        insertTab(index, page.widget, page.icon, page.label);
    setCurrentWidget(page.widget);
}

void DragableTabGroup::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // Tab拖拽
    {
        event->accept();
    }

    return QTabWidget::dragEnterEvent(event);
}

void DragableTabGroup::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (count() == 0)
    {
        event->accept();
    }
    else if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // 整行拖拽
    {
        QPoint pos = event->pos();
        if (pos.y() >= tab_bar->geometry().top() - qMax(tab_bar->height(), 32) && pos.y() <= tab_bar->geometry().bottom() + qMax(tab_bar->height(), 32)) // 只有 tabBar 的位置可拖拽
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
 * 任何Tab的添加都会触发这个方法
 * 包括拖动tab到此页面
 * 在插入之后调用
 */
void DragableTabGroup::tabWidgetInserted(int index, QWidget *widget)
{
}

/**
 * 任何tab的删除都会触发这个方法
 * 包括拖拽tab出去
 * 在删除之前调用
 */
void DragableTabGroup::tabWidgetRemoved(int index, QWidget *widget)
{
    if (this->count() <= 1) // 全部移除完了，则删除当前的TabGroup
    {
        this->deleteLater();
    }
}

/**
 * 标签控件获得焦点事件
 */
void DragableTabGroup::tabWidgetFocused(int index, QWidget *widget)
{
}

/**
 * 创建一个自定义TabBar
 * 注意：实际上，这个虚函数不会被覆盖
 * 因为是在构造函数中调用，此时不会调用子类重写的方法
 */
DragableTabBar *DragableTabGroup::newTabBar(QWidget *parent)
{
    return new DragableTabBar(parent);
}

/**
 * 创建一个标签组的新方法
 * 如果需要继承的话，则需要把所有创建的地方都覆盖掉
 */
DragableTabGroup *DragableTabGroup::newTabGroup(QWidget *parent)
{
    return new DragableTabGroup(parent);
}

/**
 * 标签被拖拽事件
 * widget所在的tab还在自己这儿
 */
void DragableTabGroup::tabDraggingEvent(QWidget *widget)
{
}

/**
 * 本标签组的标签被其他标签组合并
 * 此时的widget所在的tab已经不是自己的了
 * 如果是独立窗口的唯一标签被合并，那么合并后标签组自动删除，不会触发此方法
 * （另外……这个方法会导致崩溃，已经注释掉了，永远不会触发）
 */
void DragableTabGroup::tabDragMergedEvent(QWidget *widget)
{
}

/**
 * 本标签组的标签拖动至新的窗口
 * tab也已经不在自己这儿了
 */
void DragableTabGroup::tabDragWindowedEvent(DragableTabGroup *window, QWidget *widget)
{
}

/**
 * 标签拖到上面事件
 * 在合并到自己之后
 */
void DragableTabGroup::tabDropMergedEvent(QWidget *widget)
{
}

/**
 * 移动标签到新的窗口
 * 其实是相当于触发拖拽事件一样的
 */
void DragableTabGroup::moveTabInNewWindow(int index)
{
    dragging_index = index;
    dragging_widget = this->widget(index);
    dragging_point_delta = QCursor::pos() - (parent() == nullptr ? tabBar()->mapToGlobal(tabBar()->pos()) : mapToGlobal(QPoint(0, 0)));
    createDraggedNewWindow();
}

/**
 * 开始标签拖拽
 */
void DragableTabGroup::slotStartDrag(int index)
{
    dragging_index = index;
    dragging_widget = this->widget(index);
    dragging_point_delta = QCursor::pos() - (parent() == nullptr ? tabBar()->mapToGlobal(tabBar()->pos()) : mapToGlobal(QPoint(0, 0)));
    tabDraggingEvent(dragging_widget);
    emit signalTabDragged(dragging_widget);
    _drag_merged = false;

    QPixmap pixmap(this->size());
    pixmap.fill(Qt::transparent);
    //    dragging_widget->render(&pixmap, dragging_widget->mapToGlobal(pos()) - this->mapToGlobal(pos()));
    this->render(&pixmap, this->mapToGlobal(pos()) - this->mapToGlobal(pos()));

    QMimeData *mime = new QMimeData;
    mime->setData(DRAGABLE_TAB_WINDOW_MIME_KEY, QString::number(reinterpret_cast<int>(this)).toUtf8());
    mime->setData(DRAGABLE_TAB_WIDGET_MIME_KEY, QString::number(reinterpret_cast<int>(dragging_widget)).toUtf8());
    mime->setData(DRAGABLE_TAB_LABEL_MIME_KEY, tab_bar->tabText(index).toLocal8Bit());
    dragging_label = tab_bar->tabText(index);
    dragging_icon = tab_bar->tabIcon(index);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(pixmap);
    drag->setHotSpot(dragging_point_delta);
    bool is_one = (count() == 1 && !isInMain());
    connect(drag, &QDrag::destroyed, this, [=](QObject *) {
        DRAG_DEB << "destroyed";
        // 顺序：先触发 dropEvent，在 drag::destroyed

        // 判断有没有被合并到窗口
        if (_drag_merged)
        {
            DRAG_DEB << "drag 合并标签";
            // 判断是不是唯一标签的窗口
            // 如果单窗口只有这一个标签，那么拖动结束后自己也删除了，后面的代码并没有什么意义
            if (!is_one)
            {
                // tabDragMergedEvent(dragging_widget); // 这句话会导致莫名的崩溃
                emit signalTabMerged(dragging_widget);
            }
            return;
        }

        // 没有合并到其他窗口
        if (this->count() == 1 && !isInMain())
        {
            // 单个标签拖动，移动窗口
            int titlebar_height = style()->pixelMetric(QStyle::PM_TitleBarHeight);
            this->move(QCursor::pos() - dragging_point_delta - QPoint(WIN_FRAME_LEFE_OFFSET, titlebar_height));
            DRAG_DEB << "drag 移动窗口";
        }
        else
        {
            DRAG_DEB << "drag 创新窗口";
            // 多个标签拖出，创建新窗口
            createDraggedNewWindow();
        }
    });

    // 如果只有一个标签，则假装移动整个窗口
    if (this->count() == 1)
    {
        QTimer::singleShot(0, [=] {
            //            this->hide();
            this->move(-30000, -30000); // 隐藏起来
        });
    }
    DRAG_DEB << "----------开始drag--------";
    // exec 操作会一直阻塞后面的代码，除非使用多线程或者信号槽
    drag->exec();
}

/**
 * 自己的标签拖出到新窗口
 */
DragableTabGroup *DragableTabGroup::createDraggedNewWindow()
{
    if (count() == 1) // 只有一个标签，直接移动窗口
    {
        // 会导致没有 update，第一次按下无法操作，已取消
        // move(QCursor::pos()-dragging_point_delta-QPoint(0,tab_bar->height()));
        // return ;
    }

    int titlebar_height = style()->pixelMetric(QStyle::PM_TitleBarHeight);

    DragableTabGroup *window = newTabGroup(nullptr /*_is_main ? this : this->parentWidget()*/);
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->resize(this->size());
    window->move(QCursor::pos() - dragging_point_delta - QPoint(WIN_FRAME_LEFE_OFFSET, titlebar_height));
    window->show();
    QString label = tab_bar->tabText(dragging_index);
    QIcon icon = tab_bar->tabIcon(dragging_index);
    removeTab(dragging_index);
    if (!icon.isNull())
        window->addTab(dragging_widget, icon, label);
    else
        window->addTab(dragging_widget, label);
    emit signalNewTabWindowCreated(window);
    if (!_is_main && count() == 0) // 标签拖完了
        deleteLater();
    window->raise();
    window->setFocus();
    dragging_widget->setFocus();
    dragging_widget->setFocus();
    QTimer::singleShot(0, dragging_widget, [=] {
        // 为啥要延迟……
        window->setFocus();
    });
    tabDragWindowedEvent(window, dragging_widget);
    emit signalTabWindowed(window, dragging_widget);
    return window;
}

/**
 * 另一个窗口拖拽本窗口的tabbar，合并标签
 */
bool DragableTabGroup::mergeDroppedLabel(QDropEvent *event)
{
    int insert_index = count();
    // 根据鼠标的位置判断插入的位置
    for (int i = count() - 1; i >= 0; i--)
    {
        if (tab_bar->tabRect(i).center().x() + tab_bar->pos().x() >= event->pos().x())
            insert_index = i;
    }

    // 被拖拽的信息
    const QMimeData *mime = event->mimeData();
    DragableTabGroup *window = reinterpret_cast<DragableTabGroup *>(mime->data(DRAGABLE_TAB_WINDOW_MIME_KEY).toInt());
    QWidget *widget = reinterpret_cast<QWidget *>(mime->data(DRAGABLE_TAB_WIDGET_MIME_KEY).toInt());
    QString label = QString::fromLocal8Bit(mime->data(DRAGABLE_TAB_LABEL_MIME_KEY));
    QIcon icon = dragging_icon;
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

    // 可以拖拽合并
    _drag_merged = true;

    // 移除旧的
    window->removeTab(window->currentIndex());
    window->deleteIfEmptyWindow(); // 标签拖完了（标签移除事件中也会自己删除）

    // 插入新的
    if (icon.isNull())
    {
        if (insert_index >= count()) // 加到末尾
            addTab(widget, label);
        else
            insertTab(insert_index, widget, label);
    }
    else
    {
        if (insert_index >= count()) // 加到末尾
            addTab(widget, icon, label);
        else
            insertTab(insert_index, widget, icon, label);
    }

    setCurrentIndex(insert_index);
    //    this->raise(); // 如果用frameless，raise会一直生效，导致界面会被挡住……
    this->setFocus();
    QTimer::singleShot(0, this, [=] {
        widget->setFocus();
    });
    tabDropMergedEvent(widget);
    emit signalTabDropped(widget);
    return true;
}
