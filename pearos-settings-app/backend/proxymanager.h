#pragma once
#include <QObject>
#include <QString>
#include <functional>

class ProxyManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool    enabled READ enabled NOTIFY changed)
    Q_PROPERTY(QString host    READ host    NOTIFY changed)
    Q_PROPERTY(int     port    READ port    NOTIFY changed)

public:
    explicit ProxyManager(QObject *parent = nullptr);

    bool    enabled() const { return m_enabled; }
    QString host()    const { return m_host; }
    int     port()    const { return m_port; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setEnabled(bool enabled);
    Q_INVOKABLE void setProxy(const QString &host, int port);

signals:
    void changed();

private:
    bool    m_enabled = false;
    QString m_host;
    int     m_port = 8080;

    void run(const QString &cmd, std::function<void(QString)> cb);
    void apply();
};
