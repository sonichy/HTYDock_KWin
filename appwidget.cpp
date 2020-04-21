#include "appwidget.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QDesktopWidget>

AppWidget::AppWidget(QWidget *parent) : QWidget(parent)
{
    pushButton_preview = new QPushButton;
    pushButton_preview->setFixedSize(150,100);
    pushButton_preview->setIconSize(pushButton_preview->size());
    pushButton_preview->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), pushButton_preview, SLOT(show()));
    timer->setSingleShot(true);
}

void AppWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = true;
    update();
    QScreen *screen = QApplication::primaryScreen();
    pushButton_preview->setIcon(QIcon(screen->grabWindow(wid,0,0,-1,-1).scaled(pushButton_preview->size(), Qt::KeepAspectRatio)));
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString position = settings.value("Position", "Bottom").toString();
    int x1, y1;
    if (position == "Bottom") {
        x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - pushButton_preview->width()/2;
        y1 = mapToGlobal(QPoint(0,0)).y() - pushButton_preview->height();
    } else if (position == "Top") {
        x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - pushButton_preview->width()/2;
        y1 = height();
    } else if (position == "Left") {
        x1 = width();
        y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - pushButton_preview->height()/2;
    } else if (position == "Right") {
        x1 = QApplication::desktop()->width() - width() - pushButton_preview->width();
        y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - pushButton_preview->height()/2;
    }
    pushButton_preview->move(x1, y1);
    timer->start(1000);
}

void AppWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = false;
    update();
    if(pushButton_preview->isHidden())
        timer->stop();
    else
        pushButton_preview->hide();
}

void AppWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setPen(Qt::NoPen);
    if (isActive) {
//        QRadialGradient radialGradient(width()/2,height()/2,width()/3,width()/2,height()/2);
//        radialGradient.setColorAt(0.0,Qt::white);
//        radialGradient.setColorAt(1.0,Qt::blue);
//        painter.setBrush(QBrush(radialGradient));
        painter.setBrush(QColor(48,140,198,200));
        painter.drawRect(rect().adjusted(10, height()-2, -10, 0));
    }
    if (isMouseOn) {
        painter.setBrush(QColor(255,255,255,30));
        painter.drawRoundRect(rect(), 50, 50);
    }
    painter.drawPixmap(rect().adjusted(3,3,-3,-3), icon);
}

void AppWidget::mousePressEvent(QMouseEvent *event)
{
    point_mouse = QPoint(event->x(), event->y());
    pushButton_preview->hide();
}

void AppWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(point_mouse == QPoint(ev->x(), ev->y())) emit clicked();
}
