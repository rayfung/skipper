#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
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

private slots:
    void on_pushButtonStartServer_clicked();

    void on_pushButtonStopServer_clicked();

    void on_pushButtonSelectPath_clicked();

    void on_pushButtonRefreshIPList_clicked();

private:
    void enableInputUI(bool val);
    QStringList getAllIP();
    void updateIPList();

private:
    Ui::MainWindow *ui;
    HttpServer *server;
};

#endif // MAINWINDOW_H
