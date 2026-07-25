#include "quicklookdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPixmap>
#include <QImageReader>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QPushButton>
#include <QMouseEvent>
#include "icontheme.h"
#include "bundle.h"
#include "plistparser.h"

// Sums the apparent size of every file under path, recursively -- used for
// .app/.AppDir bundles, which are directories on disk but should be quick-
// looked as a single sized "file" (an item count, as shown for ordinary
// folders, wouldn't tell the user anything useful about a bundle).
static qint64 recursiveDirSize(const QString& path) {
    qint64 total = 0;
    QDirIterator it(path, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

QuickLookDialog* QuickLookDialog::s_instance = nullptr;

// ─────────────────────────────────────────────────────────────
// Helper: build a fully-populated container widget for a file
// ─────────────────────────────────────────────────────────────
static QWidget* buildContainer(FmFileInfo* info, int& outWidth, int& outHeight, QuickLookDialog* parent) {
    QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(info)));
    FmMimeType* mime = fm_file_info_get_mime_type(info);
    const char* mt   = mime ? fm_mime_type_get_type(mime) : nullptr;
    QString mimeStr  = mt ? QString::fromUtf8(mt) : "";

    bool isImage = mimeStr.startsWith("image/");
    bool isText  = mimeStr.startsWith("text/");
    bool isDir   = (bool)fm_file_info_is_dir(info);
    // A .app/.AppDir bundle is a directory on disk, but Quick Looking it as
    // a plain folder (just an item count) hides everything actually useful
    // about it -- show it as an application instead: name/version/copyright
    // from Info.plist, plus its real icon and total size.
    bool isAppBundle = isDir && Fm::checkWhetherAppDirOrBundle(info);
    QHash<QString, QString> plist;
    if(isAppBundle) {
        QString plistPath = path + "/Contents/Info.plist";
        if(QFile::exists(plistPath))
            plist = Fm::parseInfoPlist(plistPath);
    }

    outWidth  = 600;
    outHeight = 450;

    QWidget* container = new QWidget(parent);
    container->setObjectName("qlContainer");
    container->setStyleSheet(
        "#qlContainer {"
        "  background-color: rgba(28, 28, 30, 245);"
        "  border-radius: 16px;"
        "  border: 1px solid rgba(255,255,255,0.12);"
        "}"
    );
    // Purely decorative -- let clicks fall through to the QDialog itself so
    // the window can be dragged by its background, not just a title bar it
    // doesn't have. Interactive widgets (the text preview, the close
    // button below) are left alone and keep consuming their own clicks.
    container->setAttribute(Qt::WA_TransparentForMouseEvents);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 150));
    container->setGraphicsEffect(shadow);

    QVBoxLayout* lay = new QVBoxLayout(container);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(10);

    // ── Preview area ────────────────────────────────
    if (isImage) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            QPixmap px = QPixmap::fromImageReader(&reader);
            if (!px.isNull()) {
                if (px.width() > 900 || px.height() > 700)
                    px = px.scaled(900, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                QLabel* img = new QLabel(container);
                img->setPixmap(px);
                img->setAlignment(Qt::AlignCenter);
                img->setStyleSheet("background: transparent;");
                img->setAttribute(Qt::WA_TransparentForMouseEvents);
                lay->addWidget(img);

                outWidth  = qMax(400, px.width()  + 64);
                outHeight = qMax(300, px.height() + 120);
            }
        }
    } else if (isText) {
        QTextEdit* te = new QTextEdit(container);
        te->setReadOnly(true);
        te->setStyleSheet(
            "QTextEdit { background: transparent; color: #e8e8e8; border: none; }"
            "QScrollBar:vertical { background: rgba(255,255,255,0.05); width: 8px; border-radius: 4px; }"
            "QScrollBar::handle:vertical { background: rgba(255,255,255,0.3); border-radius: 4px; }"
        );
        QFont mono;
        mono.setFamily("Menlo");
        mono.setPointSize(12);
        te->setFont(mono);
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            te->setPlainText(QString::fromUtf8(file.read(150 * 1024)));
        lay->addWidget(te);
        outWidth  = 780;
        outHeight = 560;
    } else {
        outWidth  = 540;
        outHeight = isAppBundle ? 230 : 180;
    }

    // ── Separator (only for image/text) ──────────────
    if (isImage || isText) {
        QFrame* sep = new QFrame(container);
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("QFrame { color: rgba(255,255,255,0.1); }");
        sep->setAttribute(Qt::WA_TransparentForMouseEvents);
        lay->addWidget(sep);
    }

    // ── Info bar ─────────────────────────────────────
    QHBoxLayout* row = new QHBoxLayout();
    row->setSpacing(12);

    QLabel* iconLabel = new QLabel(container);
    QIcon qicon = isAppBundle ? Fm::getIconForBundle(info) : Fm::IconTheme::icon(fm_file_info_get_icon(info));
    int iconSz  = (isImage || isText) ? 32 : 96;
    iconLabel->setPixmap(qicon.pixmap(iconSz, iconSz));
    iconLabel->setStyleSheet("background: transparent;");
    iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    row->addWidget(iconLabel, 0, Qt::AlignVCenter);

    QVBoxLayout* meta = new QVBoxLayout();
    meta->setSpacing(2);

    QString displayName = QString::fromUtf8(fm_file_info_get_disp_name(info));
    if(isAppBundle) {
        QString bundleName = plist.value("CFBundleDisplayName");
        if(bundleName.isEmpty())
            bundleName = plist.value("CFBundleName");
        if(!bundleName.isEmpty())
            displayName = bundleName;
        else if(displayName.endsWith(".app", Qt::CaseInsensitive)
                || displayName.endsWith(".kext", Qt::CaseInsensitive))
            displayName.chop(displayName.endsWith(".app", Qt::CaseInsensitive) ? 4 : 5);
    }
    QLabel* nameL = new QLabel(displayName, container);
    QFont nf = nameL->font();
    nf.setPointSize((isImage || isText) ? nf.pointSize() + 1 : nf.pointSize() + 6);
    nf.setBold(true);
    nameL->setFont(nf);
    nameL->setStyleSheet("color: #ffffff; background: transparent;");
    nameL->setAttribute(Qt::WA_TransparentForMouseEvents);
    meta->addWidget(nameL);

    if(isAppBundle) {
        QString version = plist.value("CFBundleShortVersionString");
        if(version.isEmpty())
            version = plist.value("CFBundleVersion");
        if(!version.isEmpty()) {
            QLabel* versionL = new QLabel("Version " + version, container);
            versionL->setStyleSheet("color: #a0a0a0; background: transparent;");
            versionL->setAttribute(Qt::WA_TransparentForMouseEvents);
            meta->addWidget(versionL);
        }
        QString copyright = plist.value("NSHumanReadableCopyright");
        if(!copyright.isEmpty()) {
            QLabel* copyrightL = new QLabel(copyright, container);
            copyrightL->setStyleSheet("color: #a0a0a0; background: transparent;");
            copyrightL->setWordWrap(true);
            copyrightL->setAttribute(Qt::WA_TransparentForMouseEvents);
            meta->addWidget(copyrightL);
        }
    }

    auto formatSize = [](qint64 sz) -> QString {
        if      (sz < 1024LL)               return QString::number(sz) + " bytes";
        else if (sz < 1024LL*1024)          return QString::number(sz / 1024.0, 'f', 1) + " KB";
        else if (sz < 1024LL*1024*1024)     return QString::number(sz / (1024.0*1024), 'f', 1) + " MB";
        else                                return QString::number(sz / (1024.0*1024*1024), 'f', 2) + " GB";
    };

    QString sizeStr;
    if (isAppBundle) {
        sizeStr = formatSize(recursiveDirSize(path));
    } else if (isDir) {
        QDir dir(path);
        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);
        sizeStr = QString::number(dir.count()) + " items";
    } else {
        sizeStr = formatSize(fm_file_info_get_size(info));
    }
    QLabel* sizeL = new QLabel(sizeStr, container);
    sizeL->setStyleSheet("color: #a0a0a0; background: transparent;");
    sizeL->setAttribute(Qt::WA_TransparentForMouseEvents);
    meta->addWidget(sizeL);

    QDateTime mtime = QDateTime::fromSecsSinceEpoch(fm_file_info_get_mtime(info));
    QLabel* dateL = new QLabel("Last modified: " + mtime.toString("d MMM yyyy 'at' hh:mm:ss"), container);
    dateL->setStyleSheet("color: #a0a0a0; background: transparent;");
    dateL->setAttribute(Qt::WA_TransparentForMouseEvents);
    meta->addWidget(dateL);

    row->addLayout(meta);
    row->addStretch();

    QLabel* hint = new QLabel("Space to close", container);
    hint->setStyleSheet(
        "color: rgba(255,255,255,0.35); background: rgba(255,255,255,0.06);"
        "border-radius: 4px; padding: 2px 8px; font-size: 11px;"
    );
    hint->setAttribute(Qt::WA_TransparentForMouseEvents);
    row->addWidget(hint, 0, Qt::AlignVCenter);

    // Small round close button -- same subtle/transparent visual language as
    // the "Space to close" hint chip right next to it, so it reads as part
    // of the same design rather than a bolted-on control.
    QPushButton* closeBtn = new QPushButton(QString::fromUtf8("\xC3\x97"), container); // "×"
    closeBtn->setFixedSize(22, 22);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  color: rgba(255,255,255,0.6); background: rgba(255,255,255,0.08);"
        "  border: none; border-radius: 11px; font-size: 13px; padding-bottom: 2px;"
        "}"
        "QPushButton:hover { background: rgba(255,255,255,0.18); color: #ffffff; }"
        "QPushButton:pressed { background: rgba(255,255,255,0.25); }"
    );
    QObject::connect(closeBtn, &QPushButton::clicked, parent, &QuickLookDialog::closeWithAnimation);
    row->addWidget(closeBtn, 0, Qt::AlignVCenter);

    lay->addLayout(row);

    return container;
}

// ─────────────────────────────────────────────
QuickLookDialog::QuickLookDialog(FmFileInfo* info, const QRect& sourceRect, QWidget* parent)
    : QDialog(parent), container_(nullptr), sourceRect_(sourceRect), closing_(false)
{
    s_instance = this;
    currentPath_ = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(info)));

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    // Qt::Tool windows aren't always given keyboard focus by the window
    // manager on show() (confirmed on this KWin/Wayland setup) -- without
    // focus, Space/Escape presses go to whatever had focus before (usually
    // the desktop icon view) instead of this dialog's keyPressEvent(), which
    // is exactly what made "Space to close" feel unreliable.
    setFocusPolicy(Qt::StrongFocus);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(12, 12, 12, 12);
    mainLayout_->setSpacing(0);

    animation_ = new QPropertyAnimation(this, "geometry", this);

    int w = 600, h = 450;
    container_ = buildContainer(info, w, h, this);
    mainLayout_->addWidget(container_);

    QScreen* screen = QGuiApplication::primaryScreen();
    if (parentWidget() && parentWidget()->screen())
        screen = parentWidget()->screen();
    QRect sg = screen->availableGeometry();
    targetRect_ = QRect(sg.center().x() - w/2, sg.center().y() - h/2, w, h);

    setGeometry(sourceRect_);
    animation_->setDuration(220);
    animation_->setEasingCurve(QEasingCurve::OutCubic);
    animation_->setStartValue(sourceRect_);
    animation_->setEndValue(targetRect_);
    animation_->start(QAbstractAnimation::KeepWhenStopped);

    activateWindow();
    setFocus(Qt::PopupFocusReason);
}

QuickLookDialog::~QuickLookDialog() {
    if (s_instance == this) s_instance = nullptr;
}

// ─────────────────────────────────────────────
void QuickLookDialog::refreshWith(FmFileInfo* info, const QRect& sourceRect) {
    if (closing_) return;

    currentPath_ = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(info)));
    sourceRect_ = sourceRect;
    animation_->stop();

    // Build new container BEFORE touching old one to avoid any flicker
    int w = 600, h = 450;
    QWidget* newContainer = buildContainer(info, w, h, this);

    // Swap: hide & schedule old for deletion, show new immediately
    if (container_) {
        mainLayout_->removeWidget(container_);
        container_->hide();
        container_->deleteLater();  // safe: no re-entrancy, just queued
    }
    container_ = newContainer;
    mainLayout_->addWidget(container_);

    // Recalculate centered target
    QScreen* screen = QGuiApplication::primaryScreen();
    if (parentWidget() && parentWidget()->screen())
        screen = parentWidget()->screen();
    QRect sg = screen->availableGeometry();
    targetRect_ = QRect(sg.center().x() - w/2, sg.center().y() - h/2, w, h);

    animation_->setDuration(120);
    animation_->setEasingCurve(QEasingCurve::OutCubic);
    animation_->setStartValue(geometry());
    animation_->setEndValue(targetRect_);
    animation_->start(QAbstractAnimation::KeepWhenStopped);

    raise();
    activateWindow();
    setFocus(Qt::PopupFocusReason);
}

// ─────────────────────────────────────────────
void QuickLookDialog::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Escape)
        closeWithAnimation();
    else
        QDialog::keyPressEvent(event);
}

void QuickLookDialog::closeEvent(QCloseEvent* event) {
    QDialog::closeEvent(event);
}

// The window is frameless (no native title bar to drag by), and the
// container's background/decorative labels are marked
// Qt::WA_TransparentForMouseEvents so their clicks land here instead --
// letting the whole panel (minus the text preview and close button, which
// keep their own interactions) act as a drag handle.
void QuickLookDialog::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) {
        dragging_ = true;
        dragOffset_ = event->globalPos() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void QuickLookDialog::mouseMoveEvent(QMouseEvent* event) {
    if(dragging_ && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - dragOffset_);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void QuickLookDialog::mouseReleaseEvent(QMouseEvent* event) {
    dragging_ = false;
    QDialog::mouseReleaseEvent(event);
}

void QuickLookDialog::closeWithAnimation() {
    if (closing_) return;
    closing_ = true;

    animation_->stop();
    animation_->setDuration(180);
    animation_->setStartValue(geometry());
    animation_->setEndValue(sourceRect_);
    animation_->setEasingCurve(QEasingCurve::InCubic);
    connect(animation_, &QPropertyAnimation::finished, this, &QuickLookDialog::closeAnimationFinished);
    animation_->start(QAbstractAnimation::KeepWhenStopped);
}

void QuickLookDialog::closeAnimationFinished() {
    close();
}
