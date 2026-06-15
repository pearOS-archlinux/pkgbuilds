#include "bluetoothmanager.h"
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>

BluetoothManager::BluetoothManager(QObject *parent) : QObject(parent) {}

bool BluetoothManager::isValidMac(const QString &mac) {
    static QRegularExpression re("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
    return re.match(mac).hasMatch();
}

void BluetoothManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void BluetoothManager::refreshStatus() {
    run("bluetoothctl show | grep -i 'powered:'", [this](QString out) {
        m_enabled = out.toLower().contains("yes");
        run("bluetoothctl show | grep -i 'name:'", [this](QString nameOut) {
            static QRegularExpression re("name:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
            auto m = re.match(nameOut.trimmed());
            m_deviceName = m.hasMatch() ? m.captured(1).trimmed() : "Computer";
            emit statusChanged();
        });
    });
}

void BluetoothManager::toggleBluetooth(bool enabled) {
    QString cmd = enabled ? "bluetoothctl power on" : "bluetoothctl power off";
    run(cmd, [this](QString) { refreshStatus(); });
}

void BluetoothManager::fetchDevicesFromPaths(const QStringList &paths, bool nearby) {
    if (paths.isEmpty()) {
        if (nearby) { m_nearbyDevices.clear(); emit nearbyDevicesChanged(); }
        else { m_devices.clear(); emit devicesChanged(); }
        return;
    }

    auto results = std::make_shared<QVariantList>();
    auto pending = std::make_shared<int>(paths.size());

    for (const QString &path : paths) {
        auto *proc = new QProcess(this);
        QString getCmd = QString("dbus-send --system --dest=org.bluez --print-reply %1 "
                                 "org.freedesktop.DBus.Properties.GetAll string:org.bluez.Device1 2>/dev/null")
                             .arg(path);
        connect(proc, &QProcess::finished, this, [this, proc, path, results, pending, nearby](int, QProcess::ExitStatus) {
            QString out = QString::fromUtf8(proc->readAllStandardOutput());
            proc->deleteLater();

            QVariantMap dev;
            static QRegularExpression nameRe("\"Name\".*?variant.*?string\\s+\"([^\"]+)\"");
            static QRegularExpression connRe("\"Connected\".*?variant.*?boolean\\s+(true|false)");
            static QRegularExpression iconRe("\"Icon\".*?variant.*?string\\s+\"([^\"]+)\"");
            static QRegularExpression macRe("dev_([0-9A-Fa-f_]{17})");

            auto nm = nameRe.match(out); dev["name"] = nm.hasMatch() ? nm.captured(1) : "Unknown Device";
            auto cm = connRe.match(out); dev["connected"] = cm.hasMatch() && cm.captured(1) == "true";
            auto im = iconRe.match(out); dev["type"] = im.hasMatch() ? im.captured(1) : "Device";
            auto mm = macRe.match(path);
            dev["macAddress"] = mm.hasMatch() ? mm.captured(1).replace('_', ':').toUpper() : QString();

            results->append(dev);
            if (--(*pending) == 0) {
                if (nearby) { m_nearbyDevices = *results; emit nearbyDevicesChanged(); }
                else { m_devices = *results; emit devicesChanged(); }
            }
        });
        connect(proc, &QProcess::errorOccurred, proc, [proc, pending, results, this, nearby]() {
            proc->deleteLater();
            if (--(*pending) == 0) {
                if (nearby) { m_nearbyDevices = *results; emit nearbyDevicesChanged(); }
                else { m_devices = *results; emit devicesChanged(); }
            }
        });
        proc->start("bash", {"-c", getCmd});
    }
}

void BluetoothManager::refreshDevices() {
    run("dbus-send --system --dest=org.bluez --print-reply / "
        "org.freedesktop.DBus.ObjectManager.GetManagedObjects 2>&1",
        [this](QString out) {
            QStringList paths;
            static QRegularExpression pathRe("object path\\s+\"([^\"]+)\"");
            QRegularExpressionMatchIterator it = pathRe.globalMatch(out);
            QStringList allPaths;
            while (it.hasNext()) allPaths << it.next().captured(1);
            for (const QString &p : allPaths)
                if (out.contains(p) && out.indexOf("org.bluez.Device1", out.indexOf(p)) != -1)
                    paths << p;
            fetchDevicesFromPaths(paths, false);
        });
}

void BluetoothManager::refreshNearbyDevices() {
    run("dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0 "
        "org.freedesktop.DBus.Properties.Set string:org.bluez.Adapter1 "
        "string:Powered variant:boolean:true 2>/dev/null", [this](QString) {
        QTimer::singleShot(2000, this, [this]() { refreshDevices(); });
    });
}

void BluetoothManager::connectDevice(const QString &macAddress) {
    if (!isValidMac(macAddress)) { emit operationResult(false, "Invalid MAC address"); return; }
    QString devPath = QString("/org/bluez/hci0/dev_%1").arg(QString(macAddress).replace(':', '_'));
    auto *proc = new QProcess(this);
    proc->start("dbus-send", {"--system", "--dest=org.bluez", "--print-reply",
                               devPath, "org.bluez.Device1.Connect"});
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        proc->deleteLater();
        emit operationResult(code == 0, code == 0 ? QString() : "Connection failed");
        if (code == 0) refreshDevices();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}

void BluetoothManager::disconnectDevice(const QString &macAddress) {
    if (!isValidMac(macAddress)) { emit operationResult(false, "Invalid MAC address"); return; }
    QString devPath = QString("/org/bluez/hci0/dev_%1").arg(QString(macAddress).replace(':', '_'));
    auto *proc = new QProcess(this);
    proc->start("dbus-send", {"--system", "--dest=org.bluez", "--print-reply",
                               devPath, "org.bluez.Device1.Disconnect"});
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        proc->deleteLater();
        emit operationResult(code == 0, code == 0 ? QString() : "Disconnect failed");
        if (code == 0) refreshDevices();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}
