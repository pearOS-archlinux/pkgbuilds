#ifndef USER_PAGE_WIDGET_H
#define USER_PAGE_WIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>

class InstalledWidget;
class SettingsWidget;
class MainWindow;

class UserPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit UserPageWidget(InstalledWidget* installedWidget, MainWindow* mainWindow, QWidget* parent = nullptr);

    void setSettingsWidget(SettingsWidget* w);

private:
    QStackedWidget* m_innerStack = nullptr;
    QWidget* m_settingsPlaceholder = nullptr;
    MainWindow* m_mainWindow = nullptr;
    QPushButton* m_installedTabButton = nullptr;
    QPushButton* m_settingsTabButton = nullptr;
    int m_currentIndex = 0;
};

#endif // USER_PAGE_WIDGET_H
