#ifndef DRAGABLETABAREA_H
#define DRAGABLETABAREA_H

/**
  * 一整个编辑区域
  * 支持多个TabWidget各种模式平铺……
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

    DragableTabGroup* createTabGroup(QWidget* widget = nullptr, QString label = "");
    DragableTabGroup* splitGroupLayout(DragableTabGroup* base, QBoxLayout::Direction direction = QBoxLayout::LeftToRight);
    DragableTabGroup* splitGroupTab(DragableTabGroup* group, int index, QBoxLayout::Direction direction = QBoxLayout::LeftToRight, bool copy = false);
    DragableTabGroup* createTabWindow(QWidget* widget = nullptr, QString label = "");
    DragableTabGroup* createTabWindow(DragableTabGroup* group, int index = -1);

    int count();
    int countInMain();
    void addTab(QWidget* widget, QString label = "");
    bool removeTab(QWidget* widget);
    bool hasTab(QWidget* widget);
    DragableTabGroup *mergeGroup(DragableTabGroup* group);
    DragableTabGroup* mergeGroup(DragableTabGroup* dead, DragableTabGroup* live);
    void closeGroup(DragableTabGroup* group);
    DragableTabGroup* focusGroup(DragableTabGroup* group = nullptr);
    DragableTabGroup* focusGroupTab(QWidget* widget);
    DragableTabGroup* currentGroup();
    QBoxLayout* getGroupLayout(DragableTabGroup* group);
    QList<QBoxLayout*> getGroupLayoutPath(DragableTabGroup* group);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

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

private:
    QHBoxLayout* main_layout;
    QList<DragableTabGroup*> tab_groups; // 所有标签组
    // QList<DragableTabArea*> area_windows; // 所有的area（包括自己，自己是main）
    // bool _is_main = true; // tab_group被清空后，是否删除tab_area（如果不是main的话）
    DragableTabGroup* current_group = nullptr; // 全局当前
};

#endif // DRAGABLETABAREA_H
