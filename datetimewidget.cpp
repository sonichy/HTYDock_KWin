#include "datetimewidget.h"
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QAction>
#include <QtMath>
#include <QSettings>
#include <QCoreApplication>
//#include <QMetaEnum>

DatetimeWidget::DatetimeWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QMenu { color:white; background: rgba(0,0,0,100);}"
                  "QMenu::item:selected { background: rgba(48,140,198,200);}");

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    clock_type = settings.value("ClockType").toString();
    if(clock_type == "")
        clock_type = "TEXT_CLOCK";
    QAction *action_text = new QAction("文字", this);
    connect(action_text, &QAction::triggered, [=](){
        clock_type = "TEXT_CLOCK";
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("ClockType", clock_type);
    });
    addAction(action_text);
    QAction *action_digital = new QAction("数字", this);
    connect(action_digital, &QAction::triggered, [=](){
        clock_type = "DIGITAL_CLOCK";
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("ClockType", clock_type);
    });
    addAction(action_digital);
    QAction *action_analog = new QAction("模拟", this);
    connect(action_analog, &QAction::triggered, [=](){
        clock_type = "ANALOG_CLOCK";
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("ClockType", clock_type);
    });
    addAction(action_analog);
    setContextMenuPolicy(Qt::ActionsContextMenu);
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
        QTime time = QTime::currentTime();
        int hour = time.hour();
        int m = time.minute();
        int s = time.second();
        // face
        painter.setPen(QPen(Qt::black, 2));
        painter.setBrush(QBrush(Qt::white));
        painter.drawEllipse(QPoint(w/2,h/2), (int)(w/2*0.9), (int)(h/2*0.9));
        painter.setBrush(QBrush(Qt::black));
        painter.drawEllipse(QPoint(w/2,h/2), (int)(w*0.015), (int)(h*0.015));
        // mark
        qreal da = 2 * M_PI / 60;
        for(int i=0; i<12; i++){
            int r = w * 0.415;
            //if(i % 5 == 0) r = w * 0.375;
            int x1 = r * qCos(M_PI/2 - i * da * 60 / 12) + w/2;
            int y1 = - r * qSin(M_PI/2 - i * da * 60 / 12) + h/2;
            int x2 = w * 0.425 * qCos(M_PI/2 - i * da* 60 / 12) + w/2;
            int y2 = - h * 0.425 * qSin(M_PI/2 - i * da* 60 / 12) + h/2;
            painter.setPen(QPen(Qt::black,1));
            painter.drawLine(QPoint(x1,y1), QPoint(x2,y2));
        }
        // second hand
        int x = w * 0.4 * qCos(M_PI/2 - s * da) + w / 2;
        int y = - h * 0.4 * qSin(M_PI/2 - s * da) + h / 2;
        painter.setPen(QPen(Qt::black, 1));
        painter.drawLine(QPoint(w/2,h/2), QPoint(x,y));
        // minute hand
        x = w * 0.35 * qCos(M_PI/2 - m * da - s * da / 60) + w / 2;
        y = - h * 0.35 * qSin(M_PI/2 - m * da - s * da / 60) + h / 2;
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(QPoint(w/2,h/2), QPoint(x,y));
        // hour hand
        da = 2 * M_PI / 12;
        if(hour >= 12) hour -= 12;
        x = w*0.25 * qCos(M_PI/2 - hour * da - m * da / 60) + w/2;
        y = - h*0.25 * qSin(M_PI/2 - hour * da - m * da / 60) + h/2;
        //qDebug() << "x =" << x << ", y =" << y;
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(QPoint(w/2,h/2), QPoint(x,y));
    }
}