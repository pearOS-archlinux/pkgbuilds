#pragma once
#include <QObject>
#include <functional>

class TrackpadManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool   enabled                  READ enabled                  NOTIFY trackpadChanged)
    Q_PROPERTY(bool   tapToClick               READ tapToClick               NOTIFY trackpadChanged)
    Q_PROPERTY(int    clickMethod              READ clickMethod              NOTIFY trackpadChanged)
    Q_PROPERTY(bool   leftHanded               READ leftHanded               NOTIFY trackpadChanged)
    Q_PROPERTY(double pointerAcceleration      READ pointerAcceleration      NOTIFY trackpadChanged)
    Q_PROPERTY(int    accelProfile             READ accelProfile             NOTIFY trackpadChanged)

    Q_PROPERTY(bool   naturalScroll            READ naturalScroll            NOTIFY trackpadChanged)
    Q_PROPERTY(int    scrollMethod             READ scrollMethod             NOTIFY trackpadChanged)
    Q_PROPERTY(double scrollFactor             READ scrollFactor             NOTIFY trackpadChanged)
    Q_PROPERTY(bool   disableWhileTyping       READ disableWhileTyping       NOTIFY trackpadChanged)
    Q_PROPERTY(bool   disableEventsOnExtMouse  READ disableEventsOnExtMouse  NOTIFY trackpadChanged)

    Q_PROPERTY(bool   tapAndDrag              READ tapAndDrag              NOTIFY trackpadChanged)
    Q_PROPERTY(bool   tapDragLock             READ tapDragLock             NOTIFY trackpadChanged)
    Q_PROPERTY(bool   middleEmulation         READ middleEmulation         NOTIFY trackpadChanged)
    Q_PROPERTY(bool   lmrTapButtonMap         READ lmrTapButtonMap         NOTIFY trackpadChanged)

public:
    explicit TrackpadManager(QObject *parent = nullptr);

    bool   enabled()                 const { return m_enabled; }
    bool   tapToClick()              const { return m_tapToClick; }
    int    clickMethod()             const { return m_clickMethod; }
    bool   leftHanded()              const { return m_leftHanded; }
    double pointerAcceleration()     const { return m_pointerAcceleration; }
    int    accelProfile()            const { return m_accelProfile; }

    bool   naturalScroll()           const { return m_naturalScroll; }
    int    scrollMethod()            const { return m_scrollMethod; }
    double scrollFactor()            const { return m_scrollFactor; }
    bool   disableWhileTyping()      const { return m_disableWhileTyping; }
    bool   disableEventsOnExtMouse() const { return m_disableEventsOnExtMouse; }

    bool   tapAndDrag()              const { return m_tapAndDrag; }
    bool   tapDragLock()             const { return m_tapDragLock; }
    bool   middleEmulation()         const { return m_middleEmulation; }
    bool   lmrTapButtonMap()         const { return m_lmrTapButtonMap; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setEnabled(bool v);
    Q_INVOKABLE void setTapToClick(bool v);
    Q_INVOKABLE void setClickMethod(int v);
    Q_INVOKABLE void setLeftHanded(bool v);
    Q_INVOKABLE void setPointerAcceleration(double v);
    Q_INVOKABLE void setAccelProfile(int v);
    Q_INVOKABLE void setNaturalScroll(bool v);
    Q_INVOKABLE void setScrollMethod(int v);
    Q_INVOKABLE void setScrollFactor(double v);
    Q_INVOKABLE void setDisableWhileTyping(bool v);
    Q_INVOKABLE void setDisableEventsOnExtMouse(bool v);
    Q_INVOKABLE void setTapAndDrag(bool v);
    Q_INVOKABLE void setTapDragLock(bool v);
    Q_INVOKABLE void setMiddleEmulation(bool v);
    Q_INVOKABLE void setLmrTapButtonMap(bool v);

signals:
    void trackpadChanged();

private:
    QString m_devicePath;

    bool   m_enabled                = true;
    bool   m_tapToClick             = true;
    int    m_clickMethod            = 2;
    bool   m_leftHanded             = false;
    double m_pointerAcceleration    = 0.0;
    int    m_accelProfile           = 2;

    bool   m_naturalScroll          = false;
    int    m_scrollMethod           = 1;
    double m_scrollFactor           = 1.0;
    bool   m_disableWhileTyping     = true;
    bool   m_disableEventsOnExtMouse = false;

    bool   m_tapAndDrag             = true;
    bool   m_tapDragLock            = false;
    bool   m_middleEmulation        = false;
    bool   m_lmrTapButtonMap        = false;

    void findDevicePath();
    void run(const QString &cmd, std::function<void(QString)> cb);
    QString getProp(const QString &prop) const;
    void setProp(const QString &prop, const QString &value);
};
