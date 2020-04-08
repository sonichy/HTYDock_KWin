#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define DOCK Qt::UserRole

#include "datetimewidget.h"
#include <QMainWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QSettings>
#include <KF5/KWindowSystem/netwm_def.h>

struct Dock : QObjectUserData
{
    WId wid;
    QString desktopPath;
    QString name;
    QString className;
    QString iconName;
    QString exec;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum {
        BIG,
        MEDIUM,
        SMALL
    } dock_size;
    int h;
    QString qss, dir_trash, mode, position;
    QWidget *widget_app;
    QPushButton *pushButton_launcher, *pushButton_trash, *pushButton_desktop;
    QBoxLayout *boxLayout, *boxLayout_app;
    QSize size;
    QButtonGroup *buttonGroup;
    NET::Properties properties = NET::WMState | NET::XAWMState | NET::WMDesktop |  NET::WMVisibleName | NET::WMGeometry | NET::WMWindowType;
    NET::Properties2 properties2 = NET::WM2WindowClass | NET::WM2AllowedActions | NET::WM2DesktopFileName;
    QList<Dock*> list_dock;
    QSettings settings;
    DatetimeWidget *datetimeWidget;
    void addMenus();
    void refit();
    void resizeIcon(int w);

private slots:
    void buttonClicked(QAbstractButton *button);
    void trashChanged(QString path);
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void activeWindowChanged(WId wid);
    void windowChanged(WId wid, NET::Properties properties, NET::Properties2 properties2);

};

#endif // MAINWINDOW_H