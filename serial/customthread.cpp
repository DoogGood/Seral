#include "customthread.h"
#include<QDebug>


void CustomThread::run()
{

    while(true){
        qDebug()<<"Thread start";
        emit ThreadTimeOut();
        msleep(100);
    }
}

CustomThread::CustomThread(QWidget *parent):QThread(parent)
{

}
