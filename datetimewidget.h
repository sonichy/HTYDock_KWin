#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>
#include <QCalendarWidget>
#include <QMouseEvent>

class DatetimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DatetimeWidget(QWidget *parent = 0);

private:
    QString clock_type;
    bool isMouseOn = false;
    QCalendarWidget *calendar;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:

public slots:

};

#endif // DATETIMEWIDGET_H