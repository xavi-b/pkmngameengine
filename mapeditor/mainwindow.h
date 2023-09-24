#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mappereditor.h"

#include <QMainWindow>
#include <QSettings>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    void    createMenus();
    QString saveFile(bool saveAs = false);

    MapperEditor* mapArea;
    QString       openedFileName;
};
#endif // MAINWINDOW_H
