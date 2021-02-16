#include "appwidget.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QDesktopWidget>

AppWidget::AppWidget(QWidget *parent) : QWidget(parent)
{
    widget_preview = new QWidget;
    boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    widget_preview->setLayout(boxLayout);
    widget_preview->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    buttonGroup = new QButtonGroup;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), widget_preview, SLOT(show()));
    timer->setSingleShot(true);
}

void AppWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = true;
    update();
    QScreen *screen = QApplication::primaryScreen();
    for (int i=0; i<buttonGroup->buttons().count(); i++) {
        QAbstractButton *button = buttonGroup->buttons().at(i);
        Dock *dock = static_cast<Dock*>(button->userData(Qt::UserRole));
        button->setIcon(QIcon(screen->grabWindow(dock->wid,0,0,-1,-1).scaled(widget_preview->size(), Qt::KeepAspectRatio)));
    }
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString position = settings.value("Position", "Bottom").toString();
    int x1=0, y1=0;
    if (position == "Bottom") {
        x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - widget_preview->width()/2;
        y1 = mapToGlobal(QPoint(0,0)).y() - widget_preview->height();
    } else if (position == "Top") {
        x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - widget_preview->width()/2;
        y1 = height();
    } else if (position == "Left") {
        x1 = width();
        y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - widget_preview->height()/2;
    } else if (position == "Right") {
        x1 = QApplication::desktop()->width() - width() - widget_preview->width();
        y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - widget_preview->height()/2;
    }
    widget_preview->move(x1, y1);
    timer->start(1000);
}

void AppWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = false;
    update();
    if(widget_preview->isHidden())
        timer->stop();
    else
        widget_preview->hide();
}

void AppWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setPen(Qt::NoPen);
    if (isActive) {
//渐变色失败
//        QRadialGradient radialGradient(width()/2,height()/2,width()/3,width()/2,height()/2);
//        radialGradient.setColorAt(0.0,Qt::white);
//        radialGradient.setColorAt(1.0,Qt::blue);
//        painter.setBrush(QBrush(radialGradient));
        //painter.setBrush(QColor(48,140,198,200));//下划线
        //painter.drawRect(rect().adjusted(10, height()-2, -10, 0));
        painter.setBrush(QColor(255,255,255,30));//蒙板
        painter.drawRoundRect(rect(), 50, 50);
    }
    if (isMouseOn) {
        painter.setBrush(QColor(255,255,255,30));
        painter.drawRoundRect(rect(), 50, 50);
    }
    painter.drawPixmap(rect().adjusted(3,3,-3,-3), icon);
    if (list_wid.count() > 1) {
        painter.setBrush(QColor(255,0,0,100));
        painter.drawEllipse(QRect(0,0,11,11));
        painter.setPen(Qt::white);
        QFont font = this->font();
        font.setPointSize(6);
        painter.setFont(font);
        painter.drawText(QRect(0,0,10,10), Qt::AlignCenter, QString::number(list_wid.count()));
    }
}

void AppWidget::mousePressEvent(QMouseEvent *event)
{
    point_mouse = QPoint(event->x(), event->y());
    widget_preview->hide();
}

void AppWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if (point_mouse == QPoint(ev->x(), ev->y()))
        emit clicked();
}

void AppWidget::addPreview(WId wid)
{
    QPushButton *button = new QPushButton;
    button->setFixedSize(150,100);
    button->setIconSize(button->size());
    Dock *dock = new Dock;
    dock->wid = wid;
    button->setUserData(Qt::UserRole, dock);
    buttonGroup->addButton(button);
    boxLayout->addWidget(button);
}

void AppWidget::removePreview(WId wid)
{
    for (int i=0; i<buttonGroup->buttons().count(); i++) {
        QAbstractButton *button = buttonGroup->buttons().at(i);
        Dock *dock = static_cast<Dock*>(button->userData(Qt::UserRole));
        if (dock->wid == wid) {
            //qDebug() << dock->wid;
            boxLayout->removeWidget(button);
            buttonGroup->removeButton(button);
            button->deleteLater();
        }
    }
}