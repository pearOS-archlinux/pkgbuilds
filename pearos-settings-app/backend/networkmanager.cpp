#include "networkmanager.h"
#include <QProcess>
#include <QRegularExpression>
#include <QDateTime>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {}

void NetworkManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void NetworkManager::refreshStatus() {
    run("nmcli radio wifi", [this](QString out) {
        m_wifiEnabled = out.trimmed().toLower().contains("enabled");
        if (m_wifiEnabled) {
            run("nmcli -t -f active,ssid,security dev wifi | grep '^yes:'", [this](QString ssidOut) {
                auto parts = ssidOut.trimmed().split(':');
                m_isConnected = !parts.isEmpty() && parts.size() >= 2;
                m_currentSSID = m_isConnected ? parts.value(1) : QString();
                m_currentSecurity = m_isConnected ? parts.value(2) : QString();
                emit statusChanged();
            });
        } else {
            m_isConnected = false;
            m_currentSSID.clear();
            m_currentSecurity.clear();
            emit statusChanged();
        }
    });
}

void NetworkManager::refreshNetworks() {
    run("nmcli -t -f SSID,SIGNAL,SECURITY dev wifi list | grep -v '^--' | head -20", [this](QString out) {
        m_networks.clear();
        for (const QString &line : out.trimmed().split('\n')) {
            if (line.trimmed().isEmpty()) continue;
            auto parts = line.split(':');
            if (parts.size() < 2) continue;
            QVariantMap net;
            net["ssid"] = parts.value(0);
            net["signal"] = parts.value(1).toInt();
            net["security"] = parts.value(2, "--");
            if (!net["ssid"].toString().isEmpty())
                m_networks.append(net);
        }
        emit networksChanged();
    });
}

void NetworkManager::toggleWifi(bool enabled) {
    QString cmd = enabled ? "nmcli radio wifi on" : "nmcli radio wifi off";
    run(cmd, [this](QString) { refreshStatus(); });
}

void NetworkManager::connectToNetwork(const QString &ssid, const QString &password) {
    QString escapedSSID = ssid;
    escapedSSID.replace("'", "'\\''");
    QString escapedPw = password;
    escapedPw.replace("'", "'\\''");
    QString connName = QString("wifi-%1").arg(QDateTime::currentMSecsSinceEpoch());

    QString cmd = QString("nmcli connection add type wifi con-name '%1' ifname '*' ssid '%2' "
                          "wifi-sec.key-mgmt wpa-psk wifi-sec.psk '%3' && nmcli connection up '%1'")
                      .arg(connName, escapedSSID, escapedPw);
    run(cmd, [this, escapedSSID, escapedPw](QString out) {
        if (out.contains("Error") || out.isEmpty()) {
            QString fallback = QString("nmcli device wifi connect '%1' password '%2'")
                                   .arg(escapedSSID, escapedPw);
            run(fallback, [this](QString) { refreshStatus(); emit connectResult(true, QString()); });
        } else {
            refreshStatus();
            emit connectResult(true, QString());
        }
    });
}

void NetworkManager::forgetNetwork(const QString &ssid) {
    run("nmcli -t -f NAME,TYPE connection show", [this, ssid](QString out) {
        for (const QString &line : out.trimmed().split('\n')) {
            if (!line.contains(":802-11-wireless:")) continue;
            QString connName = line.split(':').value(0);
            QString escaped = connName;
            escaped.replace("'", "'\\''");
            run(QString("nmcli -t -f 802-11-wireless.ssid connection show '%1'").arg(escaped),
                [this, ssid, escaped](QString ssidOut) {
                    if (ssidOut.trimmed() == ssid) {
                        run(QString("nmcli connection delete '%1'").arg(escaped),
                            [this](QString) { refreshNetworks(); });
                    }
                });
        }
    });
}

void NetworkManager::refreshActiveConnections() {
    run("nmcli -t -f TYPE,NAME,DEVICE,STATE connection show --active", [this](QString out) {
        m_activeConnections.clear();
        for (const QString &line : out.trimmed().split('\n')) {
            auto parts = line.split(':');
            if (parts.size() < 4) continue;
            QVariantMap c;
            c["type"] = parts.value(0).toLower();
            c["name"] = parts.value(1);
            c["device"] = parts.value(2);
            c["status"] = parts.value(3);
            m_activeConnections.append(c);
        }
        emit connectionsChanged();
    });
}
