#include "viewoptionspopup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>

using namespace Filer;

ViewOptionsPopup::ViewOptionsPopup(QWidget* parent):
  QDialog(parent, Qt::Tool) { // Use Qt::Tool for floating utility window
  setWindowTitle(tr("View Options"));
  setFixedSize(300, 350);
  
  QVBoxLayout* layout = new QVBoxLayout(this);
  
  // Stack by
  QHBoxLayout* stackLayout = new QHBoxLayout();
  stackLayout->addWidget(new QLabel(tr("Stack By:"), this));
  stackByCombo_ = new QComboBox(this);
  stackByCombo_->addItems({tr("None"), tr("Kind"), tr("Date Last Opened"), tr("Date Added"), tr("Date Modified"), tr("Date Created"), tr("Tags")});
  stackLayout->addWidget(stackByCombo_);
  layout->addLayout(stackLayout);
  
  // Sort by
  QHBoxLayout* sortLayout = new QHBoxLayout();
  sortLayout->addWidget(new QLabel(tr("Sort By:"), this));
  sortByCombo_ = new QComboBox(this);
  sortByCombo_->addItems({tr("None"), tr("Snap To Grid"), tr("Kind"), tr("Date Last Opened"), tr("Date Added"), tr("Date Modified"), tr("Date Created"), tr("Tags")});
  sortLayout->addWidget(sortByCombo_);
  layout->addLayout(sortLayout);
  
  // Icon size
  QHBoxLayout* iconSizeLayout = new QHBoxLayout();
  iconSizeLayout->addWidget(new QLabel(tr("Icon size:"), this));
  iconSizeSlider_ = new QSlider(Qt::Horizontal, this);
  iconSizeSlider_->setRange(16, 128);
  iconSizeSlider_->setValue(64);
  iconSizeLayout->addWidget(iconSizeSlider_);
  layout->addLayout(iconSizeLayout);
  
  // Grid spacing
  QHBoxLayout* gridSpacingLayout = new QHBoxLayout();
  gridSpacingLayout->addWidget(new QLabel(tr("Grid spacing:"), this));
  gridSpacingSlider_ = new QSlider(Qt::Horizontal, this);
  gridSpacingSlider_->setRange(60, 400);
  gridSpacingSlider_->setValue(120);
  gridSpacingLayout->addWidget(gridSpacingSlider_);
  layout->addLayout(gridSpacingLayout);
  
  // Text size
  QHBoxLayout* textSizeLayout = new QHBoxLayout();
  textSizeLayout->addWidget(new QLabel(tr("Text size:"), this));
  textSizeSpinBox_ = new QSpinBox(this);
  textSizeSpinBox_->setRange(10, 16);
  textSizeSpinBox_->setValue(12);
  // The value is applied via QFont::setPointSize() (desktopitemdelegate.cpp),
  // i.e. it's already in points, not pixels -- label it explicitly so it
  // isn't mistaken for a px value.
  textSizeSpinBox_->setSuffix(tr(" pt"));
  textSizeLayout->addWidget(textSizeSpinBox_);
  layout->addLayout(textSizeLayout);
  
  // Label Position
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->addWidget(new QLabel(tr("Label position:"), this));
  labelBottomRadio_ = new QRadioButton(tr("Bottom"), this);
  labelRightRadio_ = new QRadioButton(tr("Right"), this);
  labelBottomRadio_->setChecked(true);
  QButtonGroup* labelGroup = new QButtonGroup(this);
  labelGroup->addButton(labelBottomRadio_);
  labelGroup->addButton(labelRightRadio_);
  labelLayout->addWidget(labelBottomRadio_);
  labelLayout->addWidget(labelRightRadio_);
  layout->addLayout(labelLayout);
  
  // Checkboxes
  showItemInfoCheck_ = new QCheckBox(tr("Show item info"), this);
  layout->addWidget(showItemInfoCheck_);
  showIconPreviewCheck_ = new QCheckBox(tr("Show icon preview"), this);
  layout->addWidget(showIconPreviewCheck_);
  
  layout->addStretch();
  
  // Connections
  connect(stackByCombo_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ViewOptionsPopup::stackByChanged);
  connect(sortByCombo_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ViewOptionsPopup::sortByChanged);
  connect(iconSizeSlider_, &QSlider::valueChanged, this, &ViewOptionsPopup::iconSizeChanged);
  connect(gridSpacingSlider_, &QSlider::valueChanged, this, &ViewOptionsPopup::gridSpacingChanged);
  connect(textSizeSpinBox_, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ViewOptionsPopup::textSizeChanged);
  connect(labelRightRadio_, &QRadioButton::toggled, this, &ViewOptionsPopup::labelPositionChanged);
  connect(showItemInfoCheck_, &QCheckBox::toggled, this, &ViewOptionsPopup::showItemInfoChanged);
  connect(showIconPreviewCheck_, &QCheckBox::toggled, this, &ViewOptionsPopup::showIconPreviewChanged);
}

void ViewOptionsPopup::setIconSizeValue(int size) {
    iconSizeSlider_->blockSignals(true);
    iconSizeSlider_->setValue(size);
    iconSizeSlider_->blockSignals(false);
}

void ViewOptionsPopup::setGridSpacingValue(int spacing) {
    gridSpacingSlider_->blockSignals(true);
    gridSpacingSlider_->setValue(spacing);
    gridSpacingSlider_->blockSignals(false);
}

void ViewOptionsPopup::setTextSizeValue(int size) {
    textSizeSpinBox_->blockSignals(true);
    textSizeSpinBox_->setValue(size);
    textSizeSpinBox_->blockSignals(false);
}

void ViewOptionsPopup::setLabelPositionValue(bool right) {
    labelRightRadio_->blockSignals(true);
    labelBottomRadio_->blockSignals(true);
    labelRightRadio_->setChecked(right);
    labelBottomRadio_->setChecked(!right);
    labelRightRadio_->blockSignals(false);
    labelBottomRadio_->blockSignals(false);
}

void ViewOptionsPopup::setShowItemInfoValue(bool show) {
    showItemInfoCheck_->blockSignals(true);
    showItemInfoCheck_->setChecked(show);
    showItemInfoCheck_->blockSignals(false);
}

void ViewOptionsPopup::setShowIconPreviewValue(bool show) {
    showIconPreviewCheck_->blockSignals(true);
    showIconPreviewCheck_->setChecked(show);
    showIconPreviewCheck_->blockSignals(false);
}
