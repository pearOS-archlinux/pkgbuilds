#!/usr/bin/env python3
import json
import os
import shutil
import subprocess
import tempfile
import tkinter as tk
from tkinter import ttk, messagebox, filedialog

# Paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CONFIG_LOCAL = os.path.join(SCRIPT_DIR, "config.json")
CONFIG_SYSTEM = "/usr/share/extras/pearos-themesw/config.json"

# Default config template
DEFAULT_CONFIG = {
    "light": {
        "colorScheme": "pearOS",
        "plasmaTheme": "pearOS",
        "iconTheme": "pearOS",
        "cursorTheme": "pearOS-light",
        "auroraeTheme": "__aurorae__svg__pearOS",
        "kvantumTheme": "pearOS",
        "gtkTheme": "pearOS-Light",
        "sddmTheme": "pearOS",
        "appStyle": "kvantum"
    },
    "dark": {
        "colorScheme": "pearOS-dark",
        "plasmaTheme": "pearOS-dark",
        "iconTheme": "pearOS",
        "cursorTheme": "pearOS-dark",
        "auroraeTheme": "__aurorae__svg__pearOS-dark",
        "kvantumTheme": "pearOS-dark",
        "gtkTheme": "pearOS-Dark",
        "sddmTheme": "pearOS-dark",
        "appStyle": "kvantum"
    }
}

# Discovery helpers
def uniq(seq):
    seen = set()
    out = []
    for x in seq:
        if x not in seen:
            seen.add(x)
            out.append(x)
    return out

def list_color_schemes():
    roots = [
        os.path.expanduser("~/.local/share/color-schemes"),
        "/usr/share/color-schemes",
    ]
    names = []
    for r in roots:
        if os.path.isdir(r):
            for fn in os.listdir(r):
                if fn.endswith(".colors"):
                    names.append(os.path.splitext(fn)[0])
    return sorted(uniq(names))

def list_plasma_themes():
    roots = [
        os.path.expanduser("~/.local/share/plasma/desktoptheme"),
        "/usr/share/plasma/desktoptheme",
    ]
    names = []
    for r in roots:
        if os.path.isdir(r):
            for d in os.listdir(r):
                if os.path.isdir(os.path.join(r, d)):
                    names.append(d)
    return sorted(uniq(names))

def list_icon_themes():
    roots = [
        os.path.expanduser("~/.icons"),
        os.path.expanduser("~/.local/share/icons"),
        "/usr/share/icons",
    ]
    names = []
    for r in roots:
        if os.path.isdir(r):
            for d in os.listdir(r):
                dp = os.path.join(r, d)
                if os.path.isdir(dp) and os.path.isfile(os.path.join(dp, "index.theme")):
                    names.append(d)
    return sorted(uniq(names))

def list_cursor_themes():
    # Cursors are icon themes that include cursors; list same as icons
    return list_icon_themes()

def list_aurorae_themes():
    roots = [
        os.path.expanduser("~/.local/share/aurorae/themes"),
        os.path.expanduser("~/.local/share/aurorae"),
        "/usr/share/aurorae/themes",
        "/usr/share/aurorae",
    ]
    names = []
    for r in roots:
        if os.path.isdir(r):
            for d in os.listdir(r):
                dp = os.path.join(r, d)
                if os.path.isdir(dp):
                    names.append(d)
    return sorted(uniq(names))

def list_kvantum_themes():
    roots = [
        os.path.expanduser("~/.config/Kvantum"),
        "/usr/share/Kvantum",
    ]
    names = []
    for r in roots:
        if os.path.isdir(r):
            for d in os.listdir(r):
                dp = os.path.join(r, d)
                if os.path.isdir(dp):
                    names.append(d)
    return sorted(uniq(names))

def list_gtk_themes():
    roots = [
        os.path.expanduser("~/.themes"),
        "/usr/share/themes",
    ]
    names = []
    for r in roots:
        if os.path.isdir(r):
            for d in os.listdir(r):
                dp = os.path.join(r, d)
                if os.path.isdir(dp):
                    names.append(d)
    return sorted(uniq(names))

def list_sddm_themes():
    r = "/usr/share/sddm/themes"
    names = []
    if os.path.isdir(r):
        for d in os.listdir(r):
            if os.path.isdir(os.path.join(r, d)):
                names.append(d)
    return sorted(uniq(names))

# Config IO
def load_config():
    if os.path.isfile(CONFIG_LOCAL):
        path = CONFIG_LOCAL
    elif os.path.isfile(CONFIG_SYSTEM):
        path = CONFIG_SYSTEM
    else:
        return DEFAULT_CONFIG.copy(), CONFIG_LOCAL
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        # Ensure required structure
        for mode in ("light", "dark"):
            if mode not in data:
                data[mode] = DEFAULT_CONFIG[mode]
        return data, path
    except Exception as e:
        messagebox.showerror("Error", f"Failed to load config: {e}")
        return DEFAULT_CONFIG.copy(), CONFIG_LOCAL

def save_config(data, path):
    try:
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        return True, ""
    except Exception as e:
        return False, str(e)

def set_default_system(data):
    tmp = None
    try:
        tmp = tempfile.NamedTemporaryFile("w", delete=False, encoding="utf-8")
        json.dump(data, tmp, indent=2, ensure_ascii=False)
        tmp.close()
        target = CONFIG_SYSTEM
        os.makedirs(os.path.dirname(target), exist_ok=True)
        if shutil.which("pkexec"):
            subprocess.check_call(["pkexec", "install", "-Dm0644", tmp.name, target])
        else:
            # Fallback: ask for sudo in a terminal-like prompt
            subprocess.check_call(["sudo", "install", "-Dm0644", tmp.name, target])
        return True, ""
    except subprocess.CalledProcessError as e:
        return False, f"Privilege escalation failed: {e}"
    except Exception as e:
        return False, str(e)
    finally:
        if tmp and os.path.exists(tmp.name):
            try:
                os.unlink(tmp.name)
            except Exception:
                pass

COMPONENTS = [
    ("colorScheme", "Color Schemes", list_color_schemes),
    ("plasmaTheme", "Plasma Desktop Themes", list_plasma_themes),
    ("iconTheme", "Icon Themes", list_icon_themes),
    ("cursorTheme", "Cursor Themes", list_cursor_themes),
    ("auroraeTheme", "Aurorae (Window Decorations)", list_aurorae_themes),
    ("kvantumTheme", "Kvantum Themes", list_kvantum_themes),
    ("gtkTheme", "GTK Themes", list_gtk_themes),
    ("sddmTheme", "SDDM Themes", list_sddm_themes),
    ("appStyle", "Application Style (plugin)", lambda: ["kvantum", "kvantum-light", "kvantum-dark"]),
]

class App(ttk.Frame):
    def __init__(self, master):
        super().__init__(master)
        self.master = master
        self.master.title("pearOS Theme Config")
        self.master.geometry("980x620")
        self.grid(sticky="nsew")
        self.master.rowconfigure(0, weight=1)
        self.master.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        self.data, self.loaded_path = load_config()
        self.modified = False

        # UI: Left - component & available list, Right - current values & actions
        container = ttk.Panedwindow(self, orient="horizontal")
        container.grid(row=0, column=0, sticky="nsew", padx=8, pady=8)

        left_frame = ttk.Frame(container)
        right_frame = ttk.Frame(container)
        container.add(left_frame, weight=1)
        container.add(right_frame, weight=1)

        # Left: component selector and listbox
        ttk.Label(left_frame, text="Component").grid(row=0, column=0, sticky="w")
        self.comp_var = tk.StringVar(value=COMPONENTS[0][0])
        self.comp_combo = ttk.Combobox(left_frame, textvariable=self.comp_var, state="readonly",
                                       values=[c[0] for c in COMPONENTS])
        self.comp_combo.grid(row=1, column=0, sticky="ew", pady=4)
        left_frame.columnconfigure(0, weight=1)
        self.comp_combo.bind("<<ComboboxSelected>>", self.refresh_available)

        self.available_list = tk.Listbox(left_frame, selectmode="browse")
        self.available_list.grid(row=2, column=0, sticky="nsew")
        left_frame.rowconfigure(2, weight=1)

        # Right: current values, set buttons, JSON editor, save/discard
        cur_box = ttk.LabelFrame(right_frame, text="Current configuration")
        cur_box.grid(row=0, column=0, sticky="ew", padx=4, pady=4)
        for i in range(2):
            cur_box.columnconfigure(i, weight=1)
        ttk.Label(cur_box, text="Light:").grid(row=0, column=0, sticky="w")
        ttk.Label(cur_box, text="Dark:").grid(row=1, column=0, sticky="w")
        self.cur_light = ttk.Label(cur_box, text="")
        self.cur_dark = ttk.Label(cur_box, text="")
        self.cur_light.grid(row=0, column=1, sticky="w")
        self.cur_dark.grid(row=1, column=1, sticky="w")

        action_box = ttk.Frame(right_frame)
        action_box.grid(row=1, column=0, sticky="ew", padx=4, pady=4)
        ttk.Button(action_box, text="Set as Light", command=self.set_as_light).grid(row=0, column=0, padx=2)
        ttk.Button(action_box, text="Set as Dark", command=self.set_as_dark).grid(row=0, column=1, padx=2)

        json_box = ttk.LabelFrame(right_frame, text="Edit config.json")
        json_box.grid(row=2, column=0, sticky="nsew", padx=4, pady=4)
        right_frame.rowconfigure(2, weight=1)
        self.json_text = tk.Text(json_box, wrap="none", height=12)
        self.json_text.grid(row=0, column=0, sticky="nsew")
        json_box.rowconfigure(0, weight=1)
        json_box.columnconfigure(0, weight=1)
        btns = ttk.Frame(json_box)
        btns.grid(row=1, column=0, sticky="e", pady=(6,0))
        ttk.Button(btns, text="Load", command=self.load_json_into_editor).grid(row=0, column=0, padx=2)
        ttk.Button(btns, text="Apply From Editor", command=self.apply_from_editor).grid(row=0, column=1, padx=2)

        bottom = ttk.Frame(self)
        bottom.grid(row=1, column=0, sticky="ew", padx=8, pady=(0,8))
        ttk.Button(bottom, text="Save (local)", command=self.save_local).grid(row=0, column=0, padx=2)
        ttk.Button(bottom, text="Discard", command=self.discard).grid(row=0, column=1, padx=2)
        ttk.Button(bottom, text="Set as default (system)", command=self.save_system).grid(row=0, column=2, padx=2)
        ttk.Button(bottom, text="Open config.json…", command=self.open_config_file).grid(row=0, column=3, padx=2)
        self.status = ttk.Label(bottom, text=f"Loaded: {self.loaded_path}")
        self.status.grid(row=0, column=4, sticky="w", padx=8)

        self.refresh_available()
        self.refresh_current_labels()
        self.load_json_into_editor()

    def current_component(self):
        key = self.comp_var.get()
        for c in COMPONENTS:
            if c[0] == key:
                return c
        return COMPONENTS[0]

    def refresh_available(self, *_):
        key, title, lister = self.current_component()
        self.available_list.delete(0, tk.END)
        try:
            items = lister()
        except Exception as e:
            items = []
        # Special handling for aurorae: show folder names, but selected will be converted to __aurorae__svg__<name>
        for name in items:
            self.available_list.insert(tk.END, name)
        self.refresh_current_labels()

    def refresh_current_labels(self):
        key, _, _ = self.current_component()
        self.cur_light.config(text=self.data.get("light", {}).get(key, ""))
        self.cur_dark.config(text=self.data.get("dark", {}).get(key, ""))

    def selected_value(self):
        idxs = self.available_list.curselection()
        if not idxs:
            return None
        return self.available_list.get(idxs[0])

    def _convert_value_for_key(self, key, value):
        if key == "auroraeTheme":
            # convert folder name to __aurorae__svg__<Name> if needed
            if not value.startswith("__aurorae__svg__"):
                return f"__aurorae__svg__{value}"
        return value

    def set_as_light(self):
        key, _, _ = self.current_component()
        val = self.selected_value()
        if val is None:
            messagebox.showwarning("No selection", "Select a value from the list.")
            return
        val = self._convert_value_for_key(key, val)
        self.data.setdefault("light", {})[key] = val
        self.modified = True
        self.refresh_current_labels()
        self.load_json_into_editor()

    def set_as_dark(self):
        key, _, _ = self.current_component()
        val = self.selected_value()
        if val is None:
            messagebox.showwarning("No selection", "Select a value from the list.")
            return
        val = self._convert_value_for_key(key, val)
        self.data.setdefault("dark", {})[key] = val
        self.modified = True
        self.refresh_current_labels()
        self.load_json_into_editor()

    def load_json_into_editor(self):
        try:
            txt = json.dumps(self.data, indent=2, ensure_ascii=False)
            self.json_text.delete("1.0", tk.END)
            self.json_text.insert("1.0", txt)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to render JSON: {e}")

    def apply_from_editor(self):
        try:
            new_data = json.loads(self.json_text.get("1.0", tk.END))
            # rudimentary validation
            for mode in ("light", "dark"):
                if mode not in new_data:
                    raise ValueError(f"Missing top-level key: {mode}")
            self.data = new_data
            self.modified = True
            self.refresh_current_labels()
            self.status.config(text=f"Edited (not saved)")
        except Exception as e:
            messagebox.showerror("Invalid JSON", f"{e}")

    def save_local(self):
        ok, err = save_config(self.data, CONFIG_LOCAL)
        if ok:
            self.loaded_path = CONFIG_LOCAL
            self.modified = False
            self.status.config(text=f"Saved: {self.loaded_path}")
            messagebox.showinfo("Saved", f"Saved to {CONFIG_LOCAL}")
        else:
            messagebox.showerror("Error", f"Failed to save: {err}")

    def discard(self):
        if self.modified and not messagebox.askyesno("Discard", "Discard unsaved changes?"):
            return
        self.data, self.loaded_path = load_config()
        self.modified = False
        self.refresh_available()
        self.refresh_current_labels()
        self.load_json_into_editor()
        self.status.config(text=f"Loaded: {self.loaded_path}")

    def save_system(self):
        ok, err = set_default_system(self.data)
        if ok:
            messagebox.showinfo("Saved", f"Saved as default to {CONFIG_SYSTEM}")
        else:
            messagebox.showerror("Error", f"Failed to save as default: {err}")

    def open_config_file(self):
        # Open local config in external editor or let user choose a path
        path = CONFIG_LOCAL if os.path.isfile(CONFIG_LOCAL) else self.loaded_path
        if not os.path.isfile(path):
            # create local
            ok, err = save_config(self.data, CONFIG_LOCAL)
            if not ok:
                messagebox.showerror("Error", f"Cannot create local config: {err}")
                return
            path = CONFIG_LOCAL
        # Open with xdg-open or default app
        opener = shutil.which("xdg-open")
        try:
            if opener:
                subprocess.Popen([opener, path])
            else:
                filedialog.askopenfilename(initialdir=os.path.dirname(path), title="Open with your editor")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to open file: {e}")

def main():
    root = tk.Tk()
    # Tk scaling for HiDPI
    try:
        root.tk.call('tk', 'scaling', 1.0)
    except Exception:
        pass
    App(root)
    root.mainloop()

if __name__ == "__main__":
    main()


