#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpfileresource.h"
#include "tea.h"
#include "countermode.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QtDebug>
#include <ctime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->server = new HttpServer(this);
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

void MainWindow::on_pushButtonEncryptFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("加密文件"), QString(), QString("文本文件 (*.txt)"));

    if (path.isEmpty())
        return;

    TEA tea;
    tea.setKey(QCryptographicHash::hash("ray", QCryptographicHash::Md5));

    CounterMode ctr(&tea);
    QByteArray nonce;

    qsrand(time(NULL));
    nonce.resize(8);
    for (int i = 0; i < 8; ++i)
        nonce[i] = qrand();

    ctr.setNonce(nonce);

    QFile in(path);
    QFile out(path);

    if (!in.open(QFile::ReadOnly))
    {
        QMessageBox::information(this, tr("提示"), tr("以只读方式打开文件失败！"));
        return;
    }

    if (!out.open(QFile::ReadWrite))
    {
        QMessageBox::information(this, tr("提示"), tr("以读写方式打开文件失败！"));
        return;
    }

    QByteArray data1;
    QByteArray data2;

    ctr.startTask();

    data1 = in.read(1024);

    out.write(nonce);

    while ((data2 = in.read(1024)).size() > 0)
    {
        out.write(ctr.updateTask(data1));
        data1.swap(data2);
    }

    if (data1.size() > 0)
    {
        out.write(ctr.updateTask(data1));
    }

    in.close();
    out.close();

    QMessageBox::information(this, tr("提示"), tr("已完成文件加密"));
}

void MainWindow::on_pushButtonDecryptFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("解密文件"), QString(), QString("文本文件 (*.txt)"));

    if (path.isEmpty())
        return;

    QFile in(path);

    if (!in.open(QFile::ReadOnly))
    {
        QMessageBox::information(this, tr("提示"), tr("以只读方式打开文件失败！"));
        return;
    }

    if (in.size() < 8)
    {
        QMessageBox::information(this, tr("提示"), tr("此文件不是有效的加密文件"));
        return;
    }

    QFile out(path);

    if (!out.open(QFile::ReadWrite))
    {
        QMessageBox::information(this, tr("提示"), tr("以读写方式打开文件失败！"));
        return;
    }

    TEA tea;
    tea.setKey(QCryptographicHash::hash("ray", QCryptographicHash::Md5));

    CounterMode ctr(&tea);

    ctr.setNonce(in.read(8));
    ctr.startTask();

    QByteArray data;

    while ((data = in.read(1024)).size() > 0)
    {
        out.write(ctr.updateTask(data));
    }

    out.resize(out.pos());

    in.close();
    out.close();

    QMessageBox::information(this, tr("提示"), tr("解密文件成功！"));
}
