#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QHash>
#include <QIODevice>

#define HTTP_INVALID (-1)
#define HTTP_READING_HEADER 0
#define HTTP_SENDING_RESPONSE 1

class HttpResource
{
public:
    explicit HttpResource();
    virtual ~HttpResource();
    virtual bool open(const QString &path)=0;
    virtual void close()=0;
    virtual qint64 size()=0;
    virtual qint64 pos()=0;
    virtual bool seek(qint64 pos)=0;
    virtual qint64 read(char *data, qint64 maxSize)=0;
    virtual QString contentType()=0;
};

class HttpResourceFactory
{
public:
    explicit HttpResourceFactory();
    virtual ~HttpResourceFactory();
    virtual HttpResource *createResource()=0;
};

typedef struct
{
    QTcpSocket *client;
    int state;
    QByteArray requestData;
    QHash<QString, QString> requestHeader;
    qint64 len;
    HttpResource *resource;
    bool keepAlive;
    char *buffer;

#ifdef DEBUG_SKIPPER
    QString logPrefix;
#endif

}HttpData;

class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = 0);
    ~HttpServer();
    bool start(const QString &bindAddress, quint16 bindPort, HttpResourceFactory *factory = NULL);
    void stop();

private:
    void _printLog(const QString &id, const QString &msg);
    void _printLog(QTcpSocket *client, const QString &msg);
    bool parseRange(const QString &range, qint64 size, qint64 *start, qint64 *len);
    QString parsePath(const QByteArray &path);
    bool parseHeader(const QByteArray &data, QHash<QString, QString> *hash);
    void removeConnection(QTcpSocket *client);

signals:

private slots:
    void newConnection();
    void deleteConnection();
    void handlePacket();
    void clientBytesWritten(qint64 len);

private:
    QTcpServer *m_server;
    QHash<QTcpSocket *, HttpData *> m_Hash;
    HttpResourceFactory *m_factory;

private:
    static const qint64 s_writeBufferLimit;
    static const int s_connectionBufferSize;
};

#endif // HTTPSERVER_H
