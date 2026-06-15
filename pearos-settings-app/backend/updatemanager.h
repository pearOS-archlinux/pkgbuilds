#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class UpdateManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool checking        READ checking        NOTIFY stateChanged)
    Q_PROPERTY(bool updating        READ updating        NOTIFY stateChanged)
    Q_PROPERTY(int  updateCount     READ updateCount     NOTIFY stateChanged)
    Q_PROPERTY(QVariantList updates READ updates         NOTIFY stateChanged)
    Q_PROPERTY(QString lastOutput   READ lastOutput      NOTIFY outputChanged)

public:
    explicit UpdateManager(QObject *parent = nullptr);
    bool checking()       const { return m_checking; }
    bool updating()       const { return m_updating; }
    int  updateCount()    const { return m_updates.size(); }
    QVariantList updates() const { return m_updates; }
    QString lastOutput()  const { return m_output; }

    Q_INVOKABLE void checkUpdates();
    Q_INVOKABLE void applyUpdates();

signals:
    void stateChanged();
    void outputChanged();

private:
    bool m_checking = false;
    bool m_updating = false;
    QVariantList m_updates;
    QString m_output;
    void run(const QString &cmd, std::function<void(QString)> cb);
    void runStreaming(const QString &cmd);
};
