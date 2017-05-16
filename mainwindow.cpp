#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpfileresource.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUrl>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    fsModel = new QFileSystemModel();

    ui->treeViewFileSystem->setModel(fsModel);

    int count = fsModel->columnCount();
    for(int i = 1; i < count; ++i)
    {
        ui->treeViewFileSystem->hideColumn(i);
    }

    connect(ui->comboBoxIP, SIGNAL(currentTextChanged(QString)), this, SLOT(updatePathPrefixPreview()));
    connect(ui->spinBoxPort, SIGNAL(valueChanged(int)), this, SLOT(updatePathPrefixPreview()));
    connect(ui->lineEditPathPrefix, SIGNAL(textChanged(QString)), this, SLOT(updatePathPrefixPreview()));

    this->server = new HttpServer(this);
    enableInputUI(true);
    updateIPList();
}

MainWindow::~MainWindow()
{
    delete this->server;
    delete fsModel;
    delete ui;
}

void MainWindow::enableInputUI(bool val)
{
    ui->widgetTop->setEnabled(val);
    ui->pushButtonStartServer->setEnabled(val);
    ui->pushButtonStopServer->setEnabled(!val);
    ui->pushButtonCopyURL->setEnabled(!val);
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
        fsModel->setRootPath(rootPath);
        ui->treeViewFileSystem->setRootIndex(fsModel->index(rootPath));
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

void MainWindow::on_pushButtonGeneratePathPrefix_clicked()
{
    QByteArray data = QString::number(QDateTime::currentMSecsSinceEpoch()).toUtf8();

    data = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    ui->lineEditPathPrefix->setText(data.toHex());
}

QString MainWindow::getUrlPrefix()
{
    QString url = "http://";

    url += ui->comboBoxIP->currentText();
    if(ui->spinBoxPort->value() != 80)
        url += ":" + QString::number(ui->spinBoxPort->value());
    url += "/" + ui->lineEditPathPrefix->text();

    return url;
}

void MainWindow::updatePathPrefixPreview()
{
    ui->labelPathPreview->setText(getUrlPrefix());
}

void MainWindow::on_pushButtonCopyURL_clicked()
{
    QModelIndex index = ui->treeViewFileSystem->currentIndex();
    QModelIndex rootIndex = fsModel->index(fsModel->rootPath());

    if(index.isValid() && ui->treeViewFileSystem->selectionModel()->isSelected(index))
    {
        QStringList list;

        while(index.isValid() && index != rootIndex)
        {
            list.append(fsModel->fileName(index));
            index = index.parent();
        }

        QString url = getUrlPrefix();

        if(url.endsWith('/'))
            url.chop(1);

        for(int i = list.length() - 1; i >= 0; i--)
        {
            url += "/" + QUrl::toPercentEncoding(list.at(i));
        }

        QApplication::clipboard()->setText(url);
    }
}
