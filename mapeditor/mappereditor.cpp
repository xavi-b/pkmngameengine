#include "mappereditor.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include "checkablecombobox.h"
#include "levelsmodel.h"
#include "layersmodel.h"

MapperEditor::MapperEditor(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* mapLayout = new QHBoxLayout;
    mapLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* levelLayout = new QHBoxLayout;
    levelLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* layerLayout = new QHBoxLayout;
    layerLayout->setContentsMargins(0, 0, 0, 0);

    mapperViewer = new MapperViewer;

    QSpinBox* mapWidthSpinBox = new QSpinBox;
    mapWidthSpinBox->setMinimum(0);
    mapWidthSpinBox->setMaximum(300);
    mapWidthSpinBox->setValue(int(mapperViewer->contentWidget()->getMap()->getNCol()));
    QSpinBox* mapHeightSpinBox = new QSpinBox;
    mapHeightSpinBox->setMinimum(0);
    mapHeightSpinBox->setMaximum(300);
    mapHeightSpinBox->setValue(int(mapperViewer->contentWidget()->getMap()->getNRow()));

    QPushButton*       addButton      = new QPushButton(tr("Add"));
    CheckableComboBox* levelSelection = new CheckableComboBox;
    LevelsModel*       levelsModel    = new LevelsModel(this);
    levelsModel->setLevelsReference(mapperViewer->contentWidget());
    levelSelection->setModel(levelsModel);

    QCheckBox*         belowLevelsCheckBox       = new QCheckBox(tr("Below level opacity"));
    QCheckBox*         eventLayerEditionCheckBox = new QCheckBox(tr("Edit event layer"));
    QPushButton*       removeButton              = new QPushButton(tr("Remove"));
    CheckableComboBox* layerSelection            = new CheckableComboBox;
    LayersModel*       layersModel               = new LayersModel(this);
    layersModel->setLayersReference(mapperViewer->contentWidget());
    layerSelection->setModel(layersModel);

    mapLayout->addWidget(new QLabel(tr("Width:")));
    mapLayout->addWidget(mapWidthSpinBox);
    mapLayout->addWidget(new QLabel(tr("Height:")));
    mapLayout->addWidget(mapHeightSpinBox);
    mapLayout->addStretch(1);
    l->addLayout(mapLayout);
    levelLayout->addWidget(addButton);
    levelLayout->addWidget(levelSelection);
    levelLayout->addWidget(removeButton);
    levelLayout->addWidget(belowLevelsCheckBox);
    levelLayout->addStretch(1);
    l->addLayout(levelLayout);
    layerLayout->addWidget(layerSelection);
    layerLayout->addWidget(eventLayerEditionCheckBox);
    layerLayout->addStretch(1);
    l->addLayout(layerLayout);
    l->addWidget(mapperViewer, 1);

    setLayout(l);

    connect(mapperViewer->contentWidget(), &MapperWidget::reset, this, [=]() {
        mapWidthSpinBox->setValue(int(mapperViewer->contentWidget()->getMap()->getNCol()));
        mapHeightSpinBox->setValue(int(mapperViewer->contentWidget()->getMap()->getNRow()));
    });
    connect(mapWidthSpinBox, &QSpinBox::editingFinished, this, [=]() {
        mapperViewer->contentWidget()->setMapWidth(size_t(mapWidthSpinBox->value()));
    });
    connect(mapHeightSpinBox, &QSpinBox::editingFinished, this, [=]() {
        mapperViewer->contentWidget()->setMapHeight(size_t(mapHeightSpinBox->value()));
    });
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
    connect(eventLayerEditionCheckBox, &QCheckBox::clicked, this, [=](bool edit) {
        mapperViewer->contentWidget()->setEventLayerEdition(edit);
        layerSelection->setDisabled(edit);
    });
}

MapperViewer* MapperEditor::viewer() const
{
    return mapperViewer;
}
