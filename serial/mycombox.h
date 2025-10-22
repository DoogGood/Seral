#ifndef MYCOMBOX_H
#define MYCOMBOX_H

#include <QWidget>
#include <QComboBox>

class MyComBox : public QComboBox
{
    Q_OBJECT
public:
    MyComBox(QWidget*parent);
protected:
    void mousePressEvent(QMouseEvent*e) override;
signals:
    void refresh();
};

#endif // MYCOMBOX_H
