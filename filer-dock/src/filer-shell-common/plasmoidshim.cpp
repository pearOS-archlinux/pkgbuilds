#include "plasmoidshim.h"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QKeySequence>
#include <QScreen>
#include <QStandardPaths>

#include <KGlobalAccel>

PlasmoidConfig::PlasmoidConfig(const QString &settingsFileName, QObject *parent)
    : QQmlPropertyMap(this, parent)
    , m_settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                     + QLatin1Char('/') + settingsFileName,
                 QSettings::IniFormat)
{
    connect(this, &QQmlPropertyMap::valueChanged, this, &PlasmoidConfig::save);
}

void PlasmoidConfig::seed(const QString &key, const QVariant &defaultValue)
{
    const QVariant value = m_settings.value(key, defaultValue);
    // insert() (QQmlPropertyMap's own API) avoids re-triggering save() for
    // values that just came from QSettings itself.
    insert(key, value);
}

void PlasmoidConfig::setStringList(const QString &key, const QStringList &value)
{
    // insert() alone (like seed() uses) wouldn't run save() -- this is the
    // opposite case, an intentional QML-driven change that must still
    // persist, just via a QStringList-typed QVariant instead of whatever a
    // bare `= value` assignment would have produced.
    insert(key, QVariant(value));
    save(key, QVariant(value));
}

void PlasmoidConfig::save(const QString &key, const QVariant &value)
{
    // QML code that builds a new list as a plain JS array hands this a
    // generic QVariantList rather than a QStringList -- even though every
    // element is a string. QSettings' INI writer only recognizes
    // QVariant::StringList for its plain comma-separated text format;
    // anything else falls back to opaque `@Variant(...)` binary encoding,
    // which silently breaks persistence. Normalizing here covers every key
    // uniformly, regardless of which side produced the list.
    QVariant normalized = value;
    if (normalized.canConvert<QStringList>() && normalized.metaType() != QMetaType::fromType<QStringList>()) {
        normalized = normalized.toStringList();
    }

    m_settings.setValue(key, normalized);

    // Keeps the shared settings file as the two-way source of truth for the
    // keys it owns -- skip while we're the ones applying a reload from that
    // same file, or every external edit would immediately read right back as
    // a (no-op, but pointless) write.
    if (m_reloadingSharedSettings || !m_sharedSettings || !m_sharedKeys.contains(key)) {
        return;
    }
    QString sharedKey = key;
    if (!sharedKey.isEmpty()) {
        sharedKey[0] = sharedKey[0].toUpper();
    }
    m_sharedSettings->beginGroup(m_sharedGroup);
    m_sharedSettings->setValue(sharedKey, normalized);
    m_sharedSettings->endGroup();
}

void PlasmoidConfig::watchSharedSettings(const QString &path, const QString &group)
{
    m_sharedGroup = group;
    m_sharedSettings = new QSettings(path, QSettings::IniFormat, this);
    reloadSharedSettings();

    // QSettings doesn't watch its own file for external edits, so this is
    // what actually makes editing the shared settings file while the host is
    // running take effect without a restart.
    m_sharedSettingsWatcher.addPath(path);
    connect(&m_sharedSettingsWatcher, &QFileSystemWatcher::fileChanged, this, [this, path]() {
        reloadSharedSettings();
        // Editors commonly replace-then-rename on save, which drops the
        // path from the watch list; re-adding it if that happened is what
        // keeps this working across more than one edit.
        if (!m_sharedSettingsWatcher.files().contains(path)) {
            m_sharedSettingsWatcher.addPath(path);
        }
    });
}

void PlasmoidConfig::reloadSharedSettings()
{
    if (!m_sharedSettings) {
        return;
    }
    m_reloadingSharedSettings = true;
    m_sharedSettings->sync();
    m_sharedSettings->beginGroup(m_sharedGroup);
    const QStringList keys = m_sharedSettings->allKeys();
    for (const QString &key : keys) {
        // Matches this file's own camelCase QML property names
        // (BottomGap -> bottomGap) so QML can keep reading
        // Plasmoid.configuration.bottomGap unchanged.
        QString qmlKey = key;
        if (!qmlKey.isEmpty()) {
            qmlKey[0] = qmlKey[0].toLower();
        }
        if (!m_sharedKeys.contains(qmlKey)) {
            m_sharedKeys.append(qmlKey);
        }
        insert(qmlKey, m_sharedSettings->value(key));
    }
    m_sharedSettings->endGroup();
    m_reloadingSharedSettings = false;
}

PlasmoidCoronaShim::PlasmoidCoronaShim(QObject *parent)
    : QObject(parent)
{
}

void PlasmoidCoronaShim::setReservedEdgeHeight(int height)
{
    m_reservedEdgeHeight = height;
}

QRect PlasmoidCoronaShim::screenGeometry(int screenId) const
{
    Q_UNUSED(screenId);
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        return screen->geometry();
    }
    return QRect();
}

QRect PlasmoidCoronaShim::availableScreenRect(int screenId) const
{
    QRect geom = screenGeometry(screenId);
    // Matches ViewerCorona::availableScreenRect()'s original reasoning: the
    // reserved strip is always at this host's anchored edge (top for
    // filer-topbar, effectively unused/0 for filer-dock's bottom anchoring
    // since popups there already open upward above the dock itself).
    geom.setTop(geom.top() + m_reservedEdgeHeight);
    return geom;
}

QRegion PlasmoidCoronaShim::availableScreenRegion(int screenId) const
{
    return QRegion(availableScreenRect(screenId));
}

int PlasmoidCoronaShim::screenForContainment(QObject *containment) const
{
    Q_UNUSED(containment);
    return 0;
}

PlasmoidContainmentShim::PlasmoidContainmentShim(PlasmoidCoronaShim *corona, QObject *parent)
    : QObject(parent)
    , m_corona(corona)
{
    // The host is always anchored full-width to one screen and doesn't
    // migrate live, so there's currently nothing that changes this after
    // construction -- the signal exists only because some plasmoids'
    // `Connections { target: Plasmoid.containment; onScreenGeometryChanged }`
    // expect it to be connectable.
}

QRect PlasmoidContainmentShim::screenGeometry() const
{
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        return screen->geometry();
    }
    return QRect();
}

QVariant PlasmoidContainmentShim::internalAction(const QString &name) const
{
    Q_UNUSED(name);
    // No KCM-backed "configure" action exists outside a real Containment.
    // Some QML assigns this straight to a `property PlasmaCore.Action`
    // (a QAction*-typed QML property) -- a bare invalid QVariant() coerces to
    // `undefined`, which QML's property system rejects for object-pointer
    // properties ("Cannot assign [undefined] to QAction*"), aborting that
    // whole component's creation. A QVariant explicitly wrapping a null
    // QObject* assigns cleanly as `null`, which such menus already handle.
    return QVariant::fromValue<QObject *>(nullptr);
}

PlasmoidShim::PlasmoidShim(const QString &pluginId, int location, int formFactor,
                           PlasmoidCoronaShim *corona, QObject *parent)
    : QObject(parent)
    , m_pluginId(pluginId)
    , m_configuration(new PlasmoidConfig(pluginId + QStringLiteral("-config.ini"), this))
    , m_containment(new PlasmoidContainmentShim(corona, this))
    , m_location(location)
    , m_formFactor(formFactor)
    , m_id(pluginId)
{
}

void PlasmoidShim::setStatus(int status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;
    emit statusChanged();
}

void PlasmoidShim::setUserConfiguring(bool configuring)
{
    if (m_userConfiguring == configuring) {
        return;
    }
    m_userConfiguring = configuring;
    emit userConfiguringChanged();
}

void PlasmoidShim::setBackgroundHints(int hints)
{
    if (m_backgroundHints == hints) {
        return;
    }
    m_backgroundHints = hints;
    emit backgroundHintsChanged();
}

void PlasmoidShim::setConstraintHints(int hints)
{
    if (m_constraintHints == hints) {
        return;
    }
    m_constraintHints = hints;
    emit constraintHintsChanged();
}

void PlasmoidShim::setIcon(const QVariant &icon)
{
    if (m_icon == icon) {
        return;
    }
    m_icon = icon;
    emit iconChanged();
}

void PlasmoidShim::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    emit titleChanged();
}

void PlasmoidShim::setExpanded(bool expanded)
{
    if (m_expanded == expanded) {
        return;
    }
    m_expanded = expanded;
    emit expandedChanged();
}

void PlasmoidShim::setCompactRepresentation(const QVariant &component)
{
    m_compactRepresentation = component;
    emit compactRepresentationChanged();
}

void PlasmoidShim::setFullRepresentation(const QVariant &component)
{
    m_fullRepresentation = component;
    emit fullRepresentationChanged();
}

void PlasmoidShim::setPreferredRepresentation(const QVariant &component)
{
    m_preferredRepresentation = component;
    emit preferredRepresentationChanged();
}

void PlasmoidShim::contextualActionsAppend(QQmlListProperty<QObject> *prop, QObject *obj)
{
    auto *self = static_cast<PlasmoidShim *>(prop->object);
    self->m_contextualActions.append(obj);
    emit self->contextualActionsChanged();
}

qsizetype PlasmoidShim::contextualActionsCount(QQmlListProperty<QObject> *prop)
{
    return static_cast<PlasmoidShim *>(prop->object)->m_contextualActions.size();
}

QObject *PlasmoidShim::contextualActionsAt(QQmlListProperty<QObject> *prop, qsizetype index)
{
    return static_cast<PlasmoidShim *>(prop->object)->m_contextualActions.at(index);
}

void PlasmoidShim::contextualActionsClear(QQmlListProperty<QObject> *prop)
{
    auto *self = static_cast<PlasmoidShim *>(prop->object);
    self->m_contextualActions.clear();
    emit self->contextualActionsChanged();
}

QQmlListProperty<QObject> PlasmoidShim::contextualActions()
{
    return QQmlListProperty<QObject>(this, nullptr,
                                      &PlasmoidShim::contextualActionsAppend,
                                      &PlasmoidShim::contextualActionsCount,
                                      &PlasmoidShim::contextualActionsAt,
                                      &PlasmoidShim::contextualActionsClear);
}

void PlasmoidShim::setModel(const QVariant &model)
{
    m_model = model;
    emit modelChanged();
}

void PlasmoidShim::setGlobalShortcut(const QString &shortcut)
{
    if (m_globalShortcut == shortcut) {
        return;
    }
    m_globalShortcut = shortcut;

    if (!m_globalShortcutAction) {
        auto *action = new QAction(this);
        // KGlobalAccel keys shortcuts by (componentName, objectName) --
        // componentName defaults to QCoreApplication::applicationName(),
        // which differs between filer-dock and filer-topbar processes, so
        // pluginId alone is enough to keep this unique within each.
        action->setObjectName(m_pluginId + QStringLiteral("_globalShortcut"));
        action->setText(m_pluginId + QStringLiteral(" global shortcut"));
        connect(action, &QAction::triggered, this, &PlasmoidShim::activated);
        m_globalShortcutAction = action;
    }

    auto *action = qobject_cast<QAction *>(m_globalShortcutAction);
    const QKeySequence sequence(shortcut);
    // NoAutoloading is required to actually force this key sequence: with
    // the default Autoloading, KGlobalAccel looks up any existing saved
    // shortcut for (componentName, action) first and uses that instead --
    // for a brand-new action that's an empty sequence, which silently
    // dropped the one we just set (confirmed via `qdbus
    // org.kde.kglobalaccel /component/<name> allShortcutInfos`: the action
    // registered but with no active key combination).
    KGlobalAccel::self()->setDefaultShortcut(action, {sequence});
    KGlobalAccel::self()->setShortcut(action, {sequence}, KGlobalAccel::NoAutoloading);

    emit globalShortcutChanged();
}

QVariant PlasmoidShim::internalAction(const QString &name) const
{
    Q_UNUSED(name);
    // See PlasmoidContainmentShim::internalAction() -- same reasoning.
    return QVariant::fromValue<QObject *>(nullptr);
}
