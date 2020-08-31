#include "dragabletabarea.h"

DragableTabArea::DragableTabArea(QWidget *parent) : QScrollArea(parent)
{
    connect(this, SIGNAL(signalTabGroupCreated(DragableTabGroup*)), this, SLOT(slotTabGroupCreated(DragableTabGroup*)));

    initView();
}

/**
 * 会先进行TabArea析构
 * 再通过TabGroup的信号槽监听到TabGroup的destroy事件
 */
DragableTabArea::~DragableTabArea()
{
    main_layout = nullptr;
}

void DragableTabArea::initView()
{
    setAcceptDrops(true);

    main_layout = new QHBoxLayout(this);
    main_layout->setMargin(0);
    main_layout->setSpacing(0);

    // createTabGroup(); // 创建一个默认 TabGroup
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
    return path.size() ? path.last() : nullptr;
}

/**
 * 创建标签组，并放置到主布局中
 * @param widget 初始化的标签页。如果!=nullptr，则设置为tab1
 * @return 标签组指针
 */
DragableTabGroup *DragableTabArea::createMainTabGroup(QWidget *widget, QString label)
{
    DragableTabGroup* group = createTabGroup(widget, label);
    main_layout->addWidget(group);
    return group;
}

/**
 * 创建标签组
 * @param widget 初始化的标签页。如果!=nullptr，则设置为tab1
 * @return 标签组指针
 */
DragableTabGroup *DragableTabArea::createTabGroup(QWidget *widget, QString label)
{
    DragableTabGroup* group = newTabGroup(this);
    groupCreateEvent(group);
    if (widget)
        group->addTab(widget, label);
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
            return createMainTabGroup();
        }
        else // 已经有标签组，新窗口
        {
            return createTabWindow();
        }
    }

    QBoxLayout* layout = getGroupLayout(base); // 获取当前group所在的layout
    if (layout == nullptr) // 如果是单独一个窗口，那么就是nullptr，不允许分割
    {
        qDebug() << "未找到标签组的布局，无法分割";
        return nullptr;
    }
    int index_in_layout = layout->indexOf(base); // 保存当前的索引
    if (index_in_layout == -1)
    {
        qDebug() << "无法获取标签组在布局中的索引";
        return nullptr;
    }
    DragableTabGroup* group = createTabGroup();
    auto layout_direction = layout->direction();
    if (layout_direction == direction) // 同个方向的，直接添加控件即可
    {
        layout->insertWidget(index_in_layout+1, group);
    }
    else // 不同方向，需要先添加布局，再在新的布局中添加控件
    {
        // 从先前的布局中移除
        QBoxLayout* layout = removeGroupUpperEmptyLayouts(base);
        if (!layout)
        {
            qDebug() << "无法找到layout";
            return nullptr;
        }

        // 添加到新的布局中
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
 * 虽然有个copy的选项，但这里没啥用，交给子类去写吧
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
    DragableTabGroup* group = newTabGroup(nullptr);
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

    DragableTabGroup* window = newTabGroup(nullptr);
    window->resize(this->size());
    window->move(this->mapToGlobal(pos()));
    window->show();

    QString label = group->tabText(index);
    QWidget* widget = group->widget(index);
    group->removeTab(index);
    window->addTab(widget, label);
    emit group->signalNewTabWindowCreated(window);
    group->deleteIfEmptyWindow();
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
        // 窗口类型是nullptr，area中的是this
        if (group->parentWidget() != nullptr)
            c++;
    }
    return c;
}

/**
 * 在当前标签组添加一个Tab
 */
DragableTabGroup* DragableTabArea::addTab(QWidget *widget, QString label)
{
    if (count() == 0)
        createMainTabGroup();
    if (current_group == nullptr)
        current_group = tab_groups.first();
    current_group->addTab(widget, label);
    return current_group;
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
    dead->deleteIfEmptyWindow();
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

QWidget *DragableTabArea::focusCurrentWidget()
{
    if (!current_group)
    {
        if (tab_groups.size() == 0)
            return nullptr;
        current_group = tab_groups.first();
    }
    if (current_group->currentWidget())
        current_group->currentWidget()->setFocus();
}

/**
 * 获取当前焦点所在的标签组
 * 如果没有组，则返回nullptr
 * 如果没有焦点，则获取最后一个
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

QList<TabPageBean>& DragableTabArea::getClosedStack()
{
    return DragableTabGroup::closed_stack;
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
    if (mime->hasFormat(DRAGABLE_TAB_WIDGET_MIME_KEY)) // Tab拖拽
    {
        event->accept();
        if (countInMain() == 0) // 没有标签组，拖动标签至当前页面
        {
            DRAG_DEB << "area 创建标签组";
            auto group = createMainTabGroup();
            group->move(0, 0);
            group->mergeDroppedLabel(event);
        }
        else // 拖动至当前标签组
        {
            DRAG_DEB << "area 拖动至标签组";
            bool merged = false;
            foreach (auto group, tab_groups)
            {
                if (group->geometry().contains(event->pos()))
                {
                    if (group->parentWidget() != nullptr)
                    {
                        group->mergeDroppedLabel(event);
                        merged = true;
                        break;
                    }
                }
            }
            // 如果这边结束后没有合并，那么会识别为拖拽标签
            // 要么是移动窗口，要么是标签拖拽出新窗口
            if (!merged)
            {
                DRAG_DEB << "没有合并！！！";
            }
        }
    }
    else
    {
        return QScrollArea::dropEvent(event);
    }
}

/**
 * 创建一个新的标签组
 * 如果和 DragableTagGroup::newTabGroup 不一样的话，很可能会出错
 */
DragableTabGroup *DragableTabArea::newTabGroup(QWidget *parent)
{
    return new DragableTabGroup(parent);
}

void DragableTabArea::groupCreateEvent(DragableTabGroup *group)
{
    connect(group, &DragableTabGroup::signalNewTabWindowCreated, this, [=](DragableTabGroup* group) {
        groupCreateEvent(group);
    });
    connect(group, &QObject::destroyed, this, [=](QObject*) {
        if (!main_layout) // 正在准备全部析构，不用管这些了
            return ;
        removeGroupUpperEmptyLayouts(group);
        tab_groups.removeOne(group);
        if (current_group == group)
            current_group = nullptr;
    });
    connect(group, &DragableTabGroup::signalJsonToWidget, this, [=](QJsonObject object){
        jsonToWidget(group, object);
    });
}

/**
 * 将标签组从布局中删除
 * 由于树状布局的关系，为了自适应布局，循环删除上一层的空布局
 * 即，布局中移除控件后，如果布局为空，则删除本布局
 */
QBoxLayout* DragableTabArea::removeGroupUpperEmptyLayouts(DragableTabGroup *group)
{
    QList<QBoxLayout*> layouts;
    QBoxLayout* layout = getGroupLayoutPath(layouts, main_layout, group);
    if (!layout)
        return nullptr;
    layout->removeWidget(group);
    layouts.removeLast();
    // 如果布局所在的布局已经没有其他子控件或子布局，则向上删除布局
    while (layouts.count() > 1) // 直到删除到main_layout
    {
        QBoxLayout* last = layouts.last();
        if (last->count() != 0)
            break;
        // 只剩下一个
        layouts.removeLast();
        QBoxLayout* last2 = layouts.last();
        last2->removeItem(layout);
        layout = last2;
    }
    return layout;
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

    // 会先进行TabArea析构
    // 再通过TabGroup的信号槽监听到TabGroup的destroy事件
    connect(group, &DragableTabGroup::destroyed, this, [=](QObject*) {
        if (!main_layout) // 正在准备全部析构，不用管这些了
            return ;
        tab_groups.removeOne(group);
        if (group == current_group || !tab_groups.size())
            current_group = nullptr;

        // 如果它所在的layout是空的，移除
        removeGroupUpperEmptyLayouts(group);
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


QJsonObject DragableTabArea::layoutToJson(QBoxLayout *layout) const
{
    QJsonObject object;
    QBoxLayout::Direction direction = layout->direction();
    // 包含direction的是布局，不包含的是标签组或者控件
    object.insert("direction", direction);
    QJsonArray array;
    for (int i = 0; i < layout->count(); i++)
    {
        auto it = layout->itemAt(i);
        auto lay = qobject_cast<QBoxLayout *>(it->layout());
        auto grp = qobject_cast<DragableTabGroup *>(it->widget());
        if (lay) // 是子布局
        {
            array.append(layoutToJson(lay));
        }
        else if (grp)
        {
            array.append(grp->toJson());
        }
    }
    object.insert("children", array);
    return object;
}

/**
 * 根据BoxLayout来保存布局
 * MainLayout{type:"tab_group,h_layout,v_layout", "child":[
 *   childLayout,
 *   childWidget
 * ]}
 */
QString DragableTabArea::toJsonString()
{
    // 生成布局的JSON树
    QJsonObject object;
    object.insert("layout", layoutToJson(main_layout));

    // 保存到文件
    QJsonDocument document;
    document.setObject(object);
    QByteArray ba = document.toJson(QJsonDocument::Indented);
    return QString(ba);
}

void DragableTabArea::jsonToLayout(QBoxLayout* layout, QJsonObject object)
{
    int direction = object.value("direction").toInt();
    QBoxLayout* lay = new QBoxLayout((QBoxLayout::Direction)direction);
    layout->addLayout(lay);
    QJsonArray array = object.value("children").toArray();
    for (int i = 0; i < array.count(); i++)
    {
        QJsonObject obj = array[i].toObject();
        if (obj.contains("direction")) // 还原布局
        {
            jsonToLayout(lay, obj);
        }
        else // 还原标签组
        {
            jsonToGroup(lay, obj);
        }
    }
}

void DragableTabArea::jsonToGroup(QBoxLayout* layout, QJsonObject object)
{
    DragableTabGroup* group = static_cast<DragableTabGroup*>(createMainTabGroup());
    layout->addWidget(group);
    current_group = group;

    QJsonArray array = object.value("tabs").toArray();
    for (int i = 0; i < array.size(); i++)
    {
        QJsonObject tab = array.at(i).toObject();
        jsonToWidget(group, tab);
    }

    int index = object.value("current").toInt(-1);
    if (index >= 0 && index < group->count())
    {
        group->setCurrentIndex(index);
    }
}

/**
 * 因为添加的widget可能需要录入到子类area中，所以需要的这里恢复
 */
void DragableTabArea::jsonToWidget(DragableTabGroup *group, QJsonObject object)
{
    Q_UNUSED(group)
    Q_UNUSED(object)
}

void DragableTabArea::fromJsonString(QString s)
{
    if (s.trimmed().isEmpty())
        return ;

    // 关闭main_layout中所有内容
    while (!tab_groups.empty())
    {
        DragableTabGroup* group = static_cast<DragableTabGroup*>(tab_groups.takeFirst());
        group->deleteAllWidget();
        group->deleteLater();
    }

    // 解析JSON
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(s.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << error.errorString();
        return;
    }

    QJsonObject object = document.object().value("layout").toObject();
    // 因为已经有MainLayout了，所以最外层的布局需要单独拿出来判断
    QJsonArray array = object.value("children").toArray();
    for (int i = 0; i < array.size(); i++)
    {
        QJsonObject obj = array[i].toObject();
        if (obj.contains("direction")) // 还原布局
        {
            jsonToLayout(main_layout, obj);
        }
        else // 还原标签组
        {
            jsonToGroup(main_layout, obj);
        }
    }
}

void DragableTabArea::restoreClosedTab()
{
    currentGroup()->restoreClosedTab();
}

void DragableTabArea::clearClosedStack()
{
    QList<TabPageBean>& stacks = DragableTabGroup::closed_stack;
    while (stacks.size())
    {
        QWidget* widget = stacks.takeLast().widget;
        qDebug() << "widget.deleter";
        widget->deleteLater();
    }
}
