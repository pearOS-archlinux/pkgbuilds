#pragma once
#include <QObject>
#include <QVariantList>
#include <QProcess>
#include <functional>

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool wifiEnabled READ wifiEnabled NOTIFY statusChanged)
    Q_PROPERTY(QString currentSSID READ currentSSID NOTIFY statusChanged)
    Q_PROPERTY(QString currentSecurity READ currentSecurity NOTIFY statusChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY statusChanged)
    Q_PROPERTY(QVariantList networksModel READ networksModel NOTIFY networksChanged)
    Q_PROPERTY(QVariantList activeConnections READ activeConnections NOTIFY connectionsChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr);

    bool wifiEnabled() const { return m_wifiEnabled; }
    QString currentSSID() const { return m_currentSSID; }
    QString currentSecurity() const { return m_currentSecurity; }
    bool isConnected() const { return m_isConnected; }
    QVariantList networksModel() const { return m_networks; }
    QVariantList activeConnections() const { return m_activeConnections; }

public slots:
    Q_INVOKABLE void refreshStatus();
    Q_INVOKABLE void refreshNetworks();
    Q_INVOKABLE void toggleWifi(bool enabled);
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password);
    Q_INVOKABLE void forgetNetwork(const QString &ssid);
    Q_INVOKABLE void refreshActiveConnections();

signals:
    void statusChanged();
    void networksChanged();
    void connectionsChanged();
    void connectResult(bool success, const QString &message);

private:
    bool m_wifiEnabled = false;
    QString m_currentSSID;
    QString m_currentSecurity;
    bool m_isConnected = false;
    QVariantList m_networks;
    QVariantList m_activeConnections;

    void run(const QString &cmd, std::function<void(QString)> cb);
};
