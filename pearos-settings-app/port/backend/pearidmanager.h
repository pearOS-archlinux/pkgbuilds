#pragma once
#include <QObject>
#include <functional>

class PearIDManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state     READ state     NOTIFY stateChanged)
    Q_PROPERTY(QString userName  READ userName  NOTIFY userInfoChanged)
    Q_PROPERTY(QString userEmail READ userEmail NOTIFY userInfoChanged)

public:
    explicit PearIDManager(QObject *parent = nullptr);
    QString state()     const { return m_state; }
    QString userName()  const { return m_userName; }
    QString userEmail() const { return m_userEmail; }

    Q_INVOKABLE void checkState();
    Q_INVOKABLE void login(const QString &email, const QString &password);
    Q_INVOKABLE void logout();

signals:
    void stateChanged();
    void userInfoChanged();
    void loginResult(bool success, const QString &message);

private:
    QString m_state = "loading";
    QString m_userName, m_userEmail;
    QString m_scriptDir;
    void run(const QString &cmd, std::function<void(QString, int)> cb);
    void fetchUserInfo();
};
