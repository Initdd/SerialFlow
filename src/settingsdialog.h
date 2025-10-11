#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // Getters
    bool hexDisplay() const;
    bool autoScroll() const;
    bool showTimestamp() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::Parity parity() const;
    QMap<QString, QString> shortcuts() const;

    // Setters
    void setHexDisplay(bool enabled);
    void setAutoScroll(bool enabled);
    void setShowTimestamp(bool enabled);
    void setDataBits(QSerialPort::DataBits dataBits);
    void setStopBits(QSerialPort::StopBits stopBits);
    void setParity(QSerialPort::Parity parity);
    void setShortcuts(const QMap<QString, QString> &shortcuts);

private:
    void loadShortcuts();

    Ui::SettingsDialog *ui;
    QMap<QString, QString> m_shortcuts;
};

#endif // SETTINGSDIALOG_H
