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
#include <QRegExpValidator>
#include <QSettings>
#include <QFileIconProvider>
#include <QCloseEvent>
#include <QMenu>

#include "qrdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRegExp re("^[^/\\\\]*$");
    QRegExpValidator *prefixValidator = new QRegExpValidator(re, this);

    ui->lineEditPathPrefix->setValidator(prefixValidator);

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
    connect(ui->groupBoxPathMapping, SIGNAL(toggled(bool)), this, SLOT(updatePathPrefixPreview()));

    this->server = new HttpServer(this);
    enableInputUI(true);
    updateIPList();
    loadSettings();

    createTrayIcon();
}

MainWindow::~MainWindow()
{
    delete this->server;
    delete fsModel;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    int btn = QMessageBox::question(this, "关闭提示", "你要彻底关闭程序还是隐藏到系统托盘？", "关闭", "隐藏", "取消");

    if(btn == 0)
    {
        e->accept();
    }
    else if(btn == 1)
    {
        e->ignore();
        this->hide();
    }
    else
    {
        e->ignore();
    }
}

void MainWindow::createTrayIcon()
{
    QAction *showAction = new QAction("显示主窗口", this);
    QAction *quitAction = new QAction("退出程序", this);

    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(quitAction);

    connect(showAction, SIGNAL(triggered(bool)), this, SLOT(showNormal()));
    connect(quitAction, SIGNAL(triggered(bool)), qApp, SLOT(quit()));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QFileIconProvider().icon(QFileIconProvider::Network));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayIconClicked(QSystemTrayIcon::ActivationReason)));

    trayIcon->show();
}

void MainWindow::systemTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        this->showNormal();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("Ray Fung", "Skipper");

    ui->lineEditPath->setText(settings.value("root_path", QString()).toString());
    ui->comboBoxIP->setCurrentText(settings.value("ip", QString()).toString());
    ui->spinBoxPort->setValue(settings.value("port", 80).toInt());
    ui->groupBoxPathMapping->setChecked(settings.value("enable_mapping", false).toBool());
    ui->lineEditPathPrefix->setText(settings.value("path_prefix", QString()).toString());
}

void MainWindow::saveSettings()
{
    QSettings settings("Ray Fung", "Skipper");

    settings.setValue("root_path", ui->lineEditPath->text());
    settings.setValue("ip", ui->comboBoxIP->currentText());
    settings.setValue("port", ui->spinBoxPort->value());
    settings.setValue("enable_mapping", ui->groupBoxPathMapping->isChecked());
    settings.setValue("path_prefix", ui->lineEditPathPrefix->text());
}

void MainWindow::enableInputUI(bool val)
{
    ui->widgetTop->setEnabled(val);
    ui->pushButtonStartServer->setEnabled(val);
    ui->pushButtonStopServer->setEnabled(!val);
    ui->pushButtonCopyURL->setEnabled(!val);
    ui->pushButtonCopyQRCode->setEnabled(!val);
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
                QAbstractSocket::NetworkLayerProtocol protocol = addr.protocol();

                if (protocol == QAbstractSocket::IPv4Protocol || protocol == QAbstractSocket::IPv6Protocol)
                {
                    if (addr.isLinkLocal() == false)
                    {
                        ipList.append(addr.toString());
                    }
                }
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
    quint16 port = static_cast<quint16>(ui->spinBoxPort->value());

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

    QString prefix = getPathPrefix();
    bool ok = this->server->start(ipAddress, port, new HttpFileResourceFactory(rootPath, prefix));

    if(ok)
    {
        saveSettings();
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

QString MainWindow::getPathPrefix()
{
    return ui->groupBoxPathMapping->isChecked() ? ui->lineEditPathPrefix->text() : QString();
}

QString MainWindow::getUrlPrefix()
{
    QString url = "http://";
    QString ip = ui->comboBoxIP->currentText();
    bool isIPv6 = ip.contains(':');

    if (isIPv6)
    {
        url += QString("[%1]").arg(ip);
    }
    else
    {
        url += ip;
    }

    if(ui->spinBoxPort->value() != 80)
        url += ":" + QString::number(ui->spinBoxPort->value());
    url += "/" + QUrl::toPercentEncoding(getPathPrefix());

    return url;
}

void MainWindow::updatePathPrefixPreview()
{
    ui->labelPathPreview->setText(getUrlPrefix());
}

void MainWindow::on_pushButtonCopyURL_clicked()
{
    QString url = getSelectedURL();
    QApplication::clipboard()->setText(url);
}

void MainWindow::on_pushButtonCopyQRCode_clicked()
{
    QString url = getSelectedURL();
    QrDialog dialog;

    dialog.setUrlString(url);
    dialog.copyQrCode();
    dialog.exec();
}

QString MainWindow::getSelectedURL()
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

        return url;
    }
    else
    {
        return QString();
    }
}
