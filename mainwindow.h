#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QFileSystemModel>
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

private:
    void enableInputUI(bool val);
    QStringList getAllIP();
    void updateIPList();
    QString getPathPrefix();
    QString getUrlPrefix();
    void loadSettings();
    void saveSettings();

private:
    Ui::MainWindow *ui;
    HttpServer *server;
    QFileSystemModel *fsModel;
};

#endif // MAINWINDOW_H
