/*
 * Stand-in for the `Plasmoid` attached property that org.kde.plasma.plasmoid's
 * PlasmoidItem normally provides -- but backed by a plain QQuickView/QWindow
 * host instead of a real Plasma::Applet hosted by a Plasma::Containment/Corona.
 * Shared by filer-dock (PearDock, bottom-edge) and filer-topbar (top-bar
 * plasmoids, top-edge) -- both host plasmoid QML directly via a plain
 * QQuickView, with none of libplasma's C++ Applet/Containment/Corona/
 * PluginLoader hosting machinery linked in at all.
 *
 * Scope is deliberately narrow: only the Plasmoid.* members actually read by
 * the plasmoids hosted by either project are implemented (see each project's
 * README for the audit that produced this file's property list). Anything not
 * listed here was verified unused.
 */

#ifndef PLASMOIDSHIM_H
#define PLASMOIDSHIM_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QQmlListProperty>
#include <QQmlPropertyMap>
#include <QRect>
#include <QRegion>
#include <QSettings>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

// Backs `Plasmoid.configuration.<key>` -- QQmlPropertyMap exposes arbitrary
// dynamic properties to QML by name, both readable and writable
// (Plasmoid.configuration.iconSize = newSize, used by main.qml's drag-to-resize
// handle, works the same way it would against a real KConfigGroup-backed
// configuration map). Persisted to a plain ini file via QSettings instead of
// KConfig -- this class owns config storage outright now, so the KConfigLoader
// snapshot-timing race that main.cpp used to work around (see the old
// Plasmoid.configuration.fill comment, removed) can't happen: there is only
// ever one copy of this data.
class PlasmoidConfig : public QQmlPropertyMap
{
    Q_OBJECT
public:
    explicit PlasmoidConfig(const QString &settingsFileName, QObject *parent = nullptr);

    // Seeds a key with `defaultValue` if QSettings has no saved value for it
    // yet. Call once per key at startup, mirroring main.xml's <default>s.
    void seed(const QString &key, const QVariant &defaultValue);

    // Live-syncs a handful of visual-tuning knobs from a shared settings.conf
    // ([Dock]/[TopBar]-style group), so editing that one file Filer already
    // reloads live updates the hosted plasmoid too, without needing a
    // filer-dock/filer-topbar restart. Watches the file via QFileSystemWatcher
    // and re-reads the group on every change; call once at startup.
    void watchSharedSettings(const QString &path, const QString &group);

    // Plain `Plasmoid.configuration.someKey = [a, b, c]` from QML does NOT
    // reliably set a list-valued dynamic property: QQmlPropertyMap has no
    // declared type for an arbitrary key to coerce the assignment against,
    // so the JS array gets converted via its default Array.prototype
    // .toString() (a bare comma-joined string, no way back to separate
    // elements) before it ever reaches save() below. Call this instead for
    // any key that holds a list (pinnedFiles' main.qml call sites are the
    // motivating case) -- it inserts a genuine QStringList-typed QVariant
    // and still runs the normal save()/shared-settings-sync path.
    Q_INVOKABLE void setStringList(const QString &key, const QStringList &value);

private slots:
    // Also fires on plain local changes (drag-resize, a future settings
    // UI, ...); only writes through to m_sharedSettings for keys that
    // reloadSharedSettings() populated in the first place (m_sharedKeys),
    // so e.g. iconSize (plasmoid-local-config-only) doesn't leak into the
    // shared settings file's group.
    void save(const QString &key, const QVariant &value);
    void reloadSharedSettings();

private:
    QSettings m_settings;
    QSettings *m_sharedSettings = nullptr;
    QFileSystemWatcher m_sharedSettingsWatcher;
    QString m_sharedGroup;
    QStringList m_sharedKeys;
    bool m_reloadingSharedSettings = false;
};

// Backs `Plasmoid.containment.corona` -- the small subset of Plasma::Corona's
// API that hosted popups (pearmenu's dropdown, PearControlCentre's popup)
// call through `Plasmoid.containment.corona.*` to avoid rendering under the
// bar/dock's own reserved screen-edge strip. One shared instance per process
// (all plasmoids in one host share the same screen/reserved-height state).
class PlasmoidCoronaShim : public QObject
{
    Q_OBJECT
public:
    explicit PlasmoidCoronaShim(QObject *parent = nullptr);

    // In px, the strip reserved at whichever screen edge the host anchors to
    // (topbar's height / dock's height+margin) that popups must not render
    // under. Set once at startup from main.cpp.
    void setReservedEdgeHeight(int height);

    Q_INVOKABLE QRect screenGeometry(int screenId = 0) const;
    Q_INVOKABLE QRect availableScreenRect(int screenId = 0) const;
    Q_INVOKABLE QRegion availableScreenRegion(int screenId = 0) const;
    Q_INVOKABLE int screenForContainment(QObject *containment) const;

private:
    int m_reservedEdgeHeight = 0;
};

// Backs `Plasmoid.containment` -- only the subset hosted plasmoids actually
// call: `screenGeometry` (used to center popups and scope task lists to a
// screen), `internalAction()` (looked up for a "configure" context-menu item
// that already null-guards its absence), and `corona` (see above).
class PlasmoidContainmentShim : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)
    Q_PROPERTY(QObject *corona READ corona CONSTANT)
public:
    explicit PlasmoidContainmentShim(PlasmoidCoronaShim *corona, QObject *parent = nullptr);

    QRect screenGeometry() const;
    QObject *corona() const { return m_corona; }

    Q_INVOKABLE QVariant internalAction(const QString &name) const;

signals:
    void screenGeometryChanged();

private:
    PlasmoidCoronaShim *m_corona;
};

// Backs `Plasmoid` itself.
class PlasmoidShim : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PlasmoidConfig *configuration READ configuration CONSTANT)
    Q_PROPERTY(PlasmoidContainmentShim *containment READ containment CONSTANT)
    // NOTIFY (never emitted for location/formFactor -- fixed for the process's
    // lifetime) instead of CONSTANT purely so `Connections { onLocationChanged }`
    // in QML resolves to a real signal instead of warning about a missing one.
    Q_PROPERTY(int location READ location NOTIFY locationChanged)
    Q_PROPERTY(int formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(int immutability READ immutability CONSTANT)
    Q_PROPERTY(QString pluginName READ pluginName CONSTANT)
    Q_PROPERTY(int status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(bool userConfiguring READ userConfiguring WRITE setUserConfiguring NOTIFY userConfiguringChanged)
    Q_PROPERTY(int backgroundHints READ backgroundHints WRITE setBackgroundHints NOTIFY backgroundHintsChanged)
    Q_PROPERTY(int constraintHints READ constraintHints WRITE setConstraintHints NOTIFY constraintHintsChanged)
    Q_PROPERTY(int containmentDisplayHints READ containmentDisplayHints CONSTANT)
    Q_PROPERTY(QVariant icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool expanded READ expanded WRITE setExpanded NOTIFY expandedChanged)
    Q_PROPERTY(QVariant compactRepresentation READ compactRepresentation WRITE setCompactRepresentation NOTIFY compactRepresentationChanged)
    Q_PROPERTY(QVariant fullRepresentation READ fullRepresentation WRITE setFullRepresentation NOTIFY fullRepresentationChanged)
    Q_PROPERTY(QVariant preferredRepresentation READ preferredRepresentation WRITE setPreferredRepresentation NOTIFY preferredRepresentationChanged)
    Q_PROPERTY(QVariantMap metaData READ metaData CONSTANT)
    // QQmlListProperty<QObject>, not QVariant/QList<QObject*>/QVariantList --
    // all three of those rejected the `Plasmoid.contextualActions:
    // [Action{...}, ...]` list-literal syntax some plasmoids use ("Cannot
    // assign multiple values to a singular property"). QQmlListProperty is
    // the actual mechanism QML recognizes for a `prop: [a, b, c]` literal to
    // append each element one at a time, for both ordinary and attached
    // properties -- read-only (no WRITE) is correct here, QML calls the
    // append callback per element instead of assigning the property outright.
    Q_PROPERTY(QQmlListProperty<QObject> contextualActions READ contextualActions NOTIFY contextualActionsChanged)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString globalShortcut READ globalShortcut WRITE setGlobalShortcut NOTIFY globalShortcutChanged)
    // Constraint-hint enum constants (Plasma::Applet::ConstraintHint), exposed
    // as CONSTANT properties so QML reads `Plasmoid.CanFillArea`/`Plasmoid.NoHint`
    // exactly like the real attached property did.
    Q_PROPERTY(int NoHint READ noHint CONSTANT)
    Q_PROPERTY(int CanFillArea READ canFillArea CONSTANT)
public:
    // location/formFactor: plain Plasma::Types::Location/FormFactor integer
    // values (see filer-shell-common/README.md for where these were sourced
    // from -- /usr/include/Plasma/plasma/plasma.h -- so no #include <Plasma/Plasma>
    // is needed just to get a stable enum value). Fixed for this shim
    // instance's lifetime (the host never migrates screens/edges at runtime).
    explicit PlasmoidShim(const QString &pluginId, int location, int formFactor,
                          PlasmoidCoronaShim *corona, QObject *parent = nullptr);

    PlasmoidConfig *configuration() const { return m_configuration; }
    PlasmoidContainmentShim *containment() const { return m_containment; }

    int location() const { return m_location; }
    int formFactor() const { return m_formFactor; }
    int immutability() const { return m_immutability; }
    QString pluginName() const { return m_pluginId; }

    int status() const { return m_status; }
    void setStatus(int status);

    bool userConfiguring() const { return m_userConfiguring; }
    void setUserConfiguring(bool configuring);

    int backgroundHints() const { return m_backgroundHints; }
    void setBackgroundHints(int hints);

    int constraintHints() const { return m_constraintHints; }
    void setConstraintHints(int hints);

    int containmentDisplayHints() const { return 0; }

    QVariant icon() const { return m_icon; }
    void setIcon(const QVariant &icon);

    QString title() const { return m_title; }
    void setTitle(const QString &title);

    bool expanded() const { return m_expanded; }
    void setExpanded(bool expanded);

    QVariant compactRepresentation() const { return m_compactRepresentation; }
    void setCompactRepresentation(const QVariant &component);
    QVariant fullRepresentation() const { return m_fullRepresentation; }
    void setFullRepresentation(const QVariant &component);
    QVariant preferredRepresentation() const { return m_preferredRepresentation; }
    void setPreferredRepresentation(const QVariant &component);

    QVariantMap metaData() const { return m_metaData; }
    void setMetaData(const QVariantMap &metaData) { m_metaData = metaData; }

    QQmlListProperty<QObject> contextualActions();

private:
    static void contextualActionsAppend(QQmlListProperty<QObject> *prop, QObject *obj);
    static qsizetype contextualActionsCount(QQmlListProperty<QObject> *prop);
    static QObject *contextualActionsAt(QQmlListProperty<QObject> *prop, qsizetype index);
    static void contextualActionsClear(QQmlListProperty<QObject> *prop);

public:

    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }

    // Backs the appmenu plasmoid's non-standard `Plasmoid.model = appMenuModel`
    // assignment (mirrors AppMenuApplet's one custom Q_PROPERTY) and the
    // systemtray plasmoid's flat tray-icon model, generically as a QObject*
    // wrapped in QVariant so any QAbstractItemModel-derived class works.
    QVariant model() const { return m_model; }
    void setModel(const QVariant &model);

    // KGlobalAccel-backed: setting a non-empty shortcut string registers it
    // via KGlobalAccel::self()->setShortcut() against a QAction owned by this
    // shim, whose triggered() re-emits activated() below -- this is what
    // actually makes pearmenu's ForceQuit hotkey (Meta+Alt+Escape) work,
    // unlike the old shim which declared activated() but never fired it.
    QString globalShortcut() const { return m_globalShortcut; }
    void setGlobalShortcut(const QString &shortcut);

    static constexpr int noHint() { return 0; } // Plasma::Applet::NoHint
    static constexpr int canFillArea() { return 1; } // Plasma::Applet::CanFillArea

    // Called like a function from QML (ContextMenu.qml); a Q_SIGNAL can be
    // invoked that way -- QML just emits it. No C++ side currently listens.
    Q_INVOKABLE void contextualActionsAboutToShow() { emit contextualActionsAboutToShowSignal(); }

    // Same null-safe pattern as PlasmoidContainmentShim::internalAction().
    Q_INVOKABLE QVariant internalAction(const QString &name) const;

signals:
    void statusChanged();
    void userConfiguringChanged();
    void backgroundHintsChanged();
    void constraintHintsChanged();
    void contextualActionsAboutToShowSignal();
    void locationChanged();
    void formFactorChanged();
    void iconChanged();
    void titleChanged();
    void expandedChanged();
    void compactRepresentationChanged();
    void fullRepresentationChanged();
    void preferredRepresentationChanged();
    void contextualActionsChanged();
    void modelChanged();
    void globalShortcutChanged();
    // Fired for real now (via the KGlobalAccel QAction's triggered() signal)
    // once a non-empty globalShortcut is set -- see setGlobalShortcut().
    void activated();

private:
    QString m_pluginId;
    PlasmoidConfig *m_configuration;
    PlasmoidContainmentShim *m_containment;
    int m_location;
    int m_formFactor;
    int m_immutability = 1; // Plasma::Types::Mutable
    int m_status = 2; // Plasma::Types::ActiveStatus
    bool m_userConfiguring = false;
    int m_backgroundHints = 1; // Plasma::Types::StandardBackground-ish default ("has a background")
    int m_constraintHints = 0; // Plasma::Applet::NoHint
    QVariant m_icon;
    QString m_title;
    bool m_expanded = false;
    QVariant m_compactRepresentation;
    QVariant m_fullRepresentation;
    QVariant m_preferredRepresentation;
    QVariantMap m_metaData;
    QList<QObject *> m_contextualActions;
    QString m_id;
    QVariant m_model;
    QString m_globalShortcut;
    QObject *m_globalShortcutAction = nullptr;
};

#endif // PLASMOIDSHIM_H
