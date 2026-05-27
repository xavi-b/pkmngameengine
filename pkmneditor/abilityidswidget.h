#ifndef ABILITYIDSWIDGET_H
#define ABILITYIDSWIDGET_H

#include <QListView>
#include <QStringListModel>
#include <QWidget>
#include <string>
#include <vector>

class AbilityIdsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AbilityIdsWidget(QWidget* parent = nullptr);

    std::vector<std::string> getAbilityIds() const;
    void                     setAbilityIds(std::vector<std::string> const& abilityIds);
    void                     setAvailableIds(QStringList const& availableIds);

signals:
    void abilityIdsChanged();

private:
    QStringListModel* model;
    QStringList       availableIds;
};

#endif // ABILITYIDSWIDGET_H
