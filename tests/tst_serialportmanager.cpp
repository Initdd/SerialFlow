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

  void testLineBuffer_NoEmitWithoutTerminator();
  void testLineBuffer_SingleLineLF();
  void testLineBuffer_SingleLineCR();
  void testLineBuffer_SingleLineCRLF();
  void testLineBuffer_MultipleLinesSingleChunk();
  void testLineBuffer_LineReassembledAcrossChunks();
  void testLineBuffer_CRLFSplitAcrossChunks();
  void testLineBuffer_TrailingDataBuffered();
  void testLineBuffer_EmptyLines();
  void testLineBuffer_LFBeforeCR();
  void testLineBuffer_CRAtEndWaitsForNextChunk();

private:
  QProcess *m_socatProcess;
  QString m_port1Name;
  QString m_port2Name;
};

TestSerialPortManager::TestSerialPortManager() {
  m_socatProcess = new QProcess(this);
  m_port1Name = "/tmp/ttyV0";
  m_port2Name = "/tmp/ttyV1";
}

TestSerialPortManager::~TestSerialPortManager() {}

void TestSerialPortManager::initTestCase() {
  QStringList args;
  args << "-d" << "-d"
       << "pty,raw,echo=0,link=" + m_port1Name
       << "pty,raw,echo=0,link=" + m_port2Name;

  m_socatProcess->start("socat", args);
  QVERIFY(m_socatProcess->waitForStarted());

  QThread::msleep(500);

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

  bool success = manager.openPort(m_port1Name, 9600);
  QVERIFY2(success, "Failed to open port 1");
  QVERIFY(manager.isOpen());
  QCOMPARE(manager.getCurrentPortName(), m_port1Name);
  QCOMPARE(connectionSpy.count(), 1);
  QCOMPARE(connectionSpy.takeFirst().at(0).toBool(), true);

  manager.closePort();
  QVERIFY(!manager.isOpen());
  QCOMPARE(connectionSpy.count(), 1);
  QCOMPARE(connectionSpy.takeFirst().at(0).toBool(), false);
}

void TestSerialPortManager::testSendReceive() {
  SerialPortManager sender;
  SerialPortManager receiver;

  QVERIFY(sender.openPort(m_port1Name, 9600));
  QVERIFY(receiver.openPort(m_port2Name, 9600));

  QSignalSpy receiveSpy(&receiver, &SerialPortManager::dataReceived);

  QString testMessage = "Hello Integration Test\n";  // terminator required by line buffer
  bool sent = sender.sendText(testMessage);
  QVERIFY(sent);
  QVERIFY(receiveSpy.wait(1000));
  QCOMPARE(receiveSpy.count(), 1);
  QByteArray receivedData = receiveSpy.takeFirst().at(0).toByteArray();
  QCOMPARE(receivedData, testMessage.toUtf8());

  sender.closePort();
  receiver.closePort();
}

void TestSerialPortManager::testErrorHandling() {
  SerialPortManager manager;
  QSignalSpy errorSpy(&manager, &SerialPortManager::errorOccurred);

  bool success = manager.openPort("/dev/non_existent_port_12345", 9600);
  QVERIFY(!success);
  QCOMPARE(errorSpy.count(), 1);
  QVERIFY(!errorSpy.takeFirst().at(0).toString().isEmpty());
}

static QList<QByteArray> collectLines(SerialPortManager &mgr,
                                      const QByteArray &data) {
  QList<QByteArray> lines;
  QObject::connect(&mgr, &SerialPortManager::dataReceived,
                   [&lines](const QByteArray &d) { lines.append(d); });
  mgr.processIncomingData(data);
  return lines;
}

void TestSerialPortManager::testLineBuffer_NoEmitWithoutTerminator() {
  SerialPortManager mgr;
  QSignalSpy spy(&mgr, &SerialPortManager::dataReceived);
  mgr.processIncomingData("Hello, world");
  QCOMPARE(spy.count(), 0);
}

void TestSerialPortManager::testLineBuffer_SingleLineLF() {
  SerialPortManager mgr;
  const QList<QByteArray> lines = collectLines(mgr, "Hello\n");

  QCOMPARE(lines.size(), 1);
  QCOMPARE(lines.at(0), QByteArray("Hello\n"));
}

void TestSerialPortManager::testLineBuffer_SingleLineCR() {
  // Trailing 'X' gives the buffer a lookahead byte to confirm the \r is lone CR.
  SerialPortManager mgr;
  const QList<QByteArray> lines = collectLines(mgr, "Hello\rX");
  QCOMPARE(lines.size(), 1);
  QCOMPARE(lines.at(0), QByteArray("Hello\r"));
}

void TestSerialPortManager::testLineBuffer_SingleLineCRLF() {
  SerialPortManager mgr;
  const QList<QByteArray> lines = collectLines(mgr, "Hello\r\n");

  QCOMPARE(lines.size(), 1);
  QCOMPARE(lines.at(0), QByteArray("Hello\r\n"));
}

void TestSerialPortManager::testLineBuffer_MultipleLinesSingleChunk() {
  SerialPortManager mgr;
  const QList<QByteArray> lines =
      collectLines(mgr, "Line1\nLine2\r\nLine3\n");

  QCOMPARE(lines.size(), 3);
  QCOMPARE(lines.at(0), QByteArray("Line1\n"));
  QCOMPARE(lines.at(1), QByteArray("Line2\r\n"));
  QCOMPARE(lines.at(2), QByteArray("Line3\n"));
}

void TestSerialPortManager::testLineBuffer_LineReassembledAcrossChunks() {
  SerialPortManager mgr;
  QSignalSpy spy(&mgr, &SerialPortManager::dataReceived);

  mgr.processIncomingData("Hel");
  QCOMPARE(spy.count(), 0);

  mgr.processIncomingData("lo\n");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toByteArray(), QByteArray("Hello\n"));
}

void TestSerialPortManager::testLineBuffer_CRLFSplitAcrossChunks() {
  SerialPortManager mgr;
  QSignalSpy spy(&mgr, &SerialPortManager::dataReceived);

  mgr.processIncomingData("Hello\r");
  QCOMPARE(spy.count(), 0);

  mgr.processIncomingData("\n");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toByteArray(), QByteArray("Hello\r\n"));
}

void TestSerialPortManager::testLineBuffer_TrailingDataBuffered() {
  SerialPortManager mgr;
  QSignalSpy spy(&mgr, &SerialPortManager::dataReceived);

  mgr.processIncomingData("Complete\nIncomplete");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toByteArray(), QByteArray("Complete\n"));

  mgr.processIncomingData(" data\n");
  QCOMPARE(spy.count(), 2);
  QCOMPARE(spy.at(1).at(0).toByteArray(), QByteArray("Incomplete data\n"));
}

void TestSerialPortManager::testLineBuffer_EmptyLines() {
  SerialPortManager mgr;
  QSignalSpy spy(&mgr, &SerialPortManager::dataReceived);

  mgr.processIncomingData("\n\n");
  QCOMPARE(spy.count(), 2);
  QCOMPARE(spy.at(0).at(0).toByteArray(), QByteArray("\n"));
  QCOMPARE(spy.at(1).at(0).toByteArray(), QByteArray("\n"));
}

void TestSerialPortManager::testLineBuffer_LFBeforeCR() {
  // Trailing 'C' gives the buffer lookahead to immediately classify the \r.
  SerialPortManager mgr;
  const QList<QByteArray> lines = collectLines(mgr, "A\nB\rC\n");

  QCOMPARE(lines.size(), 3);
  QCOMPARE(lines.at(0), QByteArray("A\n"));
  QCOMPARE(lines.at(1), QByteArray("B\r"));
  QCOMPARE(lines.at(2), QByteArray("C\n"));
}

void TestSerialPortManager::testLineBuffer_CRAtEndWaitsForNextChunk() {
  SerialPortManager mgr;
  QSignalSpy spy(&mgr, &SerialPortManager::dataReceived);

  mgr.processIncomingData("Data\r");
  QCOMPARE(spy.count(), 0);

  // 'M' (not '\n') resolves the ambiguity: the deferred \r was a lone CR.
  mgr.processIncomingData("More\n");
  QCOMPARE(spy.count(), 2);
  QCOMPARE(spy.at(0).at(0).toByteArray(), QByteArray("Data\r"));
  QCOMPARE(spy.at(1).at(0).toByteArray(), QByteArray("More\n"));
}

QTEST_MAIN(TestSerialPortManager)
#include "tst_serialportmanager.moc"
