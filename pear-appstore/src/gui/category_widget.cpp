#include "category_widget.h"
#include "package_card.h"
#include "scroll_bar_fade.h"
#include "../core/alpm_wrapper.h"
#include "../core/appstream_helper.h"
#include "../utils/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QSet>
#include <QTimer>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QMouseEvent>

// ── Hero card (clickable QFrame) ─────────────────────────────────────────────

class HeroCard : public QFrame {
    Q_OBJECT
public:
    explicit HeroCard(const QPair<QString,QString>& app, QWidget* parent = nullptr)
        : QFrame(parent), m_info()
    {
        m_info.name = app.first;
        m_info.repository = app.second;

        setObjectName("heroCard");
        setStyleSheet(
            "QFrame#heroCard {"
            "  background-color: #242730;"
            "  border-radius: 18px;"
            "  border: 1px solid #3a3d4a;"
            "}"
            "QFrame#heroCard:hover {"
            "  background-color: #2a2e38;"
            "  border: 1px solid #4a5068;"
            "}");
        setFixedHeight(182);
        setCursor(Qt::PointingHandCursor);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(24, 20, 20, 20);
        layout->setSpacing(18);

        // Left: icon
        m_iconLabel = new QLabel(this);
        m_iconLabel->setFixedSize(80, 80);
        m_iconLabel->setAlignment(Qt::AlignCenter);
        m_iconLabel->setStyleSheet(
            "background-color: #2e3240; border-radius: 18px; border: none;");
        setLetterIcon(80);
        layout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);

        // Right: text column
        auto* textCol = new QVBoxLayout();
        textCol->setSpacing(4);
        textCol->setContentsMargins(0, 0, 0, 0);
        textCol->addStretch(1);

        auto* tagLbl = new QLabel(tr("FEATURED"), this);
        tagLbl->setStyleSheet(
            "background: transparent; color: #5b8dee;"
            "font-size: 10px; font-weight: 700; letter-spacing: 1.2px;");
        textCol->addWidget(tagLbl);

        m_nameLbl = new QLabel(app.first, this);
        auto nf = m_nameLbl->font();
        nf.setPointSize(20);
        nf.setBold(true);
        m_nameLbl->setFont(nf);
        m_nameLbl->setStyleSheet("background: transparent; color: #f4f4f5;");
        textCol->addWidget(m_nameLbl);

        m_descLbl = new QLabel(this);
        m_descLbl->setStyleSheet(
            "background: transparent; color: #71717a; font-size: 13px;");
        m_descLbl->setWordWrap(true);
        m_descLbl->setMaximumHeight(40);
        textCol->addWidget(m_descLbl);

        textCol->addStretch(1);

        layout->addLayout(textCol, 1);

        // Right: Get button
        auto* btnCol = new QVBoxLayout();
        btnCol->setContentsMargins(0, 0, 0, 0);
        btnCol->addStretch();
        m_getBtn = new QPushButton(tr("Get"), this);
        m_getBtn->setObjectName("discoverDownloadButton");
        m_getBtn->setFixedSize(62, 30);
        m_getBtn->setCursor(Qt::PointingHandCursor);
        connect(m_getBtn, &QPushButton::clicked, this, &HeroCard::onGetClicked);
        btnCol->addWidget(m_getBtn);
        btnCol->addStretch();
        layout->addLayout(btnCol, 0);

        // Load AppStream async for icon + description
        auto* watcher = new QFutureWatcher<AppStreamData>(this);
        connect(watcher, &QFutureWatcher<AppStreamData>::finished, this,
                [this, watcher]() {
            AppStreamData data = watcher->result();
            watcher->deleteLater();
            updateFromAppStream(data);
        });
        QString pkgName = app.first;
        watcher->setFuture(QtConcurrent::run([pkgName]() {
            return AppStreamHelper::getDataForPackage(pkgName);
        }));

        // Also try ALPM description in background
        (void)QtConcurrent::run([pkgName, this]() {
            PackageInfo real = AlpmWrapper::instance().getPackageInfo(pkgName);
            if (!real.description.isEmpty()) {
                QMetaObject::invokeMethod(this, [this, real]() {
                    m_info.version    = real.version;
                    m_info.repository = real.repository.isEmpty() ? m_info.repository : real.repository;
                    if (m_descLbl->text().isEmpty())
                        m_descLbl->setText(real.description);
                }, Qt::QueuedConnection);
            }
        });
    }

    void updateInstallStatus(bool installed) {
        m_isInstalled = installed;
        if (m_getBtn) {
            m_getBtn->setText(installed ? tr("Installed") : tr("Get"));
            m_getBtn->setEnabled(!installed);
        }
    }

    const PackageInfo& packageInfo() const { return m_info; }

signals:
    void openRequested(const PackageInfo& info);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            m_pressPos = event->pos();
        }
        QFrame::mousePressEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton &&
            (event->pos() - m_pressPos).manhattanLength() < 8) {
            emit openRequested(m_info);
        }
        QFrame::mouseReleaseEvent(event);
    }

private slots:
    void onGetClicked() {
        emit openRequested(m_info);
    }

private:
    void setLetterIcon(int size) {
        QPixmap px(size, size);
        px.fill(Qt::transparent);
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#2e3240"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, 0, size, size, size / 5, size / 5);
        p.setPen(QColor("#71717a"));
        QFont f = p.font();
        f.setPointSize(size / 3);
        f.setBold(true);
        p.setFont(f);
        QString letter = m_info.name.isEmpty() ? QStringLiteral("?")
                                                : m_info.name.left(1).toUpper();
        p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, letter);
        p.end();
        m_iconLabel->setPixmap(px);
    }

    void updateFromAppStream(const AppStreamData& data) {
        const int sz = 80;
        QPixmap px;
        if (!data.iconPath.isEmpty())
            px.load(data.iconPath);
        if (px.isNull() && !data.iconStockName.isEmpty())
            px = QIcon::fromTheme(data.iconStockName).pixmap(sz, sz);
        if (px.isNull()) {
            const QString name = m_info.name;
            for (const QString& c : {name, name.toLower(),
                                      QString(name).replace(QLatin1Char('-'), QLatin1Char('_')),
                                      name.toLower().replace(QLatin1Char('-'), QLatin1Char('_'))}) {
                QIcon ic = QIcon::fromTheme(c);
                if (!ic.isNull()) { px = ic.pixmap(sz, sz); break; }
            }
        }
        if (!px.isNull()) {
            m_iconLabel->setStyleSheet("background: transparent; border: none;");
            m_iconLabel->setPixmap(px.scaled(sz, sz, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
        }
        QString desc = data.summary.isEmpty() ? data.description : data.summary;
        if (!desc.isEmpty() && m_descLbl->text().isEmpty())
            m_descLbl->setText(desc);
    }

    PackageInfo     m_info;
    QLabel*         m_iconLabel  = nullptr;
    QLabel*         m_nameLbl    = nullptr;
    QLabel*         m_descLbl    = nullptr;
    QPushButton*    m_getBtn     = nullptr;
    QPoint          m_pressPos;
    bool            m_isInstalled = false;
};

#include "category_widget.moc"

// ── helpers ──────────────────────────────────────────────────────────────────

static QLabel* makeSectionHeader(const QString& title, QWidget* parent) {
    auto* lbl = new QLabel(title, parent);
    auto f = lbl->font();
    f.setPointSize(17);
    f.setBold(true);
    lbl->setFont(f);
    lbl->setStyleSheet("color: #f9fafb; background: transparent;");
    return lbl;
}

static QFrame* makeSeparator(QWidget* parent) {
    auto* sep = new QFrame(parent);
    sep->setFixedHeight(1);
    sep->setStyleSheet("QFrame { background-color: rgba(255,255,255,0.07); border: none; }");
    return sep;
}

// ── CategoryWidget ────────────────────────────────────────────────────────────

CategoryWidget::CategoryWidget(const QString& categoryTitle,
                               const QString& categorySubtitle,
                               const QVector<AppSection>& sections,
                               QWidget* parent)
    : QWidget(parent)
{
    buildUi(categoryTitle, categorySubtitle, sections);

    auto refreshStatus = [this]() {
        (void)QtConcurrent::run([this]() {
            QVector<PackageInfo> installed = AlpmWrapper::instance().getInstalledPackages();
            QSet<QString> names;
            for (const auto& p : installed) names.insert(p.name);
            QMetaObject::invokeMethod(this, [this, names]() {
                updateAllInstallStatus(names);
            }, Qt::QueuedConnection);
        });
    };
    refreshStatus();
    QTimer::singleShot(4000, this, refreshStatus);
}

void CategoryWidget::updateAllInstallStatus(const QSet<QString>& installed) {
    for (auto* card : m_allCards)
        card->updateInstallStatus(installed.contains(card->packageInfo().name));
    for (auto* hero : m_heroCards)
        hero->updateInstallStatus(installed.contains(hero->packageInfo().name));
}

// ── Grid section ──────────────────────────────────────────────────────────────

QWidget* CategoryWidget::buildGridSection(const QVector<QPair<QString,QString>>& apps,
                                          QWidget* parent)
{
    auto* container = new QWidget(parent);
    container->setStyleSheet("background: transparent;");
    auto* grid = new QGridLayout(container);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);      // PackageCard separators serve as visual dividers
    grid->setHorizontalSpacing(24);

    int row = 0, col = 0;
    for (const auto& [name, repo] : apps) {
        PackageInfo pkg;
        pkg.name       = name;
        pkg.repository = repo;
        pkg.version    = "Latest";

        auto* card = new PackageCard(pkg, container, PackageCard::Discover);
        card->loadAppStreamAsync();

        connect(card, &PackageCard::clicked, this, [this](const PackageInfo& info) {
            emit openPackageRequested(info);
        });

        m_allCards.append(card);
        grid->addWidget(card, row, col);
        ++col;
        if (col >= 2) { col = 0; ++row; }
    }
    // Allow columns to share width equally
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    return container;
}

// ── Main layout ───────────────────────────────────────────────────────────────

void CategoryWidget::buildUi(const QString& title, const QString& subtitle,
                              const QVector<AppSection>& sections)
{
    auto* outerScroll = new QScrollArea(this);
    outerScroll->setWidgetResizable(true);
    outerScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outerScroll->setFrameShape(QFrame::NoFrame);
    outerScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    applyScrollBarFade(outerScroll);

    auto* content = new QWidget();
    content->setStyleSheet("background: transparent;");
    m_mainLayout = new QVBoxLayout(content);
    m_mainLayout->setContentsMargins(0, 0, 16, 32);
    m_mainLayout->setSpacing(0);

    // Page title
    auto* titleLbl = new QLabel(title, content);
    {
        auto tf = titleLbl->font();
        tf.setPointSize(26);
        tf.setBold(true);
        titleLbl->setFont(tf);
    }
    titleLbl->setStyleSheet("color: #f9fafb; background: transparent;");
    m_mainLayout->addWidget(titleLbl);

    if (!subtitle.isEmpty()) {
        auto* subLbl = new QLabel(subtitle, content);
        subLbl->setStyleSheet("color: #71717a; font-size: 14px; background: transparent;");
        subLbl->setWordWrap(true);
        m_mainLayout->addWidget(subLbl);
    }
    m_mainLayout->addSpacing(24);

    bool first = true;
    for (const auto& section : sections) {
        if (!first) {
            m_mainLayout->addSpacing(8);
            m_mainLayout->addWidget(makeSeparator(content));
            m_mainLayout->addSpacing(28);
        }

        // Hero card — only for the first section that declares featuredApp
        if (first && !section.featuredApp.isEmpty()) {
            QPair<QString,QString> heroApp = {section.featuredApp,
                section.apps.isEmpty() ? QStringLiteral("extra") : section.apps.first().second};
            auto* hero = new HeroCard(heroApp, content);
            connect(hero, &HeroCard::openRequested, this, &CategoryWidget::openPackageRequested);
            m_heroCards.append(hero);
            m_mainLayout->addWidget(hero);
            m_mainLayout->addSpacing(28);
        }

        m_mainLayout->addWidget(makeSectionHeader(section.title, content));
        m_mainLayout->addSpacing(8);
        m_mainLayout->addWidget(buildGridSection(section.apps, content));

        first = false;
    }
    m_mainLayout->addStretch();

    outerScroll->setWidget(content);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(outerScroll);
    setLayout(rootLayout);
}
