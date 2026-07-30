#pragma once
#include <QObject>
#include <QStringList>
#include <QHash>
#include <functional>

class HotCornersManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString top         READ top         NOTIFY cornersChanged)
    Q_PROPERTY(QString topRight    READ topRight    NOTIFY cornersChanged)
    Q_PROPERTY(QString right       READ right       NOTIFY cornersChanged)
    Q_PROPERTY(QString bottomRight READ bottomRight NOTIFY cornersChanged)
    Q_PROPERTY(QString bottom      READ bottom      NOTIFY cornersChanged)
    Q_PROPERTY(QString bottomLeft  READ bottomLeft  NOTIFY cornersChanged)
    Q_PROPERTY(QString left        READ left        NOTIFY cornersChanged)
    Q_PROPERTY(QString topLeft     READ topLeft     NOTIFY cornersChanged)
    Q_PROPERTY(QStringList availableActions READ availableActions CONSTANT)

public:
    explicit HotCornersManager(QObject *parent = nullptr);

    QString top()         const { return m_corners.value("Top"); }
    QString topRight()    const { return m_corners.value("TopRight"); }
    QString right()       const { return m_corners.value("Right"); }
    QString bottomRight() const { return m_corners.value("BottomRight"); }
    QString bottom()      const { return m_corners.value("Bottom"); }
    QString bottomLeft()  const { return m_corners.value("BottomLeft"); }
    QString left()        const { return m_corners.value("Left"); }
    QString topLeft()     const { return m_corners.value("TopLeft"); }
    QStringList availableActions() const {
        return { "None", "Application Launcher", "Overview", "Present Windows", "Lock Screen" };
    }

    Q_INVOKABLE void refresh();
    // corner: one of Top/TopRight/Right/BottomRight/Bottom/BottomLeft/Left/TopLeft
    Q_INVOKABLE void setCorner(const QString &corner, const QString &action);

signals:
    void cornersChanged();

private:
    QHash<QString, QString> m_corners;
    void run(const QString &cmd, std::function<void(QString)> cb);
};
