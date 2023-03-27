#include "mappereditor.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>

MapperEditor::MapperEditor(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    l->addLayout(hLayout);

    mapperViewer = new MapperViewer;
    l->addWidget(mapperViewer, 1);

    QComboBox* layerSelection = new QComboBox;

    int i = 0;
    for (auto& layer : mapperViewer->contentWidget()->getMap()->getTileLayers())
    {
        QString layerName = layer->getTypeName().c_str();

        layerSelection->addItem(layerName);

        QCheckBox* checkBox = new QCheckBox(layerName);
        checkBox->setChecked(true);
        hLayout->addWidget(checkBox);

        connect(mapperViewer->contentWidget(),
                &MapperWidget::layerVisibleChanged,
                this,
                [checkBox, i](int index, bool visible) {
                    if (i == index)
                        checkBox->setChecked(visible);
                });
        connect(checkBox, &QCheckBox::clicked, this, [this, i](bool checked) {
            mapperViewer->contentWidget()->setLayerVisible(i, checked);
        });

        ++i;
    }
    hLayout->addWidget(layerSelection);
    layerSelection->setCurrentIndex(mapperViewer->contentWidget()->getWorkingLayerIndex());
    hLayout->addStretch(1);

    setLayout(l);

    connect(mapperViewer->contentWidget(),
            &MapperWidget::workingLayerIndexChanged,
            layerSelection,
            &QComboBox::setCurrentIndex);
    connect(layerSelection,
            &QComboBox::currentIndexChanged,
            mapperViewer->contentWidget(),
            &MapperWidget::setWorkingLayerIndex);
}

MapperViewer* MapperEditor::viewer() const
{
    return mapperViewer;
}
