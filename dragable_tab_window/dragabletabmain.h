#ifndef DRAGABLETABMAIN_H
#define DRAGABLETABMAIN_H

#include "dragabletabwindow.h"

class DragableTabMain : public DragableTabWindow
{
    Q_OBJECT
public:
    DragableTabMain(QWidget* parent = nullptr);

private slots:
    void slotTabWindowCreated(DragableTabWindow* window);

private:
    QList<DragableTabWindow*> tab_windows; // 所有生成的子窗口（不包括自己的）
};

#endif // DRAGABLETABMAIN_H
