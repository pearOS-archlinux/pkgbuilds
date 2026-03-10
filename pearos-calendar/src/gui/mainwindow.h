#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPoint>
#include <QFlags>
#include <QTimer>
#include <QVector>
#include <QDate>

class QTabBar;
class QScrollArea;
class QCalendarWidget;
class QPropertyAnimation;
#include "../utils/types.h"

struct CalendarEvent {
    QString id;
    QDate   date;
    QString title;
    QString details;
};

/**
 * @brief Main application window - Frameless, transparent with blur (Classmorphism).
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    bool hasEventOn(const QDate& date) const;
    /** Event titles on the given date (for calendar cell label). */
    QStringList eventTitlesOn(const QDate& date) const;
    /** Returns the month/year currently shown in the calendar header (title). Used for day text color. */
    bool getDisplayedMonthYear(int& year, int& month) const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    static const int RESIZE_MARGIN = 8;

    void setupUi();
    void setupHeader();
    void setupSidebar();
    void setupContent();
    void loadPearIdDisplayName();
    void createMenuBar();
    void loadStyleSheet();
    QString themeOverlayFromPalette() const;
    void updateSidebarReflection();
    void enableBlurBehind();
    Qt::Edges resizeEdgesAt(const QPoint& pos) const;
    void loadEvents();
    void saveEvents() const;
    void refreshSidebarEvents();
    void updateCalendarHighlights();
    void updateMiniCalendar();
    void openEventDialogForDate(const QDate& date);

    QWidget* m_centralWidget = nullptr;
    QWidget* m_headerWidget = nullptr;
    QWidget* m_leftSideWidget = nullptr;
    QWidget* m_rightColumn = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    QLabel* m_sidebarReflection = nullptr;
    QTimer* m_reflectionDebounceTimer = nullptr;
    QTimer* m_reflectionUpdateTimer = nullptr;

    QLineEdit* m_sidebarSearch = nullptr;
    QListWidget* m_sideList = nullptr;
    QLabel* m_sidebarProfileName = nullptr;

    QWidget* m_headerTrafficWidget = nullptr;
    QWidget* m_sidebarTrafficWidget = nullptr;

    QLabel* m_monthTitleLabel = nullptr;

    // Calendar UI
    QTabBar* m_tabBar = nullptr;
    QStackedWidget* m_viewStack = nullptr;
    QScrollArea* m_monthScrollArea = nullptr;
    QVector<QCalendarWidget*> m_monthCalendars;  // stacked months (prev2, prev1, current, next1, next2)
    QCalendarWidget* m_monthCalendar = nullptr; // middle one (current month), for title
    QCalendarWidget* m_miniCalendar = nullptr;

    // Sidebar toggle (Events button: one in header when sidebar closed, one in sidebar when open)
    QPushButton* m_sidebarToggleButton = nullptr;   // header
    QPushButton* m_sidebarEventsButton = nullptr;  // sidebar
    QPropertyAnimation* m_sidebarAnimation = nullptr;
    bool m_sidebarVisible = false;

    // Events
    QVector<CalendarEvent> m_events;
};

#endif // MAINWINDOW_H
