#include "sidereflection.h"
#include <QDebug>

SidebarReflection::SidebarReflection(QObject* parent) : QObject(parent) {}

QImage SidebarReflection::currentImage() const {
    QMutexLocker lock(&m_mutex);
    return m_image;
}

void SidebarReflection::setWindow(QQuickWindow* win) {
    m_window = win;
}

void SidebarReflection::doGrab(qreal rightPanelX) {
    if (!m_window || m_grabbing) return;
    m_grabbing = true;

    QImage full = m_window->grabWindow();
    m_grabbing = false;

    if (full.isNull()) return;

    qreal dpr = m_window->devicePixelRatio();
    int x = qRound(rightPanelX * dpr);
    int grabW = full.width() - x;  // grab entire right panel

    if (x < 0 || grabW <= 0) return;

    QImage strip = full.copy(x, 0, grabW, full.height());
    qDebug() << "[SideRefl] strip format:" << strip.format()
             << "center pixel:" << QColor(strip.pixel(strip.width()/2, strip.height()/2)).name(QColor::HexArgb);
    strip.save("/tmp/refl_strip.png");

    QImage blurred = applyBlur(strip);
    blurred.save("/tmp/refl_blurred.png");

    {
        QMutexLocker lock(&m_mutex);
        m_image = blurred;
        m_version++;
    }
    emit versionChanged();
}

// Heavy blur: downscale to ~2% then upscale — equivalent to QGraphicsBlurEffect(radius=65)
QImage SidebarReflection::applyBlur(const QImage& src) {
    if (src.isNull()) return src;
    QImage img = src.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int bw = qMax(1, img.width() / 40);
    int bh = qMax(1, img.height() / 40);
    QImage small = img.scaled(bw, bh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return small.scaled(img.width(), img.height(),
                        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

SidebarReflectionProvider::SidebarReflectionProvider(SidebarReflection* ref)
    : QQuickImageProvider(QQuickImageProvider::Image), m_ref(ref) {}

QImage SidebarReflectionProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    QImage img = m_ref->currentImage();
    qDebug() << "[SideRefl] requestImage id:" << id << "img size:" << img.size() << "isNull:" << img.isNull();
    if (size) *size = img.size();
    return img;
}
