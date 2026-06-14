#pragma once
#include <QObject>
#include <functional>

class BatteryManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int percentage READ percentage NOTIFY statusChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY statusChanged)
    Q_PROPERTY(bool isPresent READ isPresent NOTIFY statusChanged)
    Q_PROPERTY(QString timeRemaining READ timeRemaining NOTIFY statusChanged)
    Q_PROPERTY(int designCapacity READ designCapacity NOTIFY statusChanged)
    Q_PROPERTY(int currentCapacity READ currentCapacity NOTIFY statusChanged)

public:
    explicit BatteryManager(QObject *parent = nullptr);
    int percentage() const { return m_percentage; }
    bool isCharging() const { return m_charging; }
    bool isPresent() const { return m_present; }
    QString timeRemaining() const { return m_timeRemaining; }
    int designCapacity() const { return m_designCapacity; }
    int currentCapacity() const { return m_currentCapacity; }

    Q_INVOKABLE void refresh();

signals:
    void statusChanged();

private:
    int m_percentage = 0;
    bool m_charging = false;
    bool m_present = false;
    QString m_timeRemaining;
    int m_designCapacity = 0;
    int m_currentCapacity = 0;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
