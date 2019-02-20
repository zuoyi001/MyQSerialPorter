#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMainWindow>
#include <QProcess>
#include <QStringList>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QMouseEvent>
#include <QEvent>
#include <QTimer>
#include <QTimerEvent>
#include <QLabel>
#include <QTime>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QList>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QDateTime>
#include <QIcon>

#define PORTFILENAME "serialport.dat"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:
    void SerialPortInit();

private:
    QString SerialIndx;
    int BaudRate;
    QString parityBit;
    int DataBits;
    double StopBits;
    int FlowContrlIndx;

    QSerialPort *serial;  //串口
    QList<QComboBox*> setSerialCombox;

    QMap<int,QSerialPort::DataBits> DatabitsMap;
    QMap<QString,QSerialPort::Parity> ParityMap;
    QMap<double,QSerialPort::StopBits> StopMap;
    QMap<int,QSerialPort::FlowControl> FlowControlMap;

    QTimer *serialTimer;
    QTimer *sendTimer;
    QByteArray mSerialRecData;

protected:
    void setSerial();
    bool openSerial();
    QString readASCII(const QByteArray& serialData);
    QString readHEX(const QByteArray& serialData);
    qint64 writeASCII(const QString& sendData);
    qint64 writeHEX(const QString& sendData);

private slots:
    void on_openPushButton_clicked();

    void readSerialData();

    void timerSerialRead();
    void on_sendPushButton1_clicked();
    void on_hexSendcheckBox_stateChanged(int arg1);
    void on_autoSendCheckBox_stateChanged(int arg1);

    void autoSendData();
    void on_clearRecvPushButton_clicked();
    void on_SendClearButton_clicked();
};

#endif // MAINWINDOW_H
