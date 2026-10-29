#include "MainWindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QAction>
#include <QStyle>
#include <QFontDatabase>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_rdpManager(new RdpManager(this))
{
    setupUi();
    loadSettings();

    // Connect RdpManager signals
    connect(m_rdpManager, &RdpManager::sessionStarted,
            this, &MainWindow::onSessionStarted);
    connect(m_rdpManager, &RdpManager::sessionFinished,
            this, &MainWindow::onSessionFinished);
    connect(m_rdpManager, &RdpManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_rdpManager, &RdpManager::logOutput,
            this, &MainWindow::onLogOutput);
    connect(m_rdpManager, &RdpManager::stateChanged,
            this, &MainWindow::onStateChanged);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("rdpqt"));
    resize(700, 680);
    setMinimumSize(580, 560);

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ==========================================
    // 1. Connection Details Group
    // ==========================================
    auto *connGroup = new QGroupBox(tr("Connection Details"), centralWidget);
    auto *connLayout = new QGridLayout(connGroup);
    connLayout->setSpacing(10);

    // Host & Port
    auto *hostLabel = new QLabel(tr("Host / IP:"), connGroup);
    m_hostEdit = new QLineEdit(connGroup);
    m_hostEdit->setPlaceholderText(tr("e.g. 192.168.1.100 or server.domain.com"));

    auto *portLabel = new QLabel(tr("Port:"), connGroup);
    m_portSpinBox = new QSpinBox(connGroup);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(3389);
    m_portSpinBox->setFixedWidth(90);

    auto *hostPortLayout = new QHBoxLayout();
    hostPortLayout->addWidget(m_hostEdit);
    hostPortLayout->addWidget(portLabel);
    hostPortLayout->addWidget(m_portSpinBox);

    connLayout->addWidget(hostLabel, 0, 0);
    connLayout->addLayout(hostPortLayout, 0, 1);

    // Username & Domain
    auto *userLabel = new QLabel(tr("Username:"), connGroup);
    m_usernameEdit = new QLineEdit(connGroup);
    m_usernameEdit->setPlaceholderText(tr("e.g. Administrator"));

    auto *domainLabel = new QLabel(tr("Domain:"), connGroup);
    m_domainEdit = new QLineEdit(connGroup);
    m_domainEdit->setPlaceholderText(tr("Optional (e.g. WORKGROUP)"));
    m_domainEdit->setFixedWidth(160);

    auto *userDomainLayout = new QHBoxLayout();
    userDomainLayout->addWidget(m_usernameEdit);
    userDomainLayout->addWidget(domainLabel);
    userDomainLayout->addWidget(m_domainEdit);

    connLayout->addWidget(userLabel, 1, 0);
    connLayout->addLayout(userDomainLayout, 1, 1);

    // Password
    auto *passLabel = new QLabel(tr("Password:"), connGroup);
    m_passwordEdit = new QLineEdit(connGroup);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Leave empty to prompt if required"));

    // Password visibility toggle action
    auto *togglePassAction = m_passwordEdit->addAction(
        style()->standardIcon(QStyle::SP_FileDialogContentsView),
        QLineEdit::TrailingPosition
    );
    togglePassAction->setToolTip(tr("Show / Hide password"));
    connect(togglePassAction, &QAction::triggered, this, &MainWindow::togglePasswordVisibility);

    connLayout->addWidget(passLabel, 2, 0);
    connLayout->addWidget(m_passwordEdit, 2, 1);

    mainLayout->addWidget(connGroup);

    // ==========================================
    // 2. Settings Group (Resolution, Features, Shared Drive)
    // ==========================================
    auto *settingsGroup = new QGroupBox(tr("Display & Session Settings"), centralWidget);
    auto *settingsLayout = new QGridLayout(settingsGroup);
    settingsLayout->setSpacing(10);

    // Resolution
    auto *resLabel = new QLabel(tr("Resolution:"), settingsGroup);
    m_resolutionCombo = new QComboBox(settingsGroup);
    m_resolutionCombo->setEditable(true);
    m_resolutionCombo->addItem(tr("1920x1080 (Full HD)"), "1920x1080");
    m_resolutionCombo->addItem(tr("1366x768"), "1366x768");
    m_resolutionCombo->addItem(tr("1280x720 (HD)"), "1280x720");
    m_resolutionCombo->addItem(tr("1600x900"), "1600x900");
    m_resolutionCombo->addItem(tr("2560x1440 (2K)"), "2560x1440");
    m_resolutionCombo->addItem(tr("3840x2160 (4K)"), "3840x2160");
    m_resolutionCombo->addItem(tr("Dynamic Resolution"), "dynamic");
    m_resolutionCombo->addItem(tr("Fullscreen (/f)"), "fullscreen");
    m_resolutionCombo->setCurrentIndex(0);

    settingsLayout->addWidget(resLabel, 0, 0);
    settingsLayout->addWidget(m_resolutionCombo, 0, 1, 1, 2);

    // Checkboxes: Clipboard & Cert
    m_clipboardCheck = new QCheckBox(tr("Enable Clipboard Redirection (+clipboard)"), settingsGroup);
    m_clipboardCheck->setChecked(true);
    m_ignoreCertCheck = new QCheckBox(tr("Ignore SSL/TLS Certificate Warnings (/cert:ignore)"), settingsGroup);
    m_ignoreCertCheck->setChecked(true);

    settingsLayout->addWidget(m_clipboardCheck, 1, 0, 1, 3);
    settingsLayout->addWidget(m_ignoreCertCheck, 2, 0, 1, 3);

    // Shared Drive
    m_sharedDriveCheck = new QCheckBox(tr("Share Local Folder (/drive):"), settingsGroup);
    m_sharedDrivePathEdit = new QLineEdit(settingsGroup);
    m_sharedDrivePathEdit->setPlaceholderText(tr("Select local folder to share with remote desktop..."));
    m_sharedDrivePathEdit->setEnabled(false);

    m_browseDriveBtn = new QPushButton(tr("Browse..."), settingsGroup);
    m_browseDriveBtn->setEnabled(false);

    connect(m_sharedDriveCheck, &QCheckBox::toggled, this, &MainWindow::onSharedDriveToggled);
    connect(m_browseDriveBtn, &QPushButton::clicked, this, &MainWindow::onBrowseSharedDrive);

    settingsLayout->addWidget(m_sharedDriveCheck, 3, 0);
    settingsLayout->addWidget(m_sharedDrivePathEdit, 3, 1);
    settingsLayout->addWidget(m_browseDriveBtn, 3, 2);

    mainLayout->addWidget(settingsGroup);

    // ==========================================
    // 3. Actions & Status
    // ==========================================
    auto *actionLayout = new QHBoxLayout();

    m_statusLabel = new QLabel(tr("Status: Ready to connect"), centralWidget);
    m_statusLabel->setStyleSheet("font-weight: bold; color: #555;");

    m_connectBtn = new QPushButton(tr("Connect"), centralWidget);
    m_connectBtn->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; padding: 7px 20px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #388e3c; }"
        "QPushButton:disabled { background-color: #81c784; }"
    );

    m_disconnectBtn = new QPushButton(tr("Disconnect"), centralWidget);
    m_disconnectBtn->setEnabled(false);
    m_disconnectBtn->setStyleSheet(
        "QPushButton { background-color: #c62828; color: white; font-weight: bold; padding: 7px 20px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:disabled { background-color: #e57373; }"
    );

    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    actionLayout->addWidget(m_statusLabel);
    actionLayout->addStretch();
    actionLayout->addWidget(m_connectBtn);
    actionLayout->addWidget(m_disconnectBtn);

    mainLayout->addLayout(actionLayout);

    // ==========================================
    // 4. Console / Log Viewer Group
    // ==========================================
    auto *logGroup = new QGroupBox(tr("Process Output & Logs"), centralWidget);
    auto *logLayout = new QVBoxLayout(logGroup);

    m_logViewer = new QTextEdit(logGroup);
    m_logViewer->setReadOnly(true);
    m_logViewer->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_logViewer->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-size: 12px; }");

    auto *logButtonLayout = new QHBoxLayout();
    m_clearLogBtn = new QPushButton(tr("Clear Log"), logGroup);
    connect(m_clearLogBtn, &QPushButton::clicked, m_logViewer, &QTextEdit::clear);
    logButtonLayout->addStretch();
    logButtonLayout->addWidget(m_clearLogBtn);

    logLayout->addWidget(m_logViewer);
    logLayout->addLayout(logButtonLayout);

    mainLayout->addWidget(logGroup, 1);
}

void MainWindow::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
    }
}

void MainWindow::onSharedDriveToggled(bool checked)
{
    m_sharedDrivePathEdit->setEnabled(checked);
    m_browseDriveBtn->setEnabled(checked);
}

void MainWindow::onBrowseSharedDrive()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Folder to Share via RDP"),
        m_sharedDrivePathEdit->text().isEmpty() ? QDir::homePath() : m_sharedDrivePathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        m_sharedDrivePathEdit->setText(dir);
    }
}

RdpConfig MainWindow::buildConfigFromUi() const
{
    RdpConfig cfg;
    cfg.host = m_hostEdit->text().trimmed();
    cfg.port = m_portSpinBox->value();
    cfg.username = m_usernameEdit->text().trimmed();
    cfg.password = m_passwordEdit->text();
    cfg.domain = m_domainEdit->text().trimmed();

    // Check resolution combo
    const QVariant itemData = m_resolutionCombo->currentData();
    if (itemData.isValid() && !itemData.toString().isEmpty()) {
        cfg.resolution = itemData.toString();
    } else {
        cfg.resolution = m_resolutionCombo->currentText().trimmed();
    }

    cfg.enableClipboard = m_clipboardCheck->isChecked();
    cfg.ignoreCert = m_ignoreCertCheck->isChecked();
    cfg.enableSharedDrive = m_sharedDriveCheck->isChecked();
    cfg.sharedDrivePath = m_sharedDrivePathEdit->text().trimmed();

    return cfg;
}

void MainWindow::onConnectClicked()
{
    const RdpConfig cfg = buildConfigFromUi();
    if (cfg.host.isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Please enter a valid Host/IP address."));
        m_hostEdit->setFocus();
        return;
    }

    if (cfg.enableSharedDrive && (cfg.sharedDrivePath.isEmpty() || !QDir(cfg.sharedDrivePath).exists())) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Selected shared folder does not exist."));
        return;
    }

    saveSettings();

    m_statusLabel->setText(tr("Status: Starting xfreerdp..."));
    m_statusLabel->setStyleSheet("font-weight: bold; color: #1976d2;");
    m_rdpManager->startSession(cfg);
}

void MainWindow::onDisconnectClicked()
{
    m_statusLabel->setText(tr("Status: Disconnecting..."));
    m_statusLabel->setStyleSheet("font-weight: bold; color: #e65100;");
    m_rdpManager->stopSession();
}

void MainWindow::onSessionStarted()
{
    m_statusLabel->setText(tr("Status: Connected / Running"));
    m_statusLabel->setStyleSheet("font-weight: bold; color: #2e7d32;");
}

void MainWindow::onSessionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    m_statusLabel->setText(QString(tr("Status: Disconnected (Exit code: %1)")).arg(exitCode));
    m_statusLabel->setStyleSheet("font-weight: bold; color: #555;");
}

void MainWindow::onErrorOccurred(const QString &errorMessage)
{
    QMessageBox::critical(this, tr("RDP Error"), errorMessage);
    m_statusLabel->setText(tr("Status: Error"));
    m_statusLabel->setStyleSheet("font-weight: bold; color: #c62828;");
}

void MainWindow::onLogOutput(const QString &message, bool isError)
{
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    const QString color = isError ? "#f44336" : "#4caf50";
    const QString formatted = QString("<span style='color: #888;'>[%1]</span> <span style='color: %2;'>%3</span>")
                                  .arg(timestamp, color, message.toHtmlEscaped());
    m_logViewer->append(formatted);
}

void MainWindow::onStateChanged(bool running)
{
    m_connectBtn->setEnabled(!running);
    m_disconnectBtn->setEnabled(running);

    m_hostEdit->setEnabled(!running);
    m_portSpinBox->setEnabled(!running);
    m_usernameEdit->setEnabled(!running);
    m_passwordEdit->setEnabled(!running);
    m_domainEdit->setEnabled(!running);
    m_resolutionCombo->setEnabled(!running);
    m_clipboardCheck->setEnabled(!running);
    m_ignoreCertCheck->setEnabled(!running);
    m_sharedDriveCheck->setEnabled(!running);
    m_sharedDrivePathEdit->setEnabled(!running && m_sharedDriveCheck->isChecked());
    m_browseDriveBtn->setEnabled(!running && m_sharedDriveCheck->isChecked());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_rdpManager->isRunning()) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Active Session"),
            tr("An active FreeRDP session is running. Do you want to terminate it and exit?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_rdpManager->stopSession();
            saveSettings();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        saveSettings();
        event->accept();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("FreeRDPClient", "RdpQt");
    m_hostEdit->setText(settings.value("host", "").toString());
    m_portSpinBox->setValue(settings.value("port", 3389).toInt());
    m_usernameEdit->setText(settings.value("username", "").toString());
    m_domainEdit->setText(settings.value("domain", "").toString());

    const QString res = settings.value("resolution", "1920x1080").toString();
    const int idx = m_resolutionCombo->findData(res);
    if (idx != -1) {
        m_resolutionCombo->setCurrentIndex(idx);
    } else {
        m_resolutionCombo->setCurrentText(res);
    }

    m_clipboardCheck->setChecked(settings.value("clipboard", true).toBool());
    m_ignoreCertCheck->setChecked(settings.value("ignoreCert", true).toBool());

    const bool shareDrive = settings.value("shareDrive", false).toBool();
    m_sharedDriveCheck->setChecked(shareDrive);
    m_sharedDrivePathEdit->setText(settings.value("sharedDrivePath", "").toString());
    onSharedDriveToggled(shareDrive);
}

void MainWindow::saveSettings()
{
    QSettings settings("FreeRDPClient", "RdpQt");
    settings.setValue("host", m_hostEdit->text().trimmed());
    settings.setValue("port", m_portSpinBox->value());
    settings.setValue("username", m_usernameEdit->text().trimmed());
    settings.setValue("domain", m_domainEdit->text().trimmed());

    const QVariant resData = m_resolutionCombo->currentData();
    if (resData.isValid() && !resData.toString().isEmpty()) {
        settings.setValue("resolution", resData.toString());
    } else {
        settings.setValue("resolution", m_resolutionCombo->currentText().trimmed());
    }

    settings.setValue("clipboard", m_clipboardCheck->isChecked());
    settings.setValue("ignoreCert", m_ignoreCertCheck->isChecked());
    settings.setValue("shareDrive", m_sharedDriveCheck->isChecked());
    settings.setValue("sharedDrivePath", m_sharedDrivePathEdit->text().trimmed());
}
