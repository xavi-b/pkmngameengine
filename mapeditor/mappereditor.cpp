#include "mappereditor.h"

#include "checkablecombobox.h"
#include "layersmodel.h"
#include "levelsmodel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

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

    QHBoxLayout* layersLayout = new QHBoxLayout;
    layersLayout->setContentsMargins(0, 0, 0, 0);

    mapperViewer = new MapperViewer;

    QSpinBox* mapWidthSpinBox = new QSpinBox;
    mapWidthSpinBox->setMinimum(0);
    mapWidthSpinBox->setMaximum(300);
    mapWidthSpinBox->setValue(int(mapperViewer->contentWidget()->getMap()->getNCol()));
    QSpinBox* mapHeightSpinBox = new QSpinBox;
    mapHeightSpinBox->setMinimum(0);
    mapHeightSpinBox->setMaximum(300);
    mapHeightSpinBox->setValue(int(mapperViewer->contentWidget()->getMap()->getNRow()));

    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->addLevel();
    });

    CheckableComboBox* levelSelection = new CheckableComboBox;
    LevelsModel*       levelsModel    = new LevelsModel(this);
    levelsModel->setLevelsReference(mapperViewer->contentWidget());
    levelSelection->setModel(levelsModel);
    connect(levelSelection,
            &QComboBox::currentIndexChanged,
            mapperViewer->contentWidget(),
            &MapperWidget::setWorkingLevelIndex);

    QCheckBox* belowLevelsCheckBox = new QCheckBox(tr("Below level opacity"));
    connect(belowLevelsCheckBox,
            &QCheckBox::clicked,
            mapperViewer->contentWidget(),
            &MapperWidget::setBelowLevelsOpacity);

    QCheckBox* nightCheckBox = new QCheckBox(tr("Night mode"));
    connect(nightCheckBox, &QCheckBox::clicked, mapperViewer->contentWidget(), &MapperWidget::setNight);

    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->removeLevel(levelSelection->currentIndex());
    });

    CheckableComboBox* layerSelection = new CheckableComboBox;
    LayersModel*       layersModel    = new LayersModel(this);
    layersModel->setLayersReference(mapperViewer->contentWidget());
    layerSelection->setModel(layersModel);
    connect(layerSelection,
            &QComboBox::currentIndexChanged,
            mapperViewer->contentWidget(),
            &MapperWidget::setWorkingLayerIndex);

    QComboBox* specialTileComboBox = new QComboBox;
    specialTileComboBox->setVisible(false);
    for (size_t i = 0; i < SpecialTileTypeCount; ++i)
        specialTileComboBox->addItem(
            QString("%1 - ").arg(i + SpecialTileType::GRASS)
                + QString(SpecialTileTypeToString((SpecialTileType)(i + SpecialTileType::GRASS)).c_str()),
            QVariant::fromValue(i + SpecialTileType::GRASS));
    connect(specialTileComboBox, &QComboBox::currentIndexChanged, this, [=]() {
        mapperViewer->contentWidget()->setCurrentSpecialTileType(
            (SpecialTileType)specialTileComboBox->currentData().toInt());
    });

    encountersEditor = new EncountersEditor;
    encountersEditor->setVisible(false);

    QRadioButton* tileButton = new QRadioButton("Tile layers", this);
    tileButton->setChecked(true);
    connect(tileButton, &QRadioButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->setLayerType(MapperWidget::LayerType::TILES);
        layerSelection->setVisible(true);
        specialTileComboBox->setVisible(false);
        encountersEditor->setVisible(false);
    });

    QRadioButton* eventButton = new QRadioButton("Event layer", this);
    connect(eventButton, &QRadioButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->setLayerType(MapperWidget::LayerType::EVENTS);
        layerSelection->setVisible(false);
        specialTileComboBox->setVisible(false);
        encountersEditor->setVisible(false);
    });

    QRadioButton* specialTileButton = new QRadioButton("Special tile layer", this);
    connect(specialTileButton, &QRadioButton::clicked, this, [=]() {
        mapperViewer->contentWidget()->setLayerType(MapperWidget::LayerType::SPECIAL_TILE);
        layerSelection->setVisible(false);
        specialTileComboBox->setVisible(true);
        encountersEditor->setVisible(true);
    });

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
    levelLayout->addWidget(nightCheckBox);
    levelLayout->addStretch(1);
    l->addLayout(levelLayout);
    layersLayout->addWidget(tileButton);
    layersLayout->addWidget(eventButton);
    layersLayout->addWidget(specialTileButton);
    layersLayout->addStretch(1);
    l->addLayout(layersLayout);
    layerLayout->addWidget(specialTileComboBox);
    layerLayout->addWidget(layerSelection);
    layerLayout->addStretch(1);
    l->addLayout(layerLayout);
    l->addWidget(encountersEditor);
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
}

void MapperEditor::swapMap(std::unique_ptr<Map>&& newMap)
{
    mapperViewer->contentWidget()->swapMap(std::move(newMap));
    auto& map = mapperViewer->contentWidget()->getMap();
    encountersEditor->setEncounterMethods(map->getEncounterMethods());
}

MapperViewer* MapperEditor::getMapperViewer() const
{
    return mapperViewer;
}

EncountersEditor* MapperEditor::getEncountersEditor() const
{
    return encountersEditor;
}
