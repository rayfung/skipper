#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void on_pushButtonEncryptFile_clicked();
    void on_pushButtonDecryptFile_clicked();

private:
    Ui::MainWindow *ui;
    HttpServer *server;
};

#endif // MAINWINDOW_H
