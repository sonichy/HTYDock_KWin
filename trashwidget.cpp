#include "trashwidget.h"
#include <QProcess>
#include <QPainter>
#include <QIcon>
#include <QMimeData>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QDir>

TrashWidget::TrashWidget(QWidget *parent) : QWidget(parent)
{
    pixmap_icon = QIcon::fromTheme("user-trash").pixmap(size());
    pixmap = pixmap_icon;
    pixmap_dragEnter = QIcon::fromTheme("user-trash-full-opened").pixmap(size());
    setAcceptDrops(true);

    QFileSystemWatcher *FSW = new QFileSystemWatcher;
    dir_trash = QDir::homePath() + "/.local/share/Trash/files";
    FSW->addPath(dir_trash);
    connect(FSW, SIGNAL(directoryChanged(QString)), this, SLOT(trashChanged(QString)));
    trashChanged("");
}

void TrashWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = true;
    update();
}

void TrashWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    isMouseOn = false;
    update();
}

void TrashWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setPen(Qt::NoPen);
    if (isMouseOn) {
        painter.setBrush(QColor(255,255,255,30));
        painter.drawRoundRect(rect(), 50, 50);
    }
    painter.drawPixmap(rect().adjusted(3,3,-3,-3), pixmap);
}

void TrashWidget::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << event->mimeData();
    if(event->mimeData()->hasFormat("text/uri-list")){
        pixmap = pixmap_dragEnter;
        update();
        event->acceptProposedAction();
    }
}

void TrashWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    pixmap = pixmap_icon;
    update();
}

void TrashWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return ;

    foreach (QUrl u, urls) {
        qDebug() << "trash" << u.toString();
        QProcess::startDetached("gio", QStringList() << "trash" << u.toString());
    }
    qDebug() << urls.size();
    trashChanged("");
}

void TrashWidget::mousePressEvent(QMouseEvent *event)
{
    point_mouse = QPoint(event->x(), event->y());
}

void TrashWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (point_mouse == QPoint(event->x(), event->y())) {
        QProcess::startDetached("gio", QStringList() << "open" << "trash:///");
    }
}

void TrashWidget::trashChanged(QString path)
{
    Q_UNUSED(path);
    QDir dir(dir_trash);
    int count = dir.entryList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot).count();
    if (count == 0) {
        setToolTip("回收站");
        pixmap_icon = QIcon::fromTheme("user-trash").pixmap(size());
    } else {
        setToolTip("回收站 - " + QString::number(count) + "个文件");
        pixmap_icon = QIcon::fromTheme("user-trash-full").pixmap(size());
    }
    pixmap = pixmap_icon;
    update();
}