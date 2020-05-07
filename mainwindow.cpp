#include "mainwindow.h"
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
#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/KWindowEffects>
#include <KF5/KWindowSystem/netwm.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    int w = settings.value("IconSize", 40).toInt();
    mode = settings.value("Mode", "Fashion").toString();
    position = settings.value("Position", "Bottom").toString();
    size.setWidth(w);
    size.setHeight(w);
    h = w + 20;
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
    QBoxLayout::Direction direction;
    if (position == "Top" && position == "Bottom") {
        direction = QBoxLayout::LeftToRight;
    } else if (position == "Left" && position == "Right") {
        direction = QBoxLayout::TopToBottom;
    }
    boxLayout = new QBoxLayout(direction);
    boxLayout->setMargin(0);
    widget->setLayout(boxLayout);
    setCentralWidget(widget);

    pushButton_launcher = new QPushButton(QIcon(":/launcher.png"), NULL);
    pushButton_launcher->setFixedSize(size + QSize(6,6));
    pushButton_launcher->setIconSize(size);
    pushButton_launcher->setToolTip("启动器");
    connect(pushButton_launcher, &QPushButton::pressed, [](){
        QProcess::startDetached("HTYStartMenu");
    });
    boxLayout->addWidget(pushButton_launcher);

    widget_app = new QWidget;
    boxLayout_app = new QBoxLayout(direction);
    boxLayout_app->setMargin(0);
    widget_app->setLayout(boxLayout_app);
    boxLayout->addWidget(widget_app);

    boxLayout->addStretch();

    //pushButton_trash = new QPushButton(QIcon::fromTheme("user-trash"), NULL);
    //pushButton_trash->setFixedSize(size + QSize(6,6));
    //pushButton_trash->setIconSize(size);
    //connect(pushButton_trash, &QPushButton::pressed, [](){
         //QDesktopServices::openUrl(QUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/Trash/files")); //真实地址
    //    QProcess::startDetached("gio", QStringList() << "open" << "trash:///");
    //});
    trashWidget = new TrashWidget;
    trashWidget->setFixedSize(size + QSize(6,6));
    trashWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(trashWidget, &QPushButton::customContextMenuRequested, [=](){
        //[&](const QPoint& pos)
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
        //menu->exec(pushButton_trash->mapToGlobal(pos));
        int x1, y1;
        if (position == "Top") {
            x1 = QPoint(trashWidget->mapToGlobal(QPoint(0,0))).x() + trashWidget->width()/2 - menu->width()/2;
            y1 = height();
        } else if (position == "Bottom") {
            x1 = QPoint(trashWidget->mapToGlobal(QPoint(0,0))).x() + trashWidget->width()/2 - menu->width()/2;
            y1 = y() - menu->height();
        } else if (position == "Left") {
            x1 = width();
            y1 = QPoint(trashWidget->mapToGlobal(QPoint(0,0))).y() + trashWidget->height()/2 - menu->height()/2;
        } else if (position == "Right") {
            x1 = QApplication::desktop()->width() - width() - menu->width();
            y1 = QPoint(trashWidget->mapToGlobal(QPoint(0,0))).y() + trashWidget->height()/2 - menu->height()/2;
        }
        menu->exec(QPoint(x1, y1));
    });
    boxLayout->addWidget(trashWidget);
    QFileSystemWatcher *FSW = new QFileSystemWatcher;
    dir_trash = QDir::homePath() + "/.local/share/Trash/files";
    qDebug() << dir_trash;
    FSW->addPath(dir_trash);
    connect(FSW, SIGNAL(directoryChanged(QString)), this, SLOT(trashChanged(QString)));
    trashChanged("");

    datetimeWidget = new DatetimeWidget;
    datetimeWidget->setFixedSize(size + QSize(6,6));
    boxLayout->addWidget(datetimeWidget);

    netSpeedWidget = new NetSpeedWidget;
    netSpeedWidget->setFixedSize(size + QSize(6,6));
    boxLayout->addWidget(netSpeedWidget);

    pushButton_desktop = new QPushButton(QIcon::fromTheme("computer"), NULL);
    pushButton_desktop->setFixedSize(size + QSize(6,6));
    pushButton_desktop->setIconSize(size);
    pushButton_desktop->setToolTip("显示桌面");
    connect(pushButton_desktop, &QPushButton::pressed, [](){
        KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
    });
    boxLayout->addWidget(pushButton_desktop);

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &MainWindow::windowAdded);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &MainWindow::windowRemoved);
    //connect(KWindowSystem::self(), static_cast<void(KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>(&KWindowSystem::windowChanged), this, &MainWindow::windowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &MainWindow::activeWindowChanged);

    addMenus();
    refit();
}

MainWindow::~MainWindow()
{

}

void MainWindow::addMenus()
{
    //右键菜单
    QAction *action = new QAction("模式", this);
    QMenu *menu = new QMenu;
    QAction *action_mode_fashion = new QAction("时尚", menu);
    QAction *action_mode_efficient = new QAction("高效", menu);
    action_mode_fashion->setCheckable(true);
    action_mode_efficient->setCheckable(true);
    if (mode == "Fashion")
        action_mode_fashion->setChecked(true);
    else if (mode == "Efficient")
        action_mode_efficient->setChecked(true);
    connect(action_mode_fashion, &QAction::triggered, [=](){
        mode = "Fashion";
        settings.setValue("Mode", mode);
        refit();
    });
    connect(action_mode_efficient, &QAction::triggered, [=](){
        mode = "Efficient";
        settings.setValue("Mode", mode);
        refit();
    });
    QActionGroup *actionGroup = new QActionGroup(this);
    menu->addAction(actionGroup->addAction(action_mode_fashion));
    menu->addAction(actionGroup->addAction(action_mode_efficient));
    action->setMenu(menu);
    addAction(action);

    action = new QAction("位置", this);
    menu = new QMenu;
    QAction *action_position_top = new QAction("上", menu);
    QAction *action_position_bottom = new QAction("下", menu);
    QAction *action_position_left = new QAction("左", menu);
    QAction *action_position_right = new QAction("右", menu);
    action_position_top->setCheckable(true);
    action_position_bottom->setCheckable(true);
    action_position_left->setCheckable(true);
    action_position_right->setCheckable(true);
    if (position == "Top")
        action_position_top->setChecked(true);
    else if (position == "Bottom")
        action_position_bottom->setChecked(true);
    else if (position == "Left")
        action_position_left->setChecked(true);
    else if (position == "Right")
        action_position_right->setChecked(true);
    connect(action_position_top, &QAction::triggered, [=](){
        position = "Top";
        settings.setValue("Position", position);
        refit();
    });
    connect(action_position_bottom, &QAction::triggered, [=](){
        position = "Bottom";
        settings.setValue("Position", position);
        refit();
    });
    connect(action_position_left, &QAction::triggered, [=](){
        position = "Left";
        settings.setValue("Position", position);
        refit();
    });
    connect(action_position_right, &QAction::triggered, [=](){
        position = "Right";
        settings.setValue("Position", position);
        refit();
    });
    actionGroup = new QActionGroup(this);
    menu->addAction(actionGroup->addAction(action_position_top));
    menu->addAction(actionGroup->addAction(action_position_bottom));
    menu->addAction(actionGroup->addAction(action_position_left));
    menu->addAction(actionGroup->addAction(action_position_right));
    action->setMenu(menu);
    addAction(action);

    action = new QAction("大小", this);
    QAction *action_size_big = new QAction("大", menu);
    QAction *action_size_medium = new QAction("中", menu);
    QAction *action_size_small = new QAction("小", menu);
    action_size_big->setCheckable(true);
    action_size_medium->setCheckable(true);
    action_size_small->setCheckable(true);
    int w = settings.value("IconSize", 40).toInt();
    if (w == 50) {
        action_size_big->setChecked(true);
    } else if (w == 40) {
        action_size_medium->setChecked(true);
    } else if (w == 30) {
        action_size_small->setChecked(true);
    }
    connect(action_size_big, &QAction::triggered, [=](){
        resizeIcon(50);
        settings.setValue("IconSize", 50);
    });
    connect(action_size_medium, &QAction::triggered, [=](){
        resizeIcon(40);
        settings.setValue("IconSize", 40);
    });
    connect(action_size_small, &QAction::triggered, [=](){
        resizeIcon(30);
        settings.setValue("IconSize", 30);
    });
    actionGroup = new QActionGroup(this);
    menu = new QMenu;
    menu->addAction(actionGroup->addAction(action_size_big));
    menu->addAction(actionGroup->addAction(action_size_medium));
    menu->addAction(actionGroup->addAction(action_size_small));
    action->setMenu(menu);
    addAction(action);

    action = new QAction("状态", this);
    QAction *action_always_show = new QAction("总是显示", menu);
    QAction *action_always_hide = new QAction("总是隐藏", menu);
    QAction *action_smart_hide = new QAction("智能隐藏", menu);
    action_always_show->setCheckable(true);
    action_always_hide->setCheckable(true);
    action_smart_hide->setCheckable(true);
    actionGroup = new QActionGroup(this);
    menu = new QMenu;
    menu->addAction(actionGroup->addAction(action_always_show));
    menu->addAction(actionGroup->addAction(action_always_hide));
    menu->addAction(actionGroup->addAction(action_smart_hide));
    action->setMenu(menu);
    addAction(action);

    action = new QAction("插件", this);
    QAction *action_plugin_trash = new QAction("回收站", menu);
    QAction *action_plugin_clock = new QAction("时钟", menu);
    QAction *action_plugin_netspeed = new QAction("网速", menu);
    action_plugin_trash->setCheckable(true);
    action_plugin_clock->setCheckable(true);
    action_plugin_netspeed->setCheckable(true);

    connect(action_plugin_trash, &QAction::triggered, [=](bool b){
        if (b) {
            trashWidget->show();
            settings.setValue("isShowTrash", true);
            count_plugin++;
        } else {
            trashWidget->hide();
            settings.setValue("isShowTrash", false);
            count_plugin--;
        }
        refit();
    });

    connect(action_plugin_clock, &QAction::triggered, [=](bool b){
        if (b){
            datetimeWidget->show();
            settings.setValue("isShowClock", true);
            count_plugin++;
        } else {
            datetimeWidget->hide();
            settings.setValue("isShowClock", false);
            count_plugin--;
        }
        refit();
    });

    connect(action_plugin_netspeed, &QAction::triggered, [=](bool b){
        if (b){
            netSpeedWidget->show();
            settings.setValue("isShowNetSpeed", true);
            count_plugin++;
        } else {
            netSpeedWidget->hide();
            settings.setValue("isShowNetSpeed", false);
            count_plugin--;
        }
        refit();
    });

    bool b = settings.value("isShowTrash", true).toBool();
    if (b) {
        action_plugin_trash->setChecked(true);
        count_plugin++;
    } else {
        trashWidget->hide();
    }

    b = settings.value("isShowClock", true).toBool();
    if (b) {
        action_plugin_clock->setChecked(true);
        count_plugin++;
    } else {
        datetimeWidget->hide();
    }

    b = settings.value("isShowNetSpeed", true).toBool();
    if (b) {
        action_plugin_netspeed->setChecked(true);
        count_plugin++;
    } else {
        netSpeedWidget->hide();
    }

    menu = new QMenu;
    menu->addAction(action_plugin_trash);
    menu->addAction(action_plugin_clock);
    menu->addAction(action_plugin_netspeed);
    action->setMenu(menu);
    addAction(action);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void MainWindow::resizeIcon(int w)
{
    size.setWidth(w);
    size.setHeight(w);
    if (w == 50)
        h = w + 20;
    else if (w == 40)
        h = w + 10;
    else if (w == 30)
        h = w + 6;
    for (int i=0; i<list_appWidget.count(); i++) {
        list_appWidget.at(i)->setFixedSize(size + QSize(w/10,w/10));
    }
    pushButton_launcher->setFixedSize(size + QSize(w/10,w/10));
    pushButton_launcher->setIconSize(size);
    trashWidget->setFixedSize(size + QSize(w/10,w/10));
    pushButton_desktop->setFixedSize(size+ QSize(w/10,w/10));
    pushButton_desktop->setIconSize(size);
    datetimeWidget->setFixedSize(size);
    netSpeedWidget->setFixedSize(size);
    refit();
}

void MainWindow::refit()
{
    qDebug() << count_plugin;
    int w, x1, y1;
    if (position == "Bottom") {
        boxLayout->setDirection(QBoxLayout::LeftToRight);
        boxLayout_app->setDirection(QBoxLayout::LeftToRight);
        if (mode == "Fashion")
            w = (list_appWidget.count() + count_plugin + 3) * size.width();
        else if (mode == "Efficient")
            w = QApplication::desktop()->width();
        int sh = size.height();
        if (sh == 50)
            h = sh + 20;
        else if (sh == 40)
            h = sh + 10;
        else if (sh == 30)
            h = sh + 6;
        x1 = QApplication::desktop()->width()/2 - w/2;
        y1 = QApplication::desktop()->height() - h;
    } else if (position == "Top") {
        boxLayout->setDirection(QBoxLayout::LeftToRight);
        boxLayout_app->setDirection(QBoxLayout::LeftToRight);
        if (mode == "Fashion")
            w = (list_appWidget.count() + count_plugin + 3) * size.width();
        else if (mode == "Efficient")
            w = QApplication::desktop()->width();
        h = size.height();
        x1 = QApplication::desktop()->width()/2 - w/2;
        y1 = 0;
    } else if (position == "Left") {
        boxLayout->setDirection(QBoxLayout::TopToBottom);
        boxLayout_app->setDirection(QBoxLayout::TopToBottom);
        w = size.width();
        if (mode == "Fashion")
            h = (list_appWidget.count() + count_plugin + 3) * size.height();
        else if (mode == "Efficient")
            h = QApplication::desktop()->height();
        x1 = 0;
        y1 = QApplication::desktop()->height()/2 - h/2;
    } else if (position == "Right") {
        boxLayout->setDirection(QBoxLayout::TopToBottom);
        boxLayout_app->setDirection(QBoxLayout::TopToBottom);
        w = size.width();
        if (mode == "Fashion")
            h = (list_appWidget.count() + count_plugin + 3) * size.width();
        else if (mode == "Efficient")
            h = QApplication::desktop()->height();
        x1 = QApplication::desktop()->width() - size.width();
        y1 = QApplication::desktop()->height()/2 - h/2;
    }
    resize(w, h);
    setFixedSize(w, h);
    move(x1, y1);
    qDebug() << position << x1 << y1 << w << h;

    //区域模糊
    QRegion region(rect());
    KWindowEffects::enableBlurBehind(winId(), true, region);

    //挤开桌面。为什么只需要3个参数？从各边减去width。
    NETExtendedStrut strut;
    if (position == "Bottom") {
        strut.bottom_width = height();
        strut.bottom_start = x();
        strut.bottom_end = x() + width();
    } else if (position == "Top") {
        strut.top_width = height();
        strut.top_start = x();
        strut.top_end = x() + width();
    } else if (position == "Left") {
        strut.left_width = width();
        strut.left_start = y();
        strut.left_end = y() + height();
    } else if (position == "Right") {
        strut.right_width = width();
        strut.right_start = y();
        strut.right_end = y() + height();
    }
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
        //qDebug() << winInfo.windowClassClass() << winInfo.windowType(NET::AllTypesMask);
        return;
    }
    qDebug() << winInfo.name() << winInfo.windowClassClass(); // >=5.29 winInfo.desktopFileName();
    if (winInfo.windowClassClass() == "")
        return;
    AppWidget *appWidget = new AppWidget;
    appWidget->setFixedSize(size);
    appWidget->wid = wid;
    appWidget->name = winInfo.name();
    appWidget->className = winInfo.windowClassClass();
    appWidget->icon = KWindowSystem::icon(wid);
    appWidget->setToolTip(winInfo.name());
    connect(appWidget, &AppWidget::clicked, [=](){
        buttonClicked(appWidget);
    });
    appWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(appWidget, &AppWidget::customContextMenuRequested, [=](){
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
        int x1, y1;
        if (position == "Top") {
            x1 = QPoint(appWidget->mapToGlobal(QPoint(0,0))).x() + appWidget->width()/2 - menu->width()/2;
            y1 = height();
        } else if (position == "Bottom") {
            x1 = QPoint(appWidget->mapToGlobal(QPoint(0,0))).x() + appWidget->width()/2 - menu->width()/2;
            y1 = y() - menu->height();
        } else if (position == "Left") {
            x1 = width();
            y1 = QPoint(appWidget->mapToGlobal(QPoint(0,0))).y() + appWidget->height()/2 - menu->height()/2;
        } else if (position == "Right") {
            x1 = QApplication::desktop()->width() - width() - menu->width();
            y1 = QPoint(appWidget->mapToGlobal(QPoint(0,0))).y() + appWidget->height()/2 - menu->height()/2;
        }
        menu->exec(QPoint(x1, y1));
    });
    boxLayout_app->addWidget(appWidget);
    list_appWidget.append(appWidget);
    refit();
}

void MainWindow::windowRemoved(WId wid)
{
    for (int i=0; i<list_appWidget.count(); i++) {
        AppWidget *appWidget = list_appWidget.at(i);
        if (appWidget->wid == wid) {
            //qDebug() << dock->wid;
            boxLayout_app->removeWidget(appWidget);
            list_appWidget.removeAt(i);
//            connect(appWidget, &AppWidget::destroyed, [=](){
//                refit();
//            });
            appWidget->deleteLater();
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
    for (int i=0; i<list_appWidget.count(); i++) {
        AppWidget *appWidget = list_appWidget.at(i);
        if (appWidget->wid == wid) {
            appWidget->isActive = true;
        } else {
            appWidget->isActive = false;
        }
        appWidget->update();
    }
}

void MainWindow::buttonClicked(AppWidget *appWidget)
{
    for (int i=0; i<list_appWidget.count(); i++) {
        list_appWidget.at(i)->isActive = false;
        list_appWidget.at(i)->update();
    }
    appWidget->isActive = true;
    appWidget->update();
    KWindowInfo winInfo(appWidget->wid, NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMVisibleName);
    appWidget->setToolTip(winInfo.name());//更新提示
    if (winInfo.isMinimized()) {
        KWindowSystem::unminimizeWindow(appWidget->wid);
        KWindowSystem::activateWindow(appWidget->wid);
    } else {
        if (KWindowSystem::activeWindow() == appWidget->wid) {
            KWindowSystem::minimizeWindow(appWidget->wid);
            for(int i=0; i<list_appWidget.count(); i++){
                list_appWidget.at(i)->setProperty("class", "");
            }
            setStyleSheet(qss);
        } else {
            KWindowSystem::activateWindow(appWidget->wid);
        }
    }
}

void MainWindow::trashChanged(QString path)
{
    Q_UNUSED(path);
    QDir dir(dir_trash);
    int count = dir.entryList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot).count();
    if (count == 0) {
        trashWidget->setToolTip("回收站");
        trashWidget->pixmap_icon = QIcon::fromTheme("user-trash").pixmap(trashWidget->size());
    } else {
        trashWidget->setToolTip("回收站 - " + QString::number(count) + "个文件");
        trashWidget->pixmap_icon = QIcon::fromTheme("user-trash-full").pixmap(trashWidget->size());
    }
    trashWidget->pixmap = trashWidget->pixmap_icon;
    trashWidget->update();
}