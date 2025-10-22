#ifndef CUSTOMTHREAD_H
#define CUSTOMTHREAD_H

#include <QThread>
#include <QWidget>

class CustomThread : public QThread
{   Q_OBJECT
protected:
    void run() override;
public:
    CustomThread(QWidget*parent);
signals:
    void ThreadTimeOut();
};

#endif // CUSTOMTHREAD_H
