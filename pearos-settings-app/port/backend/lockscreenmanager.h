#pragma once
#include <QObject>
#include <functional>

class LockScreenManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool   autoLock    READ autoLock    NOTIFY settingsChanged)
    Q_PROPERTY(int    lockTimeout READ lockTimeout NOTIFY settingsChanged)
    Q_PROPERTY(bool   requirePassword READ requirePassword NOTIFY settingsChanged)

public:
    explicit LockScreenManager(QObject *parent = nullptr);
    bool autoLock()          const { return m_autoLock; }
    int  lockTimeout()       const { return m_lockTimeout; }
    bool requirePassword()   const { return m_requirePassword; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setAutoLock(bool enabled);
    Q_INVOKABLE void setLockTimeout(int minutes);
    Q_INVOKABLE void setRequirePassword(bool enabled);
    Q_INVOKABLE void lockNow();

signals:
    void settingsChanged();

private:
    bool m_autoLock = true;
    int  m_lockTimeout = 5;
    bool m_requirePassword = true;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
