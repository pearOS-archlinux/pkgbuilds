#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class NotificationsManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool lowPriorityPopups  READ lowPriorityPopups  NOTIFY changed)
    Q_PROPERTY(bool lowPriorityHistory READ lowPriorityHistory NOTIFY changed)
    Q_PROPERTY(int  popupPosition      READ popupPosition      NOTIFY changed)
    Q_PROPERTY(int  popupTimeout       READ popupTimeout       NOTIFY changed)
    Q_PROPERTY(bool showPopupTimeout   READ showPopupTimeout   NOTIFY changed)
    Q_PROPERTY(QVariantList appList    READ appList            NOTIFY changed)

public:
    explicit NotificationsManager(QObject *parent = nullptr);

    bool lowPriorityPopups()  const { return m_lowPriorityPopups; }
    bool lowPriorityHistory() const { return m_lowPriorityHistory; }
    int  popupPosition()      const { return m_popupPosition; }
    int  popupTimeout()       const { return m_popupTimeout; }
    bool showPopupTimeout()   const { return m_showPopupTimeout; }
    QVariantList appList()    const { return m_appList; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setLowPriorityPopups(bool v);
    Q_INVOKABLE void setLowPriorityHistory(bool v);
    Q_INVOKABLE void setPopupPosition(int v);
    Q_INVOKABLE void setPopupTimeout(int ms);
    Q_INVOKABLE void setShowPopupTimeout(bool v);
    Q_INVOKABLE void setAppShowPopups(const QString &id, bool v);
    Q_INVOKABLE void setAppShowInHistory(const QString &id, bool v);

signals:
    void changed();

private:
    static const QString CONFIG_FILE;

    bool m_lowPriorityPopups  = true;
    bool m_lowPriorityHistory = false;
    int  m_popupPosition      = 0;
    int  m_popupTimeout       = 5000;
    bool m_showPopupTimeout   = true;
    QVariantList m_appList;

    QString readKey(const QString &group, const QString &key, const QString &defaultVal = {}) const;
    QString readKey2(const QString &group, const QString &subGroup, const QString &key, const QString &defaultVal = {}) const;
    void writeKey(const QString &group, const QString &subGroup, const QString &key, const QString &value) const;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
