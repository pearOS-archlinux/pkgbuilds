#include "spotlightmanager.h"
#include <QProcess>

SpotlightManager::SpotlightManager(QObject *parent) : QObject(parent) {}

void SpotlightManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

// `balooctl6 config show <key>` prints a header line then one value per line
// (for list-typed keys) — strip the header and blank lines.
static QVariantList parseListConfig(const QString &out) {
    QVariantList result;
    for (const QString &line : out.split('\n')) {
        QString t = line.trimmed();
        if (t.isEmpty() || t.endsWith(':')) continue;
        QVariantMap m; m["path"] = t;
        result.append(m);
    }
    return result;
}

void SpotlightManager::refresh() {
    run("balooctl6 config show hidden", [this](QString out) {
        m_indexHidden = out.trimmed() == "yes";
        emit changed();
    });
    run("balooctl6 config show contentIndexing", [this](QString out) {
        m_indexContents = out.trimmed() == "yes";
        emit changed();
    });
    run("balooctl6 config show includeFolders", [this](QString out) {
        m_includeFolders = parseListConfig(out);
        emit changed();
    });
    run("balooctl6 config show excludeFolders", [this](QString out) {
        m_excludeFolders = parseListConfig(out);
        emit changed();
    });
    run("balooctl6 status 2>&1", [this](QString out) {
        m_indexingEnabled = !out.contains("Not Running", Qt::CaseInsensitive)
                          && !out.contains("Baloo is currently disabled", Qt::CaseInsensitive);
        emit changed();
    });
    refreshStatus();
}

void SpotlightManager::refreshStatus() {
    run("balooctl6 status 2>&1", [this](QString out) {
        for (const QString &line : out.split('\n')) {
            if (line.contains("Total files indexed")) {
                m_indexedFileCount = line.section(':', 1).trimmed();
            }
        }
        emit statusChanged();
    });
    run("balooctl6 indexSize 2>&1", [this](QString out) {
        for (const QString &line : out.split('\n')) {
            if (line.trimmed().startsWith("File Size", Qt::CaseInsensitive)) {
                m_indexSize = line.section(':', 1).trimmed();
            }
        }
        emit statusChanged();
    });
}

void SpotlightManager::setIndexingEnabled(bool v) {
    m_indexingEnabled = v;
    emit changed();
    run(v ? "balooctl6 enable" : "balooctl6 disable", [this](QString) { refresh(); });
}

void SpotlightManager::setIndexHidden(bool v) {
    m_indexHidden = v;
    emit changed();
    run(QString("balooctl6 config set hidden %1").arg(v ? "true" : "false"), [](QString) {});
}

void SpotlightManager::setIndexContents(bool v) {
    m_indexContents = v;
    emit changed();
    run(QString("balooctl6 config set contentIndexing %1").arg(v ? "true" : "false"), [](QString) {});
}

void SpotlightManager::addExcludeFolder(const QString &path) {
    QString safe = path; safe.replace("'", "'\\''");
    run(QString("balooctl6 config add excludeFolders '%1'").arg(safe), [this](QString) { refresh(); });
}

void SpotlightManager::removeExcludeFolder(const QString &path) {
    QString safe = path; safe.replace("'", "'\\''");
    run(QString("balooctl6 config remove excludeFolders '%1'").arg(safe), [this](QString) { refresh(); });
}

void SpotlightManager::addIncludeFolder(const QString &path) {
    QString safe = path; safe.replace("'", "'\\''");
    run(QString("balooctl6 config add includeFolders '%1'").arg(safe), [this](QString) { refresh(); });
}

void SpotlightManager::removeIncludeFolder(const QString &path) {
    QString safe = path; safe.replace("'", "'\\''");
    run(QString("balooctl6 config remove includeFolders '%1'").arg(safe), [this](QString) { refresh(); });
}
