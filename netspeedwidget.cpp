#include "netspeedwidget.h"
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QTime>
#include <QPainter>
#include <QProcess>
#include <QDesktopWidget>
#include <KF5/KWindowSystem/KWindowEffects>

NetSpeedWidget::NetSpeedWidget(QWidget *parent) : QWidget(parent)
{
    i=db=ub=dbt=ubt=dbt1=ubt1=dbt0=ubt0=0;

    // Boot time
    QProcess *process = new QProcess;
    process->start("systemd-analyze");
    process->waitForFinished();
    QString PO = process->readAllStandardOutput();
    QString SD = PO.mid(PO.indexOf("=") + 1, PO.indexOf("\n") - PO.indexOf("=") - 1);
    startup = "SUT: " + SD;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    label = new QLabel;
    label->setMargin(5);
    label->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
}

void NetSpeedWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = true;
    update();
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString position = settings.value("Position", "Bottom").toString();
    int x1=0, y1=0;
    if (position == "Bottom") {
        x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - label->width()/2;
        y1 = mapToGlobal(QPoint(0,0)).y() - label->height();
    } else if (position == "Top") {
        x1 = mapToGlobal(QPoint(0,0)).x() + width()/2 - label->width()/2;
        y1 = height();
    } else if (position == "Left") {
        x1 = width();
        y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - label->height()/2;
    } else if (position == "Right") {
        x1 = QApplication::desktop()->width() - width() - label->width();
        y1 = mapToGlobal(QPoint(0,0)).y() + height()/2 - label->height()/2;
    }
    label->move(x1, y1);
    label->show();
}

void NetSpeedWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = false;
    update();
    label->hide();
}

void NetSpeedWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    bool b = settings.value("isShowNetSpeed", true).toBool();
    if (!b)
        return;

    // uptime
    QFile file("/proc/uptime");
    file.open(QIODevice::ReadOnly);
    QString l = file.readLine();
    file.close();
    QTime t(0,0,0);
    t = t.addSecs(l.left(l.indexOf(".")).toInt());
    QString uptime = "UTM: " + t.toString("hh:mm:ss");

    // memory
    file.setFileName("/proc/meminfo");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    long mt = l.replace("MemTotal:","").replace("kB","").replace(" ","").toLong();
    l = file.readLine();
    l = file.readLine();
    long ma = l.replace("MemAvailable:","").replace("kB","").replace(" ","").toLong();
    l = file.readLine();
    l = file.readLine();
    file.close();
    long mu = mt - ma;
    int mp = static_cast<int>(mu*100/mt);
    QString mem = "MEM: " + QString("%1 / %2 = %3").arg(KB(mu)).arg(KB(mt)).arg(QString::number(mp) + "%");

    // CPU
    file.setFileName("/proc/stat");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    QByteArray ba;
    ba = l.toLatin1();
    const char *ch;
    ch = ba.constData();
    char cpu[5];
    long user,nice,sys,idle,iowait,irq,softirq,tt;
    sscanf(ch,"%s%ld%ld%ld%ld%ld%ld%ld",cpu,&user,&nice,&sys,&idle,&iowait,&irq,&softirq);
    tt = user + nice + sys + idle + iowait + irq + softirq;
    file.close();
    QString cusage = "";
    int cp = 0;
    if (tt != tt0)
        cp = static_cast<int>(((tt-tt0)-(idle-idle0))*100/(tt-tt0));
    if (i > 0)
        cusage = "CPU: " + QString::number(cp) + "%";
    idle0 = idle;
    tt0 = tt;

    // net
    file.setFileName("/proc/net/dev");
    file.open(QIODevice::ReadOnly);
    l = file.readLine();
    l = file.readLine();
    dbt1 = ubt1 = 0;
    while(!file.atEnd()){
        l = file.readLine();
        QStringList list = l.split(QRegExp("\\s{1,}"));
        db = list.at(1).toLong();
        ub = list.at(9).toLong();
        dbt1 += db;
        ubt1 += ub;
    }
    file.close();
    QString dss = "";
    QString uss = "";
    if (i > 0) {
        long ds = dbt1 - dbt0;
        long us = ubt1 - ubt0;
        dss = NB(ds) + "/s";
        uss = NB(us) + "/s";
        dbt0 = dbt1;
        ubt0 = ubt1;
    }
    QString netspeed = "↑" + uss + "\n↓" + dss;
    QString net = "UPB: " + BS(ubt1) + "  " + uss + "\nDNB: " + BS(dbt1) + "  " + dss;

    i++;
    if (i>2) i = 2;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //mouseover
    if (isMouseOn) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255,255,255,30));
        painter.drawRoundRect(rect(), 50, 50);
    }
    //内存使用率竖条
    painter.setPen(Qt::gray);
    if (mp < 90) {
        painter.setBrush(QColor(0,255,0,150));
    } else {
        painter.setBrush(QColor(255,0,0,150));
    }
    painter.drawRect(rect().adjusted(0,height()*(100-mp)/100,-width()/2,0));
    //CPU使用率竖条
    if (cp < 90) {
        painter.setBrush(QColor(0,255,0,150));
    } else {
        painter.setBrush(QColor(255,0,0,150));
    }
    painter.drawRect(rect().adjusted(width()/2,height()*(100-cp)/100,0,0));
    //netspeed
    painter.setPen(Qt::white);
    QFont font = this->font();
    font.setFamily("Noto Mono");
    font.setPointSize(7);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, netspeed);
    //tooltip
    label->setText(startup + "\n" + uptime + "\n" + cusage + "\n" + mem + "\n" + net);
}

QString NetSpeedWidget::KB(long k)
{
    QString s = "";
    if(k > 999999){
        s = QString::number(k/(1024*1024.0),'f',2) + "GB";
    }else{
        if(k > 999){
            s = QString::number(k/1024.0,'f',2) + "MB";
        }else{
            s = QString::number(k/1.0,'f',2) + "KB";
        }
    }
    return s;
}

QString NetSpeedWidget::NB(long b)
{
    QString s = "";
    if (b>999) {
        s = QString("%1").arg(b/1024, 5, 'f', 0, QLatin1Char(' ')) + "KB";
    } else { // <1K => 0
        s = QString("%1").arg(0, 5, 'f', 0, QLatin1Char(' ')) + "KB";
    }
    return s;
}

QString NetSpeedWidget::BS(long b)
{
    QString s = "";
    if (b > 999999999) {
        //s = QString("%1").arg(b/(1024*1024*1024.0), 6, 'f', 2, QLatin1Char(' ')) + "GB";
        s = QString::number(b/(1024*1024*1024.0), 'f', 2) + "GB";
    } else {
        if (b > 999999) {
            //s = QString("%1").arg(b/(1024*1024.0), 6, 'f', 2, QLatin1Char(' ')) + "MB";
            s = QString::number(b/(1024*1024.0), 'f', 2) + "MB";
        } else {
            if (b > 999) {
                //s = QString("%1").arg(b/1024.0, 6, 'f', 2, QLatin1Char(' ')) + "KB";
                s = QString::number(b/(1024.0), 'f',2) + "KB";
            } else {
                s = QString::number(b) + "B";
            }
        }
    }
    return s;
}