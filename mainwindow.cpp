#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpfileresource.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->server = new HttpServer(this);
    this->server->start("127.0.0.1", 80, "D:/www/", new HttpFileResourceFactory());
}

MainWindow::~MainWindow()
{
    delete this->server;
    delete ui;
}
