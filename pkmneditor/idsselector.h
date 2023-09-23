#ifndef IDSSELECTOR_H
#define IDSSELECTOR_H

#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>
#include <QWidget>

class IdsSelector : public QWidget
{
    Q_OBJECT

public:
    explicit IdsSelector(QWidget* parent = nullptr);

    void        setAvailableIds(QStringList const& newAvailableIds);
    QStringList getSelectedIds() const;
    void        setSelectedIds(QStringList const& newSelectedIds);

private:
    QStringListModel* idsModel;
    QListView*        listView;
};

#endif // IDSSELECTOR_H
