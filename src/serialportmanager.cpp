#include "serialportmanager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent), m_serialPort(new QSerialPort(this)) {
  connect(m_serialPort, &QSerialPort::readyRead, this,
          &SerialPortManager::handleReadyRead);
  connect(m_serialPort, &QSerialPort::errorOccurred, this,
          &SerialPortManager::handleError);
}

SerialPortManager::~SerialPortManager() {
  if (m_serialPort->isOpen()) {
    m_serialPort->close();
  }
}

QList<QSerialPortInfo> SerialPortManager::getAvailablePorts() {
  return QSerialPortInfo::availablePorts();
}

QStringList SerialPortManager::getAvailablePortNames() {
  QStringList portNames;
  const auto ports = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : ports) {
    portNames.append(info.portName());
  }
  return portNames;
}

bool SerialPortManager::openPort(const QString &portName, qint32 baudRate,
                                 QSerialPort::DataBits dataBits,
                                 QSerialPort::StopBits stopBits,
                                 QSerialPort::Parity parity) {
  if (m_serialPort->isOpen()) {
    m_serialPort->close();
  }

  m_serialPort->setPortName(portName);
  m_serialPort->setBaudRate(baudRate);
  m_serialPort->setDataBits(dataBits);
  m_serialPort->setStopBits(stopBits);
  m_serialPort->setParity(parity);
  m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

  if (m_serialPort->open(QIODevice::ReadWrite)) {
    emit connectionStatusChanged(true);
    return true;
  } else {
    emit connectionStatusChanged(false);
    return false;
  }
}

void SerialPortManager::closePort() {
  if (m_serialPort->isOpen()) {
    m_serialPort->close();
    emit connectionStatusChanged(false);
  }
}

bool SerialPortManager::isOpen() const { return m_serialPort->isOpen(); }

bool SerialPortManager::sendData(const QByteArray &data) {
  if (!m_serialPort->isOpen()) {
    emit errorOccurred("Port is not open");
    return false;
  }

  qint64 bytesWritten = m_serialPort->write(data);
  if (bytesWritten == -1) {
    emit errorOccurred("Failed to write data: " + m_serialPort->errorString());
    return false;
  }

  m_serialPort->flush();
  return true;
}

bool SerialPortManager::sendText(const QString &text) {
  return sendData(text.toUtf8());
}

QString SerialPortManager::getCurrentPortName() const {
  return m_serialPort->portName();
}

QString SerialPortManager::getErrorString() const {
  return m_serialPort->errorString();
}

void SerialPortManager::handleReadyRead() {
  QByteArray data = m_serialPort->readAll();
  if (!data.isEmpty()) {
    emit dataReceived(data);
  }
}

void SerialPortManager::handleError(QSerialPort::SerialPortError error) {
  if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
    emit errorOccurred(m_serialPort->errorString());
    if (error == QSerialPort::ResourceError) {
      closePort();
    }
  }
}
