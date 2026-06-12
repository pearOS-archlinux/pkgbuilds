#pragma once
#include <QDialog>

class QComboBox;
class QSpinBox;
class QSlider;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void modeChanged(const QString& mode);

private slots:
    void onModeChanged(int index);
    void apply();

private:
    QComboBox* m_modeCombo    = nullptr;
    QSpinBox*  m_smallW       = nullptr;
    QSpinBox*  m_smallH       = nullptr;
    QSpinBox*  m_bigW         = nullptr;
    QSpinBox*  m_bigH         = nullptr;
    QSpinBox*  m_screenGap    = nullptr;
    QSpinBox*  m_hoverDelay   = nullptr;
    QSpinBox*  m_animDuration = nullptr;

    void loadValues();
};
