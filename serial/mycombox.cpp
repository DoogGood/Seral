#include "mycombox.h"

#include <QMouseEvent>

MyComBox::MyComBox(QWidget *parent):QComboBox (parent)
{

}

void MyComBox::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton){
        emit refresh();

    }
    QComboBox::mousePressEvent(e);


}
