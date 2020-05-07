#ifndef TRASHWIDGET_H
#define TRASHWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class TrashWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrashWidget(QWidget *parent = 0);
    QPixmap pixmap_icon, pixmap;

private:
    bool isMouseOn = false;
    QPixmap pixmap_dragEnter;
    QPoint point_mouse;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

signals:

public slots:
};

#endif // TRASHWIDGET_H