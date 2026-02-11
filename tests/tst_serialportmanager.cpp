#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QtTest>

// Include the class under test
#include "serialportmanager.h"

class TestSerialPortManager : public QObject {
  Q_OBJECT

public:
  TestSerialPortManager();
  ~TestSerialPortManager();

private slots:
  void initTestCase();
  void cleanupTestCase();
  void testOpenClose();
  void testSendReceive();
  void testErrorHandling();

private:
  QProcess *m_socatProcess;
  QString m_port1Name;
  QString m_port2Name;
};

TestSerialPortManager::TestSerialPortManager() {
  m_socatProcess = new QProcess(this);
  // Use fixed names in /tmp for simplicity and reliability in test
  m_port1Name = "/tmp/ttyV0";
  m_port2Name = "/tmp/ttyV1";
}

TestSerialPortManager::~TestSerialPortManager() {}

void TestSerialPortManager::initTestCase() {
  // Start socat to create virtual pairs
  // socat -d -d pty,raw,echo=0,link=/tmp/ttyV0 pty,raw,echo=0,link=/tmp/ttyV1
  QStringList args;
  args << "-d" << "-d"
       << "pty,raw,echo=0,link=" + m_port1Name
       << "pty,raw,echo=0,link=" + m_port2Name;

  m_socatProcess->start("socat", args);

  // Wait for socat to start
  QVERIFY(m_socatProcess->waitForStarted());

  // Give it a moment to create the links
  QThread::msleep(500); // Simple wait

  // Check if socat is running and ports exist
  QVERIFY(m_socatProcess->state() == QProcess::Running);
  QVERIFY(QFile::exists(m_port1Name));
  QVERIFY(QFile::exists(m_port2Name));
}

void TestSerialPortManager::cleanupTestCase() {
  if (m_socatProcess->state() == QProcess::Running) {
    m_socatProcess->terminate();
    m_socatProcess->waitForFinished();
  }
}

void TestSerialPortManager::testOpenClose() {
  SerialPortManager manager;

  QSignalSpy connectionSpy(&manager,
                           &SerialPortManager::connectionStatusChanged);

  // Test Open
  bool success = manager.openPort(m_port1Name, 9600);
  QVERIFY2(success, "Failed to open port 1");
  QVERIFY(manager.isOpen());
  QCOMPARE(manager.getCurrentPortName(), m_port1Name);

  // Verify signal
  QCOMPARE(connectionSpy.count(), 1);
  QCOMPARE(connectionSpy.takeFirst().at(0).toBool(), true);

  // Test Close
  manager.closePort();
  QVERIFY(!manager.isOpen());

  // Verify signal
  QCOMPARE(connectionSpy.count(), 1);
  QCOMPARE(connectionSpy.takeFirst().at(0).toBool(), false);
}

void TestSerialPortManager::testSendReceive() {
  // We need two managers, one for each end of the connection
  SerialPortManager sender;
  SerialPortManager receiver;

  QVERIFY(sender.openPort(m_port1Name, 9600));
  QVERIFY(receiver.openPort(m_port2Name, 9600));

  QSignalSpy receiveSpy(&receiver, &SerialPortManager::dataReceived);

  QString testMessage = "Hello Integration Test";
  bool sent = sender.sendText(testMessage);
  QVERIFY(sent);

  // Wait for data to arrive
  QVERIFY(receiveSpy.wait(1000));

  // Verify received data
  QCOMPARE(receiveSpy.count(), 1);
  QByteArray receivedData = receiveSpy.takeFirst().at(0).toByteArray();
  QCOMPARE(receivedData, testMessage.toUtf8());

  sender.closePort();
  receiver.closePort();
}

void TestSerialPortManager::testErrorHandling() {
  SerialPortManager manager;
  QSignalSpy errorSpy(&manager, &SerialPortManager::errorOccurred);

  // Try to open a non-existent port
  bool success = manager.openPort("/dev/non_existent_port_12345", 9600);
  QVERIFY(!success);

  QCOMPARE(errorSpy.count(), 1);
  // The error string might vary by OS, but it shouldn't be empty
  // qDebug() << "Error string: " << errorSpy.at(0).at(0).toString();
  QVERIFY(!errorSpy.takeFirst().at(0).toString().isEmpty());
}

QTEST_MAIN(TestSerialPortManager)
#include "tst_serialportmanager.moc"
