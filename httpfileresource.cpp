#include "httpfileresource.h"

HttpFileResourceFactory::HttpFileResourceFactory()
{
}

HttpFileResourceFactory::~HttpFileResourceFactory()
{
}

HttpResource *HttpFileResourceFactory::createResource()
{
    return new HttpFileResource();
}



HttpFileResource::HttpFileResource()
{
    this->m_typeHash.insert("txt", "text/plain");
    this->m_typeHash.insert("html", "text/html");
    this->m_typeHash.insert("htm", "text/html");
    this->m_typeHash.insert("jpg", "image/jpeg");
    this->m_typeHash.insert("png", "image/png");
    //this->m_typeHash.insert("mkv", "video/x-matroska");
    this->m_typeHash.insert("mp4", "video/mp4");
    this->m_typeHash.insert("ico", "image/x-icon");
}

HttpFileResource::~HttpFileResource()
{
    this->close();
}

bool HttpFileResource::open(const QString &path)
{
    this->m_file.setFileName(path);
    return this->m_file.open(QFile::ReadOnly);
}

void HttpFileResource::close()
{
    this->m_file.close();
}

qint64 HttpFileResource::size()
{
    return this->m_file.size();
}

qint64 HttpFileResource::pos()
{
    return this->m_file.pos();
}

bool HttpFileResource::seek(qint64 pos)
{
    return this->m_file.seek(pos);
}

QByteArray HttpFileResource::read(qint64 maxSize)
{
    return this->m_file.read(maxSize);
}

QString HttpFileResource::contentType()
{
    QString fileExt = this->m_file.fileName();

    int index = fileExt.lastIndexOf(".");

    if(index < 0)
        return "application/octet-stream";

    fileExt = fileExt.mid(index + 1);

    if(this->m_typeHash.contains(fileExt))
        return this->m_typeHash[fileExt];

    return QString();
    //return "application/octet-stream";
}
