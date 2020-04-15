#ifndef DRAGABLETABBAR_H
#define DRAGABLETABBAR_H

#include <QObject>
#include <QTabBar>
#include <QMouseEvent>

class DragableTabBar : public QTabBar
{
    Q_OBJECT
public:
    DragableTabBar(QWidget* parent = nullptr);

protected:
    void mousePressEvent (QMouseEvent *e);
    void mouseMoveEvent (QMouseEvent *e);
    void mouseReleaseEvent (QMouseEvent *e);
    void focusOutEvent(QFocusEvent* e);

private:

public slots:

signals:
    void signalStartDrag(int index);
    void signalEndDrag();

private:
    bool dragging;
    QPoint press_pos, release_pos;
};

#endif // DRAGABLETABBAR_H
