#include "datetimewidget.h"
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QAction>
#include <QtMath>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>
#include <QDebug>

DatetimeWidget::DatetimeWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QMenu { color:white; background: rgba(0,0,0,100);}"
                  "QMenu::item:selected { background: rgba(48,140,198,200);}");

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    clock_type = settings.value("ClockType", "TEXT_CLOCK").toString();

    calendar = new QCalendarWidget;
    calendar->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, [=](){
        //[&](const QPoint& pos)
        QMenu *menu = new QMenu;
        menu->setStyleSheet("QMenu { color:white; background: rgba(0,0,0,100);}"
                            "QMenu::item:selected { background: rgba(48,140,198,255);}");
        menu->setAttribute(Qt::WA_TranslucentBackground, true);
        menu->setAutoFillBackground(true);
        QAction *action_text = new QAction("文字", this);
        connect(action_text, &QAction::triggered, [=](){
            clock_type = "TEXT_CLOCK";
            QSettings settings(QApplication::organizationName(), QApplication::applicationName());
            settings.setValue("ClockType", clock_type);
            update();
        });
        menu->addAction(action_text);
        QAction *action_digital = new QAction("数字", this);
        connect(action_digital, &QAction::triggered, [=](){
            clock_type = "DIGITAL_CLOCK";
            QSettings settings(QApplication::organizationName(), QApplication::applicationName());
            settings.setValue("ClockType", clock_type);
            update();
        });
        menu->addAction(action_digital);
        QAction *action_analog = new QAction("模拟", this);
        connect(action_analog, &QAction::triggered, [=](){
            clock_type = "ANALOG_CLOCK";
            QSettings settings(QApplication::organizationName(), QApplication::applicationName());
            settings.setValue("ClockType", clock_type);
            update();
        });
        menu->addAction(action_analog);
        menu->adjustSize();//不加会到屏幕中间
        //menu->exec(mapToGlobal(pos));
        int x1=0, y1=0;
        QSettings settings(QApplication::organizationName(), QApplication::applicationName());
        QString position = settings.value("Position", "Bottom").toString();
        if (position == "Top") {
            x1 = QPoint(mapToGlobal(QPoint(0,0))).x() + width()/2 - menu->width()/2;
            y1 = height();
        } else if (position == "Bottom") {
            x1 = QPoint(mapToGlobal(QPoint(0,0))).x() + width()/2 - menu->width()/2;
            y1 = QPoint(mapToGlobal(QPoint(0,0))).y() - menu->height();
        } else if (position == "Left") {
            x1 = width();
            y1 = QPoint(mapToGlobal(QPoint(0,0))).y() + height()/2 - menu->height()/2;
        } else if (position == "Right") {
            x1 = QApplication::desktop()->width() - width() - menu->width();
            y1 = QPoint(mapToGlobal(QPoint(0,0))).y() + height()/2 - menu->height()/2;
        }
        menu->exec(QPoint(x1, y1));
    });
}

void DatetimeWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = true;
    update();
}

void DatetimeWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = false;
    update();
}

void DatetimeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    bool b = settings.value("isShowClock", true).toBool();
    if (!b)
        return;
    QDateTime dateTime = QDateTime::currentDateTime();
    setToolTip(dateTime.toString("yyyy/MM/dd HH:mm:ss ddd"));
    QPainter painter(this);
    if (isMouseOn) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255,255,255,30));
        painter.drawRoundRect(rect(), 50, 50);
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (clock_type == "TEXT_CLOCK") {
        painter.setPen(Qt::white);
        QFont font = this->font();
        font.setPointSize(7);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, dateTime.toString("HH:mm ddd\nyyyy/MM/dd"));
    } else if(clock_type == "DIGITAL_CLOCK") {
        painter.setPen(Qt::white);
        QFont font = this->font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, dateTime.toString("HH:mm"));
    } else if(clock_type == "ANALOG_CLOCK") {
        int w = width() - 6;
        int h = height() - 6;
        QPixmap pixmap(w, h);
        pixmap.fill(Qt::transparent);
        QPainter painter1(&pixmap);
        painter1.setRenderHint(QPainter::Antialiasing, true);
        QTime time = QTime::currentTime();
        int hour = time.hour();
        int m = time.minute();
        int s = time.second();
        // face
        painter1.setPen(QPen(Qt::black, 2));
        painter1.setBrush(QBrush(Qt::white));
        painter1.drawEllipse(QPoint(w/2,h/2), (int)(w/2*0.9), (int)(h/2*0.9));
        painter1.setBrush(QBrush(Qt::black));
        painter1.drawEllipse(QPoint(w/2,h/2), (int)(w*0.015), (int)(h*0.015));
        qreal da = 2 * M_PI / 60;
        // second hand
        int x = w * 0.4 * qCos(M_PI/2 - s * da) + w / 2;
        int y = - h * 0.4 * qSin(M_PI/2 - s * da) + h / 2;
        painter1.setPen(QPen(Qt::black, 1));
        painter1.drawLine(QPoint(w/2,h/2), QPoint(x,y));
        // minute hand
        x = w * 0.35 * qCos(M_PI/2 - m * da - s * da / 60) + w / 2;
        y = - h * 0.35 * qSin(M_PI/2 - m * da - s * da / 60) + h / 2;
        painter1.setPen(QPen(Qt::black, 2));
        painter1.drawLine(QPoint(w/2,h/2), QPoint(x,y));
        // hour hand
        da = 2 * M_PI / 12;
        if(hour >= 12) hour -= 12;
        x = w*0.25 * qCos(M_PI/2 - hour * da - m * da / 60) + w/2;
        y = - h*0.25 * qSin(M_PI/2 - hour * da - m * da / 60) + h/2;
        //qDebug() << "x =" << x << ", y =" << y;
        painter1.setPen(QPen(Qt::black, 2));
        painter1.drawLine(QPoint(w/2,h/2), QPoint(x,y));
        painter.drawPixmap(rect().center() - pixmap.rect().center(), pixmap);
    }
}

void DatetimeWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        QString position = settings.value("Position", "Bottom").toString();
        if (calendar->isHidden()) {
            int x1, y1;
            if (position == "Bottom") {
                x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - calendar->width()/2;
                y1 = mapToGlobal(QPoint(0,0)).y() - calendar->height();
            } else if (position == "Top") {
                x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - calendar->width()/2;
                y1 = height();
            } else if (position == "Left") {
                x1 = width();
                y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - calendar->height()/2;
            } else if (position == "Right") {
                x1 = QApplication::desktop()->width() - width() - calendar->width();
                y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - calendar->height()/2;
            }
            qDebug() << "QCalendarWidget" <<  x1 << y1;
            calendar->move(x1, y1);
            calendar->setSelectedDate(QDate::currentDate());
            calendar->show();
        } else {
            calendar->hide();
        }
    } else {
        calendar->hide();
    }
}