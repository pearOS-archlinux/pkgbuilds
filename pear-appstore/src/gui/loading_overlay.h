#ifndef LOADING_OVERLAY_H
#define LOADING_OVERLAY_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>

/**
 * @brief Overlay cu spinner (progress bar indeterminat) pentru încărcare.
 * Ascunde conținutul și afișează un indicator vizibil.
 */
class LoadingOverlay : public QWidget {
    Q_OBJECT
public:
    explicit LoadingOverlay(QWidget* parent = nullptr);

private:
    QProgressBar* m_spinner = nullptr;
    QLabel* m_label = nullptr;
};

#endif // LOADING_OVERLAY_H
