#pragma once
#include <QObject>
#include <QVariant>
#include <functional>

class KWalletManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled       READ enabled       NOTIFY changed)
    Q_PROPERTY(bool closeWhenIdle READ closeWhenIdle NOTIFY changed)
    Q_PROPERTY(int  idleTimeout   READ idleTimeout   NOTIFY changed)
    Q_PROPERTY(bool launchManager READ launchManager NOTIFY changed)

public:
    explicit KWalletManager(QObject *parent = nullptr);

    bool enabled()       const { return m_enabled; }
    bool closeWhenIdle() const { return m_closeWhenIdle; }
    int  idleTimeout()   const { return m_idleTimeout; }
    bool launchManager() const { return m_launchManager; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void set(const QString &key, const QVariant &value);

signals:
    void changed();

private:
    bool m_enabled       = true;
    bool m_closeWhenIdle = true;
    int  m_idleTimeout   = 10;
    bool m_launchManager = true;

    void run(const QString &cmd, std::function<void(QString)> cb);
};
