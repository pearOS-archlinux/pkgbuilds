#pragma once
#include <QObject>
#include <QString>
#include <functional>

class MissionControlManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int  desktopCount           READ desktopCount           NOTIFY changed)
    Q_PROPERTY(int  desktopRows            READ desktopRows            NOTIFY changed)
    Q_PROPERTY(bool wrapNavigation         READ wrapNavigation         NOTIFY changed)
    Q_PROPERTY(bool separateSpacesPerDisplay READ separateSpacesPerDisplay NOTIFY changed)

public:
    explicit MissionControlManager(QObject *parent = nullptr);

    int  desktopCount()             const { return m_desktopCount; }
    int  desktopRows()              const { return m_desktopRows; }
    bool wrapNavigation()           const { return m_wrapNavigation; }
    bool separateSpacesPerDisplay() const { return m_separateSpacesPerDisplay; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setDesktopCount(int n);
    Q_INVOKABLE void setDesktopRows(int rows);
    Q_INVOKABLE void setWrapNavigation(bool wrap);
    Q_INVOKABLE void setSeparateSpacesPerDisplay(bool sep);

signals:
    void changed();

private:
    int  m_desktopCount = 1;
    int  m_desktopRows  = 1;
    bool m_wrapNavigation = false;
    bool m_separateSpacesPerDisplay = false;

    void run(const QString &cmd, std::function<void(QString)> cb);
    void reconfigure();
};
