/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "appmenuapplet.h"
#include "appmenumodel.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QXmlStreamReader>

int AppMenuApplet::s_refs = 0;
namespace
{
QString viewService()
{
    return QStringLiteral("org.kde.kappmenuview");
}
}

AppMenuApplet::AppMenuApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
    ++s_refs;
    // if we're the first, register the service
    if (s_refs == 1) {
        QDBusConnection::sessionBus().interface()->registerService(viewService(),
                                                                   QDBusConnectionInterface::QueueService,
                                                                   QDBusConnectionInterface::DontAllowReplacement);
    }
    /*it registers or unregisters the service when the destroyed value of the applet change,
      and not in the dtor, because:
      when we "delete" an applet, it just hides it for about a minute setting its status
      to destroyed, in order to be able to do a clean undo: if we undo, there will be
      another destroyedchanged and destroyed will be false.
      When this happens, if we are the only appmenu applet existing, the dbus interface
      will have to be registered again*/
    connect(this, &Applet::destroyedChanged, this, [](bool destroyed) {
        if (destroyed) {
            // if we were the last, unregister
            if (--s_refs == 0) {
                QDBusConnection::sessionBus().interface()->unregisterService(viewService());
            }
        } else {
            // if we're the first, register the service
            if (++s_refs == 1) {
                QDBusConnection::sessionBus().interface()->registerService(viewService(),
                                                                           QDBusConnectionInterface::QueueService,
                                                                           QDBusConnectionInterface::DontAllowReplacement);
            }
        }
    });
}

AppMenuApplet::~AppMenuApplet() = default;

void AppMenuApplet::init()
{
}

QAbstractItemModel *AppMenuApplet::model() const
{
    return m_model;
}

void AppMenuApplet::setModel(QAbstractItemModel *model)
{
    if (m_model != model) {
        m_model = model;
        Q_EMIT modelChanged();
    }
}

int AppMenuApplet::view() const
{
    return m_viewType;
}

void AppMenuApplet::setView(int type)
{
    if (m_viewType != type) {
        m_viewType = type;
        Q_EMIT viewChanged();
    }
}

int AppMenuApplet::currentIndex() const
{
    return m_currentIndex;
}

void AppMenuApplet::setCurrentIndex(int currentIndex)
{
    if (m_currentIndex != currentIndex) {
        m_currentIndex = currentIndex;
        Q_EMIT currentIndexChanged();
    }
}

QQuickItem *AppMenuApplet::buttonGrid() const
{
    return m_buttonGrid;
}

void AppMenuApplet::setButtonGrid(QQuickItem *buttonGrid)
{
    if (m_buttonGrid != buttonGrid) {
        m_buttonGrid = buttonGrid;
        Q_EMIT buttonGridChanged();
    }
}

QMenu *AppMenuApplet::createMenu(int idx) const
{
    QMenu *menu = nullptr;

    if (view() == CompactView) {
        if (auto *menuAction = m_model->data(QModelIndex(), AppMenuModel::ActionRole).value<QAction *>()) {
            menu = menuAction->menu();
        }
    } else if (view() == FullView) {
        const QModelIndex index = m_model->index(idx, 0);
        if (auto *action = m_model->data(index, AppMenuModel::ActionRole).value<QAction *>()) {
            menu = action->menu();
        }
    }

    return menu;
}

void AppMenuApplet::onMenuAboutToHide()
{
    auto menuAction = m_currentMenu->menuAction();
    menuAction->setMenu(m_sourceMenu);
    setCurrentIndex(-1);
}

Qt::Edges edgeFromLocation(Plasma::Types::Location location)
{
    switch (location) {
    case Plasma::Types::TopEdge:
        return Qt::TopEdge;
    case Plasma::Types::BottomEdge:
        return Qt::BottomEdge;
    case Plasma::Types::LeftEdge:
        return Qt::LeftEdge;
    case Plasma::Types::RightEdge:
        return Qt::RightEdge;
    case Plasma::Types::Floating:
    case Plasma::Types::Desktop:
    case Plasma::Types::FullScreen:
        break;
    }
    return {};
}

void AppMenuApplet::trigger(QQuickItem *ctx, int idx)
{
    if (m_currentIndex == idx) {
        return;
    }

    if (!ctx || !ctx->window() || !ctx->window()->screen()) {
        return;
    }

    QMenu *actionMenu = createMenu(idx);
    if (actionMenu) {
        // this is a workaround where Qt will fail to realize a mouse has been released
        // this happens if a window which does not accept focus spawns a new window that takes focus and X grab
        // whilst the mouse is depressed
        // https://bugreports.qt.io/browse/QTBUG-59044
        // this causes the next click to go missing

        // by releasing manually we avoid that situation
        auto ungrabMouseHack = [ctx]() {
            if (ctx && ctx->window() && ctx->window()->mouseGrabberItem()) {
                // FIXME event forge thing enters press and hold move mode :/
                ctx->window()->mouseGrabberItem()->ungrabMouse();
            }
        };

        if (view() == FullView) {
            if (!m_currentMenu) {
                m_currentMenu = new QMenu(qobject_cast<QWidget *>(actionMenu->parent()));
                connect(m_currentMenu, &QMenu::aboutToHide, this, &AppMenuApplet::onMenuAboutToHide, Qt::UniqueConnection);
            } else if (m_sourceMenu != actionMenu) {
                auto menuAction = m_currentMenu->menuAction();
                for (QAction *action : m_currentMenu->actions()) {
                    m_currentMenu->removeAction(action);
                    m_sourceMenu->addAction(action);
                }
                menuAction->setMenu(m_sourceMenu);
            }
            m_sourceMenu = actionMenu;
            auto menuAction = m_sourceMenu->menuAction();
            for (QAction *action : m_sourceMenu->actions()) {
                m_sourceMenu->removeAction(action);
                m_currentMenu->addAction(action);
            }
            menuAction->setMenu(m_currentMenu);
        } else {
            m_currentMenu = actionMenu;
            m_sourceMenu = actionMenu;
        }

        QTimer::singleShot(0, ctx, ungrabMouseHack);
        // end workaround

        const auto &geo = ctx->window()->screen()->availableVirtualGeometry();

        QPoint pos = ctx->window()->mapToGlobal(ctx->mapToScene(QPointF()).toPoint());

        const Qt::Edges edges = edgeFromLocation(location());
        m_currentMenu->setProperty("_breeze_menu_seamless_edges", QVariant::fromValue(edges));

        if (location() == Plasma::Types::TopEdge) {
            pos.setY(pos.y() + ctx->height());
        }

        m_currentMenu->adjustSize();

        pos = QPoint(qBound(geo.x(), pos.x(), geo.x() + geo.width() - m_currentMenu->width()),
                     qBound(geo.y(), pos.y(), geo.y() + geo.height() - m_currentMenu->height()));

        if (view() == FullView) {
            if (m_currentMenu->isVisible()) {
                m_currentMenu->move(pos);
            } else {
                m_currentMenu->installEventFilter(this);
                m_currentMenu->winId(); // create window handle
                m_currentMenu->windowHandle()->setTransientParent(ctx->window());
                m_currentMenu->popup(pos);
            }
        } else if (view() == CompactView) {
            if (m_currentMenu->isEmpty()) {
                // don't try to popup an empty menu in case the app gives us one
                return;
            }
            m_currentMenu->popup(pos);
            connect(actionMenu, &QMenu::aboutToHide, this, &AppMenuApplet::onMenuAboutToHide, Qt::UniqueConnection);
        }

        setCurrentIndex(idx);

        // FIXME TODO connect only once
    } else { // is it just an action without a menu?
        if (auto *action = m_model->index(idx, 0).data(AppMenuModel::ActionRole).value<QAction *>()) {
            Q_ASSERT(!action->menu());
            action->trigger();
        }
    }
}

// FIXME TODO doesn't work on submenu
bool AppMenuApplet::eventFilter(QObject *watched, QEvent *event)
{
    auto *menu = qobject_cast<QMenu *>(watched);
    if (!menu) {
        return false;
    }

    if (event->type() == QEvent::KeyPress) {
        auto *e = static_cast<QKeyEvent *>(event);

        // TODO right to left languages
        if (e->key() == Qt::Key_Left) {
            int desiredIndex = m_currentIndex - 1;
            Q_EMIT requestActivateIndex(desiredIndex);
            return true;
        } else if (e->key() == Qt::Key_Right) {
            if (menu->activeAction() && menu->activeAction()->menu()) {
                return false;
            }

            int desiredIndex = m_currentIndex + 1;
            Q_EMIT requestActivateIndex(desiredIndex);
            return true;
        }

    } else if (event->type() == QEvent::MouseMove) {
        auto *e = static_cast<QMouseEvent *>(event);

        if (!m_buttonGrid || !m_buttonGrid->window()) {
            return false;
        }

        // FIXME the panel margin breaks Fitt's law :(
        const QPointF &windowLocalPos = m_buttonGrid->window()->mapFromGlobal(e->globalPosition());
        const QPointF &buttonGridLocalPos = m_buttonGrid->mapFromScene(windowLocalPos);
        auto *item = m_buttonGrid->childAt(buttonGridLocalPos.x(), buttonGridLocalPos.y());
        if (!item) {
            return false;
        }

        bool ok;
        const int buttonIndex = item->property("buttonIndex").toInt(&ok);
        if (!ok) {
            return false;
        }

        Q_EMIT requestActivateIndex(buttonIndex);
    }

    return false;
}

QList<QPair<QString, QString>> AppMenuApplet::loadRecentFolders() const
{
    QList<QPair<QString, QString>> result;
    const QString xbelPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
        + QStringLiteral("/.local/share/recently-used.xbel");
    QFile file(xbelPath);
    if (!file.open(QIODevice::ReadOnly))
        return result;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && result.size() < 10) {
        if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == QLatin1String("bookmark")) {
            const QString href = xml.attributes().value(QLatin1String("href")).toString();
            if (href.startsWith(QLatin1String("file://"))) {
                const QString path = QUrl(href).toLocalFile();
                if (QFileInfo::exists(path)) {
                    QString name = QFileInfo(path).fileName();
                    if (name.isEmpty())
                        name = path;
                    result.append({name, path});
                }
            }
        }
    }
    return result;
}

void AppMenuApplet::showBasicMenu(const QString &menuId, QQuickItem *ctx, const QVariantMap &state)
{
    if (!ctx || !ctx->window() || !ctx->window()->screen())
        return;

    auto *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);

    auto addAction = [&](const QString &text, const QString &id,
                         const QString &iconName = {}, bool enabled = true,
                         bool checkable = false, bool checked = false) -> QAction * {
        QAction *a = menu->addAction(text);
        if (!iconName.isEmpty())
            a->setIcon(QIcon::fromTheme(iconName));
        a->setEnabled(enabled);
        if (checkable) {
            a->setCheckable(true);
            a->setChecked(checked);
        }
        connect(a, &QAction::triggered, this, [this, id]() { Q_EMIT basicMenuAction(id); });
        return a;
    };

    if (menuId == QLatin1String("File")) {
        addAction(tr("New Finder Window"), QStringLiteral("file.newWindow"), QStringLiteral("window-new"));
        addAction(tr("New Folder"), QStringLiteral("file.newFolder"), QStringLiteral("folder-new"));

    } else if (menuId == QLatin1String("Edit")) {
        addAction(tr("Undo\t⌘Z"), QStringLiteral("edit.undo"));
        addAction(tr("Redo\t⇧⌘Z"), QStringLiteral("edit.redo"));
        menu->addSeparator();
        addAction(tr("Cut\t⌘X"), QStringLiteral("edit.cut"), QStringLiteral("edit-cut"));
        addAction(tr("Copy\t⌘C"), QStringLiteral("edit.copy"), QStringLiteral("edit-copy"));
        addAction(tr("Paste\t⌘V"), QStringLiteral("edit.paste"), QStringLiteral("edit-paste"));
        addAction(tr("Select All\t⌘A"), QStringLiteral("edit.selectAll"));

    } else if (menuId == QLatin1String("View")) {
        addAction(tr("Show Toolbar"), QStringLiteral("view.toolbar"), QStringLiteral("view-visible"), true, true, true);
        addAction(tr("Show Sidebar"), QStringLiteral("view.sidebar"), QStringLiteral("sidebar-show-symbolic"), true, true, true);
        addAction(tr("Show Hidden Files"), QStringLiteral("view.hidden"), QStringLiteral("show-hidden-symbolic"), true, true, false);
        menu->addSeparator();
        addAction(tr("Zoom In\t⌘+"), QStringLiteral("view.zoomIn"), QStringLiteral("zoom-in"));
        addAction(tr("Zoom Out\t⌘-"), QStringLiteral("view.zoomOut"), QStringLiteral("zoom-out"));
        addAction(tr("Actual Size\t⌘0"), QStringLiteral("view.actualSize"), QStringLiteral("zoom-fit-best"));
        menu->addSeparator();
        addAction(tr("Refresh\t⌘R"), QStringLiteral("view.refresh"), QStringLiteral("view-refresh"));

    } else if (menuId == QLatin1String("Go")) {
        addAction(tr("Back\t⌘["), QStringLiteral("go.back"));
        addAction(tr("Forward\t⌘]"), QStringLiteral("go.forward"));
        menu->addSeparator();
        addAction(tr("Recents\t⇧⌘F"), QStringLiteral("go.recents"));
        addAction(tr("Documents\t⇧⌘O"), QStringLiteral("go.documents"));
        addAction(tr("Desktop\t⇧⌘D"), QStringLiteral("go.desktop"));
        addAction(tr("Downloads\t⌥⌘L"), QStringLiteral("go.downloads"));
        addAction(tr("Home\t⇧⌘H"), QStringLiteral("go.home"));
        addAction(tr("Computer\t⇧⌘C"), QStringLiteral("go.computer"));
        addAction(tr("Applications\t⇧⌘A"), QStringLiteral("go.applications"));
        menu->addSeparator();
        QMenu *recentMenu = menu->addMenu(tr("Recent Folders"));
        const auto recentFolders = loadRecentFolders();
        if (recentFolders.isEmpty()) {
            recentMenu->addAction(tr("No recent folders"))->setEnabled(false);
        } else {
            for (const auto &[name, path] : recentFolders) {
                QAction *a = recentMenu->addAction(name);
                connect(a, &QAction::triggered, this, [this, path]() {
                    Q_EMIT basicMenuAction(QStringLiteral("go.recentFolder:") + path);
                });
            }
        }
        menu->addSeparator();
        addAction(tr("Go to Folder…\t⇧⌘G"), QStringLiteral("go.gotoFolder"));

    } else if (menuId == QLatin1String("Window")) {
        const bool hasWindow   = state.value(QStringLiteral("hasWindow"),   false).toBool();
        const bool isMin       = state.value(QStringLiteral("isMin"),       false).toBool();
        const bool isMax       = state.value(QStringLiteral("isMax"),       false).toBool();
        const bool isMaximized = state.value(QStringLiteral("isMaximized"), false).toBool();
        const bool isClose     = state.value(QStringLiteral("isClose"),     false).toBool();
        addAction(tr("Minimize\t⌘M"), QStringLiteral("window.minimize"),
                  QStringLiteral("window-minimize"), hasWindow && isMin);
        {
            QAction *a = menu->addAction(tr("Zoom\t⌥⌘Z"));
            a->setIcon(QIcon::fromTheme(QStringLiteral("window-maximize")));
            a->setEnabled(hasWindow && isMax);
            a->setCheckable(true);
            a->setChecked(isMaximized);
            connect(a, &QAction::triggered, this, [this]() { Q_EMIT basicMenuAction(QStringLiteral("window.maximize")); });
        }
        menu->addSeparator();
        addAction(tr("Close Window\t⌘W"), QStringLiteral("window.close"),
                  QStringLiteral("window-close"), hasWindow && isClose);
        menu->addSeparator();
        addAction(tr("Bring All to Front"), QStringLiteral("window.bringAll"), QStringLiteral("view-restore"));

    } else if (menuId == QLatin1String("Help")) {
        addAction(tr("Donate"),            QStringLiteral("help.donate"));
        addAction(tr("Join pearOS Discord"), QStringLiteral("help.discord"), QStringLiteral("irc-join-channel"));
        addAction(tr("Get NordVPN"),       QStringLiteral("help.nordvpn"));
        addAction(tr("Get NordPass"),      QStringLiteral("help.nordpass"));
        addAction(tr("Join Reddit"),       QStringLiteral("help.reddit"));
        addAction(tr("YouTube"),           QStringLiteral("help.youtube"));
        addAction(tr("Instagram"),         QStringLiteral("help.instagram"));
        addAction(tr("pearOS Website"),    QStringLiteral("help.website"));
        addAction(tr("pearID"),            QStringLiteral("help.pearid"));
        addAction(tr("GitHub"),            QStringLiteral("help.github"));

    } else {
        menu->deleteLater();
        return;
    }

    const Qt::Edges edges = edgeFromLocation(location());
    menu->setProperty("_breeze_menu_seamless_edges", QVariant::fromValue(edges));

    const auto &geo = ctx->window()->screen()->availableVirtualGeometry();
    QPoint pos = ctx->window()->mapToGlobal(ctx->mapToScene(QPointF()).toPoint());
    if (location() == Plasma::Types::TopEdge)
        pos.setY(pos.y() + ctx->height());

    menu->adjustSize();
    pos = QPoint(qBound(geo.x(), pos.x(), geo.x() + geo.width()  - menu->width()),
                 qBound(geo.y(), pos.y(), geo.y() + geo.height() - menu->height()));

    connect(menu, &QMenu::aboutToShow, this, [this, menuId]() { Q_EMIT basicMenuShown(menuId); });
    connect(menu, &QMenu::aboutToHide, this, [this]()           { Q_EMIT basicMenuHidden(); });

    menu->winId();
    menu->windowHandle()->setTransientParent(ctx->window());
    menu->popup(pos);
}

K_PLUGIN_CLASS_WITH_JSON(AppMenuApplet, "metadata.json")

#include "appmenuapplet.moc"
#include "moc_appmenuapplet.cpp"
