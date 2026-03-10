#include "mainwindow.h"
#include "todo_widget.h"
#include "../utils/logger.h"
#include "../utils/todo_storage.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QMouseEvent>
#include <QWindow>
#include <QShowEvent>
#include <QResizeEvent>
#include <QPalette>
#include <QTimer>
#include <QtConcurrent>
#include <QGraphicsBlurEffect>
#include <QPixmap>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QInputDialog>
#include <QRegularExpression>
#include <QTextDocument>
#ifdef HAVE_KWINDOW_EFFECTS
#include <KWindowEffects>
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::ArrowCursor);

    setupUi();
    loadStyleSheet();
    qApp->installEventFilter(this);

    if (m_headerWidget) {
        m_headerWidget->installEventFilter(this);
    }
    if (m_centralWidget) {
        m_centralWidget->installEventFilter(this);
        m_centralWidget->setMouseTracking(true);
    }

    Logger::info("MainWindow created successfully");
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("pearOS Notes");
    setMinimumSize(800, 600);
    resize(1200, 800);

    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("appContainer");
    auto* appLayout = new QVBoxLayout(m_centralWidget);
    appLayout->setContentsMargins(0, 4, 2, 4);
    appLayout->setSpacing(0);

    auto* wrapper = new QWidget(this);
    wrapper->setObjectName("wrapper");
    auto* wrapperLayout = new QHBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(8, 4, 0, 4);
    wrapperLayout->setSpacing(7);

    setupSidebar();
    wrapperLayout->addWidget(m_leftSideWidget);

    m_rightColumn = new QWidget(this);
    m_rightColumn->setObjectName("rightColumn");
    auto* rightLayout = new QVBoxLayout(m_rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    setupHeader();
    rightLayout->addWidget(m_headerWidget);

    QWidget* contentWrapper = new QWidget(this);
    contentWrapper->setObjectName("contentWrapper");
    m_contentLayout = new QVBoxLayout(contentWrapper);
    m_contentLayout->setContentsMargins(24, 16, 0, 16);
    m_contentLayout->setSpacing(0);
    setupContent();
    rightLayout->addWidget(contentWrapper, 1);

    wrapperLayout->addWidget(m_rightColumn, 1);

    m_reflectionDebounceTimer = new QTimer(this);
    m_reflectionDebounceTimer->setSingleShot(true);
    connect(m_reflectionDebounceTimer, &QTimer::timeout, this, &MainWindow::updateSidebarReflection);
    m_reflectionUpdateTimer = new QTimer(this);
    connect(m_reflectionUpdateTimer, &QTimer::timeout, this, &MainWindow::updateSidebarReflection);
    m_reflectionUpdateTimer->start(0); // 0 = cât de des permite event loop-ul, blur în timp real
    QTimer::singleShot(500, this, &MainWindow::updateSidebarReflection);

    appLayout->addWidget(wrapper, 1);

    setCentralWidget(m_centralWidget);
    createMenuBar();
}

void MainWindow::setupHeader() {
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("titleBarDrag");
    m_headerWidget->setFixedHeight(8);
    m_headerWidget->setCursor(Qt::SizeAllCursor);
    m_headerWidget->setStyleSheet("QWidget#titleBarDrag { background: transparent; }");
}

namespace {
QPixmap tintPixmap(const QPixmap& src, const QColor& color) {
    if (src.isNull()) return src;
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    const int a = color.alpha();
    const int r = color.red();
    const int g = color.green();
    const int b = color.blue();
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QRgb pixel = img.pixel(x, y);
            int alpha = qAlpha(pixel);
            img.setPixel(x, y, qRgba(r, g, b, (alpha * a) / 255));
        }
    }
    return QPixmap::fromImage(img);
}

QString iconPathForListName(const QString& name) {
    const QString n = name.toLower();
    if (n.contains(QStringLiteral("work")) || n.contains(QStringLiteral("muncă")) || n.contains(QStringLiteral("job")))
        return QStringLiteral(":/icons/sidebar_icons/work.svg");
    if (n.contains(QStringLiteral("shop")) || n.contains(QStringLiteral("market")) || n.contains(QStringLiteral("store")) ||
        n.contains(QStringLiteral("shopping")) || n.contains(QStringLiteral("cumpăr")))
        return QStringLiteral(":/icons/sidebar_icons/shopping.svg");
    return QStringLiteral(":/icons/sidebar_icons/generic.svg");
}
} // namespace

void MainWindow::setupSidebar() {
    m_leftSideWidget = new QFrame(this);
    m_leftSideWidget->setObjectName("leftSide");
    m_leftSideWidget->setFixedWidth(240);

    m_sidebarReflection = new QLabel(m_leftSideWidget);
    m_sidebarReflection->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_sidebarReflection->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_sidebarReflection->setScaledContents(true);
    m_sidebarReflection->setStyleSheet("background: transparent;");
    auto* blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(65);
    blurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
    m_sidebarReflection->setGraphicsEffect(blurEffect);

    auto* sideLayout = new QVBoxLayout(m_leftSideWidget);
    sideLayout->setContentsMargins(12, 12, 26, 20);
    sideLayout->setSpacing(8);

    QWidget* menuCircles = new QWidget(this);
    menuCircles->setObjectName("menuCircles");
    menuCircles->setCursor(Qt::ArrowCursor);
    menuCircles->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto* circlesLayout = new QHBoxLayout(menuCircles);
    circlesLayout->setContentsMargins(0, 0, 0, 0);
    circlesLayout->setSpacing(6);
    circlesLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto makeTrafficBtn = [this](const QString& bgColor, const QString& symbol) -> QPushButton* {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(15, 15);
        btn->setCursor(Qt::ArrowCursor);
        btn->setFlat(true);
        btn->setStyleSheet("background: " + bgColor + "; border: none; border-radius: 7px;");
        QLabel* lbl = new QLabel(symbol, btn);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedSize(15, 15);
        lbl->setAttribute(Qt::WA_TransparentForMouseEvents);
        lbl->setStyleSheet("color: " + bgColor + "; font-size: 13px; font-weight: bold; background: transparent;");
        lbl->raise();
        btn->setProperty("trafficLabel", QVariant::fromValue<QLabel*>(lbl));
        return btn;
    };
    QPushButton* btnClose = makeTrafficBtn(QStringLiteral("#f96057"), QStringLiteral("\u00D7"));
    connect(btnClose, &QPushButton::clicked, this, &QMainWindow::close);
    circlesLayout->addWidget(btnClose);
    QPushButton* btnMin = makeTrafficBtn(QStringLiteral("#f8ce52"), QStringLiteral("\u2212"));
    connect(btnMin, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    circlesLayout->addWidget(btnMin);
    QPushButton* btnMax = makeTrafficBtn(QStringLiteral("#5fcf65"), QStringLiteral("\u25A1"));
    connect(btnMax, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    circlesLayout->addWidget(btnMax);
    auto getTrafficLabel = [](QPushButton* b) -> QLabel* {
        return b ? b->property("trafficLabel").value<QLabel*>() : nullptr;
    };
    struct TrafficHoverFilter : QObject {
        QWidget* circles = nullptr;
        QPushButton* bClose = nullptr;
        QPushButton* bMin = nullptr;
        QPushButton* bMax = nullptr;
        QLabel* lClose = nullptr;
        QLabel* lMin = nullptr;
        QLabel* lMax = nullptr;
        void setHovered(bool hovered) {
            const QString onColor = QStringLiteral("#1a1a1a");
            auto setLabel = [hovered, onColor](QLabel* l, const QString& hideColor) {
                if (!l) return;
                l->setStyleSheet("color: " + (hovered ? onColor : hideColor) + "; font-size: 13px; font-weight: bold; background: transparent;");
            };
            if (lClose) setLabel(lClose, QStringLiteral("#f96057"));
            if (lMin)   setLabel(lMin,   QStringLiteral("#f8ce52"));
            if (lMax)   setLabel(lMax,   QStringLiteral("#5fcf65"));
        }
        bool eventFilter(QObject* o, QEvent* e) override {
            if (!circles) return QObject::eventFilter(o, e);
            if (e->type() == QEvent::Enter) {
                setHovered(true);
            } else if (e->type() == QEvent::Leave) {
                QWidget* w = qApp->widgetAt(QCursor::pos());
                bool stillInside = w && (w == circles || circles->isAncestorOf(w));
                if (!stillInside) setHovered(false);
            }
            return QObject::eventFilter(o, e);
        }
    };
    auto* trafficFilter = new TrafficHoverFilter();
    trafficFilter->circles = menuCircles;
    trafficFilter->bClose = btnClose;
    trafficFilter->bMin = btnMin;
    trafficFilter->bMax = btnMax;
    trafficFilter->lClose = getTrafficLabel(btnClose);
    trafficFilter->lMin   = getTrafficLabel(btnMin);
    trafficFilter->lMax   = getTrafficLabel(btnMax);
    trafficFilter->setParent(menuCircles);
    trafficFilter->setHovered(false);
    menuCircles->installEventFilter(trafficFilter);
    btnClose->installEventFilter(trafficFilter);
    btnMin->installEventFilter(trafficFilter);
    btnMax->installEventFilter(trafficFilter);
    sideLayout->addWidget(menuCircles);
    sideLayout->addSpacing(20);

    m_sidebarSearch = new QLineEdit(this);
    m_sidebarSearch->setObjectName("sidebarSearch");
    m_sidebarSearch->setCursor(Qt::IBeamCursor);
    m_sidebarSearch->setPlaceholderText(tr("Filter lists"));
    m_sidebarSearch->setMinimumHeight(32);
    m_sidebarSearch->setClearButtonEnabled(true);
    sideLayout->addWidget(m_sidebarSearch);
    sideLayout->addSpacing(16);

    QWidget* listsHeader = new QWidget(this);
    auto* listsHeaderLayout = new QHBoxLayout(listsHeader);
    listsHeaderLayout->setContentsMargins(0, 0, 0, 0);
    listsHeaderLayout->setSpacing(8);

    QLabel* listsLabel = new QLabel(tr("My Notes"), listsHeader);
    QFont listsFont = listsLabel->font();
    listsFont.setPointSize(11);
    listsFont.setWeight(QFont::DemiBold);
    listsLabel->setFont(listsFont);
    listsLabel->setStyleSheet("color: #a1a1aa;");
    listsHeaderLayout->addWidget(listsLabel, 1);

    QPushButton* addListButton = new QPushButton(QStringLiteral("+"), listsHeader);
    addListButton->setObjectName("addListButton");
    addListButton->setFixedSize(22, 22);
    addListButton->setCursor(Qt::PointingHandCursor);
    addListButton->setStyleSheet(
        "QPushButton#addListButton {"
        "  background-color: #27272a;"
        "  border-radius: 11px;"
        "  color: #e5e5e5;"
        "  font-size: 14px;"
        "  padding: 0;"
        "}"
        "QPushButton#addListButton:hover {"
        "  background-color: #3f3f46;"
        "}"
    );
    listsHeaderLayout->addWidget(addListButton, 0);

    sideLayout->addWidget(listsHeader);
    sideLayout->addSpacing(4);

    m_sideList = new QListWidget(this);
    m_sideList->setObjectName("sideList");
    m_sideList->setCursor(Qt::ArrowCursor);
    m_sideList->setSpacing(2);
    m_sideList->setMouseTracking(true);
    if (m_sideList->viewport())
        m_sideList->viewport()->setMouseTracking(true);
    connect(m_sidebarSearch, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (!m_sideList) return;
        const QString query = text.trimmed().toLower();
        for (int i = 0; i < m_sideList->count(); ++i) {
            QListWidgetItem* item = m_sideList->item(i);
            const bool match = query.isEmpty() ||
                item->text().toLower().contains(query);
            item->setHidden(!match);
        }
    });

    connect(m_sideList, &QListWidget::currentRowChanged, this, [this](int row) {
        setCurrentList(row);
    });
    connect(m_sideList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (!item || !m_sideList) return;
        int row = m_sideList->row(item);
        if (row < 0 || row >= m_notes.size()) return;
        const QString currentName = m_notes.at(row).name;
        bool ok = false;
        QString newName = QInputDialog::getText(
            this,
            tr("Rename note"),
            tr("Note name:"),
            QLineEdit::Normal,
            currentName,
            &ok
        ).trimmed();
        if (!ok || newName.isEmpty() || newName == currentName) return;
        m_notes[row].name = newName;
        saveNotes();
        m_currentListIndex = row;
        refreshSidebarLists();
        setCurrentList(row);
    });

    connect(m_sideList, &QListWidget::itemEntered, this, [this](QListWidgetItem* item) {
        if (!item || !m_sideList) return;
        int row = m_sideList->row(item);
        for (int i = 0; i < m_listDeleteButtons.size(); ++i) {
            if (QPushButton* b = m_listDeleteButtons[i]) {
                b->setVisible(i == row);
            }
        }
    });

    if (QWidget* vp = m_sideList->viewport()) {
        struct SideListHoverFilter : QObject {
            QVector<QPushButton*>* buttons = nullptr;
            bool eventFilter(QObject* o, QEvent* e) override {
                if (!buttons) return QObject::eventFilter(o, e);
                if (e->type() == QEvent::Leave) {
                    for (QPushButton* b : *buttons) {
                        if (b) b->setVisible(false);
                    }
                }
                return QObject::eventFilter(o, e);
            }
        };
        auto* f = new SideListHoverFilter();
        f->buttons = &m_listDeleteButtons;
        f->setParent(vp);
        vp->installEventFilter(f);
    }
    sideLayout->addWidget(m_sideList, 1);

    connect(addListButton, &QPushButton::clicked, this, [this]() {
        NoteDocument nl;
        int idx = m_notes.size() + 1;
        nl.name = tr("New Note %1").arg(idx);
        nl.id = nl.name.toLower().replace(' ', '-');
        m_notes.append(nl);
        m_currentListIndex = m_notes.size() - 1;
        saveNotes();
        refreshSidebarLists();
        if (m_currentListIndex >= 0 && m_currentListIndex < m_notes.size()) {
            setCurrentList(m_currentListIndex);
        }
    });

    m_sidebarProfileWidget = new QWidget(this);
    m_sidebarProfileWidget->setObjectName("sidebarProfile");
    auto* profileLayout = new QHBoxLayout(m_sidebarProfileWidget);
    profileLayout->setContentsMargins(12, 10, 12, 10);
    profileLayout->setSpacing(12);

    const int avatarSize = 40;
    QLabel* avatar = new QLabel(this);
    avatar->setObjectName("sidebarProfileImg");
    avatar->setFixedSize(avatarSize, avatarSize);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setScaledContents(false);
    QString facePath = QDir::homePath() + QStringLiteral("/.face.icon");
    if (QFile::exists(facePath)) {
        QPixmap face(facePath);
        if (!face.isNull()) {
            QPixmap rounded(avatarSize, avatarSize);
            rounded.fill(Qt::transparent);
            QPainter p(&rounded);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            QPainterPath path;
            path.addEllipse(0, 0, avatarSize, avatarSize);
            p.setClipPath(path);
            p.drawPixmap(0, 0, avatarSize, avatarSize, face.scaled(avatarSize, avatarSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            p.end();
            avatar->setPixmap(rounded);
        }
    }
    profileLayout->addWidget(avatar);
    m_sidebarProfileName = new QLabel(tr("User"), this);
    m_sidebarProfileName->setObjectName("sidebarProfileName");
    m_sidebarProfileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    profileLayout->addWidget(m_sidebarProfileName);
    sideLayout->addWidget(m_sidebarProfileWidget);
    loadPearIdDisplayName();
    struct ProfileClickFilter : QObject {
        MainWindow* mw = nullptr;
        bool eventFilter(QObject* o, QEvent* e) override {
            if (e->type() == QEvent::MouseButtonRelease && mw) {
                Q_UNUSED(o);
                Q_UNUSED(e);
                return true;
            }
            return QObject::eventFilter(o, e);
        }
    };
    auto* filter = new ProfileClickFilter();
    filter->mw = this;
    filter->setParent(m_sidebarProfileWidget);
    m_sidebarProfileWidget->installEventFilter(filter);
}

void MainWindow::loadPearIdDisplayName() {
    if (!m_sidebarProfileName) return;

    QString scriptPath;
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        appDir.absoluteFilePath(QStringLiteral("../pearID/get_user_info.sh")),
        appDir.absoluteFilePath(QStringLiteral("pearID/get_user_info.sh")),
        QStringLiteral("/usr/share/pearos-appstore/pearID/get_user_info.sh"),
        QStringLiteral("/usr/share/pearos-notes/pearID/get_user_info.sh")
    };
    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            scriptPath = path;
            break;
        }
    }
    if (scriptPath.isEmpty()) return;

    QProcess* process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        if (exitCode != 0 || exitStatus != QProcess::NormalExit || !m_sidebarProfileName) return;
        const QByteArray out = process->readAllStandardOutput().trimmed();
        const QList<QByteArray> lines = out.split('\n');
        QString first = QString::fromUtf8(lines.value(0).trimmed());
        QString last = QString::fromUtf8(lines.value(1).trimmed());
        if (!first.isEmpty() || !last.isEmpty()) {
            m_sidebarProfileName->setText((first + QChar(' ') + last).trimmed());
        }
    });
    process->start(scriptPath,
                   {QStringLiteral("--first-name"), QStringLiteral("--last-name")},
                   QProcess::ReadOnly);
}

void MainWindow::setupContent() {
    if (!m_contentLayout) return;

    m_notesWidget = new NotesWidget(this);
    m_contentLayout->addWidget(m_notesWidget, 1);

    loadNotes();
    refreshSidebarLists();

    connect(m_notesWidget, &NotesWidget::contentChanged, this, [this](const QString& html) {
        // Obținem textul simplu din HTML (ignorând stiluri/css) pentru titlu / logică
        QTextDocument doc;
        doc.setHtml(html);
        QString plain = doc.toPlainText();

        // Împărțim după orice spațiu (spațiu, newline etc.)
        QStringList words = plain.simplified().split(QRegularExpression(QStringLiteral("\\s+")),
                                                     Qt::SkipEmptyParts);

        // Dacă ultimul caracter nu e spațiu/newline, considerăm ultimul cuvânt "în curs de tastare"
        if (!plain.isEmpty() && !plain.at(plain.size() - 1).isSpace() && !words.isEmpty()) {
            words.removeLast();
        }

        const bool hasExistingNote =
            !m_notes.isEmpty() &&
            m_currentListIndex >= 0 &&
            m_currentListIndex < m_notes.size();

        // Dacă nu avem încă nicio notă:
        // - creăm una nouă doar când avem cel puțin un cuvânt complet (urmat de spațiu/Enter)
        if (!hasExistingNote) {
            if (words.isEmpty()) {
                // Text gol sau doar "cuvânt în curs" -> nu creăm încă o notă
                return;
            }

            const int count = qMin(4, words.size());
            QStringList firstWords;
            for (int i = 0; i < count; ++i)
                firstWords.append(words.at(i));
            QString title = firstWords.join(QLatin1Char(' '));

            NoteDocument note;
            note.name = title;
            note.id = title.toLower().replace(QLatin1Char(' '), QLatin1Char('-'));
            note.content = html;

            m_notes.append(note);
            m_currentListIndex = m_notes.size() - 1;
            saveNotes();
            refreshSidebarLists();
            // Nu chemăm setList aici ca să nu resetăm textul și poziția cursorului
            return;
        }

        // Avem deja o notă selectată; actualizăm conținutul
        // inclusiv cazul când textul este complet șters (gol).
        NoteDocument& note = m_notes[m_currentListIndex];
        note.content = html;
        saveNotes();
    });

    if (!m_notes.isEmpty()) {
        setCurrentList(0);
    }
}

void MainWindow::loadNotes() {
    m_notes = NotesStorage::load();
    if (m_notes.isEmpty()) {
        m_currentListIndex = -1;
    } else if (m_currentListIndex < 0 || m_currentListIndex >= m_notes.size()) {
        m_currentListIndex = 0;
    }
}

void MainWindow::saveNotes() const {
    // Salvează întotdeauna starea actuală, chiar dacă lista e goală,
    // ca să persiste ștergerea ultimei notițe.
    if (!NotesStorage::save(m_notes)) {
        Logger::warning(QStringLiteral("Failed to save notes"));
    } else {
        Logger::info(QStringLiteral("Notes saved from MainWindow::saveNotes"));
    }
}

void MainWindow::refreshSidebarLists() {
    if (!m_sideList) return;
    QSignalBlocker blocker(m_sideList);
    m_sideList->clear();
    m_listDeleteButtons.clear();

    const QPalette pal = qApp->palette();
    const QColor accent = pal.color(QPalette::Highlight);
    const QColor text = QColor(QStringLiteral("#fafafa"));

    for (int i = 0; i < m_notes.size(); ++i) {
        const NoteDocument& list = m_notes.at(i);
        auto* item = new QListWidgetItem(m_sideList);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        m_sideList->addItem(item);

        QWidget* row = new QWidget(m_sideList);
        row->setMinimumHeight(32);
        auto* layout = new QHBoxLayout(row);
        layout->setContentsMargins(8, 6, 8, 6);
        layout->setSpacing(10);

        const bool selected = (i == m_currentListIndex);

        // Icon
        auto* iconLabel = new QLabel(row);
        iconLabel->setFixedSize(20, 20);
        iconLabel->setAlignment(Qt::AlignCenter);
        const QString iconPath = iconPathForListName(list.name);
        QPixmap base(iconPath);
        QColor iconColor = selected ? accent : text;
        iconLabel->setPixmap(tintPixmap(base, iconColor).scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        layout->addWidget(iconLabel, 0);

        // Text
        auto* label = new QLabel(list.name, row);
        label->setStyleSheet(QStringLiteral("color: %1;").arg(selected ? accent.name() : text.name()));
        layout->addWidget(label, 1);

        // Delete button (in fixed-size container so layout does not shift on hover)
        auto* deleteContainer = new QWidget(row);
        deleteContainer->setFixedSize(20, 20);
        auto* deleteLayout = new QHBoxLayout(deleteContainer);
        deleteLayout->setContentsMargins(0, 0, 0, 0);
        deleteLayout->setSpacing(0);

        auto* deleteBtn = new QPushButton(deleteContainer);
        deleteBtn->setFlat(true);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setFixedSize(20, 20);
        QIcon delIcon(QStringLiteral(":/icons/delete.svg"));
        QPixmap px = delIcon.pixmap(16, 16);
        if (!px.isNull()) {
            QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
            const QColor color(0xF9, 0x73, 0x73); // soft red
            const int a = color.alpha();
            const int r = color.red();
            const int g = color.green();
            const int b = color.blue();
            for (int y = 0; y < img.height(); ++y) {
                QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
                for (int x = 0; x < img.width(); ++x) {
                    const int alpha = qAlpha(line[x]);
                    line[x] = qRgba(r, g, b, (alpha * a) / 255);
                }
            }
            deleteBtn->setIcon(QPixmap::fromImage(img));
            deleteBtn->setIconSize(QSize(16, 16));
        }
        deleteBtn->setStyleSheet("QPushButton { background: transparent; border: none; }"
                                 "QPushButton:hover { background-color: rgba(248, 113, 113, 0.16); border-radius: 10px; }");
        deleteBtn->setVisible(false);
        deleteLayout->addWidget(deleteBtn);
        layout->addWidget(deleteContainer, 0);

        m_sideList->setItemWidget(item, row);
        // Asigură înălțime suficientă pentru text + icon (evită tăierea)
        QSize rowHint = row->sizeHint();
        if (rowHint.height() < 40)
            rowHint.setHeight(40);
        item->setSizeHint(rowHint);
        m_listDeleteButtons.append(deleteBtn);

        connect(deleteBtn, &QPushButton::clicked, this, [this, i]() {
            if (i < 0 || i >= m_notes.size()) return;
            m_notes.removeAt(i);
            saveNotes();
            if (m_notes.isEmpty()) {
                m_currentListIndex = -1;
                if (m_notesWidget) {
                    NoteDocument empty;
                    empty.name = tr("Empty");
                    m_notesWidget->setList(empty);
                }
            } else if (m_currentListIndex >= m_notes.size()) {
                m_currentListIndex = m_notes.size() - 1;
            }
            refreshSidebarLists();
            if (m_currentListIndex >= 0 && m_currentListIndex < m_notes.size()) {
                setCurrentList(m_currentListIndex);
            }
        });
    }

    if (!m_notes.isEmpty()) {
        const int idx = (m_currentListIndex >= 0 && m_currentListIndex < m_notes.size())
                        ? m_currentListIndex
                        : 0;
        QSignalBlocker blocker2(m_sideList);
        m_sideList->setCurrentRow(idx);
    }
}

void MainWindow::setCurrentList(int index) {
    if (index < 0 || index >= m_notes.size()) {
        m_currentListIndex = -1;
        return;
    }
    m_currentListIndex = index;
    if (m_notesWidget) {
        m_notesWidget->setList(m_notes.at(index));
    }
    // Rebuild sidebar rows so icon + text reflect accent color on selection
    refreshSidebarLists();
    if (m_sideList && m_sideList->currentRow() != index) {
        QSignalBlocker blocker3(m_sideList);
        m_sideList->setCurrentRow(index);
    }
}

void MainWindow::updateSidebarReflection() {
    if (!m_rightColumn || !m_sidebarReflection || !m_leftSideWidget) return;
    if (!m_rightColumn->isVisible() || m_rightColumn->width() < 20 || m_rightColumn->height() < 20) return;

    const int displayWidth = 100;
    const int grabWidth = 72;
    const int h = m_rightColumn->height();
    QPixmap strip = m_rightColumn->grab(QRect(0, 0, grabWidth, h));
    if (strip.isNull()) return;

    const int w = m_leftSideWidget->width();
    m_sidebarReflection->setGeometry(w - displayWidth, 0, displayWidth, m_leftSideWidget->height());
    m_sidebarReflection->setPixmap(strip.scaled(displayWidth, h, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    m_sidebarReflection->lower();
}

void MainWindow::createMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");
    auto* quitAction = new QAction("&Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(quitAction);

    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About pearOS Notes"),
            tr("pearOS Notes\n\n"
               "A simple notes application for pearOS\n"
               "Built with Qt6 and C++17."));
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(":/stylesheet.qss");
    if (!styleFile.exists()) styleFile.setFileName("stylesheet.qss");
    if (!styleFile.exists()) styleFile.setFileName("/usr/share/pearos-appstore/stylesheet.qss");

    QString base;
    if (styleFile.open(QFile::ReadOnly)) {
        base = QString::fromUtf8(styleFile.readAll());
        styleFile.close();
        Logger::info(QString("Stylesheet loaded from: %1").arg(styleFile.fileName()));
    } else {
        Logger::warning("Could not load stylesheet");
    }
    base += "\n/* Theme from system palette */\n" + themeOverlayFromPalette();
    qApp->setStyleSheet(base);
}

QString MainWindow::themeOverlayFromPalette() const {
    const QPalette p = qApp->palette();
    const QString accent   = p.color(QPalette::Highlight).name();
    const QString accentText = p.color(QPalette::HighlightedText).name();

    // Culori: bg #1e2123, search #393a3b, selected sidebar #303132, carduri #393b3e
    const QString bg       = QStringLiteral("#1e2123");
    const QString surface  = QStringLiteral("#27272a");
    const QString cardBg   = QStringLiteral("#393b3e");
    const QString border   = QStringLiteral("#52525b");
    const QString text     = QStringLiteral("#fafafa");
    const QString textMuted= QStringLiteral("#a1a1aa");
    const QString placeholder = QStringLiteral("#71717a");
    const QString btnBg    = QStringLiteral("#27272a");
    const QString btnHover = QStringLiteral("#3f3f46");
    const QString searchBoxBg = QStringLiteral("#393a3b");
    const QString sidebarSelectedBg = QStringLiteral("#303132");

    return QStringLiteral(
        "QWidget#appContainer { background-color: %1; }\n"
        "QWidget#rightColumn { background-color: %1; }\n"
        "QFrame#leftSide { background-color: %1; }\n"
        "QWidget#titleBarDrag { background-color: %1; border: none; }\n"
        "QLineEdit#sidebarSearch { background-color: %12; color: %5; selection-background-color: %8; border: none; border-radius: 16px; padding: 0 14px; min-height: 32px; }\n"
        "QLineEdit#sidebarSearch::placeholder { color: %7; }\n"
        "QListWidget#sideList::item { color: %5; }\n"
        "QListWidget#sideList::item:hover { background-color: transparent; }\n"
        "QListWidget#sideList::item:selected { background-color: %13; color: %8; font-weight: 500; }\n"
        "QLabel#sidebarProfileImg { background-color: %3; border: 1px solid %4; border-radius: 20px; }\n"
        "QLabel#sidebarProfileName { color: %5; }\n"
        "QWidget#sidebarProfile[selected=\"true\"] { background-color: %13; border-radius: 10px; padding: 10px 12px; min-height: 24px; }\n"
        "QWidget#sidebarProfile[selected=\"true\"] QLabel#sidebarProfileName { color: %8; font-weight: 500; }\n"
        "QWidget#contentWrapper, QStackedWidget#mainContainer { background-color: %1; }\n"
        "QWidget { color: %5; }\n"
        "QLabel { color: %5; }\n"
        "QPushButton { background-color: %9; color: %5; border: none; }\n"
        "QPushButton:hover { background-color: %10; color: %5; }\n"
        "QPushButton:disabled { background-color: %4; color: %7; }\n"
        "QPushButton#discoverDownloadButton { background-color: #ffffff; color: %8; border: none; border-radius: 14px; min-width: 58px; max-width: 58px; min-height: 28px; max-height: 28px; font-size: 11px; font-weight: 700; padding: 0 6px; }\n"
        "QPushButton#discoverDownloadButton:hover:!disabled { background-color: #e5e7eb; color: %8; }\n"
        "QPushButton#discoverDownloadButton:disabled { background-color: #ffffff; color: %7; }\n"
        "QLineEdit { background-color: %2; color: %5; border: 1px solid %4; selection-background-color: %8; }\n"
        "QLineEdit::placeholder { color: %7; }\n"
        "QComboBox { background-color: %2; color: %5; border: 1px solid %4; }\n"
        "QComboBox QAbstractItemView { background-color: %2; color: %5; selection-background-color: %3; }\n"
        "QWidget[class=\"package-card\"] { background-color: %3; border-color: %1; }\n"
        "QWidget[class=\"package-card\"]:hover, QWidget[class=\"package-card\"][hovered=\"true\"] { background-color: %3; border-color: %4; }\n"
        "QLabel[class=\"status-badge\"] { background-color: %8; color: %11; }\n"
        "QLabel[class=\"repo-badge\"] { background-color: %4; color: %6; }\n"
        "QGroupBox { background-color: %3; border-color: %1; }\n"
        "QGroupBox::title { color: %7; }\n"
        "QMenuBar { background-color: %1; color: %5; }\n"
        "QMenuBar::item:selected { background-color: %3; }\n"
        "QMenu { background-color: %2; color: %5; border: 1px solid %4; }\n"
        "QMenu::item:selected { background-color: %3; }\n"
        "QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background-color: %4; }\n"
        "QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { background-color: #3a3d4a; }\n"
        "QMessageBox QPushButton { background-color: %9; color: %5; }\n"
        "QMessageBox QPushButton:hover { background-color: %10; }\n"
        "QProgressBar::chunk { background-color: %8; }\n"
        "QCheckBox::indicator:checked { background-color: %8; border-color: %8; }\n"
    ).arg(bg, surface, cardBg, border, text, textMuted, placeholder, accent, btnBg, btnHover, accentText, searchBoxBg, sidebarSelectedBg);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange) {
        loadStyleSheet();
        return false;
    }
    if (watched == m_headerWidget && event->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            QWindow* win = windowHandle();
            if (win && win->startSystemMove()) {
                return true;
            }
        }
    }
    if (watched == m_centralWidget) {
        if (event->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(event);
            Qt::Edges edges = resizeEdgesAt(me->pos());
            if (edges != Qt::Edges()) {
                if ((edges & Qt::LeftEdge) && (edges & Qt::TopEdge)) {
                    m_centralWidget->setCursor(Qt::SizeFDiagCursor);
                } else if ((edges & Qt::RightEdge) && (edges & Qt::TopEdge)) {
                    m_centralWidget->setCursor(Qt::SizeBDiagCursor);
                } else if ((edges & Qt::LeftEdge) && (edges & Qt::BottomEdge)) {
                    m_centralWidget->setCursor(Qt::SizeBDiagCursor);
                } else if ((edges & Qt::RightEdge) && (edges & Qt::BottomEdge)) {
                    m_centralWidget->setCursor(Qt::SizeFDiagCursor);
                } else if ((edges & Qt::LeftEdge) || (edges & Qt::RightEdge)) {
                    m_centralWidget->setCursor(Qt::SizeHorCursor);
                } else {
                    m_centralWidget->setCursor(Qt::SizeVerCursor);
                }
                return false;
            }
            m_centralWidget->unsetCursor();
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                Qt::Edges edges = resizeEdgesAt(me->pos());
                if (edges != Qt::Edges()) {
                    QWindow* win = windowHandle();
                    if (win && win->startSystemResize(edges)) {
                        return true;
                    }
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

Qt::Edges MainWindow::resizeEdgesAt(const QPoint& pos) const {
    if (!m_centralWidget) return Qt::Edges();
    const QRect r = m_centralWidget->rect();
    const int m = RESIZE_MARGIN;
    Qt::Edges edges;
    if (pos.x() <= r.x() + m) edges |= Qt::LeftEdge;
    if (pos.x() >= r.right() - m) edges |= Qt::RightEdge;
    if (pos.y() <= r.y() + m) edges |= Qt::TopEdge;
    if (pos.y() >= r.bottom() - m) edges |= Qt::BottomEdge;
    return edges;
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    static bool blurDone = false;
    if (!blurDone) {
        blurDone = true;
        enableBlurBehind();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (m_reflectionDebounceTimer) {
        m_reflectionDebounceTimer->start(150);
    }
}

void MainWindow::enableBlurBehind() {
    QWindow* win = windowHandle();
    if (!win) return;
#ifdef HAVE_KWINDOW_EFFECTS
    if (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind)) {
        KWindowEffects::enableBlurBehind(win, true);
        Logger::info("Blur behind enabled (KWindowEffects)");
    }
#else
    Q_UNUSED(win);
#endif
}
