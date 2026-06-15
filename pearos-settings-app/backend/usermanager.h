#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class UserManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList users      READ users          NOTIFY usersChanged)
    Q_PROPERTY(QString currentUser     READ currentUser    NOTIFY usersChanged)
    Q_PROPERTY(QString avatarPath      READ avatarPath     NOTIFY usersChanged)
    Q_PROPERTY(QString autoLoginUser   READ autoLoginUser  NOTIFY usersChanged)

public:
    explicit UserManager(QObject *parent = nullptr);
    QVariantList users()    const { return m_users; }
    QString currentUser()   const { return m_currentUser; }
    QString avatarPath()    const { return m_avatarPath; }
    QString autoLoginUser() const { return m_autoLoginUser; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setFullName(const QString &username, const QString &fullName);
    Q_INVOKABLE void setPassword(const QString &username, const QString &password);
    Q_INVOKABLE void setAvatar(const QString &username, const QString &imagePath);
    Q_INVOKABLE void setAutoLogin(const QString &username);
    Q_INVOKABLE void signOut();

signals:
    void usersChanged();
    void operationResult(bool success, const QString &message);

private:
    QVariantList m_users;
    QString m_currentUser, m_avatarPath, m_autoLoginUser;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
