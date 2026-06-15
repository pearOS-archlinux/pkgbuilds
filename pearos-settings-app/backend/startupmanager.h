#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class StartupManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList startupApps READ startupApps NOTIFY appsChanged)

public:
    explicit StartupManager(QObject *parent = nullptr);
    QVariantList startupApps() const { return m_apps; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setEnabled(const QString &name, bool enabled);
    Q_INVOKABLE void removeApp(const QString &name);

signals:
    void appsChanged();

private:
    QVariantList m_apps;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
