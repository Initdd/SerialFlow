#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSerialPort>
#include <QMap>
#include <QLineEdit>
#include <QPushButton>
#include <QKeySequenceEdit>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // Getters
    bool hexDisplay() const { return m_hexDisplayCheckBox->isChecked(); }
    bool autoScroll() const { return m_autoScrollCheckBox->isChecked(); }
    bool showTimestamp() const { return m_showTimestampCheckBox->isChecked(); }
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::Parity parity() const;
    QMap<QString, QString> shortcuts() const;

    // Setters
    void setHexDisplay(bool enabled) { m_hexDisplayCheckBox->setChecked(enabled); }
    void setAutoScroll(bool enabled) { m_autoScrollCheckBox->setChecked(enabled); }
    void setShowTimestamp(bool enabled) { m_showTimestampCheckBox->setChecked(enabled); }
    void setDataBits(QSerialPort::DataBits dataBits);
    void setStopBits(QSerialPort::StopBits stopBits);
    void setParity(QSerialPort::Parity parity);
    void setShortcuts(const QMap<QString, QString> &shortcuts);

private:
    void setupUI();
    void loadShortcuts();

    // Display settings
    QCheckBox *m_hexDisplayCheckBox;
    QCheckBox *m_autoScrollCheckBox;
    QCheckBox *m_showTimestampCheckBox;

    // Connection settings
    QComboBox *m_dataBitsComboBox;
    QComboBox *m_stopBitsComboBox;
    QComboBox *m_parityComboBox;

    // Shortcuts
    QMap<QString, QString> m_shortcuts;
    QMap<QString, QKeySequenceEdit*> m_shortcutEdits;
};

#endif // SETTINGSDIALOG_H
