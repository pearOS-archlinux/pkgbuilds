#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class AudioManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int outputVolume READ outputVolume NOTIFY outputVolumeChanged)
    Q_PROPERTY(int inputVolume READ inputVolume NOTIFY inputVolumeChanged)
    Q_PROPERTY(QString currentOutputName READ currentOutputName NOTIFY devicesChanged)
    Q_PROPERTY(QString currentInputName READ currentInputName NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList outputDevices READ outputDevices NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList inputDevices READ inputDevices NOTIFY devicesChanged)

public:
    explicit AudioManager(QObject *parent = nullptr);

    int outputVolume() const { return m_outputVolume; }
    int inputVolume() const { return m_inputVolume; }
    QString currentOutputName() const { return m_currentOutput; }
    QString currentInputName() const { return m_currentInput; }
    QVariantList outputDevices() const { return m_outputs; }
    QVariantList inputDevices() const { return m_inputs; }

    Q_INVOKABLE void refreshOutputs();
    Q_INVOKABLE void refreshInputs();
    Q_INVOKABLE void refreshVolumes();
    Q_INVOKABLE void setOutputVolume(int vol);
    Q_INVOKABLE void setInputVolume(int vol);
    Q_INVOKABLE void setOutputDevice(const QString &index);
    Q_INVOKABLE void setInputDevice(const QString &index);

signals:
    void outputVolumeChanged();
    void inputVolumeChanged();
    void devicesChanged();

private:
    int m_outputVolume = 50;
    int m_inputVolume = 50;
    QString m_currentOutput;
    QString m_currentInput;
    QVariantList m_outputs;
    QVariantList m_inputs;
    void run(const QString &cmd, std::function<void(QString)> cb);
    QVariantList parseSinks(const QString &out, bool filterMonitor = false);
};
