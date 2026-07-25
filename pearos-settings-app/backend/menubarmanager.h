#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class MenuBarManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool autoHide     READ autoHide     NOTIFY changed)
    Q_PROPERTY(QVariantList trayItems READ trayItems NOTIFY changed)

public:
    explicit MenuBarManager(QObject *parent = nullptr);

    bool autoHide() const { return m_autoHide; }
    QVariantList trayItems() const { return m_trayItems; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setAutoHide(bool v);
    Q_INVOKABLE void setTrayItemVisible(const QString &id, bool visible);

signals:
    void changed();

private:
    static const QString CONFIG_FILE;

    QString m_containmentId;
    QString m_systrayAppletId;
    bool m_autoHide = false;
    QVariantList m_trayItems;

    void findIds();
    QString readKey(const QString &groupPath, const QString &key, const QString &defaultVal = {}) const;
    void writeKey(const QString &groupPath, const QString &key, const QString &value) const;
    QStringList readList(const QString &groupPath, const QString &key) const;
    void notifyMenuBar() const;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
