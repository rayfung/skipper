#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QFileSystemModel>
#include <QSystemTrayIcon>
#include "httpserver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updatePathPrefixPreview();

private slots:
    void on_pushButtonStartServer_clicked();

    void on_pushButtonStopServer_clicked();

    void on_pushButtonSelectPath_clicked();

    void on_pushButtonRefreshIPList_clicked();

    void on_pushButtonGeneratePathPrefix_clicked();

    void on_pushButtonCopyURL_clicked();

    void systemTrayIconClicked(QSystemTrayIcon::ActivationReason reason);

    void on_pushButtonCopyQRCode_clicked();

protected:
    void closeEvent(QCloseEvent *);

private:
    void enableInputUI(bool val);
    QStringList getAllIP();
    void updateIPList();
    QString getPathPrefix();
    QString getUrlPrefix();
    void loadSettings();
    void saveSettings();
    void createTrayIcon();
    QString getSelectedURL();

private:
    Ui::MainWindow *ui;
    HttpServer *server;
    QFileSystemModel *fsModel;
    QSystemTrayIcon *trayIcon;
};

#endif // MAINWINDOW_H
