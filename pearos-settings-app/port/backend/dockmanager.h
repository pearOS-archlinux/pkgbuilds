#pragma once
#include <QObject>
#include <QStringList>
#include <functional>

class DockManager : public QObject {
    Q_OBJECT

    // ── Appearance ────────────────────────────────────────────────────────
    Q_PROPERTY(QString skinName     READ skinName     NOTIFY changed)
    Q_PROPERTY(int    iconSize      READ iconSize     NOTIFY changed)
    Q_PROPERTY(double magnification READ magnification NOTIFY changed)
    Q_PROPERTY(double amplitud      READ amplitud     NOTIFY changed)
    Q_PROPERTY(int    iconSpacing   READ iconSpacing  NOTIFY changed)
    Q_PROPERTY(bool   showReflection READ showReflection NOTIFY changed)

    // ── Visual effects (skin-level) ───────────────────────────────────────
    Q_PROPERTY(bool   skinBlur             READ skinBlur             NOTIFY changed)
    Q_PROPERTY(int    skinBlurRadius       READ skinBlurRadius       NOTIFY changed)
    Q_PROPERTY(bool   skinLiquidGelEffect  READ skinLiquidGelEffect  NOTIFY changed)
    Q_PROPERTY(double skinRefractionStrength READ skinRefractionStrength NOTIFY changed)
    Q_PROPERTY(double skinRgbFringing      READ skinRgbFringing      NOTIFY changed)
    Q_PROPERTY(int    skinPositionTaskIndicator READ skinPositionTaskIndicator NOTIFY changed)

    // ── Behavior ──────────────────────────────────────────────────────────
    Q_PROPERTY(bool   showToolTips              READ showToolTips              NOTIFY changed)
    Q_PROPERTY(bool   highlightWindows          READ highlightWindows          NOTIFY changed)
    Q_PROPERTY(bool   indicateAudioStreams      READ indicateAudioStreams      NOTIFY changed)
    Q_PROPERTY(bool   interactiveMute           READ interactiveMute           NOTIFY changed)
    Q_PROPERTY(bool   tooltipControls           READ tooltipControls           NOTIFY changed)
    Q_PROPERTY(bool   minimizeActiveTaskOnClick READ minimizeActiveTaskOnClick NOTIFY changed)
    Q_PROPERTY(int    middleClickAction         READ middleClickAction         NOTIFY changed)
    Q_PROPERTY(int    wheelEnabled              READ wheelEnabled              NOTIFY changed)
    Q_PROPERTY(bool   wheelSkipMinimized        READ wheelSkipMinimized        NOTIFY changed)
    Q_PROPERTY(int    groupingStrategy          READ groupingStrategy          NOTIFY changed)
    Q_PROPERTY(int    groupedTaskVisualization  READ groupedTaskVisualization  NOTIFY changed)
    Q_PROPERTY(int    sortingStrategy           READ sortingStrategy           NOTIFY changed)

    // ── Task visibility ───────────────────────────────────────────────────
    Q_PROPERTY(bool showOnlyCurrentDesktop  READ showOnlyCurrentDesktop  NOTIFY changed)
    Q_PROPERTY(bool showOnlyCurrentActivity READ showOnlyCurrentActivity NOTIFY changed)
    Q_PROPERTY(bool showOnlyCurrentScreen   READ showOnlyCurrentScreen   NOTIFY changed)
    Q_PROPERTY(bool showOnlyMinimized       READ showOnlyMinimized       NOTIFY changed)
    Q_PROPERTY(bool unhideOnAttention       READ unhideOnAttention       NOTIFY changed)

    // ── Margins ───────────────────────────────────────────────────────────
    Q_PROPERTY(int skinLeftMargin          READ skinLeftMargin          NOTIFY changed)
    Q_PROPERTY(int skinTopMargin           READ skinTopMargin           NOTIFY changed)
    Q_PROPERTY(int skinRightMargin         READ skinRightMargin         NOTIFY changed)
    Q_PROPERTY(int skinBottomMargin        READ skinBottomMargin        NOTIFY changed)
    Q_PROPERTY(int skinOutsideLeftMargin   READ skinOutsideLeftMargin   NOTIFY changed)
    Q_PROPERTY(int skinOutsideTopMargin    READ skinOutsideTopMargin    NOTIFY changed)
    Q_PROPERTY(int skinOutsideRightMargin  READ skinOutsideRightMargin  NOTIFY changed)
    Q_PROPERTY(int skinOutsideBottomMargin READ skinOutsideBottomMargin NOTIFY changed)

    // ── Available skins ───────────────────────────────────────────────────
    Q_PROPERTY(QStringList availableSkins READ availableSkins NOTIFY changed)

public:
    explicit DockManager(QObject *parent = nullptr);

    // Getters
    QString skinName()     const { return m_skinName; }
    int    iconSize()      const { return m_iconSize; }
    double magnification() const { return m_magnification; }
    double amplitud()      const { return m_amplitud; }
    int    iconSpacing()   const { return m_iconSpacing; }
    bool   showReflection() const { return m_showReflection; }

    bool   skinBlur()             const { return m_skinBlur; }
    int    skinBlurRadius()       const { return m_skinBlurRadius; }
    bool   skinLiquidGelEffect()  const { return m_skinLiquidGelEffect; }
    double skinRefractionStrength() const { return m_skinRefractionStrength; }
    double skinRgbFringing()      const { return m_skinRgbFringing; }
    int    skinPositionTaskIndicator() const { return m_skinPositionTaskIndicator; }

    bool   showToolTips()              const { return m_showToolTips; }
    bool   highlightWindows()          const { return m_highlightWindows; }
    bool   indicateAudioStreams()      const { return m_indicateAudioStreams; }
    bool   interactiveMute()           const { return m_interactiveMute; }
    bool   tooltipControls()           const { return m_tooltipControls; }
    bool   minimizeActiveTaskOnClick() const { return m_minimizeActiveTaskOnClick; }
    int    middleClickAction()         const { return m_middleClickAction; }
    int    wheelEnabled()              const { return m_wheelEnabled; }
    bool   wheelSkipMinimized()        const { return m_wheelSkipMinimized; }
    int    groupingStrategy()          const { return m_groupingStrategy; }
    int    groupedTaskVisualization()  const { return m_groupedTaskVisualization; }
    int    sortingStrategy()           const { return m_sortingStrategy; }

    bool showOnlyCurrentDesktop()  const { return m_showOnlyCurrentDesktop; }
    bool showOnlyCurrentActivity() const { return m_showOnlyCurrentActivity; }
    bool showOnlyCurrentScreen()   const { return m_showOnlyCurrentScreen; }
    bool showOnlyMinimized()       const { return m_showOnlyMinimized; }
    bool unhideOnAttention()       const { return m_unhideOnAttention; }

    int skinLeftMargin()          const { return m_skinLeftMargin; }
    int skinTopMargin()           const { return m_skinTopMargin; }
    int skinRightMargin()         const { return m_skinRightMargin; }
    int skinBottomMargin()        const { return m_skinBottomMargin; }
    int skinOutsideLeftMargin()   const { return m_skinOutsideLeftMargin; }
    int skinOutsideTopMargin()    const { return m_skinOutsideTopMargin; }
    int skinOutsideRightMargin()  const { return m_skinOutsideRightMargin; }
    int skinOutsideBottomMargin() const { return m_skinOutsideBottomMargin; }

    QStringList availableSkins() const { return m_availableSkins; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void set(const QString &key, const QVariant &value);

signals:
    void changed();

private:
    // Current group path (e.g. "Containments/180/Applets/181/Configuration/General")
    QString m_groupPath;

    QString m_skinName   = "Tahoe Dark";
    int    m_iconSize    = 50;
    double m_magnification = 80.0;
    double m_amplitud    = 1.5;
    int    m_iconSpacing = 1;
    bool   m_showReflection = false;

    bool   m_skinBlur = true;
    int    m_skinBlurRadius = 24;
    bool   m_skinLiquidGelEffect = true;
    double m_skinRefractionStrength = 8.0;
    double m_skinRgbFringing = 2.0;
    int    m_skinPositionTaskIndicator = 3;

    bool   m_showToolTips = true;
    bool   m_highlightWindows = false;
    bool   m_indicateAudioStreams = true;
    bool   m_interactiveMute = true;
    bool   m_tooltipControls = true;
    bool   m_minimizeActiveTaskOnClick = true;
    int    m_middleClickAction = 2;
    int    m_wheelEnabled = 0;
    bool   m_wheelSkipMinimized = true;
    int    m_groupingStrategy = 1;
    int    m_groupedTaskVisualization = 0;
    int    m_sortingStrategy = 1;

    bool m_showOnlyCurrentDesktop  = true;
    bool m_showOnlyCurrentActivity = true;
    bool m_showOnlyCurrentScreen   = false;
    bool m_showOnlyMinimized       = false;
    bool m_unhideOnAttention       = true;

    int m_skinLeftMargin = 20, m_skinTopMargin = 20, m_skinRightMargin = 20, m_skinBottomMargin = 20;
    int m_skinOutsideLeftMargin = 20, m_skinOutsideTopMargin = 0;
    int m_skinOutsideRightMargin = 20, m_skinOutsideBottomMargin = -10;

    QStringList m_availableSkins;

    void findGroupPath();
    QString readKey(const QString &key, const QString &defaultVal = {}) const;
    void writeKey(const QString &key, const QString &value) const;
    void notifyPlasmashell(const QString &key, const QString &value) const;
    void run(const QString &cmd, std::function<void(QString)> cb);
    void runSync(const QString &cmd, std::function<void(QString)> cb);
};
