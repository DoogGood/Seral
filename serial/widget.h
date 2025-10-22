#ifndef WIDGET_H
#define WIDGET_H

#include <QSerialPort>
#include <QTimer>
#include <QWidget>
#include <qpushbutton.h>
#include<customthread.h>
#include <QLineEdit>
#include <QCheckBox>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    //打开关闭串口
    void on_btnCloseOrOpen_clicked();

    //发送按钮按下
    void on_btnSend_clicked();

    //接收串口收到数据
    void my_SerialData_readyToRead();

    //定时发送选项
    void on_checkBoxSendInTime_clicked(bool checked);

    //清空接收内容
    void on_btnRevClear_clicked();

    //保存接收内容
    void on_btnRevSave_clicked();

    //时钟刷新
    void time_reflash();

    //Hex显示按钮
    void on_checkBoxHexDisplay_clicked(bool checked);

    void on_btnHideWidget_clicked(bool checked);

    void on_btnHideHistory_clicked(bool checked);

    void refreshSerialName();

    //多文本控件
    void my_command_button_clicked();

    void on_checkBox_10_clicked(bool checked);

    void buttons_handler();
    void on_btnInit_clicked();

    void on_btnSave_clicked();

    void on_btnLoad_clicked();

private:
    Ui::Widget *ui;
    QSerialPort* serialPort_send;
    QSerialPort* serialPort_rev;
    qint64 WriteCntTotal;
    int readCntTotal;
    QString sendBack;
    QString myTime;
    bool SerialStatus;

    QTimer* timer;
    QTimer* buttonsConTimer;

    QList<QPushButton*> buttons;
    int buttonsIndex;
    void getSystemTime();

    CustomThread* myThread;


    QList<QLineEdit*>lineEdits;
    QList<QCheckBox*>checkBoxs;



};

#endif // WIDGET_H
