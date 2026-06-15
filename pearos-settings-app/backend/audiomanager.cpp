#include "audiomanager.h"
#include <QProcess>
#include <QRegularExpression>

AudioManager::AudioManager(QObject *parent) : QObject(parent) {}

void AudioManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

QVariantList AudioManager::parseSinks(const QString &out, bool filterMonitor) {
    QVariantList list;
    static QRegularExpression indexRe("Sink #(\\d+)|Source #(\\d+)");
    static QRegularExpression nameRe("Name:\\s*(.+)");
    static QRegularExpression descRe("Description:\\s*(.+)");

    QVariantMap current;
    for (const QString &line : out.split('\n')) {
        auto im = indexRe.match(line);
        if (im.hasMatch()) {
            if (!current.isEmpty()) list.append(current);
            current.clear();
            current["index"] = im.captured(1).isEmpty() ? im.captured(2) : im.captured(1);
        }
        auto nm = nameRe.match(line.trimmed());
        if (nm.hasMatch()) current["name"] = nm.captured(1).trimmed();
        auto dm = descRe.match(line.trimmed());
        if (dm.hasMatch()) current["description"] = dm.captured(1).trimmed();
    }
    if (!current.isEmpty()) list.append(current);

    if (filterMonitor) {
        QVariantList filtered;
        for (const QVariant &v : list) {
            QString name = v.toMap()["name"].toString();
            if (!name.endsWith(".monitor")) filtered.append(v);
        }
        return filtered;
    }
    return list;
}

void AudioManager::refreshOutputs() {
    run("pactl list sinks", [this](QString out) {
        m_outputs = parseSinks(out, false);
        run("pactl info | grep 'Default Sink:' | awk '{print $3}'", [this](QString s) {
            m_currentOutput = s.trimmed();
            emit devicesChanged();
        });
    });
}

void AudioManager::refreshInputs() {
    run("pactl list sources", [this](QString out) {
        m_inputs = parseSinks(out, true);
        run("pactl info | grep 'Default Source:' | awk '{print $3}'", [this](QString s) {
            m_currentInput = s.trimmed();
            emit devicesChanged();
        });
    });
}

void AudioManager::refreshVolumes() {
    run("pactl get-sink-volume @DEFAULT_SINK@ | head -1", [this](QString out) {
        static QRegularExpression re("(\\d+)%");
        auto m = re.match(out);
        m_outputVolume = m.hasMatch() ? m.captured(1).toInt() : 50;
        emit outputVolumeChanged();
    });
    run("pactl get-source-volume @DEFAULT_SOURCE@ | head -1", [this](QString out) {
        static QRegularExpression re("(\\d+)%");
        auto m = re.match(out);
        m_inputVolume = m.hasMatch() ? m.captured(1).toInt() : 50;
        emit inputVolumeChanged();
    });
}

void AudioManager::setOutputVolume(int vol) {
    m_outputVolume = qBound(0, vol, 100);
    emit outputVolumeChanged();
    run(QString("pactl set-sink-volume @DEFAULT_SINK@ %1%").arg(vol), [](QString) {});
}

void AudioManager::setInputVolume(int vol) {
    m_inputVolume = qBound(0, vol, 100);
    emit inputVolumeChanged();
    run(QString("pactl set-source-volume @DEFAULT_SOURCE@ %1%").arg(vol), [](QString) {});
}

void AudioManager::setOutputDevice(const QString &index) {
    run(QString("pactl list sinks | grep -A 10 'Sink #%1' | grep 'Name:' | head -1 | awk '{print $2}'").arg(index),
        [this](QString name) {
            if (!name.trimmed().isEmpty())
                run(QString("pactl set-default-sink %1").arg(name.trimmed()),
                    [this](QString) { refreshOutputs(); });
        });
}

void AudioManager::setInputDevice(const QString &index) {
    run(QString("pactl list sources | grep -A 10 'Source #%1' | grep 'Name:' | head -1 | awk '{print $2}'").arg(index),
        [this](QString name) {
            if (!name.trimmed().isEmpty())
                run(QString("pactl set-default-source %1").arg(name.trimmed()),
                    [this](QString) { refreshInputs(); });
        });
}
