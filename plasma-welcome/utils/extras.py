"""
Utility functions for desktop environment detection, URL opening, and Calamares
"""

import os
import subprocess
from pathlib import Path
import threading


# Global state for Calamares
_calamares_running = False
_calamares_lock = threading.Lock()


def get_desktop_environment():
    """
    Get the current desktop environment
    Returns: str - Desktop environment name (lowercase)
    """
    desktop = os.environ.get('XDG_CURRENT_DESKTOP', '').lower()
    return desktop


def open_url(url):
    """
    Open URL in default browser using QDesktopServices
    Args:
        url: str - URL to open
    """
    try:
        from PySide6.QtGui import QDesktopServices
        from PySide6.QtCore import QUrl
        QDesktopServices.openUrl(QUrl(url))
    except Exception as e:
        print(f"URL Error: {e}")
        # Fallback to xdg-open if Qt method fails
        try:
            subprocess.run(['xdg-open', url], check=False)
        except Exception as e2:
            print(f"Fallback URL Error: {e2}")


def check_if_live_iso():
    """
    Check if running from Live ISO
    Returns: bool - True if running from live ISO, False otherwise
    """
    return Path('/run/archiso').exists()
    # return True


def is_calamares_running():
    """
    Check if Calamares is currently running
    Returns: bool - True if Calamares is running, False otherwise
    """
    global _calamares_running
    return _calamares_running


def run_calamares_if_live_iso(is_live_iso):
    """
    Run Calamares installer if on Live ISO
    Args:
        is_live_iso: bool - Whether running from live ISO
    """
    if not is_live_iso:
        return
    
    global _calamares_running
    
    with _calamares_lock:
        if _calamares_running:
            print("Calamares is already running")
            return
        _calamares_running = True
    
    def run_calamares():
        global _calamares_running
        try:
            result = subprocess.run(
                ['bash' 'bin_install'],
                capture_output=True,
                text=True
            )
            
            if result.returncode != 0:
                print(f"bin_install exit code: {result.returncode}")
                print(f"Error: {result.stderr}")
            else:
                print(result.stdout)
                
        except Exception as e:
            print(f"runBinInstall Error: {e}")
        finally:
            with _calamares_lock:
                _calamares_running = False
    
    # Run in separate thread to avoid blocking UI
    thread = threading.Thread(target=run_calamares, daemon=True)
    thread.start()
