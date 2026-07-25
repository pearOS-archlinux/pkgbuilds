#pragma once
#include <QObject>
#include <functional>

class AccessibilityManager : public QObject {
    Q_OBJECT

    // Keyboard (XKB AccessX — same protocol layer KDE's kaccess uses)
    Q_PROPERTY(bool stickyKeys        READ stickyKeys        NOTIFY accessibilityChanged)
    Q_PROPERTY(bool slowKeys          READ slowKeys          NOTIFY accessibilityChanged)
    Q_PROPERTY(int  slowKeysDelay     READ slowKeysDelay     NOTIFY accessibilityChanged)
    Q_PROPERTY(bool bounceKeys        READ bounceKeys        NOTIFY accessibilityChanged)
    Q_PROPERTY(int  bounceKeysDelay   READ bounceKeysDelay   NOTIFY accessibilityChanged)

    // Pointer
    Q_PROPERTY(bool mouseKeys           READ mouseKeys           NOTIFY accessibilityChanged)
    Q_PROPERTY(int  mouseKeysMaxSpeed   READ mouseKeysMaxSpeed   NOTIFY accessibilityChanged)

    // Sound
    Q_PROPERTY(bool audibleBell       READ audibleBell       NOTIFY accessibilityChanged)

    // Display
    Q_PROPERTY(bool reduceMotion      READ reduceMotion      NOTIFY accessibilityChanged)
    Q_PROPERTY(int  contrast          READ contrast          NOTIFY accessibilityChanged)
    Q_PROPERTY(int  cursorSize        READ cursorSize        NOTIFY accessibilityChanged)

public:
    explicit AccessibilityManager(QObject *parent = nullptr);

    bool stickyKeys()      const { return m_stickyKeys; }
    bool slowKeys()        const { return m_slowKeys; }
    int  slowKeysDelay()   const { return m_slowKeysDelay; }
    bool bounceKeys()      const { return m_bounceKeys; }
    int  bounceKeysDelay() const { return m_bounceKeysDelay; }

    bool mouseKeys()         const { return m_mouseKeys; }
    int  mouseKeysMaxSpeed() const { return m_mouseKeysMaxSpeed; }

    bool audibleBell()     const { return m_audibleBell; }

    bool reduceMotion()    const { return m_reduceMotion; }
    int  contrast()        const { return m_contrast; }
    int  cursorSize()      const { return m_cursorSize; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setStickyKeys(bool v);
    Q_INVOKABLE void setSlowKeys(bool v);
    Q_INVOKABLE void setSlowKeysDelay(int ms);
    Q_INVOKABLE void setBounceKeys(bool v);
    Q_INVOKABLE void setBounceKeysDelay(int ms);
    Q_INVOKABLE void setMouseKeys(bool v);
    Q_INVOKABLE void setMouseKeysMaxSpeed(int v);
    Q_INVOKABLE void setAudibleBell(bool v);
    Q_INVOKABLE void setReduceMotion(bool v);
    Q_INVOKABLE void setContrast(int v);
    Q_INVOKABLE void setCursorSize(int v);

signals:
    void accessibilityChanged();

private:
    bool m_stickyKeys        = false;
    bool m_slowKeys          = false;
    int  m_slowKeysDelay     = 300;
    bool m_bounceKeys        = false;
    int  m_bounceKeysDelay   = 300;

    bool m_mouseKeys         = false;
    int  m_mouseKeysMaxSpeed = 300;

    bool m_audibleBell       = true;

    bool m_reduceMotion      = false;
    int  m_contrast          = 4;
    int  m_cursorSize        = 24;

    void run(const QString &cmd, std::function<void(QString)> cb);
    void applyXkbControl(unsigned int mask, bool enable);
    void notifyGlobalChange();
};
