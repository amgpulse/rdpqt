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
#include <QInputDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QAction>
#include <QStyle>
#include <QFontDatabase>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QEvent>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_rdpManager(new RdpManager(this))
{
    setupUi();
    setupTrayIcon();
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
    // Profile / Bookmarks & Theme Bar
    // ==========================================
    auto *profileLayout = new QHBoxLayout();
    profileLayout->setSpacing(8);

    auto *profileLabel = new QLabel(tr("Profile:"), centralWidget);
    profileLabel->setStyleSheet("font-weight: bold;");

    m_profileCombo = new QComboBox(centralWidget);
    m_profileCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_profileCombo->setToolTip(tr("Select saved connection profile"));

    m_newProfileBtn = new QPushButton(tr("New"), centralWidget);
    m_newProfileBtn->setToolTip(tr("Create a new connection profile"));

    m_saveProfileBtn = new QPushButton(tr("Save"), centralWidget);
    m_saveProfileBtn->setToolTip(tr("Save current settings to selected profile"));

    m_renameProfileBtn = new QPushButton(tr("Rename"), centralWidget);
    m_renameProfileBtn->setToolTip(tr("Rename selected profile"));

    m_deleteProfileBtn = new QPushButton(tr("Delete"), centralWidget);
    m_deleteProfileBtn->setToolTip(tr("Delete selected profile"));

    m_darkModeCheck = new QCheckBox(tr("Dark mode"), centralWidget);

    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onProfileIndexChanged);
    connect(m_newProfileBtn, &QPushButton::clicked,
            this, &MainWindow::onNewProfileClicked);
    connect(m_saveProfileBtn, &QPushButton::clicked,
            this, &MainWindow::onSaveProfileClicked);
    connect(m_renameProfileBtn, &QPushButton::clicked,
            this, &MainWindow::onRenameProfileClicked);
    connect(m_deleteProfileBtn, &QPushButton::clicked,
            this, &MainWindow::onDeleteProfileClicked);

    connect(m_darkModeCheck, &QCheckBox::toggled, this, [this](bool enabled) {
        applyTheme();
        QSettings settings("FreeRDPClient", "RdpQt");
        settings.setValue("darkMode", enabled);
    });

    profileLayout->addWidget(profileLabel);
    profileLayout->addWidget(m_profileCombo, 1);
    profileLayout->addWidget(m_newProfileBtn);
    profileLayout->addWidget(m_saveProfileBtn);
    profileLayout->addWidget(m_renameProfileBtn);
    profileLayout->addWidget(m_deleteProfileBtn);
    profileLayout->addSpacing(12);
    profileLayout->addWidget(m_darkModeCheck);

    mainLayout->addLayout(profileLayout);

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

    m_connectBtn = new QPushButton(tr("Connect"), centralWidget);
    m_connectBtn->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; padding: 7px 20px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #388e3c; }"
        "QPushButton:disabled { background-color: #81c784; }"
    );

    m_disconnectBtn = new QPushButton(tr("Disconnect"), centralWidget);
    m_disconnectBtn->setVisible(false);
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

    auto *logButtonLayout = new QHBoxLayout();
    m_clearLogBtn = new QPushButton(tr("Clear Log"), logGroup);
    connect(m_clearLogBtn, &QPushButton::clicked, m_logViewer, &QTextEdit::clear);
    logButtonLayout->addStretch();
    logButtonLayout->addWidget(m_clearLogBtn);

    logLayout->addWidget(m_logViewer);
    logLayout->addLayout(logButtonLayout);

    mainLayout->addWidget(logGroup, 1);
    applyTheme();
}

void MainWindow::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
    }
}

void MainWindow::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    m_trayMenu = new QMenu(this);
    m_trayStatusAction = new QAction(tr("Connection: Disconnected"), this);
    m_trayStatusAction->setEnabled(false);
    m_trayToggleAction = new QAction(tr("Show / Restore"), this);
    m_trayQuitAction = new QAction(tr("Quit"), this);

    connect(m_trayToggleAction, &QAction::triggered, this, &MainWindow::toggleWindowVisibility);
    connect(m_trayQuitAction, &QAction::triggered, this, [this]() {
        if (m_rdpManager->isRunning()) {
            const QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                tr("Active Session"),
                tr("An active FreeRDP session is running. Quit rdpqt and stop the session?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );

            if (reply != QMessageBox::Yes) {
                return;
            }

            m_rdpManager->stopSession();
        }

        saveSettings();
        qApp->quit();
    });

    m_trayMenu->addAction(m_trayStatusAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_trayToggleAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_trayQuitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setToolTip(tr("rdpqt"));
    m_trayIcon->setIcon(QIcon(QStringLiteral(":/icons/app-icon")));

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            toggleWindowVisibility();
        }
    });

    m_trayIcon->show();
    m_trayToggleAction->setText(tr("Show / Restore"));
    updateTrayStatus(tr("Disconnected"));
}

void MainWindow::toggleWindowVisibility()
{
    if (m_trayIcon == nullptr) {
        return;
    }

    if (isVisible()) {
        hide();
        m_trayToggleAction->setText(tr("Show / Restore"));
    } else {
        show();
        raise();
        activateWindow();
        m_trayToggleAction->setText(tr("Hide to tray"));
    }
}

void MainWindow::updateTrayStatus(const QString &status)
{
    if (m_trayIcon == nullptr || m_trayStatusAction == nullptr) {
        return;
    }

    const QString displayStatus = m_currentProfileName.isEmpty()
        ? status
        : QString("%1 (%2)").arg(status, m_currentProfileName);

    m_trayStatusAction->setText(tr("Connection: %1").arg(displayStatus));
    m_trayIcon->setToolTip(tr("rdpqt - %1").arg(displayStatus));
}

void MainWindow::onProfileIndexChanged(int index)
{
    if (index < 0 || m_isUpdatingProfileList) {
        return;
    }

    const QString newProfile = m_profileCombo->itemText(index);
    if (newProfile.isEmpty()) {
        return;
    }

    // Auto-save the previous profile before switching
    if (!m_currentProfileName.isEmpty() && m_currentProfileName != newProfile) {
        saveProfile(m_currentProfileName);
    }

    m_currentProfileName = newProfile;
    loadProfile(m_currentProfileName);

    QSettings settings("FreeRDPClient", "RdpQt");
    settings.setValue("currentProfile", m_currentProfileName);
}

void MainWindow::onNewProfileClicked()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this,
        tr("New Profile"),
        tr("Enter profile name:"),
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok || name.isEmpty()) {
        return;
    }

    if (m_profileCombo->findText(name) != -1) {
        QMessageBox::warning(this, tr("Profile Exists"), tr("A profile named '%1' already exists.").arg(name));
        return;
    }

    // Save previous active profile first
    if (!m_currentProfileName.isEmpty()) {
        saveProfile(m_currentProfileName);
    }

    // Save the new profile with current UI values
    m_currentProfileName = name;
    saveProfile(name);

    updateProfileList(name);
    setStatus(tr("Status: Profile '%1' created").arg(name), StatusTone::Success);
}

void MainWindow::onSaveProfileClicked()
{
    if (m_currentProfileName.isEmpty()) {
        return;
    }

    saveProfile(m_currentProfileName);
    setStatus(tr("Status: Profile '%1' saved").arg(m_currentProfileName), StatusTone::Info);
}

void MainWindow::onRenameProfileClicked()
{
    if (m_currentProfileName.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString newName = QInputDialog::getText(
        this,
        tr("Rename Profile"),
        tr("Enter new name for profile '%1':").arg(m_currentProfileName),
        QLineEdit::Normal,
        m_currentProfileName,
        &ok
    ).trimmed();

    if (!ok || newName.isEmpty() || newName == m_currentProfileName) {
        return;
    }

    if (m_profileCombo->findText(newName) != -1) {
        QMessageBox::warning(this, tr("Profile Exists"), tr("A profile named '%1' already exists.").arg(newName));
        return;
    }

    // Save current settings first
    saveProfile(m_currentProfileName);

    // Copy keys from old profile group to new group
    QSettings settings("FreeRDPClient", "RdpQt");
    settings.beginGroup("Profiles");
    settings.beginGroup(m_currentProfileName);
    const QStringList keys = settings.allKeys();
    QMap<QString, QVariant> values;
    for (const QString &k : keys) {
        values[k] = settings.value(k);
    }
    settings.endGroup();

    // Remove old profile
    settings.remove(m_currentProfileName);

    // Save under new name
    settings.beginGroup(newName);
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        settings.setValue(it.key(), it.value());
    }
    settings.endGroup();
    settings.endGroup();

    m_currentProfileName = newName;
    settings.setValue("currentProfile", m_currentProfileName);

    updateProfileList(m_currentProfileName);
    setStatus(tr("Status: Profile renamed to '%1'").arg(newName), StatusTone::Info);
}

void MainWindow::onDeleteProfileClicked()
{
    if (m_currentProfileName.isEmpty()) {
        return;
    }

    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Delete Profile"),
        tr("Are you sure you want to delete profile '%1'?").arg(m_currentProfileName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    deleteProfile(m_currentProfileName);

    // Check remaining profiles
    QSettings settings("FreeRDPClient", "RdpQt");
    settings.beginGroup("Profiles");
    const QStringList remaining = settings.childGroups();
    settings.endGroup();

    if (remaining.isEmpty()) {
        const QString defaultName = tr("Default");
        saveProfile(defaultName);
        m_currentProfileName = defaultName;
    } else {
        m_currentProfileName = remaining.first();
    }

    settings.setValue("currentProfile", m_currentProfileName);
    updateProfileList(m_currentProfileName);
    loadProfile(m_currentProfileName);
    setStatus(tr("Status: Profile deleted"), StatusTone::Neutral);
}

void MainWindow::updateProfileList(const QString &selectedProfile)
{
    m_isUpdatingProfileList = true;
    m_profileCombo->clear();

    QSettings settings("FreeRDPClient", "RdpQt");
    settings.beginGroup("Profiles");
    const QStringList profiles = settings.childGroups();
    settings.endGroup();

    m_profileCombo->addItems(profiles);

    const QString target = selectedProfile.isEmpty() ? m_currentProfileName : selectedProfile;
    const int idx = m_profileCombo->findText(target);
    if (idx != -1) {
        m_profileCombo->setCurrentIndex(idx);
    } else if (m_profileCombo->count() > 0) {
        m_profileCombo->setCurrentIndex(0);
        m_currentProfileName = m_profileCombo->currentText();
    }

    m_isUpdatingProfileList = false;
}

void MainWindow::deleteProfile(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    QSettings settings("FreeRDPClient", "RdpQt");
    settings.beginGroup("Profiles");
    settings.remove(name);
    settings.endGroup();
}

void MainWindow::loadProfile(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    QSettings settings("FreeRDPClient", "RdpQt");
    settings.beginGroup("Profiles");
    settings.beginGroup(name);

    m_hostEdit->setText(settings.value("host", "").toString());
    m_portSpinBox->setValue(settings.value("port", 3389).toInt());
    m_usernameEdit->setText(settings.value("username", "").toString());
    m_domainEdit->setText(settings.value("domain", "").toString());
    m_passwordEdit->clear(); // Clear password on profile switch for security

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

    settings.endGroup();
    settings.endGroup();
}

void MainWindow::saveProfile(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    QSettings settings("FreeRDPClient", "RdpQt");
    settings.beginGroup("Profiles");
    settings.beginGroup(name);

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

    settings.endGroup();
    settings.endGroup();
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

    setStatus(tr("Status: Starting xfreerdp..."), StatusTone::Info);
    updateTrayStatus(tr("Connecting"));
    m_rdpManager->startSession(cfg);
}

void MainWindow::onDisconnectClicked()
{
    setStatus(tr("Status: Disconnecting..."), StatusTone::Warning);
    m_rdpManager->stopSession();
}

void MainWindow::onSessionStarted()
{
    setStatus(tr("Status: Connected / Running"), StatusTone::Success);
    updateTrayStatus(tr("Connected"));
}

void MainWindow::onSessionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    setStatus(QString(tr("Status: Disconnected (Exit code: %1)")).arg(exitCode), StatusTone::Neutral);
    updateTrayStatus(tr("Disconnected"));
}

void MainWindow::onErrorOccurred(const QString &errorMessage)
{
    QMessageBox::critical(this, tr("RDP Error"), errorMessage);
    setStatus(tr("Status: Error"), StatusTone::Error);
    updateTrayStatus(tr("Disconnected"));
}

void MainWindow::applyTheme()
{
    const bool dark = m_darkModeCheck->isChecked();
    const QString background = dark ? "#1e1e1e" : "#f5f5f5";
    const QString surface = dark ? "#252526" : "#ffffff";
    const QString foreground = dark ? "#e6e6e6" : "#202124";
    const QString border = dark ? "#484848" : "#c8c8c8";
    const QString input = dark ? "#303030" : "#ffffff";
    const QString selection = dark ? "#264f78" : "#cce4ff";

    setStyleSheet(QString(
        "QMainWindow, QWidget { background-color: %1; color: %2; }"
        "QGroupBox { background-color: %3; border: 1px solid %4; border-radius: 5px; margin-top: 8px; padding-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }"
        "QLineEdit, QSpinBox, QComboBox, QTextEdit { background-color: %5; color: %2; border: 1px solid %4; border-radius: 3px; padding: 4px; selection-background-color: %6; }"
        "QComboBox QAbstractItemView { background-color: %5; color: %2; selection-background-color: %6; }"
        "QCheckBox { background-color: transparent; }"
        "QPushButton { background-color: %3; color: %2; border: 1px solid %4; border-radius: 4px; padding: 5px 10px; }"
        "QPushButton:hover { background-color: %6; }"
        "QPushButton:disabled { color: %4; }"
    ).arg(background, foreground, surface, border, input, selection));

    const QString logBackground = dark ? "#1e1e1e" : "#ffffff";
    const QString logForeground = dark ? "#d4d4d4" : "#202124";
    m_logViewer->setStyleSheet(QString(
        "QTextEdit { background-color: %1; color: %2; font-size: 12px; }"
    ).arg(logBackground, logForeground));

    QString statusColor;
    switch (m_statusTone) {
    case StatusTone::Info:
        statusColor = dark ? "#64b5f6" : "#1976d2";
        break;
    case StatusTone::Warning:
        statusColor = dark ? "#ffb74d" : "#e65100";
        break;
    case StatusTone::Success:
        statusColor = dark ? "#81c784" : "#2e7d32";
        break;
    case StatusTone::Error:
        statusColor = dark ? "#ef5350" : "#c62828";
        break;
    case StatusTone::Neutral:
        statusColor = dark ? "#bdbdbd" : "#555555";
        break;
    }
    m_statusLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(statusColor));
}

void MainWindow::setStatus(const QString &text, StatusTone tone)
{
    m_statusTone = tone;
    m_statusLabel->setText(text);
    applyTheme();
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
    m_connectBtn->setVisible(!running);
    m_connectBtn->setEnabled(!running);
    m_disconnectBtn->setVisible(running);
    m_disconnectBtn->setEnabled(running);

    m_profileCombo->setEnabled(!running);
    m_newProfileBtn->setEnabled(!running);
    m_saveProfileBtn->setEnabled(!running);
    m_renameProfileBtn->setEnabled(!running);
    m_deleteProfileBtn->setEnabled(!running);

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

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && m_trayIcon != nullptr) {
        const auto state = windowState();
        if ((state & Qt::WindowMinimized) && isVisible()) {
            hide();
            event->ignore();
            return;
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon != nullptr && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
        return;
    }

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
    m_darkModeCheck->setChecked(settings.value("darkMode", false).toBool());

    settings.beginGroup("Profiles");
    QStringList profiles = settings.childGroups();
    settings.endGroup();

    if (profiles.isEmpty()) {
        // Migration of legacy single connection or brand new setup
        const QString defaultName = tr("Default");
        const QString legacyHost = settings.value("host", "").toString();
        const int legacyPort = settings.value("port", 3389).toInt();
        const QString legacyUser = settings.value("username", "").toString();
        const QString legacyDomain = settings.value("domain", "").toString();
        const QString legacyRes = settings.value("resolution", "1920x1080").toString();
        const bool legacyClipboard = settings.value("clipboard", true).toBool();
        const bool legacyCert = settings.value("ignoreCert", true).toBool();
        const bool legacyShareDrive = settings.value("shareDrive", false).toBool();
        const QString legacyDrivePath = settings.value("sharedDrivePath", "").toString();

        settings.beginGroup("Profiles");
        settings.beginGroup(defaultName);
        settings.setValue("host", legacyHost);
        settings.setValue("port", legacyPort);
        settings.setValue("username", legacyUser);
        settings.setValue("domain", legacyDomain);
        settings.setValue("resolution", legacyRes);
        settings.setValue("clipboard", legacyClipboard);
        settings.setValue("ignoreCert", legacyCert);
        settings.setValue("shareDrive", legacyShareDrive);
        settings.setValue("sharedDrivePath", legacyDrivePath);
        settings.endGroup();
        settings.endGroup();

        profiles << defaultName;
    }

    QString currentProfile = settings.value("currentProfile", "").toString();
    if (currentProfile.isEmpty() || !profiles.contains(currentProfile)) {
        currentProfile = profiles.first();
    }

    m_currentProfileName = currentProfile;
    updateProfileList(m_currentProfileName);
    loadProfile(m_currentProfileName);
}

void MainWindow::saveSettings()
{
    QSettings settings("FreeRDPClient", "RdpQt");
    settings.setValue("darkMode", m_darkModeCheck->isChecked());
    settings.setValue("currentProfile", m_currentProfileName);

    if (!m_currentProfileName.isEmpty()) {
        saveProfile(m_currentProfileName);
    }
}
