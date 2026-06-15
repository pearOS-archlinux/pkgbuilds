#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class BatteryManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int percentage READ percentage NOTIFY statusChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY statusChanged)
    Q_PROPERTY(bool isPresent READ isPresent NOTIFY statusChanged)
    Q_PROPERTY(QString timeRemaining READ timeRemaining NOTIFY statusChanged)
    Q_PROPERTY(int designCapacity READ designCapacity NOTIFY statusChanged)
    Q_PROPERTY(int currentCapacity READ currentCapacity NOTIFY statusChanged)
    Q_PROPERTY(bool lowPowerMode READ lowPowerMode NOTIFY statusChanged)
    Q_PROPERTY(QString healthStatus READ healthStatus NOTIFY statusChanged)
    Q_PROPERTY(int cycleCount READ cycleCount NOTIFY statusChanged)
    Q_PROPERTY(QVariantList chargeHistory READ chargeHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList appUsage READ appUsage NOTIFY usageChanged)

public:
    explicit BatteryManager(QObject *parent = nullptr);
    int percentage()        const { return m_percentage; }
    bool isCharging()       const { return m_charging; }
    bool isPresent()        const { return m_present; }
    QString timeRemaining() const { return m_timeRemaining; }
    int designCapacity()    const { return m_designCapacity; }
    int currentCapacity()   const { return m_currentCapacity; }
    bool lowPowerMode()     const { return m_lowPowerMode; }
    QString healthStatus()  const { return m_healthStatus; }
    int cycleCount()        const { return m_cycleCount; }
    QVariantList chargeHistory() const { return m_chargeHistory; }
    QVariantList appUsage()      const { return m_appUsage; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshHistory();
    Q_INVOKABLE void refreshUsage();
    Q_INVOKABLE void setLowPowerMode(bool enabled);

signals:
    void statusChanged();
    void historyChanged();
    void usageChanged();

private:
    int m_percentage = 0;
    bool m_charging = false;
    bool m_present = false;
    QString m_timeRemaining;
    int m_designCapacity = 0;
    int m_currentCapacity = 0;
    bool m_lowPowerMode = false;
    QString m_healthStatus;
    int m_cycleCount = -1;
    QVariantList m_chargeHistory;
    QVariantList m_appUsage;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
