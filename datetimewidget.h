#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>

class DatetimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DatetimeWidget(QWidget *parent = 0);

private:
//    enum {
//        TEXT_CLOCK,
//        DIGITAL_CLOCK,
//        ANALOG_CLOCK
//    } clock_type;
    QString clock_type;
    bool isMouseOn = false;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

signals:

public slots:

};

#endif // DATETIMEWIDGET_H