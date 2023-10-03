#ifndef MOVESTOLEARNMODEL_H
#define MOVESTOLEARNMODEL_H

#include "pkmndef.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class SortMovesToLearnModel : public QSortFilterProxyModel
{
public:
    explicit SortMovesToLearnModel(QObject* parent = nullptr);

protected:
    bool lessThan(QModelIndex const& source_left, QModelIndex const& source_right) const override;
};

class MovesToLearnModel : public QAbstractListModel
{
public:
    explicit MovesToLearnModel(QObject* parent = nullptr);

    virtual int      rowCount(QModelIndex const& parent) const override;
    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    virtual bool     removeRows(int row, int count, QModelIndex const& parent = QModelIndex()) override;

    void                              addMoveToLearn(PkmnDef::MoveToLearn const& newMoveToLearn);
    std::vector<PkmnDef::MoveToLearn> getMovesToLearn() const;
    void                              setMovesToLearn(std::vector<PkmnDef::MoveToLearn> const& newMovesToLearn);

private:
    std::vector<PkmnDef::MoveToLearn> movesToLearn;
};

#endif // MOVESTOLEARNMODEL_H
