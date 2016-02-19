#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpfileresource.h"
#include <QMessageBox>
#include "tea.h"
#include "countermode.h"
#include <QtDebug>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->server = new HttpServer(this);

    TEA tea;
    tea.setKey(QCryptographicHash::hash("TEA", QCryptographicHash::Md5));

    QByteArray plainText = QByteArray("Skipper!");
    QByteArray encrypted = tea.encryptBlock(plainText);
    QByteArray decrypted = tea.decryptBlock(encrypted);

    qDebug() << plainText.size() << plainText.toHex();
    qDebug() << encrypted.size() << encrypted.toHex();
    qDebug() << decrypted.size() << decrypted.toHex();
}

MainWindow::~MainWindow()
{
    delete this->server;
    delete ui;
}

void MainWindow::on_pushButtonStartServer_clicked()
{
    bool ok = this->server->start("127.0.0.1", 80, "D:/www/", new HttpFileResourceFactory());

    if(!ok)
    {
        QMessageBox::critical(this, QString("Skipper"), QString("Failed to start server!"));
    }
}
