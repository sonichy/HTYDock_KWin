#include "mainwindow.h"
#include "datetimewidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QProcess>
#include <QMenu>
#include <QDebug>
#include <QX11Info>
#include <QFileSystemWatcher>
#include <QDir>
#include <QMetaEnum>
#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/KWindowEffects>
#include <KF5/KWindowSystem/netwm.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    h = 60;
    size.setWidth(40);
    size.setHeight(40);
    qss = ".CurrentButton { background-color: rgba(48,140,198,0.5); border-radius:10px; }"
          ".CurrentButton:hover { background-color: rgba(48,140,198,0.5); }"
          "QTooltip { color: white; border: 1px solid white; background-color: white; }"
          "QPushButton { border: none; }"
          "QPushButton:hover { color: rgba(0, 0, 0, 0.1); background-color: rgba(255,255,255,0.1); border-radius:10px; }";
    setStyleSheet(qss);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    //setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    KWindowSystem::setOnDesktop(winId(), NET::OnAllDesktops);
    KWindowSystem::setType(winId(), NET::Dock);
    setFixedHeight(h);
    QWidget *widget = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    widget->setLayout(hbox);
    setCentralWidget(widget);

    QPushButton *pushButton = new QPushButton(QIcon(":/launcher.png"), NULL);
    pushButton->setFixedSize(size + QSize(6,6));
    pushButton->setIconSize(size);
    pushButton->setToolTip("启动器");
    connect(pushButton, &QPushButton::pressed, [](){
        QProcess::startDetached("HTYStartMenu");
    });
    hbox->addWidget(pushButton);

    buttonGroup = new QButtonGroup;
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
    widget_app = new QWidget;
    hbox_app = new QHBoxLayout;
    hbox_app->setMargin(0);
    widget_app->setLayout(hbox_app);
    hbox->addWidget(widget_app);

    pushButton_trash = new QPushButton(QIcon::fromTheme("user-trash"), NULL);
    pushButton_trash->setFixedSize(size + QSize(6,6));
    pushButton_trash->setIconSize(size);
    connect(pushButton_trash, &QPushButton::pressed, [](){
         //QDesktopServices::openUrl(QUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/Trash/files"));
        QProcess::startDetached("gio", QStringList() << "open" << "trash:///");
    });
    pushButton_trash->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(pushButton_trash, &QPushButton::customContextMenuRequested, [=](){
        QMenu *menu = new QMenu;
        menu->setStyleSheet("QMenu { color:white; background: rgba(0,0,0,100);}"
                            "QMenu::item:selected { background: rgba(48,140,198,255);}");
        menu->setAttribute(Qt::WA_TranslucentBackground, true);
        menu->setAutoFillBackground(true);
        QAction *action_empty_trash = new QAction("清空", this);
        connect(action_empty_trash, &QAction::triggered, [](){
            QProcess::startDetached("gio", QStringList() << "trash" << "--empty");
        });
        menu->addAction(action_empty_trash);
        menu->adjustSize();//不加会到屏幕中间
        menu->exec(QPoint(pushButton_trash->mapToGlobal(QPoint(0,0)).x() + pushButton->width()/2 - menu->width()/2, y() - menu->height()));
    });
    hbox->addWidget(pushButton_trash);
    QFileSystemWatcher *FSW = new QFileSystemWatcher;
    dir_trash = QDir::homePath() + "/.local/share/Trash/files";
    qDebug() << dir_trash;
    FSW->addPath(dir_trash);
    connect(FSW, SIGNAL(directoryChanged(QString)), this, SLOT(trashChanged(QString)));
    trashChanged("");

    DatetimeWidget *datetimeWidget = new DatetimeWidget;
    datetimeWidget->setFixedSize(size + QSize(6,6));
    hbox->addWidget(datetimeWidget);

    pushButton = new QPushButton(QIcon::fromTheme("computer"), NULL);
    pushButton->setFixedSize(size + QSize(6,6));
    pushButton->setIconSize(size);
    pushButton->setToolTip("显示桌面");
    connect(pushButton, &QPushButton::pressed, [](){
        KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
    });
    hbox->addWidget(pushButton);

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &MainWindow::windowAdded);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &MainWindow::windowRemoved);
    //connect(KWindowSystem::self(), static_cast<void(KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>(&KWindowSystem::windowChanged), this, &MainWindow::windowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &MainWindow::activeWindowChanged);
    refit();
}

MainWindow::~MainWindow()
{

}

void MainWindow::refit()
{
    qDebug() << buttonGroup->buttons().count();
    resize((buttonGroup->buttons().count() + 6) * size.width() + 6*2, h);
    move(QApplication::desktop()->width()/2 - width()/2, QApplication::desktop()->height() - height());
    QRegion region(rect());
    //区域模糊
    KWindowEffects::enableBlurBehind(winId(), true, region);
    //挤开桌面。为什么只需要3个参数？从各边减去width。
    NETExtendedStrut strut;
    strut.bottom_width = height();
    strut.bottom_start = x();
    strut.bottom_end = x() + width();
    KWindowSystem::setExtendedStrut(winId(),
                                    strut.left_width, strut.left_start, strut.left_end,
                                    strut.right_width, strut.right_start, strut.right_end,
                                    strut.top_width, strut.top_start, strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end);
}

void MainWindow::windowAdded(WId wid)
{
    KWindowInfo winInfo(wid, NET::WMWindowType | NET::WMVisibleName, NET::WM2WindowClass |NET::WM2DesktopFileName);
    //DDE 系列的值是 NET::OverrideMask
    if (!NET::typeMatchesMask(winInfo.windowType(NET::AllTypesMask), NET::NormalMask | NET::OverrideMask)) {
        //QMetaEnum metaEnum = QMetaEnum::fromType<NET::WindowTypeMask>();
        //metaEnum.valueToKey(winInfo.windowType(NET::AllTypesMask));
        qDebug() << winInfo.windowClassClass() << winInfo.windowType(NET::AllTypesMask);
        return;
    }
    qDebug() << winInfo.name() << winInfo.windowClassClass(); // >=5.29 winInfo.desktopFileName();
    if (winInfo.windowClassClass() == "")
        return;
    Dock *dock = new Dock;
    dock->wid = wid;
    dock->name = winInfo.name();
    dock->className = winInfo.windowClassClass();
    list_dock.append(dock);
    QIcon icon = QIcon::fromTheme(winInfo.windowClassClass().toLower());
    //非系统程序图标获取
    if (icon.isNull()) {
        int pid = NETWinInfo(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMPid).pid();
        QString desktop_file_path = "";
        QFile file("/proc/" + QString::number(pid) + "/environ");
        file.open(QIODevice::ReadOnly);
        QByteArray BA = file.readAll();
        file.close();
        QList<QByteArray> list_BA = BA.split('\0');
        for(int i=0; i<list_BA.length(); i++){
            if(list_BA.at(i).startsWith("GIO_LAUNCHED_DESKTOP_FILE=")){
                desktop_file_path = list_BA.at(i);
                qDebug() << desktop_file_path;
                desktop_file_path = desktop_file_path.mid(desktop_file_path.indexOf("=") + 1);
                qDebug() << desktop_file_path;
                break;
            }
        }

        QString icon_path = "";
        file.setFileName(desktop_file_path);
        file.open(QIODevice::ReadOnly);
        BA = file.readAll();
        file.close();
        list_BA = BA.split('\n');
        for(int i=0; i<list_BA.length(); i++){
            if(list_BA.at(i).toLower().startsWith("icon=")){
                icon_path = list_BA.at(i);
                qDebug() << list_BA.at(i);
                icon_path = icon_path.mid(icon_path.indexOf("=") + 1);
                qDebug() << icon_path;
                icon = QIcon(icon_path);
                break;
            }
        }
        if (icon.isNull())
            icon = QIcon::fromTheme("application-x-executable");
    }

    QPushButton *pushButton = new QPushButton(icon, NULL);
    pushButton->setFixedSize(size + QSize(6,6));
    pushButton->setIconSize(size);
    pushButton->setToolTip(winInfo.name());
    pushButton->setUserData(DOCK, dock);
    pushButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(pushButton, &QPushButton::customContextMenuRequested, [=](){
        QMenu *menu = new QMenu;
        menu->setStyleSheet("QMenu { color:white; background: rgba(0,0,0,100);}"
                            "QMenu::item:selected { background: rgba(48,140,198,255);}");
        menu->setAttribute(Qt::WA_TranslucentBackground, true);
        menu->setAutoFillBackground(true);
        QAction *action_close = new QAction("关闭", this);
        connect(action_close, &QAction::triggered, [=](){
            NETRootInfo(QX11Info::connection(), NET::CloseWindow).closeWindowRequest(wid);
        });
        menu->addAction(action_close);
        menu->adjustSize();//不加会到屏幕中间
        menu->exec(QPoint(pushButton->mapToGlobal(QPoint(0,0)).x() + pushButton->width()/2 - menu->width()/2, y() - menu->height()));
    });
    hbox_app->addWidget(pushButton);
    buttonGroup->addButton(pushButton);
    refit();
}

void MainWindow::windowRemoved(WId wid)
{
    for (int i=0; i<buttonGroup->buttons().count(); i++) {
        QAbstractButton *button = buttonGroup->buttons().at(i);
        Dock *dock = (Dock*)(button->userData(DOCK));
        if (dock->wid == wid) {
            //qDebug() << dock->wid;
            hbox_app->removeWidget(button);
            buttonGroup->removeButton(button);
//            connect(button, &QAbstractButton::destroyed, [=](){
//                refit();
//            });
            button->deleteLater();
            refit();
        }
    }
}

void MainWindow::windowChanged(WId wid, NET::Properties properties, NET::Properties2 properties2)
{
    KWindowInfo winInfo(wid, properties, properties2);
    qDebug() << winInfo.name() << winInfo.windowClassClass();
}

void MainWindow::activeWindowChanged(WId wid)
{
    for(int i=0; i<buttonGroup->buttons().count(); i++){
        buttonGroup->buttons().at(i)->setProperty("class", "");
    }
    for(int i=0; i<buttonGroup->buttons().count(); i++){
        Dock *dock = (Dock*)(buttonGroup->buttons().at(i)->userData(DOCK));
        if(dock->wid == wid)
            buttonGroup->buttons().at(i)->setProperty("class", "CurrentButton");
    }
    setStyleSheet(qss);
}

void MainWindow::buttonClicked(QAbstractButton *button)
{
    for(int i=0; i<buttonGroup->buttons().count(); i++){
        buttonGroup->buttons().at(i)->setProperty("class", "");
    }
    button->setProperty("class", "CurrentButton");
    setStyleSheet(qss);
    Dock *dock = (Dock*)(button->userData(DOCK));
    KWindowInfo winInfo(dock->wid, NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMVisibleName);
    button->setToolTip(winInfo.name());//更新提示
    if (winInfo.isMinimized()) {
        KWindowSystem::unminimizeWindow(dock->wid);
        KWindowSystem::activateWindow(dock->wid);
    } else {
        if (KWindowSystem::activeWindow() == dock->wid) {
            KWindowSystem::minimizeWindow(dock->wid);
            for(int i=0; i<buttonGroup->buttons().count(); i++){
                buttonGroup->buttons().at(i)->setProperty("class", "");
            }
            setStyleSheet(qss);
        } else {
            KWindowSystem::activateWindow(dock->wid);
        }
    }
}

void MainWindow::trashChanged(QString path)
{
    Q_UNUSED(path);
    QDir dir(dir_trash);
    int count = dir.entryList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot).count();
    if (count == 0) {
        pushButton_trash->setToolTip("回收站");
        pushButton_trash->setIcon(QIcon::fromTheme("user-trash"));
    } else {
        pushButton_trash->setToolTip("回收站 - " + QString::number(count) + "个文件");
        pushButton_trash->setIcon(QIcon::fromTheme("user-trash-full"));
    }
}