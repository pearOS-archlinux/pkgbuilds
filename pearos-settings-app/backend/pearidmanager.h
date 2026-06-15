#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class PearIDManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state         READ state         NOTIFY stateChanged)
    Q_PROPERTY(QString userName      READ userName      NOTIFY userInfoChanged)
    Q_PROPERTY(QString userEmail     READ userEmail     NOTIFY userInfoChanged)
    Q_PROPERTY(QString avatarPath    READ avatarPath    NOTIFY userInfoChanged)
    Q_PROPERTY(QString phone         READ phone         NOTIFY userInfoChanged)
    Q_PROPERTY(QString birthdate     READ birthdate     NOTIFY userInfoChanged)
    Q_PROPERTY(QString billingAddress READ billingAddress NOTIFY userInfoChanged)
    Q_PROPERTY(QVariantList devices  READ devices       NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList apps     READ apps          NOTIFY appsChanged)

public:
    explicit PearIDManager(QObject *parent = nullptr);
    QString state()          const { return m_state; }
    QString userName()       const { return m_userName; }
    QString userEmail()      const { return m_userEmail; }
    QString avatarPath()     const { return m_avatarPath; }
    QString phone()          const { return m_phone; }
    QString birthdate()      const { return m_birthdate; }
    QString billingAddress() const { return m_billingAddress; }
    QVariantList devices()   const { return m_devices; }
    QVariantList apps()      const { return m_apps; }

    Q_INVOKABLE void checkState();
    Q_INVOKABLE void login(const QString &email, const QString &password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void fetchExtendedInfo();
    Q_INVOKABLE void fetchDevices();
    Q_INVOKABLE void fetchApps();
    Q_INVOKABLE void updateName(const QString &firstName, const QString &lastName);
    Q_INVOKABLE void updatePhone(const QString &phone);
    Q_INVOKABLE void updateBillingAddress(const QString &address);
    Q_INVOKABLE void changePassword(const QString &oldPw, const QString &newPw);

signals:
    void stateChanged();
    void userInfoChanged();
    void loginResult(bool success, const QString &message);
    void devicesChanged();
    void appsChanged();
    void updateResult(const QString &field, bool success, const QString &message);

private:
    QString m_state = "loading";
    QString m_userName, m_userEmail, m_avatarPath;
    QString m_phone, m_birthdate, m_billingAddress;
    QVariantList m_devices, m_apps;
    QString m_scriptDir;
    void run(const QString &cmd, std::function<void(QString, int)> cb);
    void fetchUserInfo();
};
