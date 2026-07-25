#pragma once
#include <QObject>
#include <functional>

class FocusManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool doNotDisturb            READ doNotDisturb            NOTIFY changed)
    Q_PROPERTY(bool allowCriticalInDnd       READ allowCriticalInDnd       NOTIFY changed)
    Q_PROPERTY(bool muteSoundsInDnd          READ muteSoundsInDnd          NOTIFY changed)
    Q_PROPERTY(bool inhibitWhenFullscreen    READ inhibitWhenFullscreen    NOTIFY changed)
    Q_PROPERTY(bool inhibitWhenScreenSharing READ inhibitWhenScreenSharing NOTIFY changed)
    Q_PROPERTY(bool inhibitWhenMirrored      READ inhibitWhenMirrored      NOTIFY changed)

public:
    explicit FocusManager(QObject *parent = nullptr);

    bool doNotDisturb()            const { return m_doNotDisturb; }
    bool allowCriticalInDnd()      const { return m_allowCriticalInDnd; }
    bool muteSoundsInDnd()         const { return m_muteSoundsInDnd; }
    bool inhibitWhenFullscreen()   const { return m_inhibitWhenFullscreen; }
    bool inhibitWhenScreenSharing() const { return m_inhibitWhenScreenSharing; }
    bool inhibitWhenMirrored()     const { return m_inhibitWhenMirrored; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setDoNotDisturb(bool v);
    Q_INVOKABLE void setAllowCriticalInDnd(bool v);
    Q_INVOKABLE void setMuteSoundsInDnd(bool v);
    Q_INVOKABLE void setInhibitWhenFullscreen(bool v);
    Q_INVOKABLE void setInhibitWhenScreenSharing(bool v);
    Q_INVOKABLE void setInhibitWhenMirrored(bool v);

signals:
    void changed();

private:
    static const QString CONFIG_FILE;

    bool m_doNotDisturb            = false;
    bool m_allowCriticalInDnd      = true;
    bool m_muteSoundsInDnd         = false;
    bool m_inhibitWhenFullscreen   = true;
    bool m_inhibitWhenScreenSharing = true;
    bool m_inhibitWhenMirrored     = true;

    QString readKey(const QString &group, const QString &key, const QString &defaultVal = {}) const;
    void writeKey(const QString &group, const QString &key, const QString &value) const;
};
