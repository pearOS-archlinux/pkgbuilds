// KWin script: print each window caption (for Wayland window title check)
try {
    for (const w of workspace.windowList()) {
        if (w.caption) print(w.caption);
    }
} catch (e) {
    if (typeof workspace.clientList === 'function') {
        const clients = workspace.clientList();
        for (let i = 0; i < clients.length; i++) {
            if (clients[i].caption) print(clients[i].caption);
        }
    }
}
