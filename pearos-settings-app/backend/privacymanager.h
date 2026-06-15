#pragma once
#include <QObject>
#include <functional>

class PrivacyManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool sendDiagnostics  READ sendDiagnostics  NOTIFY privacyChanged)
    Q_PROPERTY(bool locationServices READ locationServices NOTIFY privacyChanged)
    Q_PROPERTY(bool recentFiles      READ recentFiles      NOTIFY privacyChanged)

public:
    explicit PrivacyManager(QObject *parent = nullptr);
    bool sendDiagnostics()  const { return m_sendDiagnostics; }
    bool locationServices() const { return m_locationServices; }
    bool recentFiles()      const { return m_recentFiles; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setSendDiagnostics(bool enabled);
    Q_INVOKABLE void setLocationServices(bool enabled);
    Q_INVOKABLE void setRecentFiles(bool enabled);
    Q_INVOKABLE void clearRecentFiles();

signals:
    void privacyChanged();

private:
    bool m_sendDiagnostics = false;
    bool m_locationServices = false;
    bool m_recentFiles = true;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
