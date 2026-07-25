#include "internetaccountsmanager.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>

InternetAccountsManager::InternetAccountsManager(QObject *parent) : QObject(parent) {}

void InternetAccountsManager::run(const QString &cmd, std::function<void(QString, int)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int code, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()), code);
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, this, [proc, cb](QProcess::ProcessError) {
        cb(QString(), -1);
        proc->deleteLater();
    });
    proc->start("bash", {"-c", cmd});
}

static const char *kListScript = R"PY(
import gi, json
gi.require_version('Accounts', '1.0')
from gi.repository import Accounts
m = Accounts.Manager()
out = []
for aid in m.list():
    acc = m.get_account(aid)
    if not acc:
        continue
    out.append({
        'id': aid,
        'name': acc.get_display_name() or acc.get_provider_name(),
        'provider': acc.get_provider_name(),
        'enabled': bool(acc.get_enabled()),
    })
print(json.dumps(out))
)PY";

void InternetAccountsManager::refresh() {
    run(QString("python3 -c \"%1\" 2>/dev/null").arg(QString::fromUtf8(kListScript).replace('"', "\\\"")),
        [this](QString out, int) {
            QVariantList accounts;
            QJsonDocument doc = QJsonDocument::fromJson(out.trimmed().toUtf8());
            if (doc.isArray()) {
                for (const QJsonValue &v : doc.array()) {
                    QJsonObject o = v.toObject();
                    QVariantMap m;
                    m["id"]       = o.value("id").toInt();
                    m["name"]     = o.value("name").toString();
                    m["provider"] = o.value("provider").toString();
                    m["enabled"]  = o.value("enabled").toBool();
                    accounts.append(m);
                }
            }
            m_accounts = accounts;
            emit changed();
        });
}

void InternetAccountsManager::removeAccount(int accountId) {
    QString script = QString(
        "import gi\n"
        "gi.require_version('Accounts', '1.0')\n"
        "from gi.repository import Accounts\n"
        "m = Accounts.Manager()\n"
        "acc = m.get_account(%1)\n"
        "if acc:\n"
        "    acc.delete()\n"
        "    acc.store_blocking()\n"
    ).arg(accountId);

    run(QString("python3 -c \"%1\" 2>&1").arg(script.replace('"', "\\\"")),
        [this](QString out, int code) {
            bool ok = (code == 0);
            emit actionResult(ok, ok ? QString() : out.trimmed());
            refresh();
        });
}

void InternetAccountsManager::openAddAccount() {
    // Adding an account needs a provider-specific OAuth/credentials flow —
    // delegate to KDE's own Internet Accounts KCM rather than half-building
    // a fragile in-app version of it.
    QProcess::startDetached("kcmshell6", {"kcm_kaccounts"});
}
