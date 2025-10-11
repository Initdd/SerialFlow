#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QList>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    // Port detection
    QList<QSerialPortInfo> getAvailablePorts();
    QStringList getAvailablePortNames();

    // Connection management
    bool openPort(const QString &portName, 
                  qint32 baudRate,
                  QSerialPort::DataBits dataBits = QSerialPort::Data8,
                  QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                  QSerialPort::Parity parity = QSerialPort::NoParity);
    void closePort();
    bool isOpen() const;

    // Data transmission
    bool sendData(const QByteArray &data);
    bool sendText(const QString &text);

    // Port information
    QString getCurrentPortName() const;
    QString getErrorString() const;

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void connectionStatusChanged(bool connected);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
};

#endif // SERIALPORTMANAGER_H
