#include "typewidget.h"

TypeWidget::TypeWidget(QWidget* parent) : QWidget(parent)
{
    QFormLayout* layout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        type->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    layout->addRow(tr("ID"), idLineEdit);
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        type->setId(nameLineEdit->text().toStdString());
    });
    layout->addRow(tr("Name"), nameLineEdit);

    setLayout(layout);
}

void TypeWidget::setType(Type::TypePtr const& newType)
{
    type = newType;
    idLineEdit->setText(type->getId().c_str());
    nameLineEdit->setText(type->getName().c_str());
}
