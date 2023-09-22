#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "typesview.h"

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

    QStackedWidget* stackedWidget;
    TypesView*      typesView;

    QString openedFileName;

    std::shared_ptr<std::vector<Type::TypePtr>> types;
};
#endif // MAINWINDOW_H
