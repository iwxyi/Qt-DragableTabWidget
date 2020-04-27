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

private slots:
    void on_actioncloseGroup_triggered();

    void on_actionsplit_horizone_triggered();

    void on_actionsplit_vertical_triggered();

    void on_actioncloseTab_triggered();

    void on_actionduplicateTab_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
