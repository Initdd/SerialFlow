#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QActionGroup>
#include <QDateTime>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_serialPortManager(new SerialPortManager(this)), m_hexDisplay(false),
      m_autoScroll(true), m_showTimestamp(true), m_isLogging(false),
      m_lineEnding("LF"),

      m_logFile(nullptr), m_logStream(nullptr), m_dataBits(QSerialPort::Data8),
      m_stopBits(QSerialPort::OneStop), m_parity(QSerialPort::NoParity) {
  ui->setupUi(this);
  createMenuBar();
  createStatusBar();

  connect(ui->refreshButton, &QPushButton::clicked, this,
          &MainWindow::refreshPorts);
  connect(ui->connectButton, &QPushButton::clicked, this,
          &MainWindow::toggleConnection);
  connect(ui->settingsButton, &QPushButton::clicked, this,
          &MainWindow::openSettings);
  connect(ui->clearButton, &QPushButton::clicked, this,
          &MainWindow::clearOutput);
  connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::sendData);
  connect(ui->inputLineEdit, &QLineEdit::returnPressed, this,
          &MainWindow::sendData);
  connect(ui->lineEndingComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            m_lineEnding = ui->lineEndingComboBox->currentText();
            updateLineEndingMenu();
            saveSettings();
          });

  loadSettings();
  updateLineEndingMenu();
  applyShortcuts();

  connect(m_serialPortManager, &SerialPortManager::dataReceived, this,
          &MainWindow::onDataReceived);
  connect(m_serialPortManager, &SerialPortManager::connectionStatusChanged,
          this, &MainWindow::onConnectionStatusChanged);
  connect(m_serialPortManager, &SerialPortManager::errorOccurred, this,
          &MainWindow::onErrorOccurred);

  refreshPorts();
  updateConnectionStatus();
}

MainWindow::~MainWindow() {
  saveSettings();
  if (m_isLogging) {
    toggleLogging();
  }
  delete ui;
}

void MainWindow::createMenuBar() {
  QMenuBar *menuBar = new QMenuBar(this);
  setMenuBar(menuBar);

  QMenu *fileMenu = menuBar->addMenu("&File");

  QAction *startLoggingAction = new QAction("Start &Logging", this);
  connect(startLoggingAction, &QAction::triggered, this,
          &MainWindow::toggleLogging);
  fileMenu->addAction(startLoggingAction);

  fileMenu->addSeparator();

  QMenu *lineEndingMenu = fileMenu->addMenu("Line &Ending");

  m_lfAction = new QAction("LF (\\n)", this);
  m_lfAction->setCheckable(true);
  connect(m_lfAction, &QAction::triggered, [this]() {
    m_lineEnding = "LF";
    updateLineEndingMenu();
    saveSettings();
  });
  lineEndingMenu->addAction(m_lfAction);

  m_crAction = new QAction("CR (\\r)", this);
  m_crAction->setCheckable(true);
  connect(m_crAction, &QAction::triggered, [this]() {
    m_lineEnding = "CR";
    updateLineEndingMenu();
    saveSettings();
  });
  lineEndingMenu->addAction(m_crAction);

  m_crlfAction = new QAction("CRLF (\\r\\n)", this);
  m_crlfAction->setCheckable(true);
  connect(m_crlfAction, &QAction::triggered, [this]() {
    m_lineEnding = "CRLF";
    updateLineEndingMenu();
    saveSettings();
  });
  lineEndingMenu->addAction(m_crlfAction);

  m_noneAction = new QAction("None", this);
  m_noneAction->setCheckable(true);
  connect(m_noneAction, &QAction::triggered, [this]() {
    m_lineEnding = "None";
    updateLineEndingMenu();
    saveSettings();
  });
  lineEndingMenu->addAction(m_noneAction);

  QActionGroup *lineEndingGroup = new QActionGroup(this);
  lineEndingGroup->addAction(m_lfAction);
  lineEndingGroup->addAction(m_crAction);
  lineEndingGroup->addAction(m_crlfAction);
  lineEndingGroup->addAction(m_noneAction);

  fileMenu->addSeparator();

  QAction *exitAction = new QAction("E&xit", this);
  exitAction->setShortcut(QKeySequence::Quit);
  connect(exitAction, &QAction::triggered, this, &QWidget::close);
  fileMenu->addAction(exitAction);

  QMenu *toolsMenu = menuBar->addMenu("&Tools");

  QAction *settingsAction = new QAction("&Settings", this);
  settingsAction->setShortcut(QKeySequence::Preferences);
  connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
  toolsMenu->addAction(settingsAction);

  QMenu *helpMenu = menuBar->addMenu("&Help");

  QAction *aboutAction = new QAction("&About", this);
  connect(aboutAction, &QAction::triggered, this, [this]() {
    QMessageBox::about(this, "About SerialFlow",
                       "<h2>SerialFlow v1.0</h2>"
                       "<p>A powerful serial port monitor application.</p>"
                       "<p>Features:</p>"
                       "<ul>"
                       "<li>Auto-detect serial ports</li>"
                       "<li>Configurable connection settings</li>"
                       "<li>ASCII/HEX display modes</li>"
                       "<li>Data logging</li>"
                       "<li>Customizable shortcuts</li>"
                       "</ul>");
  });
  helpMenu->addAction(aboutAction);
}

void MainWindow::createStatusBar() {
  QStatusBar *statusBar = new QStatusBar(this);
  setStatusBar(statusBar);

  m_connectionStatusIcon = new QLabel(this);
  m_connectionStatusIcon->setFixedSize(16, 16);
  statusBar->addPermanentWidget(m_connectionStatusIcon);

  m_statusLabel = new QLabel("Disconnected", this);
  statusBar->addPermanentWidget(m_statusLabel);
}

void MainWindow::refreshPorts() {
  QString currentPort = ui->portComboBox->currentText();
  ui->portComboBox->clear();

  QStringList ports = m_serialPortManager->getAvailablePortNames();

  if (ports.isEmpty()) {
    ui->portComboBox->addItem("No ports available");
    ui->connectButton->setEnabled(false);
  } else {
    ui->portComboBox->addItems(ports);
    ui->connectButton->setEnabled(true);

    int index = ui->portComboBox->findText(currentPort);
    if (index >= 0) {
      ui->portComboBox->setCurrentIndex(index);
    }
  }
}

void MainWindow::toggleConnection() {
  if (m_serialPortManager->isOpen()) {
    m_serialPortManager->closePort();
  } else {
    QString portName = ui->portComboBox->currentText();
    qint32 baudRate = ui->baudRateComboBox->currentText().toInt();

    if (m_serialPortManager->openPort(portName, baudRate, m_dataBits,
                                      m_stopBits, m_parity)) {
      ui->outputTextEdit->append(
          QString("<span style='color: #16a34a;'>[%1] Connected to %2 at %3 "
                  "baud</span>")
              .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
              .arg(portName)
              .arg(baudRate));
    }
  }
}

void MainWindow::sendData() {
  QString text = ui->inputLineEdit->text();
  if (text.isEmpty()) {
    return;
  }

  if (!m_serialPortManager->isOpen()) {
    QMessageBox::warning(this, "Not Connected",
                         "Please connect to a serial port first.");
    return;
  }

  if (m_lineEnding == "LF") {
    text += "\n";
  } else if (m_lineEnding == "CR") {
    text += "\r";
  } else if (m_lineEnding == "CRLF") {
    text += "\r\n";
  }

  if (m_serialPortManager->sendText(text)) {
    ui->inputLineEdit->clear();

    if (m_showTimestamp) {
      ui->outputTextEdit->append(
          QString("<span style='color: #2563eb;'>[%1] TX: %2</span>")
              .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
              .arg(text.trimmed()));
    } else {
      ui->outputTextEdit->append(
          QString("<span style='color: #2563eb;'>TX: %1</span>")
              .arg(text.trimmed()));
    }
  }
}

void MainWindow::onDataReceived(const QByteArray &data) {
  QString formattedData = formatData(data);

  formattedData.replace("\r\n", "<br>");
  formattedData.replace("\n", "<br>");
  formattedData.replace("\r", "<br>");

  if (m_showTimestamp) {
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    formattedData = QString("<span style='color: #16a34a;'>[%1] RX: %2</span>")
                        .arg(timestamp)
                        .arg(formattedData);
  } else {
    formattedData = QString("<span style='color: #16a34a;'>RX: %1</span>")
                        .arg(formattedData);
  }

  // Autoscroll only if already at the bottom before appending.
  QScrollBar *scrollBar = ui->outputTextEdit->verticalScrollBar();
  bool atBottom = (scrollBar->value() == scrollBar->maximum());

  ui->outputTextEdit->append(formattedData);

  if (m_autoScroll && atBottom) {
    scrollBar->setValue(scrollBar->maximum());
  }

  if (m_isLogging) {
    logData(formattedData);
  }
}

void MainWindow::onConnectionStatusChanged(bool connected) {
  updateConnectionStatus();

  ui->connectButton->setProperty("connected", connected);
  m_connectionStatusIcon->setProperty("connected", connected);

  // Force stylesheet re-evaluation after dynamic property change.
  ui->connectButton->style()->unpolish(ui->connectButton);
  ui->connectButton->style()->polish(ui->connectButton);
  m_connectionStatusIcon->style()->unpolish(m_connectionStatusIcon);
  m_connectionStatusIcon->style()->polish(m_connectionStatusIcon);

  if (connected) {
    ui->connectButton->setText("● Disconnect");
    ui->portComboBox->setEnabled(false);
    ui->baudRateComboBox->setEnabled(false);
    ui->inputLineEdit->setEnabled(true);
    ui->sendButton->setEnabled(true);
  } else {
    ui->connectButton->setText("● Connect");
    ui->portComboBox->setEnabled(true);
    ui->baudRateComboBox->setEnabled(true);
    ui->inputLineEdit->setEnabled(false);
    ui->sendButton->setEnabled(false);

    ui->outputTextEdit->append(
        QString("<span style='color: red;'>[%1] Disconnected</span>")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
  }
}

void MainWindow::onErrorOccurred(const QString &error) {
  ui->outputTextEdit->append(
      QString("<span style='color: red;'>[%1] Error: %2</span>")
          .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
          .arg(error));

  QMessageBox::critical(this, "Serial Port Error", error);
}

void MainWindow::clearOutput() { ui->outputTextEdit->clear(); }

void MainWindow::toggleLogging() {
  if (m_isLogging) {
    if (m_logStream) {
      delete m_logStream;
      m_logStream = nullptr;
    }
    if (m_logFile) {
      m_logFile->close();
      delete m_logFile;
      m_logFile = nullptr;
    }
    m_isLogging = false;
    statusBar()->showMessage("Logging stopped", 3000);
  } else {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Select Log File",
        QDateTime::currentDateTime().toString(
            "'SerialFlow_'yyyyMMdd_HHmmss'.log'"),
        "Log Files (*.log);;Text Files (*.txt);;All Files (*)");

    if (!fileName.isEmpty()) {
      m_logFile = new QFile(fileName);
      if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append |
                          QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        m_logFilePath = fileName;
        m_isLogging = true;
        statusBar()->showMessage("Logging to: " + fileName, 3000);

        m_logStream->operator<<(
            "=== SerialFlow Log Started: " +
            QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") +
            " ===\n");
        m_logStream->flush();
      } else {
        QMessageBox::critical(this, "Logging Error",
                              "Failed to open log file for writing.");
        delete m_logFile;
        m_logFile = nullptr;
      }
    }
  }
}

void MainWindow::openSettings() {
  SettingsDialog dialog(this);

  dialog.setHexDisplay(m_hexDisplay);
  dialog.setAutoScroll(m_autoScroll);
  dialog.setShowTimestamp(m_showTimestamp);
  dialog.setDataBits(m_dataBits);
  dialog.setStopBits(m_stopBits);
  dialog.setParity(m_parity);
  dialog.setShortcuts(m_shortcuts);

  if (dialog.exec() == QDialog::Accepted) {
    m_hexDisplay = dialog.hexDisplay();
    m_autoScroll = dialog.autoScroll();
    m_showTimestamp = dialog.showTimestamp();
    m_dataBits = dialog.dataBits();
    m_stopBits = dialog.stopBits();
    m_parity = dialog.parity();
    m_shortcuts = dialog.shortcuts();

    applyShortcuts();
    saveSettings();
  }
}

void MainWindow::updateConnectionStatus() {
  if (m_serialPortManager->isOpen()) {
    m_statusLabel->setText("Connected: " +
                           m_serialPortManager->getCurrentPortName());
    m_connectionStatusIcon->setToolTip("Connected");
  } else {
    m_statusLabel->setText("Disconnected");
    m_connectionStatusIcon->setToolTip("Disconnected");
  }
}

QString MainWindow::formatData(const QByteArray &data) {
  if (m_hexDisplay) {
    QString hex = data.toHex(' ').toUpper();
    return hex;
  } else {
    return QString::fromUtf8(data);
  }
}

void MainWindow::logData(const QString &data) {
  if (m_isLogging && m_logStream) {
    m_logStream->operator<<(data + "\n");
    m_logStream->flush();
  }
}

void MainWindow::loadSettings() {
  QSettings settings;

  m_hexDisplay = settings.value("display/hexMode", false).toBool();
  m_autoScroll = settings.value("display/autoScroll", true).toBool();
  m_showTimestamp = settings.value("display/showTimestamp", true).toBool();
  m_lineEnding = settings.value("connection/lineEnding", "LF").toString();

  m_dataBits = static_cast<QSerialPort::DataBits>(
      settings.value("connection/dataBits", QSerialPort::Data8).toInt());
  m_stopBits = static_cast<QSerialPort::StopBits>(
      settings.value("connection/stopBits", QSerialPort::OneStop).toInt());
  m_parity = static_cast<QSerialPort::Parity>(
      settings.value("connection/parity", QSerialPort::NoParity).toInt());

  settings.beginGroup("shortcuts");
  QStringList keys = settings.childKeys();
  for (const QString &key : keys) {
    m_shortcuts[key] = settings.value(key).toString();
  }
  settings.endGroup();

  if (m_shortcuts.isEmpty()) {
    m_shortcuts["connect"] = "Ctrl+K";
    m_shortcuts["send"] = "Ctrl+Return";
    m_shortcuts["clear"] = "Ctrl+L";
    m_shortcuts["refresh"] = "F5";
  }

  restoreGeometry(settings.value("window/geometry").toByteArray());
}

void MainWindow::saveSettings() {
  QSettings settings;

  settings.setValue("display/hexMode", m_hexDisplay);
  settings.setValue("display/autoScroll", m_autoScroll);
  settings.setValue("display/showTimestamp", m_showTimestamp);
  settings.setValue("connection/lineEnding", m_lineEnding);

  settings.setValue("connection/dataBits", static_cast<int>(m_dataBits));
  settings.setValue("connection/stopBits", static_cast<int>(m_stopBits));
  settings.setValue("connection/parity", static_cast<int>(m_parity));

  settings.beginGroup("shortcuts");
  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    settings.setValue(it.key(), it.value());
  }
  settings.endGroup();

  settings.setValue("window/geometry", saveGeometry());
}

void MainWindow::applyShortcuts() {
  for (QAction *action : m_shortcutActions) {
    removeAction(action);
    delete action;
  }
  m_shortcutActions.clear();

  if (m_shortcuts.contains("connect")) {
    QAction *connectAction = new QAction(this);
    connectAction->setShortcut(QKeySequence(m_shortcuts["connect"]));
    connect(connectAction, &QAction::triggered, this,
            &MainWindow::toggleConnection);
    addAction(connectAction);
    m_shortcutActions.append(connectAction);
  }

  if (m_shortcuts.contains("send")) {
    QAction *sendAction = new QAction(this);
    sendAction->setShortcut(QKeySequence(m_shortcuts["send"]));
    connect(sendAction, &QAction::triggered, this, &MainWindow::sendData);
    addAction(sendAction);
    m_shortcutActions.append(sendAction);
  }

  if (m_shortcuts.contains("clear")) {
    QAction *clearAction = new QAction(this);
    clearAction->setShortcut(QKeySequence(m_shortcuts["clear"]));
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearOutput);
    addAction(clearAction);
    m_shortcutActions.append(clearAction);
  }

  if (m_shortcuts.contains("refresh")) {
    QAction *refreshAction = new QAction(this);
    refreshAction->setShortcut(QKeySequence(m_shortcuts["refresh"]));
    connect(refreshAction, &QAction::triggered, this,
            &MainWindow::refreshPorts);
    addAction(refreshAction);
    m_shortcutActions.append(refreshAction);
  }
}

void MainWindow::updateLineEndingMenu() {
  m_lfAction->setChecked(m_lineEnding == "LF");
  m_crAction->setChecked(m_lineEnding == "CR");
  m_crlfAction->setChecked(m_lineEnding == "CRLF");
  m_noneAction->setChecked(m_lineEnding == "None");

  int index = ui->lineEndingComboBox->findText(m_lineEnding);
  if (index >= 0) {
    ui->lineEndingComboBox->blockSignals(true);  // avoid re-entrant saveSettings
    ui->lineEndingComboBox->setCurrentIndex(index);
    ui->lineEndingComboBox->blockSignals(false);
  }
}
