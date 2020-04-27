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

    DragableTabGroup* createTabArea(QWidget* widget = nullptr, QString label = ""); // 创建层叠窗口
    DragableTabGroup *splitTabGroup(DragableTabGroup* base, QBoxLayout::Direction direction = QBoxLayout::LeftToRight); // 基于某一标签组，分割出来
    DragableTabGroup* createTabWindow(QWidget* widget = nullptr, QString label = "");
    DragableTabGroup* createTabWindow(DragableTabGroup* group, int index = -1);

    int count();
    void addTab(QWidget* widget, QString label = "");
    bool hasTab(QWidget* widget);
    DragableTabGroup* focusGroup(DragableTabGroup* group = nullptr);
    DragableTabGroup* raiseGroupTab(QWidget* widget);
    DragableTabGroup* currentGroup();
    QBoxLayout* getGroupLayout(DragableTabGroup* group);

protected:

private:
    void initView();
    QBoxLayout* getGroupLayout(QBoxLayout *layout, DragableTabGroup *group);

signals:
    void signalTabGroupCreated(DragableTabGroup* group);

public slots:
    void slotTabGroupCreated(DragableTabGroup* group);
    void slotTabGroupSplited(DragableTabGroup* base, QBoxLayout::Direction direction = QBoxLayout::LeftToRight);

private slots:

private:
    QHBoxLayout* main_layout;
    QList<DragableTabGroup*> tab_groups; // 当前的标签组（不一定是窗口）
    DragableTabGroup* current_group = nullptr;
};

#endif // DRAGABLETABAREA_H
