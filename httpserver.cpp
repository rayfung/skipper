#include <QUrl>
#include <QStringList>
#include "httpserver.h"

HttpResource::HttpResource()
{
}

HttpResource::~HttpResource()
{
}

HttpResourceFactory::HttpResourceFactory()
{
}

HttpResourceFactory::~HttpResourceFactory()
{
}

#ifdef DEBUG_SKIPPER
    #define sk_log(id, msg) do{ _printLog((id), (msg)); }while(0)
#else
    #define sk_log(id, msg)
#endif


//发送缓冲区最大容量为 10 MiB
const qint64 HttpServer::s_writeBufferLimit = 10L * 1024L * 1024L;

HttpServer::HttpServer(QObject *parent) : QObject(parent)
{
    this->m_server = new QTcpServer(this);

    connect(this->m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

HttpServer::~HttpServer()
{
    stop();

    delete this->m_factory;
    this->m_factory = NULL;
}

bool HttpServer::start(const QString &bindAddress, quint16 bindPort, const QString &root, HttpResourceFactory *factory)
{
    this->m_root = root;
    this->m_factory = factory;
    return this->m_server->listen(QHostAddress(bindAddress), bindPort);
}

void HttpServer::stop()
{
    this->m_server->close();

    QList<QTcpSocket *> kl = this->m_Hash.uniqueKeys();

    for(int i = 0; i < kl.length(); ++i)
    {
        removeConnection(kl.at(i));
        kl.at(i)->deleteLater();
    }

    this->m_Hash.clear();
}

void HttpServer::newConnection()
{
    QTcpSocket *client = this->m_server->nextPendingConnection();

    HttpData *httpData = new HttpData();

    httpData->client = client;
    httpData->state = HTTP_READING_HEADER;
    httpData->requestData = QByteArray();

    if(this->m_factory)
        httpData->resource = this->m_factory->createResource();
    else
        httpData->resource = NULL;

#ifdef DEBUG_SKIPPER
    httpData->logPrefix = client->peerAddress().toString() + " : " + QString::number(client->peerPort());
    sk_log(httpData->logPrefix, "Connected");
#endif

    this->m_Hash[client] = httpData;

    connect(client, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
    connect(client, SIGNAL(readyRead()), this, SLOT(handlePacket()));
    connect(client, SIGNAL(bytesWritten(qint64)), this, SLOT(clientBytesWritten(qint64)));
}

void HttpServer::removeConnection(QTcpSocket *client)
{
    if(this->m_Hash.contains(client))
    {
        HttpData *data = this->m_Hash.take(client);

        sk_log(data->logPrefix, "bye!");

        data->client = NULL;
        data->requestData.clear();
        data->state = HTTP_INVALID;
        if(data->resource)
            delete data->resource;
        data->resource = NULL;

        delete data;
    }
}

void HttpServer::deleteConnection()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    removeConnection(client);
    client->deleteLater();
}

void HttpServer::handlePacket()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    if(this->m_Hash.contains(client) == false)
    {
        sk_log(client, "Got data from unexpected client");
        client->abort();
        return;
    }

    HttpData *data = this->m_Hash[client];

    if(data->state != HTTP_READING_HEADER)
    {
        sk_log(data->logPrefix, "Got data in wrong state");
        client->abort();
        return;
    }

    data->requestData.append(client->readAll());
    if(data->requestData.contains("\r\n\r\n"))
    {
        if(data->requestData.indexOf("\r\n\r\n") + 4 != data->requestData.size())
        {
            sk_log(data->logPrefix, "HTTP body is not empty");
            client->write("HTTP/1.1 403 Forbidden\r\n\r\n");
            client->close();
            data->state = HTTP_INVALID;
            return;
        }

        int code = 403;

        if(parseHeader(data->requestData, &data->requestHeader) && data->requestHeader["method"] == "get")
        {
            if(data->resource)
            {
                if(data->resource->open(this->m_root + "/" + data->requestHeader["path"]))
                {
                    if(data->requestHeader.contains("http_range"))
                    {
                        qint64 start = 0;

                        if(parseRange(data->requestHeader.value("http_range"), data->resource->size(), &start, &data->len))
                        {
                            if(data->resource->seek(start))
                                code = 206;
                            else
                                code = 500;
                        }
                        else
                        {
                            code = 416;
                        }
                    }
                    else
                    {
                        code = 200;
                        data->len = data->resource->size();
                    }
                }
            }
        }

        if(code == 200)
        {
            data->state = HTTP_SENDING_RESPONSE;
            client->write(QString("HTTP/1.1 200 OK\r\n"
                                  "Content-Type: %1\r\n"
                                  "Content-Length: %2\r\n"
                                  "Accept-Ranges: bytes\r\n"
                                  "Connection: keep-alive\r\n"
                                  "\r\n")
                          .arg(data->resource->contentType())
                          .arg(data->resource->size()).toUtf8());
        }
        else if(code == 206)
        {
            data->state = HTTP_SENDING_RESPONSE;
            client->write(QString("HTTP/1.1 206 Partial Content\r\n"
                                  "Content-Type: %1\r\n"
                                  "Content-Length: %2\r\n"
                                  "Content-Range: bytes %3-%4/%5\r\n"
                                  "Connection: keep-alive\r\n"
                                  "\r\n")
                          .arg(data->resource->contentType())
                          .arg(data->len)
                          .arg(data->resource->pos())
                          .arg(data->resource->pos() + data->len - 1)
                          .arg(data->resource->size()).toUtf8());
        }
        else if(code == 416)
        {
            data->state = HTTP_INVALID;
            client->write("HTTP/1.1 416 Requested range not satisfiable\r\n\r\n");
            client->close();
        }
        else if(code == 403)
        {
            data->state = HTTP_INVALID;
            client->write("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nPermission denied");
            client->close();
        }
        else
        {
            data->state = HTTP_INVALID;
            client->write("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal Server Error");
            client->close();
        }
    }
    else
    {
        sk_log(data->logPrefix, "Got incomplete request");
    }
}

bool HttpServer::parseRange(const QString &range, qint64 size, qint64 *start, qint64 *len)
{
    QString t = range.trimmed();

    t.remove("bytes=");

    int sepIndex = t.indexOf("-");

    if(sepIndex < 0)
        return false;

    QString lstr = t.left(sepIndex);
    QString rstr = t.mid(sepIndex + 1);

    if(lstr.isEmpty() && rstr.isEmpty())
        return false;

    bool lok;
    bool rok;
    qint64 lnum;
    qint64 rnum;

    lnum = lstr.toLongLong(&lok);
    rnum = rstr.toLongLong(&rok);

    if(lnum < 0 || rnum < 0)
        return false;

    if(lok == false && lstr.isEmpty() == false)
        return false;

    if(rok == false && rstr.isEmpty() == false)
        return false;

    if(lok == false)
    {
        if(rnum > size)
            return false;
        (*start) = size - rnum;
        (*len) = rnum;
    }
    else if(rok == false)
    {
        if(lnum >= size)
            return false;
        (*start) = lnum;
        (*len) = size - lnum;
    }
    else
    {
        if(lnum > rnum)
            return false;
        if(lnum >= size)
            return false;
        if(rnum >= size)
            return false;

        (*start) = lnum;
        (*len) = rnum - lnum + 1;
    }
    return true;
}

QString HttpServer::parsePath(const QByteArray &path)
{
    QString t = QUrl::fromPercentEncoding(path);

    t.remove("\\");

    QStringList li = t.split("/", QString::SkipEmptyParts);

    t.clear();
    for(int i = 0; i < li.size(); ++i)
    {
        if(li.at(i) != "." && li.at(i) != "..")
        {
            t.append("/");
            t.append(li.at(i));
        }
    }
    if(t.isEmpty())
        t.append("/");
    return t;
}

bool HttpServer::parseHeader(const QByteArray &data, QHash<QString, QString> *hash)
{
    QString text = QString::fromUtf8(data);
    QStringList lineList = text.split("\r\n", QString::SkipEmptyParts);

    if(lineList.isEmpty())
        return false;

    ///
    /// 解析第一行
    ///
    /// 例子：
    /// GET /qt%20project.txt HTTP/1.1
    ///

    QStringList firstList = lineList.at(0).split(" ", QString::SkipEmptyParts);

    if(firstList.size() != 3)
        return false;

    QString method = firstList.at(0);
    QString encodedPath = firstList.at(1);
    QString protocol = firstList.at(2);

    hash->insert("method", method.toLower());
    hash->insert("path", parsePath(encodedPath.toUtf8()));
    hash->insert("protocol", protocol);

    for(int i = 1; i < lineList.length(); ++i)
    {
        QString line = lineList.at(i);
        int sepIndex = line.indexOf(":");

        if(sepIndex < 0)
            continue;

        hash->insert(QString("http_") + line.left(sepIndex).trimmed().toLower(), line.mid(sepIndex + 1).trimmed());
    }

//    for(QHash<QString, QString>::iterator it = hash->begin(); it != hash->end(); ++it)
//    {
//        printLog(it.key() + ": " + it.value());
//    }
    return true;
}

void HttpServer::clientBytesWritten(qint64 len)
{
    Q_UNUSED(len);

    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    if(this->m_Hash.contains(client) == false)
    {
        client->abort();
        return;
    }

    HttpData *data = this->m_Hash[client];

    if(data->state != HTTP_SENDING_RESPONSE)
        return;

    if(data->len <= 0)
    {
        data->state = HTTP_READING_HEADER;
        data->requestData.clear();
        data->requestHeader.clear();
        data->resource->close();
        return;
    }

    if(client->bytesToWrite() > HttpServer::s_writeBufferLimit)
        return;

    qint64 count = data->len;

    if(count > 4096)
        count = 4096;

    QByteArray t = data->resource->read(count);

    if(client->write(t) < 0)
    {
        data->state = HTTP_INVALID;
        client->abort();
        return;
    }

    data->len -= t.size();

    if(data->len <= 0)
    {
        data->state = HTTP_READING_HEADER;
        data->requestData.clear();
        data->requestHeader.clear();
        data->resource->close();
        return;
    }
}

void HttpServer::_printLog(const QString &id, const QString &msg)
{
    qDebug("(%s) %s", id.toUtf8().constData(), msg.toUtf8().constData());
}

void HttpServer::_printLog(QTcpSocket *client, const QString &msg)
{
    QString id = client->peerAddress().toString() + " : " + QString::number(client->peerPort());

    _printLog(id, msg);
}
