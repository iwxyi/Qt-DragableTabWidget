#ifndef DRAGABLETABGROUP_H
#define DRAGABLETABGROUP_H

#include <QObject>
#include <QTabWidget>
#include <QMimeData>
#include <QDrag>
#include <QScreen>
#include <QApplication>
#include <QHBoxLayout>
#include <QTimer>
#include <QDesktopWidget>
#include <QStyle>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "dragabletabbar.h"

#define DRAGABLE_TAB_WINDOW_MIME_KEY "DRAGABLE_TAB_WINDOW_MIME_KEY"
#define DRAGABLE_TAB_WIDGET_MIME_KEY "DRAGABLE_TAB_WIGET_MIME_KEY"
#define DRAGABLE_TAB_LABEL_MIME_KEY "DRAGABLE_TAB_LABEL_MIME_KEY"
#define DRAGABLE_TAB_ICON_MIME_KEY "DRAGABLE_TAB_ICON_MIME_KEY"

#define WIN_FRAME_LEFE_OFFSET 8 // 移动时左边需要偏差的像素；可能是左边的阴影？

#define DRAG_DEB if (0) qDebug()

struct TabPageBean
{
    TabPageBean(QString label, QWidget* widget)
        : label(label), widget(widget){}
    TabPageBean(QIcon icon, QString label, QWidget* widget)
        : icon(icon), label(label), widget(widget){}
    QIcon icon;
    QString label;
    QWidget* widget;
};

class DragableTabGroup : public QTabWidget
{
    Q_OBJECT
    friend class DragableTabArea;
public:
    DragableTabGroup(QWidget* parent = nullptr);

    virtual void tabInserted(int index) override;
    void removeTab(int index);

    void split(QBoxLayout::Direction direction, bool copy = true);

    bool isFocusing();
    bool hasTab(QWidget* widget);
    void deleteTab(int index);
    void deleteAllWidget();
    void removeTabs(QList<int> indexes, bool del = false);
    void removeTabs(int start, int end, bool del = false);
    void deleteIfEmptyWindow();
    bool isInMain();
    bool isRestoreTabEnabled();
    bool canRestoreTab();

    virtual QJsonObject toJson();
    virtual QJsonObject widgetToJson(QWidget* widget);
    virtual void jsonToWidget(QJsonObject object);

public slots:
    virtual void closeLeftTabs(int index = -1);
    virtual void closeRightTabs(int index = -1);
    virtual void closeOtherTabs(int index = -1);
    virtual void closeAllTabs();
    virtual void restoreClosedTab(int index = -1);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    
    virtual void tabWidgetInserted(int index, QWidget *widget);
    virtual void tabWidgetRemoved(int index, QWidget *widget);
    virtual void tabWidgetFocused(int index, QWidget *widget);

    virtual DragableTabBar* newTabBar(QWidget* parent = nullptr);
    virtual DragableTabGroup* newTabGroup(QWidget* parent = nullptr);

    virtual void tabDraggingEvent(QWidget* widget);
    virtual void tabDragMergedEvent(QWidget* widget);
    virtual void tabDragWindowedEvent(DragableTabGroup* window, QWidget* widget);
    virtual void tabDropMergedEvent(QWidget* widget);

protected slots:
    void moveTabInNewWindow(int index);
    void slotStartDrag(int index);
    DragableTabGroup *createDraggedNewWindow();
    bool mergeDroppedLabel(QDropEvent* event);

signals:
    void signalNewTabWindowCreated(DragableTabGroup* window);
    void signalWidgetFocused(QWidget* widget);
    void signalSplitCurrentTab(QBoxLayout::Direction direction, bool copy);

    void signalTabDragged(QWidget* widget); // 开始拖拽
    void signalTabMerged(QWidget* widget); // 被拖至其他标签组
    void signalTabWindowed(DragableTabGroup* window, QWidget* widget); // 被拖出来形成新窗口
    void signalTabDropped(QWidget* widget); // 合并来自其他标签组的标签
    void signalJsonToWidget(QJsonObject object);

protected:
    DragableTabBar* tab_bar;
    int dragging_index;
    QWidget* dragging_widget;
    static QString dragging_label;
    static QIcon dragging_icon;
    QPoint dragging_point_delta; // 拖拽的 鼠标-子窗口左上角

    bool _is_main; // 是不是主窗口
    static bool _drag_merged;
    bool frameless = false; // 使用无窗口边框

    static int closed_stack_max; // 已关闭标签页的最大数量
    static QList<TabPageBean> closed_stack; // 已关闭的标签页
};

#endif // DRAGABLETABGROUP_H
