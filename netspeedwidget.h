#ifndef NETSPEEDWIDGET_H
#define NETSPEEDWIDGET_H

#include <QWidget>
#include <QLabel>

class NetSpeedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetSpeedWidget(QWidget *parent = 0);

private:
    long int i, db, ub, dbt, ubt, dbt1, ubt1, dbt0, ubt0, tt0, idle0;
    QString KB(long k);
    QString NB(long b);
    QString BS(long b);
    QString startup;
    bool isMouseOn = false;
    QLabel *label;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

signals:

public slots:

};

#endif // NETSPEEDWIDGET_H