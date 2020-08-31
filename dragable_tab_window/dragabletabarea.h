#ifndef DRAGABLETABAREA_H
#define DRAGABLETABAREA_H

/**
  * 一整个编辑区域
  * 支持多个TabWidget各种模式平铺……
  * TabArea 包含多个 TabGroup（标签组）
  * TabGroup里有唯一的 TabBar
  */

#include <QObject>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "dragabletabgroup.h"

class DragableTabArea : public QScrollArea
{
    Q_OBJECT
public:
    DragableTabArea(QWidget* parent = nullptr);
    virtual ~DragableTabArea();

    DragableTabGroup* createMainTabGroup(QWidget* widget = nullptr, QString label = "");
    DragableTabGroup* createTabGroup(QWidget* widget = nullptr, QString label = "");
    DragableTabGroup* splitGroupLayout(DragableTabGroup* base, QBoxLayout::Direction direction = QBoxLayout::LeftToRight);
    virtual DragableTabGroup* splitGroupTab(DragableTabGroup* group, int index, QBoxLayout::Direction direction = QBoxLayout::LeftToRight, bool copy = false);
    DragableTabGroup* createTabWindow(QWidget* widget = nullptr, QString label = "");
    DragableTabGroup* createTabWindow(DragableTabGroup* group, int index = -1);

    int count();
    int countInMain();
    DragableTabGroup *addTab(QWidget* widget, QString label = "");
    bool removeTab(QWidget* widget);
    bool hasTab(QWidget* widget);
    DragableTabGroup *mergeGroup(DragableTabGroup* group);
    DragableTabGroup* mergeGroup(DragableTabGroup* dead, DragableTabGroup* live);
    void closeGroup(DragableTabGroup* group);
    DragableTabGroup* focusGroup(DragableTabGroup* group = nullptr);
    DragableTabGroup* focusGroupTab(QWidget* widget);
    QWidget* focusCurrentWidget();
    DragableTabGroup* currentGroup();
    QBoxLayout* getGroupLayout(DragableTabGroup* group);
    QList<QBoxLayout*> getGroupLayoutPath(DragableTabGroup* group);
    QList<TabPageBean> &getClosedStack();

    virtual QString toJsonString();
    virtual void fromJsonString(QString s);

public slots:
    virtual void restoreClosedTab();
    virtual void clearClosedStack();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    virtual DragableTabGroup* newTabGroup(QWidget* parent = nullptr);
    virtual QJsonObject layoutToJson(QBoxLayout *layout) const;
    virtual void jsonToLayout(QBoxLayout *layout, QJsonObject object);
    virtual void jsonToGroup(QBoxLayout *layout, QJsonObject object);
    virtual void jsonToWidget(DragableTabGroup* group, QJsonObject object);

protected:
    virtual void groupCreateEvent(DragableTabGroup* group);
    virtual QBoxLayout *removeGroupUpperEmptyLayouts(DragableTabGroup* group);

private:
    void initView();
    QBoxLayout* getGroupLayout(QBoxLayout *layout, DragableTabGroup *group);
    QBoxLayout *getGroupLayoutPath(QList<QBoxLayout*>&path, QBoxLayout *layout, DragableTabGroup *group);

signals:
    void signalTabGroupCreated(DragableTabGroup* group);

private slots:
    void slotTabGroupCreated(DragableTabGroup* group);
    void slotTabGroupSplited(DragableTabGroup* base, QBoxLayout::Direction direction = QBoxLayout::LeftToRight);

private slots:

protected:
    QHBoxLayout* main_layout; // 最外层的布局，任何情况下都在
    QList<DragableTabGroup*> tab_groups; // 所有标签组
    // QList<DragableTabArea*> area_windows; // 所有的area（包括自己，自己是main）
    // bool _is_main = true; // tab_group被清空后，是否删除tab_area（如果不是main的话）
    DragableTabGroup* current_group = nullptr; // 全局当前
};

#endif // DRAGABLETABAREA_H
