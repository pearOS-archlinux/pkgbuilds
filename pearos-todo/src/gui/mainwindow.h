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
#include <memory>
#include "../utils/types.h"

class TodoWidget;

/**
 * @brief Main application window - Frameless, transparent with blur (Classmorphism).
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

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
    void loadTodos();
    void saveTodos() const;
    void refreshSidebarLists();
    void setCurrentList(int index);

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
    QWidget* m_sidebarProfileWidget = nullptr;
    QLabel* m_sidebarProfileName = nullptr;

    TodoWidget* m_todoWidget = nullptr;
    QVector<TodoList> m_todoLists;
    QVector<QPushButton*> m_listDeleteButtons;
    int m_currentListIndex = -1;
};

#endif // MAINWINDOW_H
