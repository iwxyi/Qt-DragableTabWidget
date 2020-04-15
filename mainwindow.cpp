#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->addTab(new QWidget(this), "qwert");
    ui->tabWidget->addTab(new QWidget(this), "qwert");
    ui->tabWidget->addTab(new QWidget(this), "qwert");
    ui->tabWidget->addTab(new QWidget(this), "qwert");
    ui->tabWidget->addTab(new QWidget(this), "qwert");
    ui->tabWidget->addTab(new QWidget(this), "qwert");
}

MainWindow::~MainWindow()
{
    delete ui;
}

