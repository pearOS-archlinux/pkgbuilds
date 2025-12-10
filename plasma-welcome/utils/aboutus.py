"""
About Us dialog
"""

from PySide6.QtWidgets import (
    QDialog, QVBoxLayout, QLabel, QPushButton
)
from PySide6.QtCore import Qt
from PySide6.QtGui import QDesktopServices, QFont
from PySide6.QtCore import QUrl


class AboutUsDialog(QDialog):
    """About Us dialog showing application information"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        # Apply same stylesheet as main window if parent exists
        if parent:
            self.setStyleSheet(parent.styleSheet())
        self.setup_ui()
        
    def setup_ui(self):
        """Setup the About Us dialog UI"""
        self.setWindowTitle("About Us")
        self.setFixedSize(400, 350)
        self.setModal(True)
        
        # Enable transparency for blur effect
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        
        # Setup blur effect
        self.setup_blur()
        
        layout = QVBoxLayout(self)
        layout.setSpacing(10)
        layout.setContentsMargins(20, 20, 20, 20)
        
        # App name
        app_name = QLabel("Pear Software and Services S.R.L.")
        font = QFont()
        font.setPointSize(18)
        font.setBold(True)
        app_name.setFont(font)
        app_name.setAlignment(Qt.AlignCenter)
        app_name.setOpenExternalLinks(True)
        app_name.setTextFormat(Qt.RichText)
        app_name.setText(f'<a href="https://pear-software.com" style="text-decoration: none; color: inherit;">Pear Software and Services S.R.L.</a>')
        layout.addWidget(app_name)
        
        # Version
        version = QLabel("Version 25.12.10")
        version.setAlignment(Qt.AlignCenter)
        layout.addWidget(version)
        
        # Description
        description = QLabel(
            "pearOS welcome was made to help you onboard quickly"
        )
        description.setWordWrap(True)
        description.setAlignment(Qt.AlignJustify)
        layout.addWidget(description)
        
        # Developers label
        dev_label = QLabel("<b>Developers:</b>")
        dev_label.setAlignment(Qt.AlignLeft)
        layout.addWidget(dev_label)
        
        # Fork info with link
        fork_label = QLabel()
        fork_label.setOpenExternalLinks(True)
        fork_label.setTextFormat(Qt.RichText)
        fork_label.setText(
            f'<a href="https://github.com/arch-linux-gui/alg-welcome" style="text-decoration: none; color: inherit;">Fork of ALG Welcome App</a>'
        )
        fork_label.setAlignment(Qt.AlignLeft)
        layout.addWidget(fork_label)
        
        # Website button
        website_button = QPushButton("Visit our website")
        website_button.setFocusPolicy(Qt.NoFocus)
        website_button.clicked.connect(
            lambda: QDesktopServices.openUrl(QUrl("https://pear-software.com"))
        )
        layout.addWidget(website_button)
        
        # License
        license_text = QLabel("Distributed under the MIT License.")
        license_text.setWordWrap(True)
        license_text.setAlignment(Qt.AlignCenter)
        layout.addWidget(license_text)
        
        layout.addStretch()
    
    def setup_blur(self):
        """Setup blur effect using BlurWindow"""
        try:
            from BlurWindow.blurWindow import GlobalBlur
            GlobalBlur(self.winId(), Dark=True, QWidget=self)
            print("BlurWindow blur effect applied to About Us dialog")
        except ImportError:
            # Fallback to native KDE blur
            print("BlurWindow not available for About Us, using native KDE blur")
            self.setAttribute(Qt.WA_TranslucentBackground, True)
        except Exception as e:
            print(f"Error setting up blur for About Us: {e}")
            # Fallback to native KDE blur
            self.setAttribute(Qt.WA_TranslucentBackground, True)