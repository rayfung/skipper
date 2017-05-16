#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpfileresource.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->server = new HttpServer(this);
    enableInputUI(true);
    updateIPList();
}

MainWindow::~MainWindow()
{
    delete this->server;
    delete ui;
}

void MainWindow::enableInputUI(bool val)
{
    ui->widgetTop->setEnabled(val);
    ui->pushButtonStartServer->setEnabled(val);
    ui->pushButtonStopServer->setEnabled(!val);
}

QStringList MainWindow::getAllIP()
{
    QStringList ipList;
    QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();

    for(int i = 0; i < all.length(); ++i)
    {
        QFlags<QNetworkInterface::InterfaceFlag> f = all.at(i).flags();

        if((f & QNetworkInterface::IsUp) && (f & QNetworkInterface::IsRunning))
        {
            QList<QNetworkAddressEntry> list = all.at(i).addressEntries();

            for(int j = 0; j < list.length(); ++j)
            {
                QHostAddress addr = list.at(j).ip();

                if(addr.protocol() == QAbstractSocket::IPv4Protocol)
                    ipList.append(addr.toString());
            }
        }
    }

    return ipList;
}

void MainWindow::updateIPList()
{
    ui->comboBoxIP->clear();
    ui->comboBoxIP->addItems(getAllIP());
}

void MainWindow::on_pushButtonStartServer_clicked()
{
    QString rootPath = ui->lineEditPath->text();
    QString ipAddress = ui->comboBoxIP->currentText();
    quint16 port = ui->spinBoxPort->value();

    if(!QFileInfo(rootPath).isDir())
    {
        QMessageBox::information(this, QString("提示"), QString("共享路径必须是有效的目录"));
        ui->lineEditPath->setFocus();
        ui->lineEditPath->selectAll();
        return;
    }

    if(ipAddress.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("必须选择一个有效的 IP 地址"));
        ui->comboBoxIP->setFocus();
        return;
    }

    bool ok = this->server->start(ipAddress, port, new HttpFileResourceFactory(rootPath));

    if(ok)
    {
        enableInputUI(false);
    }
    else
    {
        QMessageBox::critical(this, QString("Skipper"), QString("Failed to start server!"));
    }
}

void MainWindow::on_pushButtonStopServer_clicked()
{
    this->server->stop();
    enableInputUI(true);
}

void MainWindow::on_pushButtonSelectPath_clicked()
{
    QString curDir = ui->lineEditPath->text();
    QString selectedDir = QFileDialog::getExistingDirectory(this, QString("选择目录"), curDir);

    if(selectedDir.isEmpty())
        return;

    ui->lineEditPath->setText(selectedDir);
}

void MainWindow::on_pushButtonRefreshIPList_clicked()
{
    updateIPList();
}
