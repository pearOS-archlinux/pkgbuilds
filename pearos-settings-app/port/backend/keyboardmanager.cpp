#include "keyboardmanager.h"
#include <QProcess>
#include <QRegularExpression>

KeyboardManager::KeyboardManager(QObject *parent) : QObject(parent) {}

void KeyboardManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void KeyboardManager::refresh() {
    run("setxkbmap -query 2>/dev/null", [this](QString out) {
        static QRegularExpression layoutRe("^layout:\\s*(.+)", QRegularExpression::MultilineOption);
        static QRegularExpression variantRe("^variant:\\s*(.+)", QRegularExpression::MultilineOption);
        auto lm = layoutRe.match(out); m_layout  = lm.hasMatch() ? lm.captured(1).trimmed() : "us";
        auto vm = variantRe.match(out); m_variant = vm.hasMatch() ? vm.captured(1).trimmed() : QString();
    });

    run("xset q 2>/dev/null | grep 'auto repeat delay'", [this](QString out) {
        static QRegularExpression re("delay:\\s*(\\d+)\\s+repeat rate:\\s*(\\d+)");
        auto m = re.match(out);
        if (m.hasMatch()) { m_repeatDelay = m.captured(1).toInt(); m_repeatRate = m.captured(2).toInt(); }
    });

    run("localectl list-x11-keymap-layouts 2>/dev/null", [this](QString out) {
        m_layouts.clear();
        for (const QString &line : out.split('\n')) {
            QString l = line.trimmed();
            if (!l.isEmpty()) m_layouts.append(l);
        }
        emit keyboardChanged();
    });
}

void KeyboardManager::setLayout(const QString &layout, const QString &variant) {
    m_layout = layout; m_variant = variant;
    QString cmd = variant.isEmpty()
        ? QString("setxkbmap %1").arg(layout)
        : QString("setxkbmap %1 %2").arg(layout, variant);
    run(cmd, [this](QString) { emit keyboardChanged(); });
    // persist via localectl
    QString persist = variant.isEmpty()
        ? QString("localectl set-x11-keymap %1").arg(layout)
        : QString("localectl set-x11-keymap %1 '' %2").arg(layout, variant);
    run(persist, [](QString) {});
}

void KeyboardManager::setRepeat(int delayMs, int rateHz) {
    m_repeatDelay = delayMs; m_repeatRate = rateHz;
    run(QString("xset r rate %1 %2").arg(delayMs).arg(rateHz), [this](QString) { emit keyboardChanged(); });
}
