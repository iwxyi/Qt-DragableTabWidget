#include "dragabletabmain.h"

DragableTabMain::DragableTabMain(QWidget *parent) : DragableTabWindow(parent)
{
    _is_main = true;

    connect(this, SIGNAL(signalTabWindowCreated(DragableTabWindow*)), this, SLOT(slotTabWindowCreated(DragableTabWindow*)));
}

void DragableTabMain::slotTabWindowCreated(DragableTabWindow *window)
{
    tab_windows.append(window);

    connect(window, SIGNAL(signalTabWindowCreated(DragableTabWindow*)), this, SLOT(slotTabWindowCreated(DragableTabWindow*)));
}
