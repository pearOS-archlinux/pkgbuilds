#pragma once
#include <QObject>
#include <functional>

class TrackpadManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool naturalScroll   READ naturalScroll   NOTIFY trackpadChanged)
    Q_PROPERTY(bool tapToClick      READ tapToClick      NOTIFY trackpadChanged)
    Q_PROPERTY(bool twoFingerScroll READ twoFingerScroll NOTIFY trackpadChanged)
    Q_PROPERTY(double speed         READ speed           NOTIFY trackpadChanged)

public:
    explicit TrackpadManager(QObject *parent = nullptr);
    bool   naturalScroll()   const { return m_naturalScroll; }
    bool   tapToClick()      const { return m_tapToClick; }
    bool   twoFingerScroll() const { return m_twoFingerScroll; }
    double speed()           const { return m_speed; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setNaturalScroll(bool enabled);
    Q_INVOKABLE void setTapToClick(bool enabled);
    Q_INVOKABLE void setTwoFingerScroll(bool enabled);
    Q_INVOKABLE void setSpeed(double speed);

signals:
    void trackpadChanged();

private:
    bool m_naturalScroll = false;
    bool m_tapToClick = true;
    bool m_twoFingerScroll = true;
    double m_speed = 0.0;
    QString m_deviceId;
    void run(const QString &cmd, std::function<void(QString)> cb);
    void applyProperty(const QString &prop, const QString &value);
};
