#include <QFileInfo>
#include "httpfileresource.h"

HttpFileResourceFactory::HttpFileResourceFactory(const QString &root, const QString &prefix)
{
    m_root = root;
    m_prefix = prefix;
}

HttpFileResourceFactory::~HttpFileResourceFactory()
{
}

HttpResource *HttpFileResourceFactory::createResource()
{
    HttpFileResource *res = new HttpFileResource();

    res->setRootPath(m_root);
    res->setPrefix(m_prefix);
    return res;
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

void HttpFileResource::setRootPath(const QString &root)
{
    m_root = root;
}

void HttpFileResource::setPrefix(const QString &prefix)
{
    m_prefix = prefix;
}

bool HttpFileResource::open(const QString &path)
{
    QString tmpPath = path;

    if(!m_prefix.isEmpty())
    {
        if(tmpPath.startsWith('/'))
            tmpPath = tmpPath.mid(1);

        bool ok = false;
        int cnt = m_prefix.length();
        if(tmpPath.startsWith(m_prefix))
        {
            if(tmpPath.length() == cnt)
            {
                ok = true;
                tmpPath.clear();
            }
            else if(tmpPath.length() > cnt && tmpPath.at(cnt) == '/')
            {
                ok = true;
                tmpPath = tmpPath.mid(cnt + 1);
            }
        }

        if(!ok)
            return false;
    }

    QString finalPath = m_root + "/" + tmpPath;

    if(QFileInfo(finalPath).isDir())
        finalPath += QString("/index.html");

    this->m_file.setFileName(finalPath);
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
        return QString();

    fileExt = fileExt.mid(index + 1);

    if(this->m_typeHash.contains(fileExt))
        return this->m_typeHash[fileExt];

    return QString();
}
