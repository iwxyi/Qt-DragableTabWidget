#ifndef DRAGABLETABWINDOW_H
#define DRAGABLETABWINDOW_H

#include <QObject>
#include <QTabWidget>
#include "dragabletabbar.h"

class DragableTabWindow : public QTabWidget
{
    Q_OBJECT
public:
    DragableTabWindow(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

public slots:
    void startDrag(int index);
    void endDrag();

signals:


private:
    DragableTabBar* tab_bar;

};

#endif // DRAGABLETABWINDOW_H
