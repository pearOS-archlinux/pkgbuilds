#include "SettingsDialog.h"
#include "Config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Notch Settings");
    setFixedWidth(300);

    auto* root = new QVBoxLayout(this);

    // Mode
    auto* modeGroup = new QGroupBox("Mode", this);
    auto* modeForm  = new QFormLayout(modeGroup);
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem("Dynamic Island", "island");
    m_modeCombo->addItem("Notch (top bar)", "notch");
    modeForm->addRow("Style:", m_modeCombo);
    root->addWidget(modeGroup);

    // Size
    auto* sizeGroup = new QGroupBox("Size", this);
    auto* sizeForm  = new QFormLayout(sizeGroup);

    m_smallW = new QSpinBox(this); m_smallW->setRange(80, 600);
    m_smallH = new QSpinBox(this); m_smallH->setRange(16, 80);
    m_bigW   = new QSpinBox(this); m_bigW->setRange(200, 900);
    m_bigH   = new QSpinBox(this); m_bigH->setRange(60, 300);

    sizeForm->addRow("Collapsed width:",  m_smallW);
    sizeForm->addRow("Collapsed height:", m_smallH);
    sizeForm->addRow("Expanded width:",   m_bigW);
    sizeForm->addRow("Expanded height:",  m_bigH);
    root->addWidget(sizeGroup);

    // Behaviour
    auto* behGroup = new QGroupBox("Behaviour", this);
    auto* behForm  = new QFormLayout(behGroup);

    m_screenGap    = new QSpinBox(this); m_screenGap->setRange(0, 40);
    m_screenGap->setSuffix(" px");
    m_hoverDelay   = new QSpinBox(this); m_hoverDelay->setRange(0, 2000);
    m_hoverDelay->setSuffix(" ms");
    m_animDuration = new QSpinBox(this); m_animDuration->setRange(50, 1000);
    m_animDuration->setSuffix(" ms");

    behForm->addRow("Screen gap:",       m_screenGap);
    behForm->addRow("Hover delay:",      m_hoverDelay);
    behForm->addRow("Anim duration:",    m_animDuration);
    root->addWidget(behGroup);

    // Buttons
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &SettingsDialog::apply);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onModeChanged);

    loadValues();
}

void SettingsDialog::loadValues() {
    Config& cfg = Config::instance();

    int modeIdx = (cfg.mode() == "notch") ? 1 : 0;
    m_modeCombo->setCurrentIndex(modeIdx);

    m_smallW->setValue(cfg.smallW());
    m_smallH->setValue(cfg.smallH());
    m_bigW->setValue(cfg.bigW());
    m_bigH->setValue(cfg.bigH());
    m_screenGap->setValue(cfg.screenGap());
    m_hoverDelay->setValue(cfg.hoverDelayMs());
    m_animDuration->setValue(cfg.animDurationMs());

    // Screen gap only relevant for island mode
    m_screenGap->setEnabled(cfg.mode() == "island");
}

void SettingsDialog::onModeChanged(int index) {
    m_screenGap->setEnabled(index == 0);  // 0 = island
}

void SettingsDialog::apply() {
    Config& cfg = Config::instance();

    QString newMode = m_modeCombo->currentData().toString();
    cfg.setMode(newMode);
    cfg.setSmallW(m_smallW->value());
    cfg.setSmallH(m_smallH->value());
    cfg.setBigW(m_bigW->value());
    cfg.setBigH(m_bigH->value());
    cfg.setScreenGap(m_screenGap->value());
    cfg.setHoverDelayMs(m_hoverDelay->value());
    cfg.setAnimDurationMs(m_animDuration->value());

    emit modeChanged(newMode);
    accept();
}
