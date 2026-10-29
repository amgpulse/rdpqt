#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

struct RdpConfig {
    QString host;
    int port = 3389;
    QString username;
    QString password;
    QString domain;
    QString resolution = "1920x1080"; // "1920x1080", "1366x768", "fullscreen", "dynamic"
    bool enableClipboard = true;
    bool ignoreCert = true;
    bool enableSharedDrive = false;
    QString sharedDrivePath;
};

class RdpManager : public QObject {
    Q_OBJECT

public:
    explicit RdpManager(QObject *parent = nullptr);
    ~RdpManager() override;

    bool isRunning() const;
    void startSession(const RdpConfig &config);
    void stopSession();

    QStringList buildArguments(const RdpConfig &config) const;

signals:
    void sessionStarted();
    void sessionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void errorOccurred(const QString &errorMessage);
    void logOutput(const QString &message, bool isError = false);
    void stateChanged(bool running);

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process;
};
