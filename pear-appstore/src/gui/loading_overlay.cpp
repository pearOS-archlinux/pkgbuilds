#include "loading_overlay.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QPalette>

LoadingOverlay::LoadingOverlay(QWidget* parent)
    : QWidget(parent)
    , m_spinner(new QProgressBar(this))
    , m_label(new QLabel(tr("Loading..."), this)) {

    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setStyleSheet(
        "LoadingOverlay { background-color: rgba(24, 24, 27, 0.9); }"
    );

    const QString accent = qApp->palette().color(QPalette::Highlight).name();
    m_spinner->setRange(0, 0);
    m_spinner->setMinimumWidth(48);
    m_spinner->setMinimumHeight(48);
    m_spinner->setMaximumWidth(48);
    m_spinner->setMaximumHeight(48);
    m_spinner->setTextVisible(false);
    m_spinner->setStyleSheet(
        "QProgressBar {"
        "  border: none; border-radius: 24px;"
        "  background-color: rgba(255,255,255,0.15);"
        "}"
        "QProgressBar::chunk {"
        "  border-radius: 24px;"
        "  background-color: " + accent + ";"
        "}"
    );

    m_label->setStyleSheet("color: #a1a1aa; font-size: 14px;");
    m_label->setAlignment(Qt::AlignCenter);

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(12);
    layout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    layout->addWidget(m_label, 0, Qt::AlignHCenter);
}
