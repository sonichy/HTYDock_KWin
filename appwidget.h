#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QTimer>

class AppWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppWidget(QWidget *parent = 0);
    WId wid;
    QString name, className;
    QPixmap icon;
    bool isActive = false;

private:
    bool isMouseOn = false;
    QPushButton *pushButton_preview;
    QPoint point_mouse;
    QTimer *timer;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void clicked();

public slots:

};

#endif // APPWIDGET_H