#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTabWidget>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    
    // Set up data bits combo box with user data
    ui->dataBitsComboBox->setItemData(0, QSerialPort::Data5);
    ui->dataBitsComboBox->setItemData(1, QSerialPort::Data6);
    ui->dataBitsComboBox->setItemData(2, QSerialPort::Data7);
    ui->dataBitsComboBox->setItemData(3, QSerialPort::Data8);
    
    // Set up stop bits combo box with user data
    ui->stopBitsComboBox->setItemData(0, QSerialPort::OneStop);
    ui->stopBitsComboBox->setItemData(1, QSerialPort::OneAndHalfStop);
    ui->stopBitsComboBox->setItemData(2, QSerialPort::TwoStop);
    
    // Set up parity combo box with user data
    ui->parityComboBox->setItemData(0, QSerialPort::NoParity);
    ui->parityComboBox->setItemData(1, QSerialPort::EvenParity);
    ui->parityComboBox->setItemData(2, QSerialPort::OddParity);
    ui->parityComboBox->setItemData(3, QSerialPort::SpaceParity);
    ui->parityComboBox->setItemData(4, QSerialPort::MarkParity);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::hexDisplay() const
{
    return ui->hexDisplayCheckBox->isChecked();
}

bool SettingsDialog::autoScroll() const
{
    return ui->autoScrollCheckBox->isChecked();
}

bool SettingsDialog::showTimestamp() const
{
    return ui->showTimestampCheckBox->isChecked();
}

void SettingsDialog::setHexDisplay(bool enabled)
{
    ui->hexDisplayCheckBox->setChecked(enabled);
}

void SettingsDialog::setAutoScroll(bool enabled)
{
    ui->autoScrollCheckBox->setChecked(enabled);
}

void SettingsDialog::setShowTimestamp(bool enabled)
{
    ui->showTimestampCheckBox->setChecked(enabled);
}

QSerialPort::DataBits SettingsDialog::dataBits() const
{
    return static_cast<QSerialPort::DataBits>(
        ui->dataBitsComboBox->currentData().toInt());
}

QSerialPort::StopBits SettingsDialog::stopBits() const
{
    return static_cast<QSerialPort::StopBits>(
        ui->stopBitsComboBox->currentData().toInt());
}

QSerialPort::Parity SettingsDialog::parity() const
{
    return static_cast<QSerialPort::Parity>(
        ui->parityComboBox->currentData().toInt());
}

void SettingsDialog::setDataBits(QSerialPort::DataBits dataBits)
{
    for (int i = 0; i < ui->dataBitsComboBox->count(); ++i) {
        if (ui->dataBitsComboBox->itemData(i).toInt() == static_cast<int>(dataBits)) {
            ui->dataBitsComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsDialog::setStopBits(QSerialPort::StopBits stopBits)
{
    for (int i = 0; i < ui->stopBitsComboBox->count(); ++i) {
        if (ui->stopBitsComboBox->itemData(i).toInt() == static_cast<int>(stopBits)) {
            ui->stopBitsComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsDialog::setParity(QSerialPort::Parity parity)
{
    for (int i = 0; i < ui->parityComboBox->count(); ++i) {
        if (ui->parityComboBox->itemData(i).toInt() == static_cast<int>(parity)) {
            ui->parityComboBox->setCurrentIndex(i);
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
    if (m_shortcuts.contains("connect")) {
        ui->connectEdit->setKeySequence(QKeySequence(m_shortcuts["connect"]));
    }
    if (m_shortcuts.contains("send")) {
        ui->sendEdit->setKeySequence(QKeySequence(m_shortcuts["send"]));
    }
    if (m_shortcuts.contains("clear")) {
        ui->clearEdit->setKeySequence(QKeySequence(m_shortcuts["clear"]));
    }
    if (m_shortcuts.contains("refresh")) {
        ui->refreshEdit->setKeySequence(QKeySequence(m_shortcuts["refresh"]));
    }
}

QMap<QString, QString> SettingsDialog::shortcuts() const
{
    QMap<QString, QString> currentShortcuts = m_shortcuts;
    currentShortcuts["connect"] = ui->connectEdit->keySequence().toString();
    currentShortcuts["send"] = ui->sendEdit->keySequence().toString();
    currentShortcuts["clear"] = ui->clearEdit->keySequence().toString();
    currentShortcuts["refresh"] = ui->refreshEdit->keySequence().toString();
    return currentShortcuts;
}
