#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include "serialportmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QAction;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Serial port actions
    void refreshPorts();
    void toggleConnection();
    void sendData();
    void onDataReceived(const QByteArray &data);
    void onConnectionStatusChanged(bool connected);
    void onErrorOccurred(const QString &error);
    
    // UI actions
    void clearOutput();
    void toggleLogging();
    void openSettings();
    void updateConnectionStatus();

private:
    void createMenuBar();
    void createStatusBar();
    void loadSettings();
    void saveSettings();
    void applyShortcuts();
    void updateLineEndingMenu();
    
    QString formatData(const QByteArray &data);
    void logData(const QString &data);
    
    Ui::MainWindow *ui;
    
    // Serial port manager
    SerialPortManager *m_serialPortManager;
    
    // Status indicators
    QLabel *m_statusLabel;
    QLabel *m_connectionStatusIcon;
    
    // Settings
    bool m_hexDisplay;
    bool m_autoScroll;
    bool m_showTimestamp;
    bool m_isLogging;
    QString m_lineEnding; // Line ending: "LF", "CR", "CRLF", or "None"
    QString m_logFilePath;
    QFile *m_logFile;
    QTextStream *m_logStream;
    
    // Connection settings
    QSerialPort::DataBits m_dataBits;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::Parity m_parity;
    
    // Shortcuts (stored as strings in settings)
    QMap<QString, QString> m_shortcuts;
    QList<QAction*> m_shortcutActions; // Track shortcut actions for cleanup
    
    // Line ending menu actions
    QAction *m_lfAction;
    QAction *m_crAction;
    QAction *m_crlfAction;
    QAction *m_noneAction;
};

#endif // MAINWINDOW_H
