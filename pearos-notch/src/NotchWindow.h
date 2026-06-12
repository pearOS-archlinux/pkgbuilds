#pragma once
#include <QWidget>
#include <QTimer>
#include <QSvgRenderer>
#include <QPixmap>
#include <QSize>
#include <QPropertyAnimation>
#include <QStringList>
#include <QMimeData>

class MprisWatcher;
class SettingsDialog;

enum class Tab { Music, Tray, Browser };

class NotchWindow : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QSize notchSize READ notchSize WRITE setNotchSize)

public:
    explicit NotchWindow(SettingsDialog* settingsDlg, QWidget* parent = nullptr);
    ~NotchWindow() override;

    QSize notchSize() const;
    void  setNotchSize(const QSize& sz);

public slots:
    void reinitForMode();

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void showEvent(QShowEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dropEvent(QDropEvent*) override;

private slots:
    void onHoverExpand();
    void onMprisChanged();
    void onPositionChanged(qint64 posUs);
    void onVizTimer();

private:
    // ── Layout helpers ───────────────────────────────────────────────────────
    static constexpr double TAB_H = 22.0;   // tab bar height at top of expanded

    struct ExpandedLayout {
        double artX, artY, artSize;
        double textX, textW;
        double titleY, artistY, barY, btnCY;
    };
    ExpandedLayout expandedLayout(int w, int h) const;

    QRectF tabIconRect(Tab t, int w) const;
    QRectF trayItemRect(int i, int w, int h) const;
    QRectF trayItemCloseRect(int i, int w, int h) const;

    // ── Drawing ──────────────────────────────────────────────────────────────
    QPainterPath notchShape(int w, int h);
    void drawTabBar(QPainter& p, int w, int h);
    void drawVisualizer(QPainter& p, int w, int h);
    void drawAlbumArt(QPainter& p, int w, int h);
    void drawMusicContent(QPainter& p, int w, int h);
    void drawTrayContent(QPainter& p, int w, int h);
    void drawBrowserContent(QPainter& p, int w, int h);
    void drawSvgWhite(QPainter& p, QSvgRenderer* svg, const QRectF& rect, double opacity = 1.0);

    QRectF prevButtonRect(int w, int h) const;
    QRectF playButtonRect(int w, int h) const;
    QRectF nextButtonRect(int w, int h) const;
    QRectF settingsIconRect(int w) const;

    // ── Platform ─────────────────────────────────────────────────────────────
    void positionWindow();
    void applyWindowHints();
    void setX11Hints();
    void applyLayerShell(bool updateOnly = false);
    void disableKWinEffects();
    void startSizeAnim(const QSize& target, bool expanding);
    void startVizAnim(bool playing);

    bool isExpanded() const;
    bool isWayland() const;

    // ── Tray ─────────────────────────────────────────────────────────────────
    static QString trayStorageDir();
    void loadTrayFromSettings();
    void saveTrayToSettings();
    void addToTray(const QString& path);
    void removeFromTray(int index);
    void startTrayItemDrag(int index);

    // ── State ────────────────────────────────────────────────────────────────
    double m_vizAlpha  = 0.0;
    double m_vizTarget = 0.0;
    bool   m_expanded  = false;
    bool   m_dragLock  = false;

    Tab         m_currentTab      = Tab::Music;
    QStringList m_trayPaths;
    int         m_hoveredTrayItem = -1;
    int         m_pressedTrayItem = -1;
    QPoint      m_dragStartPos;
    bool        m_incomingDrag    = false;

    QPropertyAnimation* m_sizeAnim   = nullptr;
    QTimer*             m_hoverTimer = nullptr;
    QTimer*             m_vizTimer   = nullptr;

    MprisWatcher*   m_mpris       = nullptr;
    SettingsDialog* m_settingsDlg = nullptr;

    // SVG renderers
    QSvgRenderer* m_svgPrev     = nullptr;
    QSvgRenderer* m_svgNext     = nullptr;
    QSvgRenderer* m_svgPlay     = nullptr;
    QSvgRenderer* m_svgPause    = nullptr;
    QSvgRenderer* m_svgSettings = nullptr;
    QSvgRenderer* m_svgHome     = nullptr;
    QSvgRenderer* m_svgTrayIcon = nullptr;
    QSvgRenderer* m_svgBrowser  = nullptr;
};
