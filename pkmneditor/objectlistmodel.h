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
    TemplateListItem(std::shared_ptr<T> ptr) : ptr(ptr)
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

    std::shared_ptr<T> getPtr() const
    {
        return ptr;
    }

private:
    std::shared_ptr<T> ptr;
};

class ObjectListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ObjectListModel(QObject* parent);

    virtual int      rowCount(QModelIndex const& parent) const override;
    virtual QVariant data(QModelIndex const& index, int role) const override;

    template <class T>
    void setObjects(std::vector<std::shared_ptr<T>> const& objects)
    {
        beginResetModel();
        items.clear();
        for (auto& i : objects)
        {
            if (i)
            {
                qDebug() << i->getId().c_str();
                items.push_back(std::make_unique<TemplateListItem<T>>(i));
            }
        }
        endResetModel();
    }

    template <class T>
    std::vector<std::shared_ptr<T>> getObjects() const
    {
        std::vector<std::shared_ptr<T>> objects;
        for (auto& i : items)
        {
            auto item = dynamic_cast<TemplateListItem<T>>(i.get());
            if (item)
            {
                objects.push_back(item.getPtr());
            }
        }
        return objects;
    }

    template <class T>
    void appendItem(std::shared_ptr<T> object)
    {
        beginInsertRows(QModelIndex(), items.size(), items.size());
        if (object)
        {
            qDebug() << object->getId().c_str();
            items.push_back(std::make_unique<TemplateListItem<T>>(object));
        }
        endInsertRows();
    }

    void removeItem(int row);

    ObjectListItem* getItem(QModelIndex const& index) const;

private:
    std::vector<std::unique_ptr<ObjectListItem>> items;
};

#endif // OBJECTLISTMODEL_H
