#include "widget.h"
#include "ui_widget.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QThread>
#include"mycombox.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    //界面初始化
    ui->setupUi(this);
    this->setLayout(this->ui->gridLayoutGlobal);
    WriteCntTotal=0;
    readCntTotal=0;
    buttonsIndex=0;
    sendBack=nullptr;
    SerialStatus=false;
    ui->comboBox_baudrate->setCurrentIndex(3);
    ui->comboBox_stopbit->setCurrentIndex(1);
    ui->comboBox_databit->setCurrentIndex(3);

    //定时器
    timer = new QTimer(this);
    QTimer* SystemTime=new QTimer(this);
    SystemTime->start(1000);

    //    定时器的方法实现多文本的轮询发送
    //    buttonsConTimer=new QTimer(this);
    //    connect(buttonsConTimer,&QTimer::timeout,this,&Widget::buttons_handler);

    //    线程方式实现
    myThread=new CustomThread(this);
    connect(myThread,&CustomThread::ThreadTimeOut,this,&Widget::buttons_handler);


    //把serialPort对象挂在在widget对象树下
    serialPort_send=new QSerialPort(this);
    serialPort_rev=new QSerialPort(this);

    //初始化接收串口COM1:
    serialPort_rev->setPortName("COM1");
    serialPort_rev->setBaudRate(9600);
    serialPort_rev->setDataBits(QSerialPort::DataBits(8));
    serialPort_rev->setParity(QSerialPort::NoParity);
    serialPort_rev->setStopBits(QSerialPort::OneStop);
    serialPort_rev->setFlowControl(QSerialPort::NoFlowControl);
    serialPort_rev->open(QIODevice::ReadWrite);
    qDebug()<<serialPort_rev->portName()<<"当前接收串口状态："<<serialPort_rev->isOpen();
    //

    //连接好串口与信号槽
    //接收串口收到数据
    connect(serialPort_rev,&QSerialPort::readyRead,this,&Widget::my_SerialData_readyToRead);

    connect(ui->comboBox_serialNum,&MyComBox::refresh,this,&Widget::refreshSerialName);

    connect(timer,&QTimer::timeout,[this](){
        on_btnSend_clicked();
    });

    connect(SystemTime,&QTimer::timeout,[this](){
        time_reflash();
    });

    //    将9个多文本按钮控件clicked信号全部连接上on_command_button_clicked函数

    for(int i=1;i<=9;++i){
        QString btnname=QString("pushButton_%1").arg(i);
        QPushButton* btn= findChild<QPushButton*>(btnname);
        if(btn!=nullptr){
            btn->setProperty("buttonID",i);
            buttons.append(btn);
            //QT4老语法
            //            connect(btn,SIGNAL(clicked()),this,SLOT(my_command_button_clicked()));
            connect(btn,&QPushButton::clicked,this,&Widget::my_command_button_clicked);
        }
        //        lineEdit的数组
        QString LineEditName=QString("lineEdit_%1").arg(i);
        QLineEdit* LineEdit=findChild<QLineEdit*>(LineEditName);
        lineEdits.append(LineEdit);

        //        CheckBox的数组
        QString CheckBoxName=QString("checkBox_%1").arg(i);
        QCheckBox* CheckBox=findChild<QCheckBox*>(CheckBoxName);
        checkBoxs.append(CheckBox);

    }

    //SIGNAL和SLOT是QT4老语法,必须要加();

    //启动后检测系统可用串口
    refreshSerialName();


}

Widget::~Widget()
{
    delete ui;
}

/*
    enum Parity {
        NoParity = 0,
        EvenParity = 2,
        OddParity = 3,
        SpaceParity = 4,
        MarkParity = 5,
        UnknownParity = -1
    };
    Q_ENUM(Parity)

*/

void Widget::on_btnCloseOrOpen_clicked()
{

    //按下关闭串口时
    if(SerialStatus==true){
        serialPort_send->close();
        SerialStatus=false;
        ui->groupBoxParams->setEnabled(true);
        ui->btnCloseOrOpen->setText("打开串口");
        ui->labelSendStatus->setText("Closed");
        ui->btnSend->setEnabled(false);
        ui->checkBoxSendInTime->setEnabled(false);
        ui->checkBox_10->setEnabled(false);
        ui->checkBoxSendInTime->setCheckState(Qt::Unchecked);
        ui->checkBox_10->setCheckState(Qt::Unchecked);
        //        timer->stop();
        on_checkBox_10_clicked(false);
        on_checkBoxSendInTime_clicked(false);

        qDebug()<<serialPort_send->portName()<<"当前状态："<<serialPort_send->isOpen();
        return;
    }

    //1.选择端口号
    serialPort_send->setPortName(ui->comboBox_serialNum->currentText());
    qDebug()<<"PortName:"<<serialPort_send->portName();

    //2.配置波特率
    serialPort_send->setBaudRate(ui->comboBox_baudrate->currentText().toInt());
    qDebug()<<"BaudRate:"<<serialPort_send->baudRate();

    //3.配置数据位
    serialPort_send->setDataBits(QSerialPort::DataBits( ui->comboBox_databit->currentText().toUInt()));
    qDebug()<<serialPort_send->dataBits();

    //4.配置校验位
    switch (ui->comboBox_jiaoyan->currentIndex()) {
    case 0:
        serialPort_send->setParity(QSerialPort::NoParity);
        break;
    case 1:
        serialPort_send->setParity(QSerialPort::EvenParity);
        break;
    case 2:
        serialPort_send->setParity(QSerialPort::MarkParity);
        break;
    case 3:
        serialPort_send->setParity(QSerialPort::OddParity);
        break;
    case 4:
        serialPort_send->setParity(QSerialPort::SpaceParity);
        break;
    default:
        serialPort_send->setParity(QSerialPort::UnknownParity);
        break;
    }

    //5.配置停止位
    //error

    serialPort_send->setStopBits(QSerialPort::StopBits(ui->comboBox_stopbit->currentIndex()));
    qDebug()<<serialPort_send->stopBits();

    //6.流控
    if(ui->comboBox_filecon->currentText()=="NONE"){
        serialPort_send->setFlowControl(QSerialPort::NoFlowControl);
    }

    //7.打开串口
    if(serialPort_send->open(QIODevice::ReadWrite)){
        qDebug()<<"Open success!";
        ui->labelSendStatus->setText(serialPort_send->portName()+" Open");
        ui->groupBoxParams->setEnabled(false);
        ui->btnCloseOrOpen->setText("关闭串口");
        ui->btnSend->setEnabled(true);
        ui->checkBoxSendInTime->setEnabled(true);
        ui->checkBox_10->setEnabled(true);
        SerialStatus=true;
    }else{
        QMessageBox msgbox;
        msgbox.setWindowTitle("打开窗口失败！");
        msgbox.setText("串口可能被占用！");
        msgbox.exec();

    }

}

void Widget::on_btnSend_clicked()
{
    qint64 WriteCnt;
    const char* senddata=ui->lineEditSend->text().toLocal8Bit().constData();

    qDebug()<<"Senddata:"<<senddata;

    if(ui->checkBoxHexSend->isChecked()){
        QString tmp=ui->lineEditSend->text();
        //判断是否偶数位
        QByteArray TmpArray= tmp.toLocal8Bit();

        if(TmpArray.size()%2!=0){
            //不被2整除
            ui->labelSendStatus->setText("Error Input");
            return;

        }
        //判断发送内容编辑框是否符合HEX进制表达
        for(char c:TmpArray){
            //标准库检查是否标准的16进制数据
            if(!std::isxdigit(c)){
                ui->labelSendStatus->setText("Error Input");
                return;
            }
        }

        //转化成16进制发送，0123456789ABCDEF
        QByteArray arraySend=QByteArray::fromHex(TmpArray);
        WriteCnt=serialPort_send->write(arraySend);
    }else {
        //如果发送失败，返回值是-1,成功返回写入的字节数量
        WriteCnt=serialPort_send->write(senddata);
    }

    if(WriteCnt==-1){
        ui->labelSendStatus->setText("Send Fail!");
        return;
    }else{
        WriteCntTotal+=WriteCnt;
        ui->labelSentcnt->setText("Sent:"+QString::number( WriteCntTotal));
        ui->labelSendStatus->setText("Send OK!");

        if(strcmp(senddata,sendBack.toStdString().c_str())!=0){
            sendBack=QString::fromUtf8(senddata);
            ui->textEditRecord->append(QString(senddata));
        }

    }

}

void Widget::my_SerialData_readyToRead()
{
    QString revMessage =serialPort_rev->readAll();
    if(revMessage==nullptr){
        return;
    }
    //    if(revMessage!=nullptr){
    //        if(ui->checkBoxHexDisplay->isChecked()){
    //        }
    //        QByteArray qbytehexTmp = revMessage.toUtf8().toHex();
    //        //原来旧的内容hex类型
    //        QString tmpString = ui->textEditRev->toPlainText();
    //        qbytehexTmp=tmpString.toUtf8()

    qDebug()<<"receive:"<<revMessage;

    readCntTotal+=revMessage.size();

    ui->labelRevcnt->setText("Received:"+QString::number(readCntTotal));

    //自动换行选项勾选
    if(ui->checkBoxLine->isChecked()){
        revMessage+="\r\n";
    }

    //接收时间选项勾选
    if(ui->checkBoxRevTime->isChecked()){

        ui->textEditRev->insertPlainText("【"+myTime+"】");
    }

    ui->textEditRev->insertPlainText(revMessage);

    ui->textEditRev->moveCursor(QTextCursor::End);
    ui->textEditRev->ensureCursorVisible();


}

void Widget::on_checkBoxSendInTime_clicked(bool checked)
{

    if(SerialStatus&&checked){
        ui->btnSend->setEnabled(false);
        ui->lineEditSend->setEnabled(false);
        ui->lineEditSendInTime->setEnabled(false);
        timer->start(ui->lineEditSendInTime->text().toInt());
        //定时发送：调用send按钮按下函数
        on_btnSend_clicked();
    }else{
        ui->lineEditSendInTime->setEnabled(true);
        ui->lineEditSend->setEnabled(true);
        timer->stop();
        if(SerialStatus==false){
            ui->btnSend->setEnabled(false);
            return;
        }
        ui->btnSend->setEnabled(true);

    }
}

void Widget::on_btnRevClear_clicked()
{
    ui->textEditRev->clear();
}

//IO文件操作
void Widget::on_btnRevSave_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save RevFile"),"C:/",tr("Text(*.txt)"));

    if(fileName!=nullptr){
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){
            return;
        }

        QTextStream out(&file);
        out<<ui->textEditRev->toPlainText();
        file.close();

    }
}

void Widget::time_reflash()
{
    getSystemTime();
    //    qDebug()<<myTime;
    ui->labelCurrentTime->setText(myTime);

}

void Widget::getSystemTime()
{
    QDateTime currentTime =QDateTime::currentDateTime();
    QDate date=currentTime.date();
    int year=date.year();
    int month=date.month();
    int day=date.day();

    QTime time=currentTime.time();
    int hour=time.hour();
    int min=time.minute();
    int second=time.second();
    myTime=QString("%1-%02-%03 %04:%05:%06")
            .arg(year,2,10,QChar('0'))
            .arg(month,2,10,QChar('0'))
            .arg(day,2,10,QChar('0'))
            .arg(hour,2,10,QChar('0'))
            .arg(min,2,10,QChar('0'))
            .arg(second,2,10,QChar('0'));

}

void Widget::on_checkBoxHexDisplay_clicked(bool checked)
{
    if(checked){
        //1.读取textedit上的内容
        QString strTmp= ui->textEditRev->toPlainText();

        //2.转化成HEX
        QByteArray qByteTmp=strTmp.toUtf8();
        qByteTmp=qByteTmp.toHex();
        //3.显示
        QString lastShow;
        strTmp = QString::fromUtf8(qByteTmp).toUpper();
        for (int i=0;i<strTmp.size();i+=2) {
            lastShow+=strTmp.mid(i,2)+' ';
        }

        ui->textEditRev->setText(lastShow);
    }else{

        QString strtmp=ui->textEditRev->toPlainText();
        qDebug()<<strtmp;
        QByteArray qbyteHexTmp=strtmp.toUtf8();
        qDebug()<<qbyteHexTmp;

        QByteArray qbyteTmp = QByteArray::fromHex(qbyteHexTmp);
        qDebug()<<qbyteTmp;
        ui->textEditRev->setText(qbyteTmp);

    }
}


void Widget::on_btnHideWidget_clicked(bool checked)
{
    if(checked){
        ui->btnHideWidget->setText("显示面板");
        ui->groupBoxText->hide();


    }else{
        ui->btnHideWidget->setText("隐藏面板");
        ui->groupBoxText->show();


    }


}

void Widget::on_btnHideHistory_clicked(bool checked)
{
    qDebug()<<qobject_cast<QPushButton*>(sender());

    if(checked){
        ui->btnHideHistory->setText("显示历史");
        ui->groupBoxRecord->hide();
    }else{
        ui->btnHideHistory->setText("隐藏历史");
        ui->groupBoxRecord->show();

    }
}

void Widget::refreshSerialName()
{
    ui->comboBox_serialNum->clear();
    QList<QSerialPortInfo> SerialList=QSerialPortInfo::availablePorts();
    for(QSerialPortInfo SerialInfo :SerialList){
        ui->comboBox_serialNum->addItem(SerialInfo.portName());
    }

    ui->labelSendStatus->setText("COM Refreshed!");

}

void Widget::my_command_button_clicked()
{

    QPushButton*btn=qobject_cast<QPushButton*>(sender());
    int num=btn->property("buttonID").toInt();

    qDebug()<<num;

    QString LineEditName=QString("lineEdit_%1").arg(num);

    QLineEdit* LineEdit=findChild<QLineEdit*>(LineEditName);

    if(LineEdit->text().size()<=0){
        return;

    }
    ui->lineEditSend->setText(LineEdit->text());



    QString checkboxName=QString("checkBox_%1").arg(num);
    QCheckBox* checkbox=findChild<QCheckBox*>(checkboxName);
    ui->checkBoxSendInTime->setChecked(checkbox->isChecked());




    on_btnSend_clicked();

}


void Widget::buttons_handler(){

    if(buttonsIndex<buttons.size()){
        QPushButton* btn=buttons[buttonsIndex];
        emit btn->clicked();
        buttonsIndex++;
    }else{
        buttonsIndex=0;
    }

    //        QThread::msleep(ui->spinBox->text().toUInt());不能在UI线程中延时，会导致页面刷新问题


}

//多文本循环发送checkbox被勾选槽函数

void Widget::on_checkBox_10_clicked(bool checked)
{
    if(checked){
        ui->spinBox->setEnabled(false);
        //        buttonsConTimer->start(ui->spinBox->text().toInt());
        myThread->start();
    }else{
        ui->spinBox->setEnabled(true);
        //        buttonsConTimer->stop();
        myThread->terminate();
        buttonsIndex=0;
    }


}


void Widget::on_btnInit_clicked()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("提示");
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setText("重置列表不可逆，确认是否重置！");

    //    msgbox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    QPushButton*yesButton= msgbox.addButton("是",QMessageBox::YesRole);
    msgbox.addButton("否",QMessageBox::NoRole);
    msgbox.exec();
    if(msgbox.clickedButton()==yesButton){

        //        遍历lineEdit，并且清空
        for(int i=0;i<lineEdits.size();i++){
            lineEdits[i]->clear();
            checkBoxs[i]->setChecked(false);
        }

        //        遍历checkbox，同时取消勾选



    }



}

void Widget::on_btnSave_clicked()
{

    QString filename=QFileDialog::getSaveFileName(this,tr("保存配置"),"D:/",tr("txt(*.txt)"));
    QFile file(filename);
    if(!file.open(QIODevice::Text|QIODevice::WriteOnly)){
        return;
    }
    QTextStream out(&file);

    for(int i=0;i<lineEdits.size();i++){
        out<<checkBoxs[i]->isChecked()<<"---"<<lineEdits[i]->text()<<"\n";
    }
    file.close();


}

void Widget::on_btnLoad_clicked()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("载入文件"),"D:/",tr("txt(*.txt)"));
    if(filename==nullptr){
        return;
    }
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        return;
    }
    int i=0;

    QTextStream in(&file);

    while(!in.atEnd()&&i<9){
        qDebug()<<i;
        QString line=in.readLine();
        QStringList parts = line.split("---");

        if(parts.count()==2){
            checkBoxs[i]->setChecked(parts[0].toInt());
            lineEdits[i]->setText(parts[1]);
        }
        i++;

    }

}
