#ifndef FILER_VIEWOPTIONSPOPUP_H
#define FILER_VIEWOPTIONSPOPUP_H

#include <QDialog>

class QComboBox;
class QSlider;
class QSpinBox;
class QRadioButton;
class QCheckBox;

namespace Filer {

class ViewOptionsPopup : public QDialog {
Q_OBJECT
public:
  explicit ViewOptionsPopup(QWidget* parent = 0);

  void setIconSizeValue(int size);
  void setGridSpacingValue(int spacing);
  void setTextSizeValue(int size);
  void setLabelPositionValue(bool right);
  void setShowItemInfoValue(bool show);
  void setShowIconPreviewValue(bool show);
  
Q_SIGNALS:
  void stackByChanged(int index);
  void sortByChanged(int index);
  void iconSizeChanged(int size);
  void gridSpacingChanged(int spacing);
  void textSizeChanged(int size);
  void labelPositionChanged(bool right);
  void showItemInfoChanged(bool show);
  void showIconPreviewChanged(bool show);

private:
  QComboBox* stackByCombo_;
  QComboBox* sortByCombo_;
  QSlider* iconSizeSlider_;
  QSlider* gridSpacingSlider_;
  QSpinBox* textSizeSpinBox_;
  QRadioButton* labelBottomRadio_;
  QRadioButton* labelRightRadio_;
  QCheckBox* showItemInfoCheck_;
  QCheckBox* showIconPreviewCheck_;
};

}

#endif // FILER_VIEWOPTIONSPOPUP_H
