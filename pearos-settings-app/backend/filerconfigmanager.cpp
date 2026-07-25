#include "filerconfigmanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>
#include <QVariantMap>

FilerConfigManager::FilerConfigManager(QObject *parent) : QObject(parent) {}

QString FilerConfigManager::configRelPath() {
    // Relative to $HOME/.config — kwriteconfig6/kreadconfig6 --file resolves
    // against that automatically, same as elsewhere in this codebase.
    return "filer/default/settings.conf";
}

// Infer a UI control type from the raw string value, same approach used for
// the tray-item/systray config elsewhere: bool → toggle, "#rrggbb" → color
// swatch, plain integer → text field, comma-separated → list editor
// (TopBar Items, Dock Launchers/PinnedFiles), everything else → text field.
static QString inferType(const QString &v) {
    if (v == "true" || v == "false") return "bool";
    static QRegularExpression colorRe("^#[0-9A-Fa-f]{6}$");
    if (colorRe.match(v).hasMatch()) return "color";
    static QRegularExpression intRe("^-?\\d+$");
    if (intRe.match(v).hasMatch()) return "int";
    if (v.contains(',')) return "list";
    return "string";
}

void FilerConfigManager::refresh() {
    QFile f(QDir::homePath() + "/.config/" + configRelPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_groups = {};
        emit changed();
        return;
    }

    QVariantList groups;
    QVariantMap curGroup;
    QVariantList curKeys;
    QString curName;

    static QRegularExpression groupRe(R"(^\[([^\]]+)\]$)");

    auto flush = [&]() {
        if (!curName.isEmpty()) {
            QVariantMap g;
            g["name"] = curName;
            g["keys"] = curKeys;
            groups.append(g);
        }
        curKeys.clear();
    };

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        if (auto m = groupRe.match(line); m.hasMatch()) {
            flush();
            curName = m.captured(1);
            continue;
        }

        int eq = line.indexOf('=');
        if (eq < 0) continue;
        QString key = line.left(eq);
        QString val = line.mid(eq + 1);
        // Strip surrounding quotes some values (e.g. TopBar Items) are stored with
        val.remove(QRegularExpression("^\"|\"$"));

        QVariantMap entry;
        entry["key"]   = key;
        entry["value"] = val;
        entry["type"]  = inferType(val);
        curKeys.append(entry);
    }
    flush();

    m_groups = groups;
    emit changed();
}

void FilerConfigManager::setValue(const QString &group, const QString &key, const QVariant &value) {
    // Update in-memory copy immediately for responsive UI.
    for (QVariant &gv : m_groups) {
        QVariantMap g = gv.toMap();
        if (g["name"].toString() != group) continue;
        QVariantList keys = g["keys"].toList();
        for (QVariant &kv : keys) {
            QVariantMap k = kv.toMap();
            if (k["key"].toString() == key) {
                k["value"] = value.toString();
                kv = k;
            }
        }
        g["keys"] = keys;
        gv = g;
        break;
    }
    emit changed();

    QProcess::startDetached("kwriteconfig6", {
        "--file", configRelPath(), "--group", group, "--key", key, value.toString()
    });
}
