#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createTestTabs();

private slots:
    void on_actioncloseGroup_triggered();

    void on_actionsplit_horizone_triggered();

    void on_actionsplit_vertical_triggered();

    void on_actioncloseTab_triggered();

    void on_actionduplicateTab_triggered();

    void on_actioncurrent_group_triggered();

    void on_actioncurrent_group_layout_triggered();

    void on_actionopen_tab_in_window_triggered();

    void on_actioncreate_test_tabs_triggered();

    void on_actionmerge_group_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
