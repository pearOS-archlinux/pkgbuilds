#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class DisplayManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList displays READ displays NOTIFY displaysChanged)
    Q_PROPERTY(int brightnessValue READ brightnessValue NOTIFY brightnessChanged)

public:
    explicit DisplayManager(QObject *parent = nullptr);
    QVariantList displays() const { return m_displays; }
    int brightnessValue() const { return m_brightness; }

    Q_INVOKABLE void refreshDisplays();
    Q_INVOKABLE void refreshBrightness(const QString &displayName);
    Q_INVOKABLE void setBrightness(int pct, const QString &displayName);
    Q_INVOKABLE void setResolution(const QString &displayName, const QString &resolutionType);
    Q_INVOKABLE void setDisplayEnabled(const QString &displayName, bool enabled, const QString &modeId);
    Q_INVOKABLE void setDisplayPrimary(const QString &displayName);
    Q_INVOKABLE void setNightLight(bool enabled);

signals:
    void displaysChanged();
    void brightnessChanged();

private:
    QVariantList m_displays;
    int m_brightness = 100;
    int m_brightnessMax = 100;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
