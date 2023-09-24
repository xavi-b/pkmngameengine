#include "pkmnwidget.h"

PkmnWidget::PkmnWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        pkmn->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    formLayout->addRow(tr("ID"), idLineEdit);
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        pkmn->setId(nameLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Name"), nameLineEdit);
    sprite = new QLabel;
    formLayout->addRow(tr("Sprite"), sprite);

    layout->addLayout(formLayout);

    setLayout(layout);
}

void PkmnWidget::setPkmn(PkmnDef::PkmnDefPtr const& newPkmn)
{
    pkmn = newPkmn;
    idLineEdit->setText(newPkmn->getId().c_str());
    nameLineEdit->setText(newPkmn->getName().c_str());
    sprite->setPixmap(QPixmap(QDir(spritesDirectory).absoluteFilePath(QString("Front/%1").arg(pkmn->getId().c_str()))));
}

void PkmnWidget::setSpritesDirectory(QString const& dirName)
{
    spritesDirectory = dirName;
    if (pkmn)
        sprite->setPixmap(
            QPixmap(QDir(spritesDirectory).absoluteFilePath(QString("Front/%1").arg(pkmn->getId().c_str()))));
}
