#pragma once
#include <QObject>
#include <QVariantList>
#include <functional>

class AppearanceManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString colorScheme      READ colorScheme      NOTIFY appearanceChanged)
    Q_PROPERTY(QString fontSize         READ fontSize         NOTIFY appearanceChanged)
    Q_PROPERTY(QString fontFamily       READ fontFamily       NOTIFY appearanceChanged)
    Q_PROPERTY(QString iconTheme        READ iconTheme        NOTIFY appearanceChanged)
    Q_PROPERTY(QVariantList colorSchemes READ colorSchemes    NOTIFY appearanceChanged)
    Q_PROPERTY(QVariantList iconThemes   READ iconThemes      NOTIFY appearanceChanged)

    // Liquid Gel (KWin blur effect) settings
    Q_PROPERTY(bool lgEnabled            READ lgEnabled            NOTIFY appearanceChanged)
    Q_PROPERTY(int  lgBlurStrength       READ lgBlurStrength       NOTIFY appearanceChanged)
    Q_PROPERTY(int  lgNoiseStrength      READ lgNoiseStrength      NOTIFY appearanceChanged)
    Q_PROPERTY(int  lgRefractionStrength READ lgRefractionStrength NOTIFY appearanceChanged)
    Q_PROPERTY(int  lgRefractionEdgeSize READ lgRefractionEdgeSize NOTIFY appearanceChanged)
    Q_PROPERTY(int  lgRGBFringing        READ lgRGBFringing        NOTIFY appearanceChanged)

public:
    explicit AppearanceManager(QObject *parent = nullptr);
    QString colorScheme()       const { return m_colorScheme; }
    QString fontSize()          const { return m_fontSize; }
    QString fontFamily()        const { return m_fontFamily; }
    QString iconTheme()         const { return m_iconTheme; }
    QVariantList colorSchemes() const { return m_colorSchemes; }
    QVariantList iconThemes()   const { return m_iconThemes; }

    bool lgEnabled()            const { return m_lgEnabled; }
    int  lgBlurStrength()       const { return m_lgBlurStrength; }
    int  lgNoiseStrength()      const { return m_lgNoiseStrength; }
    int  lgRefractionStrength() const { return m_lgRefractionStrength; }
    int  lgRefractionEdgeSize() const { return m_lgRefractionEdgeSize; }
    int  lgRGBFringing()        const { return m_lgRGBFringing; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setColorScheme(const QString &scheme);
    Q_INVOKABLE void setFontSize(const QString &size);
    Q_INVOKABLE void setFontFamily(const QString &family);
    Q_INVOKABLE void setIconTheme(const QString &theme);
    Q_INVOKABLE void setLgEnabled(bool enabled);
    Q_INVOKABLE void setLgBlurStrength(int v);
    Q_INVOKABLE void setLgNoiseStrength(int v);
    Q_INVOKABLE void setLgRefractionStrength(int v);
    Q_INVOKABLE void setLgRefractionEdgeSize(int v);
    Q_INVOKABLE void setLgRGBFringing(int v);

signals:
    void appearanceChanged();

private:
    QString m_colorScheme, m_fontSize, m_fontFamily, m_iconTheme;
    QVariantList m_colorSchemes, m_iconThemes;

    bool m_lgEnabled            = false;
    int  m_lgBlurStrength       = 15;
    int  m_lgNoiseStrength      = 5;
    int  m_lgRefractionStrength = 0;
    int  m_lgRefractionEdgeSize = 20;
    int  m_lgRGBFringing        = 1;

    void run(const QString &cmd, std::function<void(QString)> cb);
    void writeLgKey(const QString &key, const QString &value);
    void reconfigureLiquidGel();
};
