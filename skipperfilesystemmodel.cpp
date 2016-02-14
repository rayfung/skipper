#include "skipperfilesystemmodel.h"

SkipperFileSystemModel::SkipperFileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    for (int i = 1; i <= 7; ++i)
    {
        fileIcon.addFile(QString(":/icons/file_0%1").arg(i));
        fileIcon.addFile(QString(":/icons/file_0%1").arg(i), QSize(), QIcon::Selected);
    }
}

SkipperFileSystemModel::~SkipperFileSystemModel()
{
}

QVariant SkipperFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
        return QVariant();

    switch (role)
    {
    case Qt::DecorationRole:
        if (index.column() == 0)
        {
            if (isDir(index) == false)
                return this->fileIcon;
        }
        break;
    }

    return QFileSystemModel::data(index, role);
}
