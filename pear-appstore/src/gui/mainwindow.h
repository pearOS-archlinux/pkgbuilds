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
#include <memory>
#include "../utils/types.h"

class HomeWidget;
class SearchWidget;
class InstalledWidget;
class UpdatesWidget;
class SettingsWidget;
class UserPageWidget;

/**
 * @brief Main application window - Frameless, transparent with blur (Classmorphism).
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    /** Creează SettingsWidget dacă e nevoie și îl pasează la UserPageWidget. Apelat din UserPageWidget. */
    void ensureSettingsWidgetForUserPage();

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
    void finishContentSetup();
    void runInitialSyncIfNeeded();
    void createMenuBar();
    void loadStyleSheet();
    QString themeOverlayFromPalette() const;
    void setPage(int index);
    void updateSidebarReflection();
    void loadPearIdDisplayName();
    void onOpenPackageRequested(const PackageInfo& info);
    void onBackFromDetails();
    void enableBlurBehind();
    Qt::Edges resizeEdgesAt(const QPoint& pos) const;

    QWidget* m_centralWidget = nullptr;
    QWidget* m_headerWidget = nullptr;
    QWidget* m_leftSideWidget = nullptr;
    QWidget* m_rightColumn = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    QStackedWidget* m_contentStack = nullptr;
    QStackedWidget* m_stackedWidget = nullptr;
    QWidget* m_detailsPage = nullptr;
    QVBoxLayout* m_detailsLayout = nullptr;
    QWidget* m_loadingPlaceholder = nullptr;
    bool m_contentReady = false;
    class PackageDetailsDialog* m_currentDetailsDialog = nullptr;
    QLabel* m_sidebarReflection = nullptr;
    QTimer* m_reflectionDebounceTimer = nullptr;
    QTimer* m_reflectionUpdateTimer = nullptr;

    QLineEdit* m_sidebarSearch = nullptr;
    QListWidget* m_sideList = nullptr;
    QWidget* m_sidebarProfileWidget = nullptr;
    QLabel* m_sidebarProfileName = nullptr;

    HomeWidget* m_homeWidget = nullptr;
    SearchWidget* m_searchWidget = nullptr;
    InstalledWidget* m_installedWidget = nullptr;
    UpdatesWidget* m_updatesWidget = nullptr;
    SettingsWidget* m_settingsWidget = nullptr;

    void ensureSettingsWidget();

    UserPageWidget* m_userPageWidget = nullptr;
};

#endif // MAINWINDOW_H
