#ifndef UPDATES_WIDGET_H
#define UPDATES_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QProgressBar>
#include <QTextEdit>
#include <QLineEdit>
#include <QStackedWidget>
#include <QShowEvent>
#include "../utils/types.h"

class UpdateItem;

/**
 * @brief Widget for displaying and managing package updates.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - UpdateItem widgets are dynamically created/destroyed in displayUpdates/clearUpdates
 */
class UpdatesWidget : public QWidget {
    Q_OBJECT
signals:
    void updatesCountChanged(int count);

public:
    explicit UpdatesWidget(QWidget* parent = nullptr);
    ~UpdatesWidget() override = default;
    
    void checkForUpdates();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    void displayUpdates(const QVector<UpdateInfo>& updates);
    void clearUpdates();
    void filterUpdates(const QString& searchText);
    QString formatSize(qint64 bytes);
    void showProgress(const QString& message);
    void hideProgress();
    void toggleLogViewer();
    
    // Qt parent-child managed widgets (non-owning pointers)
    QStackedWidget* m_contentStack = nullptr;
    QLineEdit* m_searchInput = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_countLabel = nullptr;
    QPushButton* m_updateAllButton = nullptr;
    QPushButton* m_checkButton = nullptr;
    
    // Progress bar and log viewer (Qt parent-child managed)
    QWidget* m_progressWidget = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QLabel* m_progressLabel = nullptr;
    QPushButton* m_toggleLogButton = nullptr;
    QWidget* m_logWidget = nullptr;
    QTextEdit* m_logViewer = nullptr;
    bool m_logVisible = false;
    
    QVector<UpdateInfo> m_updates;
    QVector<UpdateInfo> m_filteredUpdates;
    
private slots:
    void onUpdateAll();
    void onUpdateSingle(const QString& packageName);
    void onSearchTextChanged(const QString& text);
    void onOperationStarted(const QString& message);
    void onOperationOutput(const QString& output);
    void onOperationCompleted(bool success, const QString& message);
    void onOperationError(const QString& error);
};

#endif // UPDATES_WIDGET_H
