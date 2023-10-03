#ifndef MOVESTOLEARNWIDGET_H
#define MOVESTOLEARNWIDGET_H

#include "movestolearnmodel.h"
#include "movetolearndialog.h"
#include "pkmndef.h"

#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QWidget>

class MovesToLearnWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovesToLearnWidget(QWidget* parent = nullptr);

    std::vector<PkmnDef::MoveToLearn> getMovesToLearn();
    void                              setMovesToLearn(std::vector<PkmnDef::MoveToLearn> newMovesToLearn);

signals:
    void movesToLearnChanged();

private:
    MovesToLearnModel* movesToLearnModel;
};

#endif // MOVESTOLEARNWIDGET_H
