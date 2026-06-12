#pragma once
#include <QObject>
#include <QSettings>
#include <QString>

class Config : public QObject {
    Q_OBJECT
public:
    static Config& instance();

    QString mode() const;
    void    setMode(const QString& v);

    int  smallW() const;   void setSmallW(int v);
    int  smallH() const;   void setSmallH(int v);
    int  bigW()   const;   void setBigW(int v);
    int  bigH()   const;   void setBigH(int v);
    int  screenGap()     const; void setScreenGap(int v);
    int  hoverDelayMs()  const; void setHoverDelayMs(int v);
    int  animDurationMs()const; void setAnimDurationMs(int v);

signals:
    void modeChanged(const QString& mode);
    void smallWChanged(int v);
    void smallHChanged(int v);
    void bigWChanged(int v);
    void bigHChanged(int v);
    void screenGapChanged(int v);
    void hoverDelayMsChanged(int v);
    void animDurationMsChanged(int v);

private:
    Config();
    mutable QSettings m_settings;
};
