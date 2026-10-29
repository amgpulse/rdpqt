#include "RdpManager.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

RdpManager::RdpManager(QObject *parent)
    : QObject(parent),
      m_process(new QProcess(this))
{
    connect(m_process, &QProcess::started, this, &RdpManager::onProcessStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RdpManager::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &RdpManager::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &RdpManager::onReadyReadStandardError);
    connect(m_process, &QProcess::errorOccurred,
            this, &RdpManager::onProcessError);
}

RdpManager::~RdpManager()
{
    stopSession();
}

bool RdpManager::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

QStringList RdpManager::buildArguments(const RdpConfig &config) const
{
    QStringList args;

    // Server address & port
    if (config.port > 0 && config.port != 3389) {
        args << QString("/v:%1:%2").arg(config.host).arg(config.port);
    } else {
        args << QString("/v:%1").arg(config.host);
    }

    // Username
    if (!config.username.trimmed().isEmpty()) {
        args << QString("/u:%1").arg(config.username.trimmed());
    }

    // Password
    if (!config.password.isEmpty()) {
        args << QString("/p:%1").arg(config.password);
    }

    // Domain
    if (!config.domain.trimmed().isEmpty()) {
        args << QString("/d:%1").arg(config.domain.trimmed());
    }

    // Display resolution
    if (config.resolution.compare("fullscreen", Qt::CaseInsensitive) == 0) {
        args << "/f";
    } else if (config.resolution.compare("dynamic", Qt::CaseInsensitive) == 0) {
        args << "/dynamic-resolution";
    } else if (!config.resolution.isEmpty()) {
        args << QString("/size:%1").arg(config.resolution);
    }

    // Clipboard sharing
    if (config.enableClipboard) {
        args << "+clipboard";
    } else {
        args << "-clipboard";
    }

    // Certificate verification bypass
    if (config.ignoreCert) {
        args << "/cert:ignore";
    }

    // Shared drive / local folder redirection
    if (config.enableSharedDrive && !config.sharedDrivePath.trimmed().isEmpty()) {
        QFileInfo folderInfo(config.sharedDrivePath.trimmed());
        if (folderInfo.exists() && folderInfo.isDir()) {
            QString driveName = folderInfo.fileName();
            if (driveName.isEmpty()) {
                driveName = "SharedDrive";
            }
            args << QString("/drive:%1,%2").arg(driveName, config.sharedDrivePath.trimmed());
        }
    }

    return args;
}

void RdpManager::startSession(const RdpConfig &config)
{
    if (isRunning()) {
        emit errorOccurred("A session is already running. Please disconnect first.");
        return;
    }

    if (config.host.trimmed().isEmpty()) {
        emit errorOccurred("Server host/IP cannot be empty.");
        return;
    }

    // Search for xfreerdp binary
    QString program = QStandardPaths::findExecutable("xfreerdp");
    if (program.isEmpty()) {
        program = QStandardPaths::findExecutable("xfreerdp3");
    }
    if (program.isEmpty()) {
        program = QStandardPaths::findExecutable("xfreerdp2");
    }

    if (program.isEmpty()) {
        emit errorOccurred("FreeRDP executable ('xfreerdp') not found on system PATH. Please install freerdp2-x11 or freerdp3-x11.");
        return;
    }

    const QStringList args = buildArguments(config);

    // Build masked command for logging
    QStringList maskedArgs;
    for (const QString &arg : args) {
        if (arg.startsWith("/p:")) {
            maskedArgs << "/p:********";
        } else {
            maskedArgs << arg;
        }
    }
    emit logOutput(QString("Launching: %1 %2").arg(program, maskedArgs.join(" ")));

    m_process->start(program, args);
}

void RdpManager::stopSession()
{
    if (!isRunning()) {
        return;
    }

    emit logOutput("Terminating FreeRDP session...");
    m_process->terminate();

    if (!m_process->waitForFinished(3000)) {
        emit logOutput("Process did not terminate gracefully, sending SIGKILL...", true);
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void RdpManager::onProcessStarted()
{
    emit logOutput("RDP session started successfully.");
    emit stateChanged(true);
    emit sessionStarted();
}

void RdpManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString statusStr = (exitStatus == QProcess::NormalExit) ? "Normal exit" : "Crashed";
    emit logOutput(QString("RDP session finished with exit code %1 (%2).").arg(exitCode).arg(statusStr));
    emit stateChanged(false);
    emit sessionFinished(exitCode, exitStatus);
}

void RdpManager::onReadyReadStandardOutput()
{
    const QByteArray data = m_process->readAllStandardOutput();
    const QString text = QString::fromLocal8Bit(data).trimmed();
    if (!text.isEmpty()) {
        emit logOutput(text, false);
    }
}

void RdpManager::onReadyReadStandardError()
{
    const QByteArray data = m_process->readAllStandardError();
    const QString text = QString::fromLocal8Bit(data).trimmed();
    if (!text.isEmpty()) {
        emit logOutput(text, true);
    }
}

void RdpManager::onProcessError(QProcess::ProcessError error)
{
    QString msg;
    switch (error) {
        case QProcess::FailedToStart:
            msg = "Failed to start FreeRDP. Either binary not found or missing permissions.";
            break;
        case QProcess::Crashed:
            msg = "FreeRDP crashed unexpectedly.";
            break;
        case QProcess::Timedout:
            msg = "Operation timed out.";
            break;
        case QProcess::WriteError:
            msg = "An error occurred when attempting to write to FreeRDP process.";
            break;
        case QProcess::ReadError:
            msg = "An error occurred when attempting to read from FreeRDP process.";
            break;
        case QProcess::UnknownError:
        default:
            msg = "An unknown process error occurred.";
            break;
    }
    emit errorOccurred(msg);
    emit logOutput(QString("Process error: %1").arg(msg), true);
    emit stateChanged(false);
}
