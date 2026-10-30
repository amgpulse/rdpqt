#pragma once

#include <QMainWindow>
#include <QProcess>
#include "RdpManager.h"

class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QTextEdit;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onBrowseSharedDrive();
    void onSharedDriveToggled(bool checked);
    void onSessionStarted();
    void onSessionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(const QString &errorMessage);
    void onLogOutput(const QString &message, bool isError);
    void onStateChanged(bool running);
    void togglePasswordVisibility();

private:
    enum class StatusTone {
        Neutral,
        Info,
        Warning,
        Success,
        Error
    };

    void setupUi();
    void applyTheme();
    void setStatus(const QString &text, StatusTone tone);
    void loadSettings();
    void saveSettings();
    RdpConfig buildConfigFromUi() const;

    // UI Widgets
    QCheckBox   *m_darkModeCheck;
    QLineEdit   *m_hostEdit;
    QSpinBox    *m_portSpinBox;
    QLineEdit   *m_usernameEdit;
    QLineEdit   *m_passwordEdit;
    QLineEdit   *m_domainEdit;
    QComboBox   *m_resolutionCombo;
    QCheckBox   *m_clipboardCheck;
    QCheckBox   *m_ignoreCertCheck;
    QCheckBox   *m_sharedDriveCheck;
    QLineEdit   *m_sharedDrivePathEdit;
    QPushButton *m_browseDriveBtn;

    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel      *m_statusLabel;
    QTextEdit   *m_logViewer;
    QPushButton *m_clearLogBtn;
    StatusTone   m_statusTone = StatusTone::Neutral;

    // Process manager
    RdpManager  *m_rdpManager;
};
