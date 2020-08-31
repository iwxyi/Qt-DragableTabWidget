#ifndef DRAGABLETABBAR_H
#define DRAGABLETABBAR_H

#include <QApplication>
#include <QObject>
#include <QTabBar>
#include <QMouseEvent>
#include <QDebug>

class DragableTabBar : public QTabBar
{
    Q_OBJECT
public:
    DragableTabBar(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:

signals:
    void signalStartDrag(int index);
    void signalEndDrag();
    void signalTabMenu(QPoint pos, int index);

public slots:

private:
    bool dragging;    // 是否正在拖拽
    QPoint press_pos, release_pos;
};

#endif // DRAGABLETABBAR_H
