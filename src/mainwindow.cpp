#include "mainwindow.h"
#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QSettings>
#include <QKeySequence>
#include <QAction>
#include <QActionGroup>
#include <QScrollBar>
#include <QApplication>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_serialPortManager(new SerialPortManager(this))
    , m_hexDisplay(false)
    , m_autoScroll(true)
    , m_showTimestamp(true)
    , m_isLogging(false)
    , m_darkMode(false)
    , m_lineEnding("LF") // Default to LF (Line Feed)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_dataBits(QSerialPort::Data8)
    , m_stopBits(QSerialPort::OneStop)
    , m_parity(QSerialPort::NoParity)
{
    setupUI();
    createMenuBar();
    createStatusBar();
    loadSettings();
    updateLineEndingMenu(); // Update menu to reflect loaded settings
    applyShortcuts();
    applyTheme();
    
    // Connect signals
    connect(m_serialPortManager, &SerialPortManager::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_serialPortManager, &SerialPortManager::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    connect(m_serialPortManager, &SerialPortManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    
    // Initial port refresh
    refreshPorts();
    updateConnectionStatus();
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (m_isLogging) {
        toggleLogging();
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("SerialFlow - Serial Monitor");
    resize(900, 600);
    
    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Connection settings group (compact)
    QGroupBox *connectionGroup = new QGroupBox("Connection", this);
    QHBoxLayout *connectionLayout = new QHBoxLayout(connectionGroup);
    
    // Port selection
    connectionLayout->addWidget(new QLabel("Port:", this));
    m_portComboBox = new QComboBox(this);
    m_portComboBox->setMinimumWidth(120);
    connectionLayout->addWidget(m_portComboBox);
    
    // Refresh button
    m_refreshButton = new QPushButton("↻", this);
    m_refreshButton->setMaximumWidth(35);
    m_refreshButton->setToolTip("Refresh ports");
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connectionLayout->addWidget(m_refreshButton);
    
    // Baud rate
    connectionLayout->addWidget(new QLabel("Baud:", this));
    m_baudRateComboBox = new QComboBox(this);
    m_baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_baudRateComboBox->setCurrentText("115200");
    m_baudRateComboBox->setMinimumWidth(100);
    connectionLayout->addWidget(m_baudRateComboBox);
    
    // Line ending selection
    connectionLayout->addWidget(new QLabel("Line Ending:", this));
    m_lineEndingComboBox = new QComboBox(this);
    m_lineEndingComboBox->addItems({"LF", "CR", "CRLF", "None"});
    m_lineEndingComboBox->setCurrentText("LF");
    m_lineEndingComboBox->setMinimumWidth(80);
    m_lineEndingComboBox->setToolTip("Select line termination: LF (\\n), CR (\\r), CRLF (\\r\\n), or None");
    connect(m_lineEndingComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        m_lineEnding = m_lineEndingComboBox->currentText();
        updateLineEndingMenu();
        saveSettings();
    });
    connectionLayout->addWidget(m_lineEndingComboBox);
    
    // Add spacing before connect button to make it stand out
    connectionLayout->addSpacing(20);
    
    // Connect button - make it prominent
    m_connectButton = new QPushButton("● Connect", this);
    m_connectButton->setMinimumWidth(120);
    m_connectButton->setMinimumHeight(35);
    m_connectButton->setCursor(Qt::PointingHandCursor);
    m_connectButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"  // Green
        "   color: white;"
        "   border: 2px solid #45a049;"
        "   border-radius: 6px;"
        "   padding: 5px 15px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "   border: 2px solid #3d8b40;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"
        "}"
    );
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    connectionLayout->addWidget(m_connectButton);
    
    // Settings button
    m_settingsButton = new QPushButton("⚙ Settings", this);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connectionLayout->addWidget(m_settingsButton);
    
    connectionLayout->addStretch();
    
    mainLayout->addWidget(connectionGroup);
    
    // Output text area
    QGroupBox *outputGroup = new QGroupBox("Received Data", this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    m_outputTextEdit = new QTextEdit(this);
    m_outputTextEdit->setReadOnly(true);
    m_outputTextEdit->setFont(QFont("Courier", 10));
    outputLayout->addWidget(m_outputTextEdit);
    
    // Clear button
    QHBoxLayout *outputButtonLayout = new QHBoxLayout();
    m_clearButton = new QPushButton("Clear", this);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearOutput);
    
    // Theme toggle button
    m_themeButton = new QPushButton("☾ Dark", this);
    m_themeButton->setToolTip("Toggle dark mode (Ctrl+D)");
    connect(m_themeButton, &QPushButton::clicked, this, &MainWindow::toggleDarkMode);
    
    outputButtonLayout->addStretch();
    outputButtonLayout->addWidget(m_themeButton);
    outputButtonLayout->addWidget(m_clearButton);
    outputLayout->addLayout(outputButtonLayout);
    
    mainLayout->addWidget(outputGroup);
    
    // Input area
    QGroupBox *inputGroup = new QGroupBox("Send Data", this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroup);
    
    m_inputLineEdit = new QLineEdit(this);
    m_inputLineEdit->setPlaceholderText("Type message to send...");
    connect(m_inputLineEdit, &QLineEdit::returnPressed, this, &MainWindow::sendData);
    inputLayout->addWidget(m_inputLineEdit);
    
    m_sendButton = new QPushButton("Send", this);
    m_sendButton->setMinimumWidth(80);
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::sendData);
    inputLayout->addWidget(m_sendButton);
    
    mainLayout->addWidget(inputGroup);
}

void MainWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");
    
    QAction *startLoggingAction = new QAction("Start &Logging", this);
    connect(startLoggingAction, &QAction::triggered, this, &MainWindow::toggleLogging);
    fileMenu->addAction(startLoggingAction);
    
    fileMenu->addSeparator();
    
    // Line Ending submenu
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
    
    // Create action group for mutual exclusivity
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
    
    // Tools menu
    QMenu *toolsMenu = menuBar->addMenu("&Tools");
    
    QAction *settingsAction = new QAction("&Settings", this);
    settingsAction->setShortcut(QKeySequence::Preferences);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    toolsMenu->addAction(settingsAction);
    
    toolsMenu->addSeparator();
    
    QAction *toggleThemeAction = new QAction("Toggle &Dark Mode", this);
    toggleThemeAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::toggleDarkMode);
    toolsMenu->addAction(toggleThemeAction);
    
    // Help menu
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

void MainWindow::createStatusBar()
{
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    
    // Connection status icon
    m_connectionStatusIcon = new QLabel(this);
    m_connectionStatusIcon->setFixedSize(16, 16);
    statusBar->addPermanentWidget(m_connectionStatusIcon);
    
    // Status label
    m_statusLabel = new QLabel("Disconnected", this);
    statusBar->addPermanentWidget(m_statusLabel);
}

void MainWindow::refreshPorts()
{
    QString currentPort = m_portComboBox->currentText();
    m_portComboBox->clear();
    
    QStringList ports = m_serialPortManager->getAvailablePortNames();
    
    if (ports.isEmpty()) {
        m_portComboBox->addItem("No ports available");
        m_connectButton->setEnabled(false);
    } else {
        m_portComboBox->addItems(ports);
        m_connectButton->setEnabled(true);
        
        // Try to restore previous selection
        int index = m_portComboBox->findText(currentPort);
        if (index >= 0) {
            m_portComboBox->setCurrentIndex(index);
        }
    }
}

void MainWindow::toggleConnection()
{
    if (m_serialPortManager->isOpen()) {
        m_serialPortManager->closePort();
    } else {
        QString portName = m_portComboBox->currentText();
        qint32 baudRate = m_baudRateComboBox->currentText().toInt();
        
        if (m_serialPortManager->openPort(portName, baudRate, m_dataBits, m_stopBits, m_parity)) {
            // Use green color for successful connection
            QString colorStyle = m_darkMode ? "#4ade80" : "#16a34a";
            m_outputTextEdit->append(QString("<span style='color: %1;'>[%2] Connected to %3 at %4 baud</span>")
                .arg(colorStyle)
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(portName)
                .arg(baudRate));
        }
    }
}

void MainWindow::sendData()
{
    QString text = m_inputLineEdit->text();
    if (text.isEmpty()) {
        return;
    }
    
    if (!m_serialPortManager->isOpen()) {
        QMessageBox::warning(this, "Not Connected", "Please connect to a serial port first.");
        return;
    }
    
    // Add line ending based on settings
    if (m_lineEnding == "LF") {
        text += "\n";
    } else if (m_lineEnding == "CR") {
        text += "\r";
    } else if (m_lineEnding == "CRLF") {
        text += "\r\n";
    }
    // If "None", don't add anything
    
    if (m_serialPortManager->sendText(text)) {
        m_inputLineEdit->clear();
        
        // Use blue color for TX in both modes
        QString colorStyle = m_darkMode ? "#60a5fa" : "#2563eb"; // Light blue for dark mode, darker blue for light mode
        
        if (m_showTimestamp) {
            m_outputTextEdit->append(QString("<span style='color: %1;'>[%2] TX: %3</span>")
                .arg(colorStyle)
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(text.trimmed()));
        } else {
            m_outputTextEdit->append(QString("<span style='color: %1;'>TX: %2</span>")
                .arg(colorStyle)
                .arg(text.trimmed()));
        }
    }
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    QString formattedData = formatData(data);
    
    // Use green color for RX in both light and dark modes
    QString colorStyle = m_darkMode ? "#4ade80" : "#16a34a"; // Light green for dark mode, darker green for light mode
    
    if (m_showTimestamp) {
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
        formattedData = QString("<span style='color: %1;'>[%2] RX: %3</span>")
            .arg(colorStyle)
            .arg(timestamp)
            .arg(formattedData);
    } else {
        formattedData = QString("<span style='color: %1;'>RX: %2</span>")
            .arg(colorStyle)
            .arg(formattedData);
    }
    
    m_outputTextEdit->append(formattedData);
    
    if (m_autoScroll) {
        QScrollBar *scrollBar = m_outputTextEdit->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
    
    if (m_isLogging) {
        logData(formattedData);
    }
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    updateConnectionStatus();
    
    if (connected) {
        m_connectButton->setText("● Disconnect");
        m_connectButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #f44336;"  // Red
            "   color: white;"
            "   border: 2px solid #da190b;"
            "   border-radius: 6px;"
            "   padding: 5px 15px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #da190b;"
            "   border: 2px solid #c41000;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #c41000;"
            "}"
        );
        m_portComboBox->setEnabled(false);
        m_baudRateComboBox->setEnabled(false);
        m_inputLineEdit->setEnabled(true);
        m_sendButton->setEnabled(true);
    } else {
        m_connectButton->setText("● Connect");
        m_connectButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #4CAF50;"  // Green
            "   color: white;"
            "   border: 2px solid #45a049;"
            "   border-radius: 6px;"
            "   padding: 5px 15px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #45a049;"
            "   border: 2px solid #3d8b40;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #3d8b40;"
            "}"
        );
        m_portComboBox->setEnabled(true);
        m_baudRateComboBox->setEnabled(true);
        m_inputLineEdit->setEnabled(false);
        m_sendButton->setEnabled(false);
        
        m_outputTextEdit->append(QString("<span style='color: red;'>[%1] Disconnected</span>")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
    }
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_outputTextEdit->append(QString("<span style='color: red;'>[%1] Error: %2</span>")
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
        .arg(error));
    
    QMessageBox::critical(this, "Serial Port Error", error);
}

void MainWindow::clearOutput()
{
    m_outputTextEdit->clear();
}

void MainWindow::toggleLogging()
{
    if (m_isLogging) {
        // Stop logging
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
        // Start logging
        QString fileName = QFileDialog::getSaveFileName(this,
            "Select Log File",
            QDateTime::currentDateTime().toString("'SerialFlow_'yyyyMMdd_HHmmss'.log'"),
            "Log Files (*.log);;Text Files (*.txt);;All Files (*)");
        
        if (!fileName.isEmpty()) {
            m_logFile = new QFile(fileName);
            if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                m_logStream = new QTextStream(m_logFile);
                m_logFilePath = fileName;
                m_isLogging = true;
                statusBar()->showMessage("Logging to: " + fileName, 3000);
                
                m_logStream->operator<<("=== SerialFlow Log Started: " 
                    + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") 
                    + " ===\n");
                m_logStream->flush();
            } else {
                QMessageBox::critical(this, "Logging Error", "Failed to open log file for writing.");
                delete m_logFile;
                m_logFile = nullptr;
            }
        }
    }
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    
    // Pass current settings
    dialog.setHexDisplay(m_hexDisplay);
    dialog.setAutoScroll(m_autoScroll);
    dialog.setShowTimestamp(m_showTimestamp);
    dialog.setDataBits(m_dataBits);
    dialog.setStopBits(m_stopBits);
    dialog.setParity(m_parity);
    dialog.setShortcuts(m_shortcuts);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Apply new settings
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

void MainWindow::updateConnectionStatus()
{
    if (m_serialPortManager->isOpen()) {
        m_statusLabel->setText("Connected: " + m_serialPortManager->getCurrentPortName());
        m_connectionStatusIcon->setStyleSheet("background-color: #4CAF50; border-radius: 8px;");
        m_connectionStatusIcon->setToolTip("Connected");
    } else {
        m_statusLabel->setText("Disconnected");
        m_connectionStatusIcon->setStyleSheet("background-color: #F44336; border-radius: 8px;");
        m_connectionStatusIcon->setToolTip("Disconnected");
    }
}

QString MainWindow::formatData(const QByteArray &data)
{
    if (m_hexDisplay) {
        QString hex = data.toHex(' ').toUpper();
        return hex;
    } else {
        return QString::fromUtf8(data).trimmed();
    }
}

void MainWindow::logData(const QString &data)
{
    if (m_isLogging && m_logStream) {
        m_logStream->operator<<(data + "\n");
        m_logStream->flush();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    
    m_hexDisplay = settings.value("display/hexMode", false).toBool();
    m_autoScroll = settings.value("display/autoScroll", true).toBool();
    m_showTimestamp = settings.value("display/showTimestamp", true).toBool();
    m_darkMode = settings.value("display/darkMode", false).toBool();
    m_lineEnding = settings.value("connection/lineEnding", "LF").toString();
    
    m_dataBits = static_cast<QSerialPort::DataBits>(
        settings.value("connection/dataBits", QSerialPort::Data8).toInt());
    m_stopBits = static_cast<QSerialPort::StopBits>(
        settings.value("connection/stopBits", QSerialPort::OneStop).toInt());
    m_parity = static_cast<QSerialPort::Parity>(
        settings.value("connection/parity", QSerialPort::NoParity).toInt());
    
    // Load shortcuts
    settings.beginGroup("shortcuts");
    QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        m_shortcuts[key] = settings.value(key).toString();
    }
    settings.endGroup();
    
    // Set default shortcuts if none exist
    if (m_shortcuts.isEmpty()) {
        m_shortcuts["connect"] = "Ctrl+K";
        m_shortcuts["send"] = "Ctrl+Return";
        m_shortcuts["clear"] = "Ctrl+L";
        m_shortcuts["refresh"] = "F5";
        m_shortcuts["theme"] = "Ctrl+D";
    }
    
    // Restore window geometry
    restoreGeometry(settings.value("window/geometry").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings;
    
    settings.setValue("display/hexMode", m_hexDisplay);
    settings.setValue("display/autoScroll", m_autoScroll);
    settings.setValue("display/showTimestamp", m_showTimestamp);
    settings.setValue("display/darkMode", m_darkMode);
    settings.setValue("connection/lineEnding", m_lineEnding);
    
    settings.setValue("connection/dataBits", static_cast<int>(m_dataBits));
    settings.setValue("connection/stopBits", static_cast<int>(m_stopBits));
    settings.setValue("connection/parity", static_cast<int>(m_parity));
    
    // Save shortcuts
    settings.beginGroup("shortcuts");
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        settings.setValue(it.key(), it.value());
    }
    settings.endGroup();
    
    // Save window geometry
    settings.setValue("window/geometry", saveGeometry());
}

void MainWindow::applyShortcuts()
{
    // Clear existing shortcut actions
    for (QAction *action : m_shortcutActions) {
        removeAction(action);
        delete action;
    }
    m_shortcutActions.clear();
    
    // Apply custom shortcuts
    if (m_shortcuts.contains("connect")) {
        QAction *connectAction = new QAction(this);
        connectAction->setShortcut(QKeySequence(m_shortcuts["connect"]));
        connect(connectAction, &QAction::triggered, this, &MainWindow::toggleConnection);
        addAction(connectAction);
        m_shortcutActions.append(connectAction);
    }
    
    if (m_shortcuts.contains("send")) {
        // This is handled by the line edit return press
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
        connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshPorts);
        addAction(refreshAction);
        m_shortcutActions.append(refreshAction);
    }
    
    if (m_shortcuts.contains("theme")) {
        QAction *themeAction = new QAction(this);
        themeAction->setShortcut(QKeySequence(m_shortcuts["theme"]));
        connect(themeAction, &QAction::triggered, this, &MainWindow::toggleDarkMode);
        addAction(themeAction);
        m_shortcutActions.append(themeAction);
    }
}

void MainWindow::toggleDarkMode()
{
    m_darkMode = !m_darkMode;
    applyTheme();
    saveSettings();
}

void MainWindow::applyTheme()
{
    if (m_darkMode) {
        // Dark mode palette
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        
        qApp->setPalette(darkPalette);
        
        // Dark mode stylesheet for specific widgets
        m_outputTextEdit->setStyleSheet(
            "QTextEdit { "
            "   background-color: #1e1e1e; "
            "   color: #d4d4d4; "
            "   border: 1px solid #3e3e3e; "
            "   font-family: 'Courier New', monospace; "
            "}"
        );
        
        // Update theme button for dark mode
        m_themeButton->setText("☀ Light");
        m_themeButton->setStyleSheet(
            "QPushButton {"
            "   color: #ffffff;"
            "   background-color: #3a3a3a;"
            "   border: 1px solid #555555;"
            "   padding: 5px 10px;"
            "   border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #4a4a4a;"
            "}"
        );
    } else {
        // Light mode - reset to default palette
        qApp->setPalette(qApp->style()->standardPalette());
        
        // Light mode stylesheet
        m_outputTextEdit->setStyleSheet(
            "QTextEdit { "
            "   background-color: white; "
            "   color: black; "
            "   border: 1px solid #c0c0c0; "
            "   font-family: 'Courier New', monospace; "
            "}"
        );
        
        // Update theme button for light mode
        m_themeButton->setText("☾ Dark");
        m_themeButton->setStyleSheet(
            "QPushButton {"
            "   color: #2c2c2c;"
            "   font-weight: bold;"
            "   background-color: #f0f0f0;"
            "   border: 1px solid #c0c0c0;"
            "   padding: 5px 10px;"
            "   border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #e0e0e0;"
            "   color: #1a1a1a;"
            "}"
        );
    }
    
    // Update status icon colors to work with theme
    updateConnectionStatus();
}

void MainWindow::updateLineEndingMenu()
{
    // Update the checked state of line ending menu items based on current setting
    m_lfAction->setChecked(m_lineEnding == "LF");
    m_crAction->setChecked(m_lineEnding == "CR");
    m_crlfAction->setChecked(m_lineEnding == "CRLF");
    m_noneAction->setChecked(m_lineEnding == "None");
    
    // Update the combobox to match the current setting
    int index = m_lineEndingComboBox->findText(m_lineEnding);
    if (index >= 0) {
        m_lineEndingComboBox->blockSignals(true);  // Prevent triggering the signal
        m_lineEndingComboBox->setCurrentIndex(index);
        m_lineEndingComboBox->blockSignals(false);
    }
}
