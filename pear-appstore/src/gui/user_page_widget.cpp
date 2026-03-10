#include "user_page_widget.h"
#include "installed_widget.h"
#include "settings_widget.h"
#include "mainwindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
 
UserPageWidget::UserPageWidget(InstalledWidget* installedWidget, MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget* tabBar = new QWidget(this);
    auto* tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(0, 0, 16, 12);
    tabLayout->setSpacing(8);

    m_installedTabButton = new QPushButton(tr("Installed"), tabBar);
    m_settingsTabButton = new QPushButton(tr("Settings"), tabBar);
    auto* spacer = new QWidget(tabBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto applyTabStyle = [](QPushButton* btn, bool active) {
        const QString base =
            "QPushButton { border: none; padding: 6px 18px; border-radius: 999px; font-size: 13px; min-height: 30px; }";
        if (active) {
            btn->setStyleSheet(base + " QPushButton { background-color: #3f3f46; color: #fafafa; font-weight: 600; }");
        } else {
            btn->setStyleSheet(base + " QPushButton { background-color: #27272a; color: #a1a1aa; }"
                                     " QPushButton:hover { background-color: #3f3f46; color: #fafafa; }");
        }
    };

    applyTabStyle(m_installedTabButton, true);
    applyTabStyle(m_settingsTabButton, false);

    tabLayout->addWidget(m_installedTabButton);
    tabLayout->addWidget(m_settingsTabButton);
    tabLayout->addWidget(spacer);
    mainLayout->addWidget(tabBar, 0);

    m_innerStack = new QStackedWidget(this);
    m_innerStack->addWidget(installedWidget);
    m_settingsPlaceholder = new QWidget(this);
    auto* placeLayout = new QVBoxLayout(m_settingsPlaceholder);
    placeLayout->addStretch();
    auto* placeLabel = new QLabel(tr("Loading..."), m_settingsPlaceholder);
    placeLabel->setAlignment(Qt::AlignCenter);
    placeLabel->setStyleSheet("color: #a1a1aa; font-size: 15px;");
    placeLayout->addWidget(placeLabel);
    placeLayout->addStretch();
    m_innerStack->addWidget(m_settingsPlaceholder);
    mainLayout->addWidget(m_innerStack, 1);

    m_currentIndex = 0;

    connect(m_installedTabButton, &QPushButton::clicked, this, [this, applyTabStyle]() {
        m_currentIndex = 0;
        if (m_innerStack) m_innerStack->setCurrentIndex(0);
        applyTabStyle(m_installedTabButton, true);
        applyTabStyle(m_settingsTabButton, false);
    });

    connect(m_settingsTabButton, &QPushButton::clicked, this, [this, applyTabStyle]() {
        m_currentIndex = 1;
        if (m_mainWindow) m_mainWindow->ensureSettingsWidgetForUserPage();
        if (m_innerStack) m_innerStack->setCurrentIndex(1);
        applyTabStyle(m_installedTabButton, false);
        applyTabStyle(m_settingsTabButton, true);
    });
}
 
void UserPageWidget::setSettingsWidget(SettingsWidget* w) {
    if (!w || !m_innerStack || !m_settingsPlaceholder) return;
    m_innerStack->removeWidget(m_settingsPlaceholder);
    m_settingsPlaceholder->deleteLater();
    m_settingsPlaceholder = nullptr;
    m_innerStack->addWidget(w);
    if (m_currentIndex == 1) {
        m_innerStack->setCurrentIndex(1);
    }
}
