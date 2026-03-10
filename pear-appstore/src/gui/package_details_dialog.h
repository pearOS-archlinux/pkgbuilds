#ifndef PACKAGE_DETAILS_DIALOG_H
#define PACKAGE_DETAILS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QFutureWatcher>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QShowEvent>
#include "../utils/types.h"
#include "../core/appstream_helper.h"

/**
 * @brief Dialog showing detailed package information and actions.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 */
class PackageDetailsDialog : public QDialog {
    Q_OBJECT
    
public:
    /** @param embedded true = afișat în fereastră (fără popup), fără buton Close */
    explicit PackageDetailsDialog(const PackageInfo& info, QWidget* parent = nullptr, bool embedded = false);
    ~PackageDetailsDialog() override = default;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void updateDetailsContentMinHeight();

private:
    void setupUi();
    void updateButtonStates();
    void checkInstallStatus();
    void showProgress(const QString& message);
    void hideProgress();
    void toggleLogViewer();
    void parseProgressOutput(const QString& output);
    QString findDesktopFile() const;
    bool verifyDesktopFile(const QString& desktopFilePath,
                           const QStringList& nameVariants) const;
    void launchApplication();
    void onScreenshotLoaded(QNetworkReply* reply, QLabel* label, const QString& cachedPath = QString());
    void loadAppStreamAsync();
    void onAppStreamDataReady();
    void setPlaceholderIcon(int size);
    QString iconPathFromDesktopFile(const QString& desktopPath) const;

    PackageInfo m_info;
    bool m_isInstalled = false;
    AppStreamData m_appstream;
    
    QNetworkAccessManager* m_network = nullptr;
    
    // Qt parent-child managed widgets (non-owning pointers)
    QLabel* m_appstreamIconLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_versionLabel = nullptr;
    QLabel* m_repositoryLabel = nullptr;
    QLabel* m_maintainerLabel = nullptr;
    QLabel* m_urlLabel = nullptr;
    QTextEdit* m_descriptionText = nullptr;
    QTextEdit* m_dependenciesText = nullptr;
    QLabel* m_lastUpdatedLabel = nullptr;

    QPushButton* m_installButton = nullptr;
    QPushButton* m_uninstallButton = nullptr;
    QPushButton* m_launchButton = nullptr;
    QPushButton* m_closeButton = nullptr;

    QLabel* m_statusBadge = nullptr;

    QProgressBar* m_progressBar = nullptr;
    QLabel* m_progressLabel = nullptr;
    QWidget* m_progressWidget = nullptr;
    
    // Log viewer (Qt parent-child managed)
    QTextEdit* m_logViewer = nullptr;
    QPushButton* m_toggleLogButton = nullptr;
    QWidget* m_logWidget = nullptr;
    bool m_logVisible = false;
    QWidget* m_screenshotsWidget = nullptr;
    QVBoxLayout* m_contentMainLayout = nullptr;
    QFutureWatcher<AppStreamData>* m_appstreamWatcher = nullptr;
    QWidget* m_depsContent = nullptr;
    QPushButton* m_depsExpandButton = nullptr;
    QLabel* m_categoryLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QWidget* m_galleryContainer = nullptr;
    QScrollArea* m_detailsScrollArea = nullptr;
    QWidget* m_detailsContentWidget = nullptr;

    // Progress tracking
    QString m_currentOperation;
    int m_totalPackages = 0;
    int m_currentPackage = 0;
    
private slots:
    void onDependenciesToggled();
    void onActionClicked();
    void onInstall();
    void onUninstall();
    void onOperationStarted(const QString& message);
    void onOperationOutput(const QString& output);
    void onOperationCompleted(bool success, const QString& message);
    void onOperationError(const QString& error);
};

#endif // PACKAGE_DETAILS_DIALOG_H
