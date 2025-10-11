#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTabWidget>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setupUI();
    resize(500, 400);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    QTabWidget *tabWidget = new QTabWidget(this);
    
    // ========== Display Settings Tab ==========
    QWidget *displayTab = new QWidget();
    QVBoxLayout *displayLayout = new QVBoxLayout(displayTab);
    
    QGroupBox *displayGroup = new QGroupBox("Display Options");
    QVBoxLayout *displayGroupLayout = new QVBoxLayout(displayGroup);
    
    m_hexDisplayCheckBox = new QCheckBox("Display data in HEX format");
    m_hexDisplayCheckBox->setToolTip("Show received data as hexadecimal values");
    displayGroupLayout->addWidget(m_hexDisplayCheckBox);
    
    m_autoScrollCheckBox = new QCheckBox("Auto-scroll to bottom");
    m_autoScrollCheckBox->setToolTip("Automatically scroll to the latest received data");
    displayGroupLayout->addWidget(m_autoScrollCheckBox);
    
    m_showTimestampCheckBox = new QCheckBox("Show timestamps");
    m_showTimestampCheckBox->setToolTip("Display timestamp for each message");
    displayGroupLayout->addWidget(m_showTimestampCheckBox);
    
    displayLayout->addWidget(displayGroup);
    displayLayout->addStretch();
    
    tabWidget->addTab(displayTab, "Display");
    
    // ========== Connection Settings Tab ==========
    QWidget *connectionTab = new QWidget();
    QFormLayout *connectionLayout = new QFormLayout(connectionTab);
    
    QGroupBox *connectionGroup = new QGroupBox("Serial Port Configuration");
    QFormLayout *connectionGroupLayout = new QFormLayout(connectionGroup);
    
    // Data bits
    m_dataBitsComboBox = new QComboBox();
    m_dataBitsComboBox->addItem("5", QSerialPort::Data5);
    m_dataBitsComboBox->addItem("6", QSerialPort::Data6);
    m_dataBitsComboBox->addItem("7", QSerialPort::Data7);
    m_dataBitsComboBox->addItem("8", QSerialPort::Data8);
    m_dataBitsComboBox->setCurrentIndex(3); // Default: 8
    connectionGroupLayout->addRow("Data Bits:", m_dataBitsComboBox);
    
    // Stop bits
    m_stopBitsComboBox = new QComboBox();
    m_stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    m_stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    m_stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
    m_stopBitsComboBox->setCurrentIndex(0); // Default: 1
    connectionGroupLayout->addRow("Stop Bits:", m_stopBitsComboBox);
    
    // Parity
    m_parityComboBox = new QComboBox();
    m_parityComboBox->addItem("None", QSerialPort::NoParity);
    m_parityComboBox->addItem("Even", QSerialPort::EvenParity);
    m_parityComboBox->addItem("Odd", QSerialPort::OddParity);
    m_parityComboBox->addItem("Space", QSerialPort::SpaceParity);
    m_parityComboBox->addItem("Mark", QSerialPort::MarkParity);
    m_parityComboBox->setCurrentIndex(0); // Default: None
    connectionGroupLayout->addRow("Parity:", m_parityComboBox);
    
    connectionLayout->addRow(connectionGroup);
    
    QLabel *noteLabel = new QLabel("<i>Note: These settings will be applied on next connection.</i>");
    noteLabel->setWordWrap(true);
    connectionLayout->addRow(noteLabel);
    
    tabWidget->addTab(connectionTab, "Connection");
    
    // ========== Shortcuts Tab ==========
    QWidget *shortcutsTab = new QWidget();
    QFormLayout *shortcutsLayout = new QFormLayout(shortcutsTab);
    
    QGroupBox *shortcutsGroup = new QGroupBox("Keyboard Shortcuts");
    QFormLayout *shortcutsGroupLayout = new QFormLayout(shortcutsGroup);
    
    // Connect/Disconnect
    QKeySequenceEdit *connectEdit = new QKeySequenceEdit();
    m_shortcutEdits["connect"] = connectEdit;
    shortcutsGroupLayout->addRow("Connect/Disconnect:", connectEdit);
    
    // Send
    QKeySequenceEdit *sendEdit = new QKeySequenceEdit();
    m_shortcutEdits["send"] = sendEdit;
    shortcutsGroupLayout->addRow("Send Data:", sendEdit);
    
    // Clear
    QKeySequenceEdit *clearEdit = new QKeySequenceEdit();
    m_shortcutEdits["clear"] = clearEdit;
    shortcutsGroupLayout->addRow("Clear Output:", clearEdit);
    
    // Refresh
    QKeySequenceEdit *refreshEdit = new QKeySequenceEdit();
    m_shortcutEdits["refresh"] = refreshEdit;
    shortcutsGroupLayout->addRow("Refresh Ports:", refreshEdit);
    
    // Toggle Theme
    QKeySequenceEdit *themeEdit = new QKeySequenceEdit();
    m_shortcutEdits["theme"] = themeEdit;
    shortcutsGroupLayout->addRow("Toggle Dark Mode:", themeEdit);
    
    shortcutsLayout->addRow(shortcutsGroup);
    
    QLabel *shortcutNote = new QLabel("<i>Note: Changes will be applied immediately after clicking OK.</i>");
    shortcutNote->setWordWrap(true);
    shortcutsLayout->addRow(shortcutNote);
    
    tabWidget->addTab(shortcutsTab, "Shortcuts");
    
    mainLayout->addWidget(tabWidget);
    
    // ========== Button Box ==========
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

QSerialPort::DataBits SettingsDialog::dataBits() const
{
    return static_cast<QSerialPort::DataBits>(
        m_dataBitsComboBox->currentData().toInt());
}

QSerialPort::StopBits SettingsDialog::stopBits() const
{
    return static_cast<QSerialPort::StopBits>(
        m_stopBitsComboBox->currentData().toInt());
}

QSerialPort::Parity SettingsDialog::parity() const
{
    return static_cast<QSerialPort::Parity>(
        m_parityComboBox->currentData().toInt());
}

void SettingsDialog::setDataBits(QSerialPort::DataBits dataBits)
{
    for (int i = 0; i < m_dataBitsComboBox->count(); ++i) {
        if (m_dataBitsComboBox->itemData(i).toInt() == static_cast<int>(dataBits)) {
            m_dataBitsComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsDialog::setStopBits(QSerialPort::StopBits stopBits)
{
    for (int i = 0; i < m_stopBitsComboBox->count(); ++i) {
        if (m_stopBitsComboBox->itemData(i).toInt() == static_cast<int>(stopBits)) {
            m_stopBitsComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsDialog::setParity(QSerialPort::Parity parity)
{
    for (int i = 0; i < m_parityComboBox->count(); ++i) {
        if (m_parityComboBox->itemData(i).toInt() == static_cast<int>(parity)) {
            m_parityComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsDialog::setShortcuts(const QMap<QString, QString> &shortcuts)
{
    m_shortcuts = shortcuts;
    loadShortcuts();
}

void SettingsDialog::loadShortcuts()
{
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        if (m_shortcutEdits.contains(it.key())) {
            m_shortcutEdits[it.key()]->setKeySequence(QKeySequence(it.value()));
        }
    }
}

QMap<QString, QString> SettingsDialog::shortcuts() const
{
    // Create a copy and update it with current shortcut values
    QMap<QString, QString> currentShortcuts = m_shortcuts;
    for (auto it = m_shortcutEdits.constBegin(); it != m_shortcutEdits.constEnd(); ++it) {
        currentShortcuts[it.key()] = it.value()->keySequence().toString();
    }
    return currentShortcuts;
}
