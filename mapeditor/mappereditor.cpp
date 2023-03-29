#include "mappereditor.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include "checkablecombobox.h"
#include "levelsmodel.h"
#include "layersmodel.h"

MapperEditor::MapperEditor(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* levelLayout = new QHBoxLayout;
    levelLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* layerLayout = new QHBoxLayout;
    layerLayout->setContentsMargins(0, 0, 0, 0);

    mapperViewer = new MapperViewer;

    QPushButton*       addButton      = new QPushButton(tr("Add"));
    CheckableComboBox* levelSelection = new CheckableComboBox;
    LevelsModel*       levelsModel    = new LevelsModel(this);
    levelsModel->setLevelsReference(mapperViewer->contentWidget());
    levelSelection->setModel(levelsModel);
    QCheckBox*         belowLevelsCheckBox = new QCheckBox(tr("Below level opacity"));
    QPushButton*       removeButton        = new QPushButton(tr("Remove"));
    CheckableComboBox* layerSelection      = new CheckableComboBox;
    LayersModel*       layersModel         = new LayersModel(this);
    layersModel->setLayersReference(mapperViewer->contentWidget());
    layerSelection->setModel(layersModel);

    levelLayout->addWidget(addButton);
    levelLayout->addWidget(levelSelection);
    levelLayout->addWidget(removeButton);
    levelLayout->addWidget(belowLevelsCheckBox);
    levelLayout->addStretch(1);
    l->addLayout(levelLayout);
    layerLayout->addWidget(layerSelection);
    layerLayout->addStretch(1);
    l->addLayout(layerLayout);
    l->addWidget(mapperViewer, 1);

    setLayout(l);

    connect(addButton, &QPushButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->addLevel();
    });
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->removeLevel(levelSelection->currentIndex());
    });
    connect(levelSelection,
            &QComboBox::currentIndexChanged,
            mapperViewer->contentWidget(),
            &MapperWidget::setWorkingLevelIndex);
    connect(layerSelection,
            &QComboBox::currentIndexChanged,
            mapperViewer->contentWidget(),
            &MapperWidget::setWorkingLayerIndex);
    connect(
        belowLevelsCheckBox, &QCheckBox::clicked, mapperViewer->contentWidget(), &MapperWidget::setBelowLevelsOpacity);
}

MapperViewer* MapperEditor::viewer() const
{
    return mapperViewer;
}
