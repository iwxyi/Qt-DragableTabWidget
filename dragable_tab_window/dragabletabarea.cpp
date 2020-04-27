#include "dragabletabarea.h"

DragableTabArea::DragableTabArea(QWidget *parent) : QScrollArea(parent)
{
    connect(this, SIGNAL(signalTabGroupCreated(DragableTabGroup*)), this, SLOT(slotTabGroupCreated(DragableTabGroup*)));

    initView();
}

void DragableTabArea::initView()
{
    setAcceptDrops(true);

    main_layout = new QHBoxLayout(this);
    main_layout->setMargin(0);
    main_layout->setSpacing(0);

    // 创建一个默认 TabGroup
    createTabGroup();
}

/**
 * 递归获取标签组所在的layout
 * （不一定在这个标签组里面）
 */
QBoxLayout *DragableTabArea::getGroupLayout(QBoxLayout *layout, DragableTabGroup *group)
{
    if (layout->indexOf(group) > -1) // 就在这个layout里面
        return layout;
    for (int i = 0; i < layout->count(); i++)
    {
        auto it = layout->itemAt(i);
        auto lay = qobject_cast<QBoxLayout*>(it->layout());
        if (lay) // 这个 LayoutItem 是 layout
        {
            auto l = getGroupLayout(lay, group); // 递归
            if (l)
                return l;
        }
    }
    return nullptr;
}

/**
 * 递归获取标签页所在的layout路径
 * 尝试的时候如栈，若没有成功则出栈
 */
QBoxLayout* DragableTabArea::getGroupLayoutPath(QList<QBoxLayout *> &path, QBoxLayout *layout, DragableTabGroup *group)
{
    if (layout->indexOf(group) > -1)
    {
        path.push_back(layout);
        return layout;
    }
    for (int i = 0; i < layout->count(); i++)
    {
        auto it = layout->itemAt(i);
        auto lay = qobject_cast<QBoxLayout*>(it->layout());
        if (lay) // 这个 LayoutItem 是 layout
        {
            path.push_back(lay);
            auto l = getGroupLayoutPath(path, lay, group);
            if (l) // 找到，返回所有路径
                return l;
            else // 没找到，pop
                path.pop_back();
        }
    }
    return nullptr;
}

/**
 * 创建标签组
 * @param widget 初始化的标签页。如果!=nullptr，则设置为tab1
 * @return 标签组指针
 */
DragableTabGroup *DragableTabArea::createTabGroup(QWidget *widget, QString label)
{
    DragableTabGroup* group = new DragableTabGroup(this);
    if (widget)
        group->addTab(widget, label);
    main_layout->addWidget(group);
    emit signalTabGroupCreated(group);
    return group;
}

/**
 * 创建标签组
 * @param base 基准标签组
 * @param vertical 横向排列（左右，默认）还是竖向（上下）
 * @return 标签组指针
 */
DragableTabGroup *DragableTabArea::splitGroupLayout(DragableTabGroup *base, QBoxLayout::Direction direction)
{
    if (base == nullptr) // 不基于某一标签组，创建全窗口标签组
    {
        if (count() == 0) // 没有标签组，创建全窗口标签组
        {
            return createTabGroup();
        }
        else // 已经有标签组，新窗口
        {
            return createTabWindow();
        }
    }

    // 分割标签组
    if (current_group == nullptr)
        return nullptr;
    DragableTabGroup* group = new DragableTabGroup(this);
    QBoxLayout* layout = getGroupLayout(current_group); // 获取当前group所在的layout
    if (layout == nullptr) // 如果是单独一个窗口，那么就是nullptr
        return nullptr;
    int index_in_layout = layout->indexOf(base); // 保存当前的索引
    auto layout_direction = layout->direction();
    if (layout_direction == direction)
    {
        layout->insertWidget(index_in_layout+1, group);
    }
    else
    {
        base->parentWidget()->layout()->removeWidget(base);
        QBoxLayout* sub_layout = new QBoxLayout(direction);
        sub_layout->addWidget(base);
        sub_layout->addWidget(group);
        if (index_in_layout == -1)
            index_in_layout = main_layout->count();
        layout->insertLayout(index_in_layout, sub_layout);

        // 连接信号槽：子Layout删除的时候，递归删除父Layout（直到根Layout）
        if (layout != main_layout)
        {
            connect(sub_layout, &QBoxLayout::destroyed, layout, [=](QObject*){
                if (layout->count() == 0)
                    layout->deleteLater();
            });
        }
    }
    emit signalTabGroupCreated(group);
    return group;
}

/**
 * 分割group的某一tab出来至新的layout
 */
DragableTabGroup *DragableTabArea::splitGroupTab(DragableTabGroup *group, int index, QBoxLayout::Direction direction, bool copy)
{
    auto new_group = splitGroupLayout(group, direction);
    if (new_group == nullptr) // tab单独一个窗口，无法分割，便会是 nullptr
        return nullptr;
    if (index > -1)
    {
        // new_group->addTab 会自动引发 old_group->removeTab
        new_group->addTab(group->currentWidget(), group->tabText(group->currentIndex()));
    }
    if (!copy && group->currentIndex() > -1)
    {
        // 其他add时，自己会自动remove
        // group->removeTab(group->currentIndex());
    }
    return new_group;
}

/**
 * 在新窗口创建标签组
 * @param widget 如果不为空，则设置为第一个tab
 * @return 标签组（新窗口）指针
 */
DragableTabGroup *DragableTabArea::createTabWindow(QWidget *widget, QString label)
{
    DragableTabGroup* group = new DragableTabGroup(nullptr);
    group->resize(this->size());
    if (widget)
        group->addTab(widget, label);
    emit signalTabGroupCreated(group);
    return group;
}

/**
 * 标签组的某一标签移动到新窗口
 */
DragableTabGroup *DragableTabArea::createTabWindow(DragableTabGroup *group, int index)
{
    if (group == nullptr)
        return nullptr;
    if (index == -1)
        index = group->currentIndex();
    if (index == -1)
        return nullptr;

    DragableTabGroup* window = new DragableTabGroup(nullptr);
    window->resize(this->size());
    window->move(this->mapToGlobal(pos()));
    window->show();

    QString label = group->tabText(index);
    QWidget* widget = group->widget(index);
    group->removeTab(index);
    window->addTab(widget, label);
    emit group->signalNewTabWindowCreated(window);
    group->deleteIfEmpty();
    return window;

}

/**
 * 获取所有标签组的数量
 */
int DragableTabArea::count()
{
    return tab_groups.count();
}

/**
 * 获取在主窗口中的标签组个数
 */
int DragableTabArea::countInMain()
{
    int c = 0;
    foreach (auto group, tab_groups)
    {
        if (group->parentWidget() != nullptr)
            c++;
    }
    return c;
}

/**
 * 在当前标签组添加一个Tab
 */
void DragableTabArea::addTab(QWidget *widget, QString label)
{
    if (count() == 0)
        createTabGroup();
    if (current_group == nullptr)
        current_group = tab_groups.first();
    current_group->addTab(widget, label);
}

bool DragableTabArea::removeTab(QWidget *widget)
{
    foreach (auto group, tab_groups)
    {
        if (group->hasTab(widget))
        {
            group->removeTab(group->indexOf(widget));
            return true;
        }
    }
    return false;
}

/**
 * 是否有某个控件
 */
bool DragableTabArea::hasTab(QWidget *widget)
{
    foreach (auto group, tab_groups)
    {
        if (group->hasTab(widget))
            return true;
    }
    return false;
}

/**
 * 合并标签组
 * @return 合并后的group
 */
DragableTabGroup* DragableTabArea::mergeGroup(DragableTabGroup *group)
{
    /*auto layout = getGroupLayout(group);
    if (layout == nullptr) // 窗口
        return nullptr;
    else if (layout->count() <= 1) // 只有一个元素
        return nullptr;*/
    QList<QBoxLayout*>path = getGroupLayoutPath(group);
    if (path.size() == 0) // 无处合并
        return nullptr;
    DragableTabGroup* target = nullptr;
    QBoxLayout* prev_layout = nullptr;
    while (path.size())
    {
        auto layout = path.takeLast();
        // 判断layout的数量，2个才能合并
        if (layout->count() > 1)
        {
            int index = (prev_layout == nullptr
                         ? layout->indexOf(group)
                         : layout->indexOf(prev_layout));
            if (index > -1)
            {
                // 首先尝试合并group
                for (int i = index-1; i >= 0; i--)
                {
                    auto it = layout->itemAt(i);
                    auto gro = qobject_cast<DragableTabGroup*>(it->widget());
                    if (gro)
                        return mergeGroup(group, gro);
                }
                for (int i = index+1; i < layout->count(); i++)
                {
                    auto it = layout->itemAt(i);
                    auto gro = qobject_cast<DragableTabGroup*>(it->layout());
                    if (gro)
                        return mergeGroup(group, gro);
                }
                // 没有可以合并的group，尝试合并至同级或子级layout的group
                for (int i = index-1; i >= 0; i--)
                {
                    auto it = layout->itemAt(i);
                    auto lay = qobject_cast<QBoxLayout*>(it->layout());
                    if (lay)
                    {
                        auto gro = qobject_cast<DragableTabGroup*>(lay->itemAt(0)->widget());
                        if (gro)
                            return mergeGroup(group, gro);
                    }
                }
                for (int i = index+1; i < layout->count(); i++)
                {
                    auto it = layout->itemAt(i);
                    auto lay = qobject_cast<QBoxLayout*>(it->layout());
                    if (lay)
                    {
                        auto gro = qobject_cast<DragableTabGroup*>(lay->itemAt(0)->widget());
                        if (gro)
                            return mergeGroup(group, gro);
                    }
                }
            }
        }
        prev_layout = layout; // 保存上次的位置，用来确定路径
    }
    if (target)
        return mergeGroup(group, target);
    else
        return nullptr;
}

/**
 * 合并标签组：dead => live
 */
DragableTabGroup *DragableTabArea::mergeGroup(DragableTabGroup *dead, DragableTabGroup *live)
{
    for (int i = 0; i < dead->count(); i ++)
    {
        live->addTab(dead->widget(i), dead->tabText(i));
    }
    dead->deleteIfEmpty();
    return live;
}

/**
 * 删除标签组
 */
void DragableTabArea::closeGroup(DragableTabGroup *group)
{
    group->deleteLater();
}

/**
 * 聚焦某一个标签组
 */
DragableTabGroup *DragableTabArea::focusGroup(DragableTabGroup *group)
{
    if (group == nullptr)
        group = currentGroup();
    current_group = group;
    if (group->count())
        group->currentWidget()->setFocus();
    else
        group->setFocus();
    return group;
}

/**
 * 聚焦某一个控件并聚焦、置顶
 */
DragableTabGroup *DragableTabArea::focusGroupTab(QWidget *widget)
{
    foreach (auto group, tab_groups)
    {
        if (group->hasTab(widget))
        {
            group->setCurrentWidget(widget);
            group->raise();
            return group;
        }
    }
    return nullptr;
}

/**
 * 获取当前焦点所在的标签组
 * 如果没有，则获取最后一个
 * TODO: 判断这个焦点算不算子控件，不算的话得自己写个递归出来
 */
DragableTabGroup *DragableTabArea::currentGroup()
{
    if (count() == 0)
        return nullptr;
    else if (count() == 1)
        return tab_groups.at(0);
    foreach (auto group, tab_groups)
    {
        if (group->isFocusing())
            return group;
    }
    return current_group;
}

/**
 * 获取某一标签组所在的layout
 */
QBoxLayout *DragableTabArea::getGroupLayout(DragableTabGroup *group)
{
    return getGroupLayout(main_layout, group);
}

/**
 * 获取某一标签组所在的layout路径
 * @return 整个路径所在的位置，若是单独一个窗口则没有layout
 */
QList<QBoxLayout *> DragableTabArea::getGroupLayoutPath(DragableTabGroup *group)
{
    QList<QBoxLayout *> path;
    getGroupLayoutPath(path, main_layout, group);
    return path;
}

void DragableTabArea::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mime = event->mimeData();
    if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // Tab拖拽
    {
        event->accept();
    }

    return QScrollArea::dragEnterEvent(event);
}

void DragableTabArea::dropEvent(QDropEvent *event)
{
    const QMimeData* mime = event->mimeData();
    if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // 整行拖拽
    {
        event->accept();
        if (countInMain() == 0) // 没有标签组，拖动标签至当前页面
        {
            auto group = createTabGroup();
            group->mergeDroppedLabel(event);
        }
    }
    else
    {
        return QScrollArea::dropEvent(event);
    }
}

/**
 * 创建新窗口事件
 * 所有子窗口的parent都是Area
 */
void DragableTabArea::slotTabGroupCreated(DragableTabGroup *group)
{
    tab_groups.append(group);

    // 标签组创建的子标签组事件统一调换到此控件来
    connect(group, SIGNAL(signalNewTabWindowCreated(DragableTabGroup*)), this, SLOT(slotTabGroupCreated(DragableTabGroup*)));

    connect(group, &DragableTabGroup::destroyed, this, [=](QObject*) {
        tab_groups.removeOne(group);
        if (group == current_group)
            current_group = nullptr;

        // 如果它所在的layout是空的，移除
        QBoxLayout* layout = getGroupLayout(group);
        if (layout != nullptr && layout != main_layout && layout->count() == 0)
            layout->deleteLater();
    });

    connect(group, &DragableTabGroup::signalWidgetFocused, this, [=](QWidget*){
        current_group = group;
    });

    connect(group, &DragableTabGroup::signalSplitCurrentTab, this, [=](QBoxLayout::Direction direction, bool copy) {
        if (group->count() == 1) // 只有一个，分什么分
            return ;
        int index = group->currentIndex();
        auto new_group = splitGroupTab(group, index, direction, copy);

        // 由于这是手动操作，先获取焦点吧
        if (new_group)
        {
            focusGroup(new_group);
        }
    });
}

/**
 * 分割标签组事件
 * 创建新的标签组
 */
void DragableTabArea::slotTabGroupSplited(DragableTabGroup *base, QBoxLayout::Direction direction)
{
    splitGroupLayout(base, direction);
}
