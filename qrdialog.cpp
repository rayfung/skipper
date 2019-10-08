#include "qrdialog.h"
#include "ui_qrdialog.h"
#include "qrcode/QrCode.hpp"

#include <QtSvg>
#include <QImage>
#include <QPainter>

using namespace qrcodegen;

QrDialog::QrDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QrDialog)
{
    ui->setupUi(this);

    m_svg = new QSvgWidget();
    ui->horizontalLayout->addWidget(m_svg);
}

QrDialog::~QrDialog()
{
    delete ui;
}

void QrDialog::setUrlString(const QString &url)
{
    const QrCode qr = QrCode::encodeText(url.toUtf8().constData(), QrCode::Ecc::LOW);
    QString svg = QString::fromStdString(qr.toSvgString(4));

    m_svg->load(svg.toUtf8());
}

void QrDialog::copyQrCode()
{
    QImage qrImage(300, 300, QImage::Format_RGB888);
    QPainter painter(&qrImage);

    m_svg->renderer()->render(&painter);
    QApplication::clipboard()->setImage(qrImage);
}
