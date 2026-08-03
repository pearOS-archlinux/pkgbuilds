#!/usr/bin/env python3
"""hyprvisor-gui — dark GTK front-end for the hyprvisor CLI.

Detects the current GPU, lets you pick nouveau / open / proprietary,
shows the exact packages that choice installs, and installs them.
Also has a "Simulate" window: type any GPU model name and see what
hyprvisor would recommend for it, with no real hardware needed.

Requires: python-gobject (GTK 3), the hyprvisor binary (built or in PATH).
"""

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gdk, GLib, Pango

import json
import os
import re
import shutil
import subprocess
import threading

_ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


def strip_ansi(text):
    return _ANSI_RE.sub("", text)

# ── locate the hyprvisor binary ────────────────────────────────────────────

def find_binary():
    p = shutil.which("hyprvisor")
    if p:
        return p
    local = os.path.join(os.path.dirname(os.path.abspath(__file__)), "/", "usr", "bin", "hyprvisor")
    local = os.path.abspath(local)
    if os.path.isfile(local):
        return local
    return None

HYPRVISOR_BIN = find_binary()

AUR_HELPERS = ["paru", "yay", "trizen"]
TERMINALS = ["x-terminal-emulator", "kitty", "alacritty", "foot",
             "gnome-terminal", "konsole", "xfce4-terminal", "xterm"]

BUCKET_LABELS = {
    "nouveau":     "Nouveau — 100% free, no blobs",
    "open":        "Open kernel module (nvidia-open)",
    "proprietary": "Proprietary (closed-source)",
}
BUCKET_ORDER = ["nouveau", "open", "proprietary"]

# A curated reference list for the Simulate window's browsable list.
SAMPLE_GPUS = [
    "NVIDIA RTX 5090", "NVIDIA RTX 4070", "NVIDIA RTX 3060",
    "NVIDIA RTX 2060", "NVIDIA GTX 1660 Super", "NVIDIA GTX 1050 Ti",
    "NVIDIA GTX 1080", "NVIDIA GTX 970", "NVIDIA GTX 960",
    "NVIDIA GTX 750 Ti", "NVIDIA GTX 680", "NVIDIA GTX 580",
    "NVIDIA GTX 460", "NVIDIA GeForce 9800 GT", "NVIDIA GeForce 8600 GT",
    "AMD RX 9070 XT", "AMD RX 7800 XT", "AMD RX 6600",
    "AMD RX 5700 XT", "AMD RX Vega 64", "AMD RX 580",
    "AMD R9 390", "AMD HD 7970", "AMD HD 6870", "AMD Radeon X800",
    "Intel Arc B580", "Intel Arc A770", "Intel Iris Xe Graphics",
    "Intel UHD Graphics 630", "Intel HD Graphics 4600",
    "S3 Chrome 530", "S3 Savage 4", "VIA Chrome 9",
]


def run_hyprvisor_json(args):
    """Run hyprvisor with the given args + --json, return parsed dict or raise."""
    if not HYPRVISOR_BIN:
        raise FileNotFoundError(
            "hyprvisor binary not found (build it first: cmake --build build/)")
    proc = subprocess.run([HYPRVISOR_BIN] + args + ["--json"],
                           capture_output=True, text=True, timeout=20)
    out = proc.stdout.strip()
    if not out:
        raise RuntimeError(proc.stderr.strip() or "hyprvisor produced no output")
    return json.loads(out)


def bucket_drivers(gpu):
    """Split a gpu's flat driver list into nouveau / open / proprietary buckets."""
    drivers = gpu.get("drivers", [])
    is_nvidia = gpu.get("vendorName") == "NVIDIA"

    if not is_nvidia:
        return {"open": {"label": "Recommended packages", "items": drivers}}

    buckets = {"nouveau": [], "open": [], "proprietary": []}
    has_open_kernel = any(d["type"] == "open-kernel" for d in drivers)

    headers = [d for d in drivers if d["type"] == "kernel-headers"]
    for d in drivers:
        if d["type"] == "kernel-headers":
            continue  # attached below, to whichever bucket(s) actually need it
        pkg = d["package"]
        if "nouveau" in pkg or pkg == "mesa":
            buckets["nouveau"].append(d)
        if d["type"] == "open-kernel" or (has_open_kernel and pkg == "nvidia-utils"):
            buckets["open"].append(d)
        if d["type"] in ("proprietary", "legacy", "aur-legacy"):
            buckets["proprietary"].append(d)

    for h in headers:
        for key, items in buckets.items():
            if any("dkms" in item["package"] for item in items):
                items.append(h)

    result = {}
    for key in BUCKET_ORDER:
        if buckets[key]:
            result[key] = {"label": BUCKET_LABELS[key], "items": buckets[key]}
    return result


def recommended_bucket(buckets):
    for key, b in buckets.items():
        if any(d["recommended"] for d in b["items"]):
            return key
    return next(iter(buckets), None)


def which_first(names):
    for n in names:
        p = shutil.which(n)
        if p:
            return p
    return None


# ── reusable package table + install panel ─────────────────────────────────

class DriverPanel(Gtk.Box):
    """Shows a GPU name, a nouveau/open/proprietary combo, its package table,
    and (optionally) an Install button with a log pane."""

    def __init__(self, allow_install=True):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        self.set_border_width(12)
        self.allow_install = allow_install
        self.buckets = {}
        self.gpu = None

        self.gpu_label = Gtk.Label(xalign=0)
        self.gpu_label.set_markup("<span size='large'><b>No GPU detected yet</b></span>")
        self.gpu_label.set_line_wrap(True)
        self.pack_start(self.gpu_label, False, False, 0)

        self.sub_label = Gtk.Label(xalign=0)
        self.sub_label.get_style_context().add_class("dim-label")
        self.pack_start(self.sub_label, False, False, 0)

        self.warn_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        self.pack_start(self.warn_box, False, False, 0)

        combo_row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        combo_row.pack_start(Gtk.Label(label="Driver:"), False, False, 0)
        self.combo = Gtk.ComboBoxText()
        self.combo.connect("changed", self.on_combo_changed)
        combo_row.pack_start(self.combo, True, True, 0)
        self.pack_start(combo_row, False, False, 0)

        self.store = Gtk.ListStore(str, str, str, str, str)
        self.tree = Gtk.TreeView(model=self.store)
        for i, title in enumerate(["Package", "Type", "AUR", "Status", "Description"]):
            renderer = Gtk.CellRendererText()
            if title == "Description":
                renderer.set_property("ellipsize", Pango.EllipsizeMode.END)
            col = Gtk.TreeViewColumn(title, renderer, text=i)
            col.set_resizable(True)
            if title == "Description":
                col.set_expand(True)
            self.tree.append_column(col)
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scroll.set_min_content_height(220)
        scroll.add(self.tree)
        self.pack_start(scroll, True, True, 0)

        if allow_install:
            btn_row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
            self.install_btn = Gtk.Button(label="Install")
            self.install_btn.get_style_context().add_class("suggested-action")
            self.install_btn.connect("clicked", self.on_install_clicked)
            btn_row.pack_start(self.install_btn, False, False, 0)
            self.status_label = Gtk.Label(xalign=0)
            btn_row.pack_start(self.status_label, True, True, 0)
            self.pack_start(btn_row, False, False, 0)

            self.log_buf = Gtk.TextBuffer()
            log_view = Gtk.TextView(buffer=self.log_buf)
            log_view.set_editable(False)
            log_view.set_monospace(True)
            log_scroll = Gtk.ScrolledWindow()
            log_scroll.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
            log_scroll.set_min_content_height(110)
            log_scroll.add(log_view)
            self.pack_start(log_scroll, False, False, 0)

    def set_gpu(self, gpu):
        self.gpu = gpu
        vm = gpu.get("_vmName", "")
        self.gpu_label.set_markup(f"<span size='large'><b>{GLib.markup_escape_text(gpu['name'])}</b></span>")
        sub = f"Vendor: {gpu.get('vendorName','?')}"
        if gpu.get("pciAddr"):
            sub += f"   PCI: {gpu['pciAddr']}   IDs: {gpu.get('vendorId','')}:{gpu.get('deviceId','')}"
        if vm:
            sub += f"   [inside {vm}]"
        self.sub_label.set_text(sub)

        self.refresh_warnings()

        self.buckets = bucket_drivers(gpu)
        rec = recommended_bucket(self.buckets)

        self.combo.handler_block_by_func(self.on_combo_changed)
        self.combo.remove_all()
        active_index = 0
        for i, key in enumerate(self.buckets):
            label = self.buckets[key]["label"]
            if key == rec:
                label += "  (Recommended)"
            self.combo.append(key, label)
            if key == rec:
                active_index = i
        self.combo.handler_unblock_by_func(self.on_combo_changed)
        self.combo.set_active(active_index)
        self.refresh_table()

    def on_combo_changed(self, _combo):
        self.refresh_table()

    def refresh_warnings(self):
        for child in list(self.warn_box.get_children()):
            self.warn_box.remove(child)
        if not self.gpu:
            return

        problems = []  # list of (label_text, [package, ...])
        for orphan in self.gpu.get("orphanedDrivers", []):
            problems.append((f"Unrelated driver installed: {orphan['package']} — {orphan['reason']}",
                              [orphan["package"]]))
        for d in self.gpu.get("drivers", []):
            if d.get("incompatible"):
                problems.append((f"Incompatible driver installed: {d['package']} — {d['description']}",
                                  [d["package"]]))
            if d.get("versionMismatch"):
                problems.append((f"Kernel/headers version mismatch: {d['package']} does not match "
                                  "the running kernel — DKMS build will fail.", []))

        if not problems:
            return

        for text, pkgs in problems:
            row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
            lbl = Gtk.Label(label="⚠ " + text, xalign=0)
            lbl.set_line_wrap(True)
            lbl.get_style_context().add_class("warn-label")
            row.pack_start(lbl, True, True, 0)
            if pkgs and self.allow_install:
                btn = Gtk.Button(label="Remove")
                btn.connect("clicked", lambda _b, p=pkgs: self.on_remove_packages(p))
                row.pack_start(btn, False, False, 0)
            self.warn_box.pack_start(row, False, False, 0)
        self.warn_box.show_all()

    def on_remove_packages(self, pkgs):
        dialog = Gtk.MessageDialog(
            transient_for=self.get_toplevel(), modal=True,
            message_type=Gtk.MessageType.QUESTION, buttons=Gtk.ButtonsType.YES_NO,
            text="Remove these package(s)?")
        dialog.format_secondary_text("sudo pacman -R " + " ".join(pkgs))
        resp = dialog.run()
        dialog.destroy()
        if resp != Gtk.ResponseType.YES:
            return
        threading.Thread(target=self._do_remove, args=(pkgs,), daemon=True).start()

    def _do_remove(self, pkgs):
        self.log("$ pkexec pacman -R --noconfirm " + " ".join(pkgs))
        try:
            proc = subprocess.Popen(
                ["pkexec", "pacman", "-R", "--noconfirm"] + pkgs,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            for line in proc.stdout:
                self.log(line.rstrip())
            proc.wait()
            self.log(f"[pacman exited with code {proc.returncode}]")
        except Exception as e:
            self.log(f"[error] {e}")
        GLib.idle_add(self._install_finished)

    def current_items(self):
        key = self.combo.get_active_id()
        if not key or key not in self.buckets:
            return []
        return self.buckets[key]["items"]

    def refresh_table(self):
        self.store.clear()
        items = self.current_items()
        for d in items:
            status = "installed" if d["installed"] else "not installed"
            aur = "AUR" if d["fromAUR"] else ""
            pkg = d["package"] + (" *" if d["recommended"] else "")
            self.store.append([pkg, d["type"], aur, status, d["description"]])
        if self.allow_install:
            missing = [d for d in items if not d["installed"]]
            if not items:
                self.status_label.set_text("")
                self.install_btn.set_sensitive(False)
            elif not missing:
                self.status_label.set_text("All packages already installed.")
                self.install_btn.set_sensitive(False)
            else:
                self.status_label.set_text(f"{len(missing)} package(s) to install.")
                self.install_btn.set_sensitive(True)

    def log(self, text):
        text = strip_ansi(text)
        def do():
            end = self.log_buf.get_end_iter()
            self.log_buf.insert(end, text + "\n")
        GLib.idle_add(do)

    def on_install_clicked(self, _btn):
        items = self.current_items()
        missing = [d for d in items if not d["installed"]]
        if not missing:
            return

        pacman_pkgs = [d["package"] for d in missing if not d["fromAUR"]]
        aur_pkgs = [d["package"] for d in missing if d["fromAUR"]]

        lines = ["The following packages will be installed:\n"]
        if pacman_pkgs:
            lines.append("pacman:  " + " ".join(pacman_pkgs))
        if aur_pkgs:
            lines.append("AUR:     " + " ".join(aur_pkgs) +
                          "\n(opens a terminal — needs your password + build confirmation)")
        dialog = Gtk.MessageDialog(
            transient_for=self.get_toplevel(), modal=True,
            message_type=Gtk.MessageType.QUESTION, buttons=Gtk.ButtonsType.YES_NO,
            text="Install driver packages?")
        dialog.format_secondary_text("\n".join(lines))
        resp = dialog.run()
        dialog.destroy()
        if resp != Gtk.ResponseType.YES:
            return

        self.install_btn.set_sensitive(False)
        threading.Thread(target=self._do_install, args=(pacman_pkgs, aur_pkgs), daemon=True).start()

    def _do_install(self, pacman_pkgs, aur_pkgs):
        if pacman_pkgs:
            self.log("$ pkexec pacman -S --needed --noconfirm " + " ".join(pacman_pkgs))
            try:
                proc = subprocess.Popen(
                    ["pkexec", "pacman", "-S", "--needed", "--noconfirm"] + pacman_pkgs,
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
                for line in proc.stdout:
                    self.log(line.rstrip())
                proc.wait()
                self.log(f"[pacman exited with code {proc.returncode}]")
            except Exception as e:
                self.log(f"[error] {e}")

        if aur_pkgs:
            helper = which_first(AUR_HELPERS)
            if not helper:
                self.log("[error] No AUR helper found (paru/yay/trizen). "
                          "Install one first, e.g.: git clone https://aur.archlinux.org/paru.git")
            else:
                term = which_first(TERMINALS)
                cmd = f"{helper} -S --needed {' '.join(aur_pkgs)}; echo; read -p 'Done — press Enter to close.'"
                if not term:
                    self.log("[error] No terminal emulator found to run the AUR helper interactively.")
                else:
                    self.log(f"Opening {os.path.basename(term)} to run: {helper} -S --needed {' '.join(aur_pkgs)}")
                    try:
                        if os.path.basename(term) in ("gnome-terminal", "xfce4-terminal"):
                            subprocess.Popen([term, "--", "bash", "-c", cmd])
                        else:
                            subprocess.Popen([term, "-e", "bash", "-c", cmd])
                    except Exception as e:
                        self.log(f"[error] {e}")

        GLib.idle_add(self._install_finished)

    def _install_finished(self):
        # Re-fetch install status from hyprvisor so the table/status reflect reality.
        if self.gpu and "_simulate" not in self.gpu:
            try:
                data = run_hyprvisor_json(["--list"])
                if data["gpus"]:
                    self.set_gpu(data["gpus"][0])
            except Exception as e:
                self.log(f"[error refreshing status] {e}")
        else:
            self.install_btn.set_sensitive(True)


# ── combined (nouveau + open + proprietary in one list) panel for Simulate ──

class CombinedDriverPanel(Gtk.Box):
    """Shows every compatible driver choice (nouveau/open/proprietary) for a
    GPU in a single list, recommended choice first, incompatible ones simply
    absent (no empty/greyed-out entries)."""

    def __init__(self):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        self.set_border_width(12)

        self.gpu_label = Gtk.Label(xalign=0)
        self.gpu_label.set_markup("<span size='large'><b>Type a GPU name and hit Check</b></span>")
        self.gpu_label.set_line_wrap(True)
        self.pack_start(self.gpu_label, False, False, 0)

        self.sub_label = Gtk.Label(xalign=0)
        self.sub_label.get_style_context().add_class("dim-label")
        self.pack_start(self.sub_label, False, False, 0)

        self.store = Gtk.ListStore(str, str, str, str, str, str)
        self.tree = Gtk.TreeView(model=self.store)
        for i, title in enumerate(["Driver choice", "Package", "Type", "AUR", "Status", "Description"]):
            renderer = Gtk.CellRendererText()
            if title == "Description":
                renderer.set_property("ellipsize", Pango.EllipsizeMode.END)
            col = Gtk.TreeViewColumn(title, renderer, text=i)
            col.set_resizable(True)
            if title == "Description":
                col.set_expand(True)
            self.tree.append_column(col)
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scroll.set_min_content_height(360)
        scroll.add(self.tree)
        self.pack_start(scroll, True, True, 0)

    def set_gpu(self, gpu):
        self.gpu_label.set_markup(f"<span size='large'><b>{GLib.markup_escape_text(gpu['name'])}</b></span>")
        self.sub_label.set_text(f"Vendor: {gpu.get('vendorName','?')}")

        buckets = bucket_drivers(gpu)
        rec = recommended_bucket(buckets)
        # recommended bucket's rows first, then whatever else is compatible;
        # anything not compatible for this GPU was already dropped by bucket_drivers.
        order = [k for k in [rec] if k] + [k for k in BUCKET_ORDER if k in buckets and k != rec]

        self.store.clear()
        for key in order:
            b = buckets[key]
            for d in b["items"]:
                if d["recommended"]:
                    tag = "  (recommended)"
                elif "NOT compatible" in d["description"]:
                    tag = "  (not compatible)"
                else:
                    tag = ""
                label = b["label"] + tag
                status = "installed" if d["installed"] else "not installed"
                aur = "AUR" if d["fromAUR"] else ""
                pkg = d["package"] + (" *" if d["recommended"] else "")
                self.store.append([label, pkg, d["type"], aur, status, d["description"]])


# ── main window ──────────────────────────────────────────────────────────

class MainWindow(Gtk.Window):
    def __init__(self):
        super().__init__(title="hyprvisor")
        self.set_default_size(760, 560)
        self.connect("destroy", Gtk.main_quit)

        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.add(box)

        header = Gtk.HeaderBar(title="hyprvisor", subtitle="GPU driver manager")
        header.set_show_close_button(True)
        refresh_btn = Gtk.Button.new_from_icon_name("view-refresh-symbolic", Gtk.IconSize.BUTTON)
        refresh_btn.set_tooltip_text("Re-detect GPU")
        refresh_btn.connect("clicked", lambda _b: self.load_gpu())
        header.pack_start(refresh_btn)
        sim_btn = Gtk.Button(label="Simulate…")
        sim_btn.connect("clicked", self.on_simulate)
        header.pack_end(sim_btn)
        clean_btn = Gtk.Button(label="Clean All…")
        clean_btn.get_style_context().add_class("destructive-action")
        clean_btn.set_tooltip_text("Remove every known GPU driver package (any vendor) for a fresh start")
        clean_btn.connect("clicked", self.on_clean)
        header.pack_end(clean_btn)
        self.set_titlebar(header)

        self.panel = DriverPanel(allow_install=True)
        box.pack_start(self.panel, True, True, 0)

        self.load_gpu()

    def load_gpu(self):
        try:
            data = run_hyprvisor_json(["--list"])
        except Exception as e:
            self.panel.gpu_label.set_markup(f"<span foreground='#ff6b6b'><b>Error: {GLib.markup_escape_text(str(e))}</b></span>")
            return
        if not data.get("gpus"):
            self.panel.gpu_label.set_markup("<span foreground='#ff6b6b'><b>No GPU detected</b></span>")
            return
        gpu = data["gpus"][0]
        gpu["_vmName"] = data.get("vmName", "")
        self.panel.set_gpu(gpu)

    def on_simulate(self, _btn):
        dlg = SimulateWindow(self)
        dlg.show_all()

    def on_clean(self, _btn):
        if not HYPRVISOR_BIN:
            return
        try:
            # Dry-run: --clean without --noconfirm lists what it would remove
            # and waits on a y/N prompt; feed "n" so nothing is touched, just
            # to read the real package list before asking our own dialog.
            proc = subprocess.run([HYPRVISOR_BIN, "--clean"], input="n\n",
                                   capture_output=True, text=True, timeout=20)
        except Exception as e:
            self.panel.log(f"[error] {e}")
            return

        out = strip_ansi(proc.stdout)
        pkgs = [line.strip()[2:].strip() for line in out.splitlines()
                if line.strip().startswith("- ")]
        if not pkgs:
            dialog = Gtk.MessageDialog(
                transient_for=self, modal=True, message_type=Gtk.MessageType.INFO,
                buttons=Gtk.ButtonsType.OK, text="Nothing to clean")
            dialog.format_secondary_text("No known GPU driver packages installed.")
            dialog.run()
            dialog.destroy()
            return

        dialog = Gtk.MessageDialog(
            transient_for=self, modal=True, message_type=Gtk.MessageType.WARNING,
            buttons=Gtk.ButtonsType.YES_NO,
            text="Remove ALL GPU driver packages (any vendor)?")
        dialog.format_secondary_text("\n".join(pkgs))
        resp = dialog.run()
        dialog.destroy()
        if resp != Gtk.ResponseType.YES:
            return

        threading.Thread(target=self._do_clean, daemon=True).start()

    def _do_clean(self):
        self.panel.log("$ hyprvisor --clean --noconfirm")
        try:
            proc = subprocess.Popen([HYPRVISOR_BIN, "--clean", "--noconfirm"],
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            for line in proc.stdout:
                self.panel.log(line.rstrip())
            proc.wait()
            self.panel.log(f"[hyprvisor exited with code {proc.returncode}]")
        except Exception as e:
            self.panel.log(f"[error] {e}")
        GLib.idle_add(self.load_gpu)


class SimulateWindow(Gtk.Window):
    def __init__(self, parent):
        super().__init__(title="Simulate — hyprvisor")
        self.set_default_size(920, 600)
        self.set_transient_for(parent)

        outer = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        self.add(outer)

        # Left: browsable reference list
        left = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        left.set_border_width(10)
        left.set_size_request(240, -1)
        left.pack_start(Gtk.Label(label="Common GPUs", xalign=0), False, False, 0)

        self.gpu_list = Gtk.ListBox()
        self.gpu_list.set_selection_mode(Gtk.SelectionMode.SINGLE)
        for name in SAMPLE_GPUS:
            row = Gtk.ListBoxRow()
            lbl = Gtk.Label(label=name, xalign=0)
            lbl.set_margin_top(3)
            lbl.set_margin_bottom(3)
            lbl.set_margin_start(4)
            row.add(lbl)
            self.gpu_list.add(row)
        self.gpu_list.connect("row-selected", self.on_row_selected)
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        scroll.add(self.gpu_list)
        left.pack_start(scroll, True, True, 0)
        outer.pack_start(left, False, False, 0)

        outer.pack_start(Gtk.Separator(orientation=Gtk.Orientation.VERTICAL), False, False, 0)

        # Right: search + results
        right = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        right.set_border_width(12)

        search_row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        self.entry = Gtk.SearchEntry()
        self.entry.set_placeholder_text("Type a GPU model, e.g. \"Gigabyte GTX 1050 Ti OC 4G\"")
        self.entry.connect("activate", self.on_check)
        search_row.pack_start(self.entry, True, True, 0)
        check_btn = Gtk.Button(label="Check")
        check_btn.get_style_context().add_class("suggested-action")
        check_btn.connect("clicked", self.on_check)
        search_row.pack_start(check_btn, False, False, 0)
        right.pack_start(search_row, False, False, 0)

        self.panel = CombinedDriverPanel()
        right.pack_start(self.panel, True, True, 0)
        outer.pack_start(right, True, True, 0)

    def on_row_selected(self, _list, row):
        if row is None:
            return
        text = row.get_child().get_text()
        self.entry.set_text(text)
        self.check_name(text)

    def on_check(self, _widget):
        self.check_name(self.entry.get_text().strip())

    def check_name(self, name):
        if not name:
            return
        try:
            data = run_hyprvisor_json(["--simulate", name])
        except Exception as e:
            self.panel.gpu_label.set_markup(f"<span foreground='#ff6b6b'><b>Error: {GLib.markup_escape_text(str(e))}</b></span>")
            return
        if not data.get("gpus"):
            return
        gpu = data["gpus"][0]
        gpu["_simulate"] = True
        self.panel.set_gpu(gpu)


DARK_CSS = b"""
window { background-color: #1e1f22; color: #e6e6e6; }
label { color: #e6e6e6; }
label.dim-label { color: #9a9a9a; }
treeview, textview text { background-color: #26272b; color: #e6e6e6; }
treeview:selected { background-color: #3a6ea5; }
headerbar { background-color: #26272b; color: #e6e6e6; }
entry, combobox button { background-color: #2c2d31; color: #e6e6e6; }
button.suggested-action { background-color: #3a6ea5; color: white; }
button.destructive-action { background-color: #a53a3a; color: white; }
label.warn-label { color: #ff6b6b; }
"""


def apply_dark_theme():
    settings = Gtk.Settings.get_default()
    if settings:
        settings.set_property("gtk-application-prefer-dark-theme", True)
    provider = Gtk.CssProvider()
    provider.load_from_data(DARK_CSS)
    Gtk.StyleContext.add_provider_for_screen(
        Gdk.Screen.get_default(), provider,
        Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)


def main():
    apply_dark_theme()
    win = MainWindow()
    win.show_all()
    Gtk.main()


if __name__ == "__main__":
    main()
