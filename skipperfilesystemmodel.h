#ifndef SKIPPERFILESYSTEMMODEL_H
#define SKIPPERFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QVariant>
#include <QIcon>

class SkipperFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit SkipperFileSystemModel(QObject *parent = 0);
    ~SkipperFileSystemModel();
    QVariant data(const QModelIndex &index, int role) const;

private:
    QIcon fileIcon;
    QIcon imageIcon;
    QIcon videoIcon;
};

#endif // SKIPPERFILESYSTEMMODEL_H
