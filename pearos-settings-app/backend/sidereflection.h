#pragma once
#include <QObject>
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>
#include <QQuickWindow>

class SidebarReflection : public QObject {
    Q_OBJECT
    Q_PROPERTY(int version READ version NOTIFY versionChanged)

public:
    explicit SidebarReflection(QObject* parent = nullptr);

    int    version() const { return m_version; }
    QImage currentImage() const;

    Q_INVOKABLE void setWindow(QQuickWindow* win);
    Q_INVOKABLE void doGrab(qreal rightPanelX);

signals:
    void versionChanged();

private:
    static QImage applyBlur(const QImage& src);

    QQuickWindow*  m_window   = nullptr;
    mutable QMutex m_mutex;
    QImage         m_image;
    int            m_version  = 0;
    bool           m_grabbing = false;
};

class SidebarReflectionProvider : public QQuickImageProvider {
public:
    explicit SidebarReflectionProvider(SidebarReflection* ref);
    QImage requestImage(const QString&, QSize* size, const QSize&) override;
private:
    SidebarReflection* m_ref;
};
