#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class DateTimeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool   autoTime     READ autoTime     NOTIFY settingsChanged)
    Q_PROPERTY(bool   autoTimezone READ autoTimezone NOTIFY settingsChanged)
    Q_PROPERTY(bool   use24h       READ use24h       NOTIFY settingsChanged)
    Q_PROPERTY(QString timezone    READ timezone     NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList timezones READ timezones NOTIFY settingsChanged)

public:
    explicit DateTimeManager(QObject *parent = nullptr);
    bool   autoTime()     const { return m_autoTime; }
    bool   autoTimezone() const { return m_autoTimezone; }
    bool   use24h()       const { return m_use24h; }
    QString timezone()    const { return m_timezone; }
    QVariantList timezones() const { return m_timezones; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setAutoTime(bool enabled);
    Q_INVOKABLE void setTimezone(const QString &tz);
    Q_INVOKABLE void setUse24h(bool enabled);
    Q_INVOKABLE void setDateTime(const QString &isoString);

signals:
    void settingsChanged();

private:
    bool m_autoTime = true, m_autoTimezone = false, m_use24h = true;
    QString m_timezone;
    QVariantList m_timezones;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
