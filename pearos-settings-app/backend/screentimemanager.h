#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class ScreenTimeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList todayApps      READ todayApps      NOTIFY changed)
    Q_PROPERTY(int          todayTotalSecs READ todayTotalSecs NOTIFY changed)
    Q_PROPERTY(QVariantList weekTotals     READ weekTotals     NOTIFY changed)
    Q_PROPERTY(bool         trackingActive READ trackingActive NOTIFY changed)

public:
    explicit ScreenTimeManager(QObject *parent = nullptr);

    QVariantList todayApps()      const { return m_todayApps; }
    int          todayTotalSecs() const { return m_todayTotalSecs; }
    QVariantList weekTotals()     const { return m_weekTotals; }
    bool         trackingActive() const { return m_trackingActive; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void ensureTrackerRunning();

signals:
    void changed();

private:
    QVariantList m_todayApps;
    int          m_todayTotalSecs = 0;
    QVariantList m_weekTotals;
    bool         m_trackingActive = false;

    static QString dataFilePath();
    static QString collectorScriptPath();
    static QString kwinScriptPath();
    void installCollectorScript();
    void installKwinScript();
    void installSystemdUnit();
    void run(const QString &cmd, std::function<void(QString)> cb);
};
