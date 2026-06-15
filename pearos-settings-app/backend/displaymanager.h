#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class DisplayManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList displays READ displays NOTIFY displaysChanged)
    Q_PROPERTY(int brightnessValue READ brightnessValue NOTIFY brightnessChanged)
    Q_PROPERTY(bool nightLightEnabled READ nightLightEnabled NOTIFY nightLightChanged)

public:
    explicit DisplayManager(QObject *parent = nullptr);
    QVariantList displays()      const { return m_displays; }
    int brightnessValue()        const { return m_brightness; }
    bool nightLightEnabled()     const { return m_nightLight; }

    Q_INVOKABLE void refreshDisplays();
    Q_INVOKABLE void refreshBrightness(const QString &displayName);
    Q_INVOKABLE void setBrightness(int pct, const QString &displayName);
    Q_INVOKABLE void setMode(const QString &outputName, int modeId);
    Q_INVOKABLE void setScale(const QString &outputName, double scale);
    Q_INVOKABLE void setDisplayEnabled(const QString &outputName, bool enabled);
    Q_INVOKABLE void setNightLight(bool enabled);

signals:
    void displaysChanged();
    void brightnessChanged();
    void nightLightChanged();

private:
    QVariantList m_displays;
    int m_brightness = 100;
    int m_brightnessMax = 100;
    bool m_nightLight = false;
    void run(const QString &cmd, std::function<void(QString)> cb);
    void runKScreen(const QString &cmd, std::function<void(QString)> cb);
};
