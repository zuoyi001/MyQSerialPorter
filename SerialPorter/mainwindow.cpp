#include "mainwindow.h"
#include "ui_mainwindow.h"

// 提取空格号前所有字符（即所需数字量）
double extract_number(const QString& str){
    QString _str = str;
//    qDebug()<<_str;
    int index = _str.indexOf(" ");
//    qDebug()<<"extra"<<index;
    QString num = _str.mid(0, index);
//    qDebug()<<"extra"<<num;
    return num.toDouble();
}

// 十六进制转十进制辅助
char ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30; //'0'的ASCII是0x30，以此类推
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10; //a表示10，b表示11，以此类推
    else return (-1);
}

//  16进制转换函数
QByteArray QString2Hex(QString str)
{
    QByteArray senddata;
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);  // 理论上，String转为HEX发送，最多仅需要原字长的1/2，如12，ASCII逐字发送为“1”、“2”共计2bits
                             // 而HEX直接发送 0C，1bit
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16) || (hexdata == -1) || (lowhexdata == -1))  // 拒绝非法字符
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        qDebug()<<"hexdata"<<hexdata;
        qDebug()<<"senddata[hexdatalen]"<<senddata[hexdatalen];
        qDebug()<<"senddata"<<senddata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);  // 考虑到空格情况，可进一步减少字长
    qDebug()<<"senddata.resize"<<senddata;
    return senddata;
}

bool check_HexData(const QString& HexData){
    QString data = HexData;
    int check = 1;
    for(int i=0;i<=data.length();i++){
        char x = data[i].toLatin1();
        check = ConvertHexChar(x);
        if(check<0)
            break;
    }
    if(check<0)
        return false;
    else
        return true;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->serial = new QSerialPort(this);


    // 串口接收设置超时处理，当接收时间大于设定值再一并取出处理
    this->serialTimer = new QTimer(this);
    this->sendTimer = new QTimer(this);
    // Disable打开串口键
    this->ui->openPushButton->setEnabled(false);
    this->ui->serialPortComboBox->clear();  // 测试用
    // 使用QMap保存串口数据位
    this->DatabitsMap.insert(5,QSerialPort::Data5);
    this->DatabitsMap.insert(6,QSerialPort::Data6);
    this->DatabitsMap.insert(7,QSerialPort::Data7);
    this->DatabitsMap.insert(8,QSerialPort::Data8);
    // 使用QMap保存串口校验方式
    this->ParityMap.insert(this->ui->checkBitComboBox->itemText(0),QSerialPort::NoParity);
    this->ParityMap.insert(this->ui->checkBitComboBox->itemText(1),QSerialPort::OddParity);
    this->ParityMap.insert(this->ui->checkBitComboBox->itemText(2),QSerialPort::EvenParity);
    this->ParityMap.insert(this->ui->checkBitComboBox->itemText(3),QSerialPort::SpaceParity);
    this->ParityMap.insert(this->ui->checkBitComboBox->itemText(4),QSerialPort::MarkParity);
    // 使用QMap保存串口停止位
    this->StopMap.insert(1.0,QSerialPort::OneStop);
    this->StopMap.insert(1.5,QSerialPort::OneAndHalfStop);
    this->StopMap.insert(2.0,QSerialPort::TwoStop);
    // 使用QMAP保存串口流控制
    this->FlowControlMap.insert(0,QSerialPort::NoFlowControl);
    this->FlowControlMap.insert(1,QSerialPort::HardwareControl);
    this->FlowControlMap.insert(2,QSerialPort::SoftwareControl);

    SerialPortInit();

    ui->serialPortComboBox->setDisabled(false);
    ui->rateComboBox->setDisabled(false);
    ui->checkBitComboBox->setDisabled(false);
    ui->dataBitComboBox->setDisabled(false);
    ui->stopBitComboBox->setDisabled(false);
    ui->flowContrlstopBitComboBox->setDisabled(false);
    ui->sendPushButton1->setEnabled(false);

    ui->autoSendCheckBox->setCheckable(false);
}


MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::readASCII(const QByteArray& serialData){
    QByteArray data = serialData;
    qDebug()<<"readASCII"<<QString(data);
    return QString(data);
}

QString MainWindow::readHEX(const QByteArray &serialData){
    QByteArray data = serialData;
    QString text = QString((data.toHex())).toUpper();
    qDebug()<<"text"<<text;
    qDebug()<<"toInt"<<(data.toHex()).toInt();
    QString textView;
    // 每一位十六进制符号用空格隔开
    for(int i = 0; i <= text.length(); i+=2){
        textView += text.mid(i,2) + QString(" ");
    }
    qDebug()<<"textvie"<<textView;
    return textView.mid(0,textView.length()-1);
}

qint64 MainWindow::writeASCII(const QString &sendData){
    QString send = sendData;
    qDebug()<<"send.toLocal8Bit(): "<<send.toLocal8Bit();
    qDebug()<<"send.toLocal8bit().data()"<<send.toLocal8Bit().data();
    return this->serial->write(send.toLocal8Bit().data());  //.tolocal8bit(),转为返回字符串的本地8位表示形式。
                                                            // 如果字符串包含本地8位编码不支持的字符，则返回的字节数组是不确定的。
                                                            // .data()，返回 char*指针
}

qint64 MainWindow::writeHEX(const QString &sendData){
    QString send = sendData;
    QByteArray sendHEX = QString2Hex(send);
    qDebug()<<"this->serial->write(sendHEX)"<<sendHEX.toHex();
    //qDebug()<<"this->serial->write(sendHEX)"<<sendHEX;
    return this->serial->write(sendHEX);
}

// 保存选择的串口设置项
void MainWindow::setSerial()
{
    //读取combox选项值
    this->SerialIndx = ui->serialPortComboBox->currentText();
    this->BaudRate = (ui->rateComboBox->currentIndex()+1)*9600;
    this->parityBit = ui->checkBitComboBox->currentText();
    this->DataBits = int(extract_number(ui->dataBitComboBox->currentText()));
    this->StopBits = extract_number(ui->stopBitComboBox->currentText());
    this->FlowContrlIndx = ui->flowContrlstopBitComboBox->currentIndex();
    //QString serial_tip = QString("正在打开串口%1\n").arg(this->SerialIndx);
    //this->ui->textBrowser->append(serial_tip);
}

// 打开串口
bool MainWindow::openSerial(){
    QMap<int, QSerialPort::DataBits> map;
    map.insert(5,QSerialPort::Data5);
    this->serial = new QSerialPort(this);
    this->serial->setPortName(this->SerialIndx);
    if(this->serial->open(QIODevice::ReadWrite)){
        // 设置串口波特率
        this->serial->setBaudRate(this->BaudRate);
        qDebug() << (QString("设定波特率： %1").arg(this->BaudRate));
        // 设置串口数据位
        this->serial->setDataBits(this->DatabitsMap[this->DataBits]);
        qDebug() << (QString("设定数据位： %1字节").arg(this->DataBits));
        // 设置串口校验位
        this->serial->setParity(this->ParityMap[this->parityBit]);
        qDebug() << (QString("设定校验位： %1").arg(this->parityBit));
        // 设置停止位
        this->serial->setStopBits(this->StopMap[this->StopBits]);
        qDebug() << (QString("设定停止位： %1字节").arg(this->StopBits));
        // 设置流控制
        this->serial->setFlowControl(this->FlowControlMap[this->FlowContrlIndx]);
        qDebug() << (QString("设定流控制： %1").arg(this->ui->flowContrlstopBitComboBox->itemText(this->FlowContrlIndx)));
        return true;
    }
    else
        return false;

}

//　初始化函数，获取当前的所有串口
void MainWindow::SerialPortInit()
{
    ui->serialPortComboBox->clear();
    foreach (const QSerialPortInfo& info, QSerialPortInfo::availablePorts()) {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite)){

            this->ui->serialPortComboBox->addItem(info.portName());

            QString isbusy;
            if(info.isBusy())
                isbusy = "Yes";
            else {
                isbusy = "No";
            }

            serial.close();
        }
    }
    if(ui->serialPortComboBox->count()==0){
        QMessageBox::warning(this, "Warning", QString("无可用串口\n请检查当前串口配置"));
        }
    else if(ui->serialPortComboBox->count()>0){

        ui->openPushButton->setEnabled(true);
    }
}

void MainWindow::on_openPushButton_clicked()
{
    if(ui->openPushButton->text()==QString("打开串口")){
        ui->openPushButton->setText(QString("关闭串口"));
        ui->openPushButton->setEnabled(true);
        // Combox控件失效

        // 保存设置选项
        MainWindow::setSerial();
        // 打开串口
        if(MainWindow::openSerial()){
            qDebug() << (QString("%1已打开\n\n").arg(this->SerialIndx));
            this->serialTimer = new QTimer(this);   // 必须新注册一个timer，用该新注册timer做处理
                                                    // 防止多次触发timeout()的事情
            // connect收到串口信号，触发处理
            QObject::connect(this->serial, SIGNAL(readyRead()),this,SLOT(timerSerialRead()));
            QObject::connect(this->serialTimer,SIGNAL(timeout()),this,SLOT(readSerialData()));
//            qDebug()<<"pinoutSignals"<<serial->pinoutSignals();

            ui->serialPortComboBox->setDisabled(true);
            ui->rateComboBox->setDisabled(true);
            ui->checkBitComboBox->setDisabled(true);
            ui->dataBitComboBox->setDisabled(true);
            ui->stopBitComboBox->setDisabled(true);
            ui->flowContrlstopBitComboBox->setDisabled(true);
            ui->sendPushButton1->setEnabled(true);

            ui->autoSendCheckBox->setCheckable(true);
        }
        else{
            //this->ui->textBrowser->append(QString("%1打开失败\n\n").arg(this->SerialIndx));
            QMessageBox::warning(this, "Warning", QString("%1打开失败\n\n").arg(this->SerialIndx) );
        }
    }

    else if(ui->openPushButton->text()==QString("关闭串口")){
        ui->openPushButton->setText(QString("打开串口"));
        this->serialTimer->stop();  // 若已回收了该指针，则无需停止，反之亦然，当然，既停止又回收也无妨
        delete this->serialTimer;  // 回收该指针，这一步非常重要。如果上述两步都没有进行，那么，当串口通讯错误而再次打开串口时
                                   // 就会一下子触发好几个timeout()信号（即原有的错误通讯也能触发timer启动，因收不到信号而无法stop
                                   // 故而timeout()信号屡屡触发
                                   // 若timer在使用时重新注册过，亦可，但最好还是要回收内存
        //this->ui->serialPortComboBox->clear();
        //SerialPortInit();
        ui->openPushButton->setEnabled(true);

//        QList<QComboBox*>::iterator i;
//        for(i=setSerialCombox.begin();i!=setSerialCombox.end();i++)
//            (*i)->setEnabled(true);
        this->serial->close();
//        delete this->serial;
        ui->serialPortComboBox->setDisabled(false);
        ui->rateComboBox->setDisabled(false);
        ui->checkBitComboBox->setDisabled(false);
        ui->dataBitComboBox->setDisabled(false);
        ui->stopBitComboBox->setDisabled(false);
         ui->flowContrlstopBitComboBox->setDisabled(false);
        ui->sendPushButton1->setEnabled(false);


        QString serial_tip = QString("串口%1已关闭\n\n").arg(this->SerialIndx);
        //this->ui->textBrowser->append(serial_tip);
        QMessageBox::warning(this, "Warning", serial_tip );

        //关闭的时候需要停掉自动发送的定时器
        this->sendTimer->stop();
        ui->autoSendCheckBox->setCheckable(false);
    }
}

// 启用定时器并接收信息
void MainWindow::timerSerialRead(){
    this->mSerialRecData.append(this->serial->readAll());
    this->serialTimer->start(100);  //100ms
    qDebug()<<"Timer start";
}

// 接收串口信息
void MainWindow::readSerialData(){
    // 停止计时器
    this->serialTimer->stop();
    QByteArray serialData = this->mSerialRecData;
    this->mSerialRecData.clear();
    QString textView;

     QDateTime currentTime = QDateTime::currentDateTime();

    //是ASCII显示还是Hex显示
    if(ui->hexShowCheckBox->isChecked() == false)
    {
        qDebug()<<"ASCII";
        textView = this->readASCII(serialData);
        ui->recvTextEdit->insertPlainText(currentTime.toString(tr("[yyyy-MM-dd hh:mm:ss][接收]"))+"[ASCII]:"+textView.toUtf8()+"\r\n");
    }
    else{
         qDebug()<<"Hex";
        textView = this->readHEX(serialData);
        ui->recvTextEdit->insertPlainText(currentTime.toString(tr("[yyyy-MM-dd hh:mm:ss][接收]"))+"[HEX]:"+textView.toUtf8()+"\r\n");
    }


    qDebug()<<"QByte"<<serialData;
    qDebug()<<"QByte to int"<<(serialData.toHex()).toInt();
    qDebug()<<"QByte to Hex"<<serialData.toHex();
    qDebug()<<"QByte to QString"<<QString(serialData);
}

void MainWindow::on_sendPushButton1_clicked()
{
    if(this->serial->isOpen()){
        QString sendData = this->ui->sendTextEdit->toPlainText();

        if(this->ui->hexSendcheckBox->isChecked() == false){
            qint64 num = this->writeASCII(sendData);
            qDebug()<<"write: "<<num;


                QDateTime currentTime = QDateTime::currentDateTime();
                ui->recvTextEdit->insertPlainText(currentTime.toString(tr("[yyyy-MM-dd hh:mm:ss][发送]"))+"[ASCII]:"+sendData.toUtf8()+"\r\n");
        }
        else{
            QString checkHexData = sendData;
            qint64 num = this->writeHEX(sendData);
            qDebug()<<"write: "<<num;

                QByteArray hexSendData = QString2Hex(sendData);
                QString viewHexData = this->readHEX(hexSendData);

                QDateTime currentTime = QDateTime::currentDateTime();
                ui->recvTextEdit->insertPlainText(currentTime.toString(tr("[yyyy-MM-dd hh:mm:ss][发送]"))+"[HEX]:"+viewHexData.toUtf8()+"\r\n");
                qDebug()<<"hexSendData"<<hexSendData;

        }
    }
    else if(!this->serial->isOpen()){
        qDebug()<<"isopen"<<this->serial->isOpen();
        QMessageBox::warning(this, "Warning", QString("需先打开串口\n方可发送信息"));
    }
}


QByteArray GetHexValue(QString str)
{
    QByteArray senddata;
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
//        hstr=str[i].toAscii();
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
//        lstr = str[i].toAscii();
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
    return senddata;
}

void MainWindow::on_hexSendcheckBox_stateChanged(int arg1)
{
    qDebug() << "hexSendcheckBox" << ui->hexSendcheckBox->checkState();

    if(ui->hexSendcheckBox->isChecked() == true)
    {
        //将ASCII转换成为Hex
        QString str=ui->sendTextEdit->toPlainText();

        QByteArray temp=str.toLatin1();//GetHexValue(str);;
        QString strHex;//16进制数据
        QDataStream out(&temp,QIODevice::ReadWrite);
        while (!out.atEnd())
        {
            qint8 outChar=0;
            out>>outChar;
            QString str=QString("%1").arg(outChar&0xFF,2,16,QLatin1Char('0'));

            if (str.length()>1)
            {
                strHex+=str+" ";
            }
            else
            {
                strHex+="0"+str+" ";
            }
        }

        ui->sendTextEdit->clear();
        ui->sendTextEdit->insertPlainText(strHex.toUpper());
    }
    else
    {
        //将Hex转换成ASCII
        QString str=ui->sendTextEdit->toPlainText();
        str = str.trimmed();    //除去前后的空格

        QByteArray outData=GetHexValue(str);

        std::string T = outData.toStdString();
        QString Ts = QString::fromStdString(T);

        qDebug() << "Text" << Ts;

        ui->sendTextEdit->clear();
        ui->sendTextEdit->insertPlainText(Ts.toUtf8());
    }
}

void MainWindow::autoSendData(){
    if(this->serial->isOpen()){
        QString sendData = this->ui->sendTextEdit->toPlainText();

        if(this->ui->hexSendcheckBox->isChecked() == false){
            qint64 num = this->writeASCII(sendData);
            qDebug()<<"write: "<<num;


                QDateTime currentTime = QDateTime::currentDateTime();
                ui->recvTextEdit->insertPlainText(currentTime.toString(tr("[yyyy-MM-dd hh:mm:ss][发送]"))+"[ASCII]:"+sendData.toUtf8()+"\r\n");
        }
        else{
            QString checkHexData = sendData;
            qint64 num = this->writeHEX(sendData);
            qDebug()<<"write: "<<num;

                QByteArray hexSendData = QString2Hex(sendData);
                QString viewHexData = this->readHEX(hexSendData);

                QDateTime currentTime = QDateTime::currentDateTime();
                ui->recvTextEdit->insertPlainText(currentTime.toString(tr("[yyyy-MM-dd hh:mm:ss][发送]"))+"[HEX]:"+viewHexData.toUtf8()+"\r\n");
                qDebug()<<"hexSendData"<<hexSendData;

        }
    }
    else if(!this->serial->isOpen()){
        this->sendTimer->stop();
        delete sendTimer;
        this->ui->autoSendCheckBox->setChecked(false);
        this->ui->sendPushButton1->setEnabled(true);
        this->ui->autoSendPeriodLineEdit->setEnabled(true);
        QMessageBox::warning(this,"Warning",QString("无法与串口通讯\n请检查串口是否一打开"));
    }
}

void MainWindow::on_autoSendCheckBox_stateChanged(int arg1)
{


    //显示"关闭串口"的话那就是已经打开串口了
    if(ui->openPushButton->text()==QString("关闭串口"))
    {
        if(ui->autoSendCheckBox->isChecked())
        {
            ui->autoSendCheckBox->setChecked(true);
            qDebug() << "Auto Send Lanch";

            //串口已经打开
            bool ok = true;
            int num = ui->autoSendPeriodLineEdit->text().toInt(&ok);
            if(ok)
            {
                this->ui->sendPushButton1->setEnabled(false);
                this->sendTimer = new QTimer(this);
                this->sendTimer->start(this->ui->autoSendPeriodLineEdit->value());
                this->ui->autoSendPeriodLineEdit->setEnabled(false);
                QObject::connect(this->sendTimer,SIGNAL(timeout()),this,SLOT(autoSendData()));
            }
            else
            {
                QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("时间间隔错误"));
                return;
            }
            qDebug() << "timer start ...";
        }
        else
        {
            ui->sendPushButton1->setChecked(false);
            this->ui->sendPushButton1->setEnabled(true);
            this->ui->autoSendPeriodLineEdit->setEnabled(true);
            this->sendTimer->stop();
        }
    }
    else
    {
        ui->autoSendCheckBox->setChecked(false);
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}

void MainWindow::on_clearRecvPushButton_clicked()
{
    this->ui->recvTextEdit->clear();
}

void MainWindow::on_SendClearButton_clicked()
{
    this->ui->sendTextEdit->clear();
}
