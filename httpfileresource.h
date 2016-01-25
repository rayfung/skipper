#ifndef HTTPFILERESOURCE_H
#define HTTPFILERESOURCE_H

#include <QFile>
#include <QHash>
#include "httpserver.h"

class HttpFileResource : public HttpResource
{
public:
    HttpFileResource();
    virtual ~HttpFileResource();
    virtual bool open(const QString &path);
    virtual void close();
    virtual qint64 size();
    virtual qint64 pos();
    virtual bool seek(qint64 pos);
    virtual QByteArray read(qint64 maxSize);
    virtual QString contentType();

private:
    QFile m_file;
    QHash<QString, QString> m_typeHash;
};

class HttpFileResourceFactory : public HttpResourceFactory
{
public:
    HttpFileResourceFactory();
    virtual ~HttpFileResourceFactory();
    virtual HttpResource *createResource();
};

#endif // HTTPFILERESOURCE_H
