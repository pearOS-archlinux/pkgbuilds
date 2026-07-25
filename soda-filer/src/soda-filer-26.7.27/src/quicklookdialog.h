#ifndef QUICKLOOKDIALOG_H
#define QUICKLOOKDIALOG_H

#include <QDialog>
#include <QRect>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <libfm/fm.h>

class QuickLookDialog : public QDialog {
    Q_OBJECT
public:
    QuickLookDialog(FmFileInfo* info, const QRect& sourceRect, QWidget* parent = nullptr);
    ~QuickLookDialog();

    // Swap content to a new file (no close/reopen needed)
    void refreshWith(FmFileInfo* info, const QRect& sourceRect);

    // Returns the currently open instance (nullptr if none)
    static QuickLookDialog* activeInstance() { return s_instance; }

    // Path of the file currently shown, so callers can tell whether a new
    // Quick Look request is for the same file (should toggle-close) or a
    // different one (should refresh in place).
    const QString& currentPath() const { return currentPath_; }

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

public Q_SLOTS:
    void closeWithAnimation();

private Q_SLOTS:
    void closeAnimationFinished();

private:
    QVBoxLayout*          mainLayout_;
    QWidget*              container_;
    QRect                 sourceRect_;
    QRect                 targetRect_;
    QString               currentPath_;
    QPropertyAnimation*   animation_;
    bool                  closing_;
    bool                  dragging_ = false;
    QPoint                dragOffset_; // cursor position relative to the window's top-left, captured at press

    static QuickLookDialog* s_instance;
};

#endif // QUICKLOOKDIALOG_H
