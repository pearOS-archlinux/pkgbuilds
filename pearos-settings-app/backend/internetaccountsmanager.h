#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

// Reads/writes the real libaccounts-glib store (~/.config/libaccounts-glib/accounts.db)
// used by KAccounts. Listing/removing is native; adding a new account needs a
// provider-specific OAuth flow, so that delegates to KDE's own kcm_kaccounts UI.
class InternetAccountsManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList accounts READ accounts NOTIFY changed)

public:
    explicit InternetAccountsManager(QObject *parent = nullptr);

    QVariantList accounts() const { return m_accounts; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void removeAccount(int accountId);
    Q_INVOKABLE void openAddAccount();

signals:
    void changed();
    void actionResult(bool success, const QString &message);

private:
    QVariantList m_accounts;
    void run(const QString &cmd, std::function<void(QString, int)> cb);
};
