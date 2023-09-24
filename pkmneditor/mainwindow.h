#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editors/pkmnsview.h"
#include "editors/typesview.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <boost/property_tree/ini_parser.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void    createMenus();
    QString saveFile(bool saveAs = false);

    pt::ptree readPropertyTree(QString const& dirName, QString const& fileName);

    QStackedWidget* stackedWidget;
    TypesView*      typesView;
    PkmnsView*      pkmnsView;

    QString openedFileName;

    std::vector<Type::TypePtr>       types;
    std::vector<PkmnDef::PkmnDefPtr> pkmns;
};

#endif // MAINWINDOW_H
