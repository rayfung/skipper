#ifndef QRDIALOG_H
#define QRDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class QrDialog;
}

class QSvgWidget;

class QrDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrDialog(QWidget *parent = 0);
    ~QrDialog();
    void setUrlString(const QString &url);
    void copyQrCode();

private:
    Ui::QrDialog *ui;
    QSvgWidget *m_svg;
};

#endif // QRDIALOG_H
