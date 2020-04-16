#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->addTab(new QTextEdit("文本1", this), "qwert");
    ui->tabWidget->addTab(new QTextEdit("文本2", this), "qwert");
    ui->tabWidget->addTab(new QTextEdit("文本3", this), "qwert");
    ui->tabWidget->addTab(new QTextEdit("文本4", this), "qwert");
    ui->tabWidget->addTab(new QTextEdit("文本5", this), "qwert");
    ui->tabWidget->addTab(new QTextEdit("文本6", this), "qwert");
    ui->tabWidget->addTab(new QTextEdit("文本7", this), "qwert");
}

MainWindow::~MainWindow()
{
    delete ui;
}

