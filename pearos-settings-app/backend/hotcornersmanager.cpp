#include "hotcornersmanager.h"
#include <QProcess>

static const QStringList kCornerKeys = {
    "Top", "TopRight", "Right", "BottomRight", "Bottom", "BottomLeft", "Left", "TopLeft"
};

HotCornersManager::HotCornersManager(QObject *parent) : QObject(parent) {
    for (const QString &k : kCornerKeys) m_corners[k] = "None";
}

void HotCornersManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void HotCornersManager::refresh() {
    for (const QString &key : kCornerKeys) {
        run(QString("kreadconfig6 --file kwinrc --group ElectricBorders --key %1 2>/dev/null").arg(key),
            [this, key](QString out) {
            m_corners[key] = out.isEmpty() ? "None" : out;
            emit cornersChanged();
        });
    }
}

void HotCornersManager::setCorner(const QString &corner, const QString &action) {
    if (!kCornerKeys.contains(corner)) return;
    m_corners[corner] = action;
    emit cornersChanged();
    run(QString("kwriteconfig6 --file kwinrc --group ElectricBorders --key %1 '%2' && "
                "qdbus6 org.kde.KWin /KWin reconfigure")
            .arg(corner, action),
        [](QString) {});
}
