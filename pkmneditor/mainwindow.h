#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGridLayout>
#include <QMenuBar>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

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

    QString openedFileName;
};
#endif // MAINWINDOW_H
