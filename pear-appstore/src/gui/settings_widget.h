#ifndef SETTINGS_WIDGET_H
#define SETTINGS_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>

/**
 * @brief Widget for application and repository settings.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 */
class SettingsWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget() override = default;
    
    bool isMultilibEnabled() const;
    bool isChaoticAurEnabled() const;
    
signals:
    void multilibStatusChanged(bool enabled);
    void chaoticAurStatusChanged(bool enabled);
    
private:
    void setupUi();
    void loadCurrentSettings();
    void createRepositorySettings();
    void createChaoticAurSettings();
    void createMaintenanceSettings();
    bool isMultilibEnabledInPacmanConf() const;
    bool isChaoticAurEnabledInPacmanConf() const;
    bool enableMultilibInPacmanConf();
    bool disableMultilibInPacmanConf();
    bool enableChaoticAurInPacmanConf();
    bool disableChaoticAurInPacmanConf();
    void applySettings();
    
    // Repository settings (Qt parent-child managed, non-owning pointers)
    QGroupBox* m_repositoryGroup = nullptr;
    QCheckBox* m_coreRepoCheckbox = nullptr;
    QCheckBox* m_extraRepoCheckbox = nullptr;
    QCheckBox* m_multilibRepoCheckbox = nullptr;
    QCheckBox* m_chaoticAurCheckbox = nullptr;
    
    // Chaotic-AUR setup (Qt parent-child managed)
    QGroupBox* m_chaoticAurGroup = nullptr;
    QPushButton* m_setupChaoticButton = nullptr;
    QPushButton* m_removeChaoticButton = nullptr;
    
    // Maintenance settings (Qt parent-child managed)
    QGroupBox* m_maintenanceGroup = nullptr;
    QPushButton* m_removeLockButton = nullptr;
    QPushButton* m_syncReposButton = nullptr;
    QPushButton* m_cancelProcessButton = nullptr;
    
    // Control buttons (Qt parent-child managed)
    QPushButton* m_applyButton = nullptr;
    
    // Status (Qt parent-child managed)
    QLabel* m_statusLabel = nullptr;
    
    // Track original state
    bool m_originalMultilibState = false;
    bool m_originalChaoticAurState = false;
    
private slots:
    void onApplyClicked();
    void onSettingsChanged();
    void onSetupChaoticClicked();
    void onRemoveChaoticClicked();
    void onRemoveLockClicked();
    void onSyncReposClicked();
    void onCancelProcessClicked();
};

#endif // SETTINGS_WIDGET_H
