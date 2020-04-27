#include "dragabletabarea.h"

DragableTabArea::DragableTabArea(QWidget *parent) : QScrollArea(parent)
{
    connect(this, SIGNAL(signalTabGroupCreated(DragableTabGroup*)), this, SLOT(slotTabGroupCreated(DragableTabGroup*)));

    initView();
}

void DragableTabArea::initView()
{
    main_layout = new QHBoxLayout(this);
    main_layout->setMargin(0);
    main_layout->setSpacing(0);

    // 创建一个默认 TabGroup
    createTabArea();
}

/**
 * 创建标签组
 * @param widget 初始化的标签页。如果!=nullptr，则设置为tab1
 * @return 标签组指针
 */
DragableTabGroup *DragableTabArea::createTabArea(QWidget *widget, QString label)
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
DragableTabGroup *DragableTabArea::createTabGroup(DragableTabGroup *base, QBoxLayout::Direction direction)
{
    if (base == nullptr) // 不基于某一标签组，创建全窗口标签组
    {
        if (count() == 0) // 没有标签组，创建全窗口标签组
        {
            return createTabArea();
        }
        else // 已经有标签组，新窗口
        {
            return createTabWindow();
        }
    }

    // 分割标签组
    DragableTabGroup* group = new DragableTabGroup(this);
    QBoxLayout* layout = static_cast<QBoxLayout*>(base->parentWidget()->layout());
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
        layout->insertLayout(index_in_layout, sub_layout);
    }
    emit signalTabGroupCreated(group);
    return group;
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

int DragableTabArea::count()
{
    return tab_groups.count();
}

/**
 * 在当前标签组添加一个Tab
 */
void DragableTabArea::addTab(QWidget *widget, QString label)
{
    if (count() == 0)
        createTabArea();
    if (current_group == nullptr)
        current_group = tab_groups.first();
    current_group->addTab(widget, label);
}

/**
 * 是否有某个控件
 */
bool DragableTabArea::hasTab(QWidget *widget)
{
    foreach (auto group, tab_groups) {
        if (group->hasTab(widget))
            return true;
    }
    return false;
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
    foreach (auto group, tab_groups) {
        if (group->hasFocus())
            return group;
    }
    return current_group;
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
    });
}

/**
 * 分割标签组事件
 * 创建新的标签组
 */
void DragableTabArea::slotTabGroupSplited(DragableTabGroup *base, QBoxLayout::Direction direction)
{
    createTabGroup(base, direction);
}
