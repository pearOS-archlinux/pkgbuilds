#pragma once
#include <QObject>
#include <QVariantList>
#include <QProcess>
#include <QRegularExpression>
#include <functional>

class BluetoothManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool bluetoothEnabled READ bluetoothEnabled NOTIFY statusChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY statusChanged)
    Q_PROPERTY(QVariantList devicesModel READ devicesModel NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList nearbyDevicesModel READ nearbyDevicesModel NOTIFY nearbyDevicesChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    bool bluetoothEnabled() const { return m_enabled; }
    QString deviceName() const { return m_deviceName; }
    QVariantList devicesModel() const { return m_devices; }
    QVariantList nearbyDevicesModel() const { return m_nearbyDevices; }

    Q_INVOKABLE void refreshStatus();
    Q_INVOKABLE void refreshDevices();
    Q_INVOKABLE void refreshNearbyDevices();
    Q_INVOKABLE void toggleBluetooth(bool enabled);
    Q_INVOKABLE void connectDevice(const QString &macAddress);
    Q_INVOKABLE void disconnectDevice(const QString &macAddress);

signals:
    void statusChanged();
    void devicesChanged();
    void nearbyDevicesChanged();
    void operationResult(bool success, const QString &message);

private:
    bool m_enabled = false;
    QString m_deviceName;
    QVariantList m_devices;
    QVariantList m_nearbyDevices;

    bool isValidMac(const QString &mac);
    void run(const QString &cmd, std::function<void(QString)> cb);
    void fetchDevicesFromPaths(const QStringList &paths, bool nearby);
};
