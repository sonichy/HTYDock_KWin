#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QTimer>
#include <QBoxLayout>
#include <QButtonGroup>

struct Dock : QObjectUserData
{
    WId wid;
};

class AppWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppWidget(QWidget *parent = 0);
    QList<WId> list_wid;
    int index=0;
    QButtonGroup *buttonGroup;
    QString name, className;
    QPixmap icon;
    bool isActive = false;
    void addPreview(WId wid);
    void removePreview(WId wid);

private:
    bool isMouseOn = false;
    QWidget *widget_preview;
    QBoxLayout *boxLayout;
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