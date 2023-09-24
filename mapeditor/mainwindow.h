#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageviewer.h"
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
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    void    createMenus();
    QString saveFile(bool saveAs = false);

    ImageViewer*  imageViewer;
    MapperEditor* mapEditor;
    QString       openedFileName;
};
#endif // MAINWINDOW_H
