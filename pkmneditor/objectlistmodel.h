#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include <QAbstractListModel>

class ObjectListItem
{
public:
    virtual QVariant data(int role) const = 0;
};

template <class T>
class TemplateListItem : public ObjectListItem
{
public:
    TemplateListItem(std::unique_ptr<T>& ptr) : ptr(ptr)
    {
    }

    virtual QVariant data(int role) const override
    {
        if (role == Qt::DisplayRole)
        {
            std::string id = ptr->getId();
            return id.c_str();
        }

        return QVariant();
    }

private:
    std::unique_ptr<T>& ptr;
};

class ObjectListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ObjectListModel(QObject* parent);

    virtual int      rowCount(QModelIndex const& parent) const override;
    virtual QVariant data(QModelIndex const& index, int role) const override;

    template <class T>
    void setObjects(std::shared_ptr<std::vector<std::unique_ptr<T>>>& objects)
    {
        beginResetModel();
        items.clear();
        std::vector<std::unique_ptr<T>>& v = *(objects.get());
        for (auto& i : v)
        {
            qDebug() << i->getId().c_str();
            items.push_back(std::make_unique<TemplateListItem<T>>(i));
        }
        endResetModel();
    }

private:
    ObjectListItem* getItem(QModelIndex const& index) const;

    std::vector<std::unique_ptr<ObjectListItem>> items;
};

#endif // OBJECTLISTMODEL_H
