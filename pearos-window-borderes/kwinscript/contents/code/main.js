// Companion script for the pearOS window decoration.
//
// It does the two things the decoration cannot do on its own: it knows the
// process id of every window, and it can move windows other than its own.
// Records are sent as one newline separated string, because callDBus cannot
// marshal a string list.

var UNIT_SEPARATOR = String.fromCharCode(31);

var SERVICE = "org.kde.pearos.WindowDecoration";
var PATH = "/Busy";

function isNormal(w) {
    return w && w.normalWindow && !w.deleted;
}

// --- process reporting ----------------------------------------------------

function publish() {
    var records = [];
    var windows = workspace.windowList();

    for (var i = 0; i < windows.length; ++i) {
        var w = windows[i];
        if (!isNormal(w) || !w.pid || w.pid <= 0) {
            continue;
        }
        records.push(w.caption + UNIT_SEPARATOR + w.pid);
    }

    callDBus(SERVICE, PATH, SERVICE, "setWindowProcesses", records.join("\n"));
}

function watch(window) {
    if (window && window.captionChanged) {
        window.captionChanged.connect(publish);
    }
}

workspace.windowAdded.connect(function (window) {
    watch(window);
    publish();
});

workspace.windowRemoved.connect(publish);

var existing = workspace.windowList();
for (var i = 0; i < existing.length; ++i) {
    watch(existing[i]);
}

publish();

// --- arranging several windows at once -------------------------------------

// Windows that can take part in an arrangement: the active one first, then the
// most recently used ones, all on the screen of the active window.
function arrangeable() {
    var active = workspace.activeWindow;
    if (!isNormal(active)) {
        return [];
    }

    var result = [active];
    var windows = workspace.stackingOrder;

    for (var i = windows.length - 1; i >= 0; --i) {
        var w = windows[i];
        if (w === active || !isNormal(w) || w.minimized) {
            continue;
        }
        if (w.output !== active.output) {
            continue;
        }
        result.push(w);
    }

    return result;
}

function place(window, x, y, width, height) {
    window.setMaximize(false, false);
    window.fullScreen = false;
    window.frameGeometry = { x: Math.round(x), y: Math.round(y),
                             width: Math.round(width), height: Math.round(height) };
}

function arrange(count) {
    var windows = arrangeable();
    if (windows.length < count) {
        return;
    }

    var area = workspace.clientArea(KWin.MaximizeArea, windows[0]);
    var halfWidth = area.width / 2;
    var halfHeight = area.height / 2;

    if (count === 2) {
        place(windows[0], area.x, area.y, halfWidth, area.height);
        place(windows[1], area.x + halfWidth, area.y, halfWidth, area.height);
    } else if (count === 3) {
        place(windows[0], area.x, area.y, halfWidth, area.height);
        place(windows[1], area.x + halfWidth, area.y, halfWidth, halfHeight);
        place(windows[2], area.x + halfWidth, area.y + halfHeight, halfWidth, halfHeight);
    } else if (count === 4) {
        place(windows[0], area.x, area.y, halfWidth, halfHeight);
        place(windows[1], area.x + halfWidth, area.y, halfWidth, halfHeight);
        place(windows[2], area.x, area.y + halfHeight, halfWidth, halfHeight);
        place(windows[3], area.x + halfWidth, area.y + halfHeight, halfWidth, halfHeight);
    }
}

// The decoration triggers these by name through kglobalaccel; they are left
// without a key binding on purpose.
registerShortcut("pearOS Arrange Two", "Arrange two windows side by side", "",
                 function () { arrange(2); });
registerShortcut("pearOS Arrange Three", "Arrange three windows", "",
                 function () { arrange(3); });
registerShortcut("pearOS Arrange Four", "Arrange four windows in a grid", "",
                 function () { arrange(4); });
