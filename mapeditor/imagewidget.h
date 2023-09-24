#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QRubberBand>

class ImageWidget : public QLabel
{
public:
    ImageWidget(QWidget* parent = nullptr);

    void setPixmap(QString const& pixmapPath);

    QPair<QString, QRect> currentSelectionPixmap() const;

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    int selectionSize = 32;

    QRubberBand* rubberBand;
    QPoint       pressPoint;
    QPoint       releasePoint;
    QString      pixmapPath;
};

#endif // IMAGEWIDGET_H
