#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createTestTabs();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createTestTabs()
{
    ui->scrollArea->addTab(new QTextEdit("文本1", this), "Tab1");
    ui->scrollArea->addTab(new QTextEdit("文本2", this), "Tab2");
    ui->scrollArea->addTab(new QTextEdit("文本3", this), "Tab3");
    ui->scrollArea->addTab(new QTextEdit("文本4", this), "Tab4");
    ui->scrollArea->addTab(new QTextEdit("文本5", this), "Tab5");
    ui->scrollArea->addTab(new QTextEdit("文本6", this), "Tab6");
    ui->scrollArea->addTab(new QTextEdit("文本7", this), "Tab7");
}


void MainWindow::on_actioncloseGroup_triggered()
{
    auto group = ui->scrollArea->currentGroup();
    if (group)
        group->deleteLater();
}

void MainWindow::on_actionsplit_horizone_triggered()
{
    /*auto rst = ui->scrollArea->splitTabGroup(ui->scrollArea->currentGroup(), QBoxLayout::LeftToRight);
    if (rst)
        ui->scrollArea->focusGroup(rst);*/
    auto group = ui->scrollArea->currentGroup();
    if (group)
        group->split(QBoxLayout::LeftToRight);
}

void MainWindow::on_actionsplit_vertical_triggered()
{
    /*auto rst = ui->scrollArea->splitGroupLayout(ui->scrollArea->currentGroup(), QBoxLayout::TopToBottom);
    if (rst)
        ui->scrollArea->focusGroup(rst);*/
    auto group = ui->scrollArea->currentGroup();
    if (group)
        group->split(QBoxLayout::TopToBottom);
}

void MainWindow::on_actioncloseTab_triggered()
{
    auto group = ui->scrollArea->currentGroup();
    if (group && group->currentIndex() > -1)
        group->removeTab(group->currentIndex());
}

void MainWindow::on_actionduplicateTab_triggered()
{

}

void MainWindow::on_actioncurrent_group_triggered()
{
    auto group = ui->scrollArea->currentGroup();
    qDebug() << "current:" << group;
    if (group && group->currentIndex() > -1)
        qDebug() << group->tabText(group->currentIndex());
}

void MainWindow::on_actioncurrent_group_layout_triggered()
{
    auto group = ui->scrollArea->currentGroup();
    qDebug() << "current:" << group;
    if (group && group->currentIndex() > -1)
        qDebug() << "nearest layout:" << ui->scrollArea->getGroupLayout(group);
}

void MainWindow::on_actionopen_tab_in_window_triggered()
{
    auto group = ui->scrollArea->currentGroup();
    ui->scrollArea->createTabWindow(group);
}

void MainWindow::on_actioncreate_test_tabs_triggered()
{
    createTestTabs();
}

void MainWindow::on_actionmerge_group_triggered()
{
    auto group = ui->scrollArea->currentGroup();
    if (group)
    {
        auto rst = ui->scrollArea->mergeGroup(group);
        if (rst)
            ui->scrollArea->focusGroup(rst);
    }
}
