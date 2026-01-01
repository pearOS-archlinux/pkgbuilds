const { app, BrowserWindow, ipcMain, dialog, shell, screen, nativeImage } = require('electron');
const path = require('path');
const { exec, spawn } = require('child_process');
const fs = require('fs');

if (process.platform === 'linux') {
  app.commandLine.appendSwitch('enable-transparent-visuals');
  
  if (process.env.WAYLAND_DISPLAY) {
    app.commandLine.appendSwitch('enable-features', 'UseOzonePlatform');
    app.commandLine.appendSwitch('ozone-platform', 'wayland');
  }
}

// Setează numele aplicației
app.setName('PearOS Settings');

// Setează iconița aplicației (folosește iconițe de diferite dimensiuni pentru compatibilitate)
function loadAppIcon() {
  // Pe Linux, folosim iconițe mai mici pentru taskbar (48x48 sau 32x32)
  const iconPaths = process.platform === 'linux' ? [
    path.join(__dirname, 'assets', 'preferences.png'),
    path.join(__dirname, 'assets', 'preferences-desktop-48.png'),
    path.join(__dirname, 'assets', 'preferences-desktop-32.png'),
    path.join(__dirname, 'assets', 'preferences-desktop-64.png'),
    path.join(__dirname, 'assets', 'preferences-desktop.png'),
    path.join(__dirname, 'assets', 'preferences-desktop.svg')
  ] : [
    path.join(__dirname, 'assets', 'preferences.png'),
    path.join(__dirname, 'assets', 'preferences-desktop.png'),
    path.join(__dirname, 'assets', 'preferences-desktop.svg'),
    path.join(__dirname, 'assets', 'gtk-preferences.png'),
    path.join(__dirname, 'assets', 'gtk-preferences.svg')
  ];
  
  for (const iconPath of iconPaths) {
    if (fs.existsSync(iconPath)) {
      try {
        const icon = nativeImage.createFromPath(iconPath);
        if (!icon.isEmpty()) {
          app.setIcon(icon);
          return icon;
        }
      } catch (error) {
        continue;
      }
    }
  }
  return null;
}

const appIcon = loadAppIcon();

let mainWindow;

function getPageArgument() {
  const args = process.argv;
  
  const pageIndex = args.findIndex(arg => arg.startsWith('--page='));
  if (pageIndex !== -1) {
    return args[pageIndex].split('=')[1];
  }
  
  const pageIndex2 = args.indexOf('--page');
  if (pageIndex2 !== -1 && args[pageIndex2 + 1]) {
    return args[pageIndex2 + 1];
  }
  
  const lastArg = args[args.length - 1];
  if (lastArg && lastArg !== '.' && !lastArg.includes('electron') && !lastArg.includes('/')) {
    return lastArg;
  }
  
  return null;
}

function createWindow() {
  const pageArg = getPageArgument();
  
  // Încarcă iconița pentru fereastră (folosește iconițe mai mici pe Linux pentru taskbar)
  let windowIcon = appIcon;
  if (!windowIcon || process.platform === 'linux') {
    const linuxIconPaths = [
      path.join(__dirname, 'assets', 'preferences.png'),
      path.join(__dirname, 'assets', 'preferences-desktop-48.png'),
      path.join(__dirname, 'assets', 'preferences-desktop-32.png'),
      path.join(__dirname, 'assets', 'preferences-desktop-64.png'),
      path.join(__dirname, 'assets', 'preferences-desktop.png')
    ];
    
    for (const iconPath of linuxIconPaths) {
      if (fs.existsSync(iconPath)) {
        try {
          const icon = nativeImage.createFromPath(iconPath);
          if (!icon.isEmpty()) {
            windowIcon = icon;
            break;
          }
        } catch (error) {
          continue;
        }
      }
    }
  }

  // Pe Linux, folosim calea directă pentru iconiță (uneori funcționează mai bine)
  let iconOption = windowIcon;
  if (process.platform === 'linux' && !iconOption) {
    // Încearcă să găsească o iconiță PNG validă
    const iconPaths = [
      path.join(__dirname, 'assets', 'preferences.png'),
      path.join(__dirname, 'assets', 'preferences-desktop-48.png'),
      path.join(__dirname, 'assets', 'preferences-desktop-32.png'),
      path.join(__dirname, 'assets', 'preferences-desktop-64.png'),
      path.join(__dirname, 'assets', 'preferences-desktop.png')
    ];
    for (const iconPath of iconPaths) {
      if (fs.existsSync(iconPath)) {
        iconOption = iconPath; // Folosește calea directă pe Linux
        break;
      }
    }
  }

  mainWindow = new BrowserWindow({
    width: 700,
    height: 600,
    minWidth: 700,
    maxWidth: 700,
    minHeight: 400,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      enableRemoteModule: false,
      backgroundThrottling: false,
      preload: path.join(__dirname, 'preload.js')
    },
    icon: iconOption || undefined,
    transparent: true,
    titleBarStyle: 'default',
    backgroundColor: '#00000000', 
    frame: false, 
    show: false 
  });
  
  // Setează iconița ferestrei explicit după creare (pentru Linux)
  if (process.platform === 'linux') {
    if (windowIcon) {
      mainWindow.setIcon(windowIcon);
    } else if (iconOption && typeof iconOption === 'string') {
      // Dacă iconOption este o cale, încarcă-o ca nativeImage
      try {
        const icon = nativeImage.createFromPath(iconOption);
        if (!icon.isEmpty()) {
          mainWindow.setIcon(icon);
        }
      } catch (error) {
        // Ignoră eroarea
      }
    }
  }

  
  mainWindow.loadFile('index.html');

  // Previne resize-ul peste maxWidth chiar și la window snap
  mainWindow.on('will-resize', (event, newBounds) => {
    if (newBounds.width > 700) {
      event.preventDefault();
      mainWindow.setBounds({
        x: newBounds.x,
        y: newBounds.y,
        width: 700,
        height: newBounds.height
      });
    }
  });

  
  mainWindow.once('ready-to-show', () => {
    mainWindow.show();
    
    if (process.platform === 'linux') {
      mainWindow.setBackgroundColor('#00000000');
      
      if (mainWindow.setVisualEffectState) {
        mainWindow.setVisualEffectState('active');
      }
      
      // Setează iconița din nou după ce fereastra este afișată (pentru taskbar)
      // Folosește iconițe mai mici pentru taskbar pe Linux
      const taskbarIconPaths = [
        path.join(__dirname, 'assets', 'preferences.png')
      ];
      
      for (const iconPath of taskbarIconPaths) {
        if (fs.existsSync(iconPath)) {
          try {
            const taskbarIcon = nativeImage.createFromPath(iconPath);
            if (!taskbarIcon.isEmpty()) {
              mainWindow.setIcon(taskbarIcon);
              // Setează și pentru aplicație
              app.setIcon(taskbarIcon);
              break;
            }
          } catch (error) {
            continue;
          }
        }
      }
    }
    
    if (pageArg) {
      const pageMap = {
        'displays': 'Displays',
        'wallpaper': 'Wallpaper',
        'notifications': 'Notifications',
        'sound': 'Sound',
        'focus': 'Focus',
        'screen-time': 'Screen Time',
        'lock-screen': 'Lock Screen',
        'privacy': 'Privacy and Security',
        'touch-id': 'Touch ID and Password',
        'users': 'Users & Groups',
        'internet-accounts': 'Internet Accounts',
        'keyboard': 'Keyboard',
        'trackpad': 'Trackpad',
        'printers': 'Printers & Scanners',
        'appearance': 'Appearance',
        'general': 'General',
        'software-update': 'Software Update',
        'battery': 'Battery',
        'wifi': 'Wi-Fi',
        'bluetooth': 'Bluetooth',
        'network': 'Network',
        'about': 'About',
        'storage': 'Storage',
        'pearcare': 'PearCare & Warranty',
        'peardrop': 'PearDrop & Handoff',
        'autofill': 'AutoFill & Passwords',
        'date-time': 'Date & Time',
        'language-region': 'Language & Region',
        'login-items': 'Login Items & Extensions',
        'sharing': 'Sharing',
        'startup-disk': 'Startup Disk',
        'time-machine': 'Time Machine',
        'device-management': 'Device Management',
        'transfer-reset': 'Transfer or Reset',
        'accessibility': 'Accessibility',
        'menu-bar': 'Menu Bar',
        'spotlight': 'Spotlight',
        'desktop-dock': 'Desktop and Dock'
      };
      
      const pageName = pageMap[pageArg];
      if (pageName) {
        mainWindow.webContents.once('did-finish-load', () => {
          setTimeout(() => {
            mainWindow.webContents.executeJavaScript(`
              (function() {
                if (typeof showPage === 'function') {
                  showPage('${pageName}');
                } else {
                  const items = document.querySelectorAll('.sidenav .item, .general-menu-item');
                  items.forEach(item => {
                    const dataPage = item.getAttribute('data-page');
                    const nameElement = item.querySelector('.name, .general-menu-label');
                    if ((dataPage && dataPage === '${pageName}') || 
                        (nameElement && nameElement.textContent.trim() === '${pageName}')) {
                      item.click();
                    }
                  });
                }
              })();
            `).catch(() => {});
          }, 1000);
        });
      }
    }
  });

  
  

  
  mainWindow.webContents.on('before-input-event', (event, input) => {
    if (input.control && input.shift && input.key.toLowerCase() === 'i') {
      if (mainWindow.webContents.isDevToolsOpened()) {
        mainWindow.webContents.closeDevTools();
      } else {
        mainWindow.webContents.openDevTools();
      }
    }
    if (input.key === 'F12') {
      if (mainWindow.webContents.isDevToolsOpened()) {
        mainWindow.webContents.closeDevTools();
      } else {
        mainWindow.webContents.openDevTools();
      }
    }
  });

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
  
  // Asigură că width-ul rămâne la maxWidth chiar și după resize (pentru window snap)
  mainWindow.on('resize', () => {
    if (mainWindow) {
      const bounds = mainWindow.getBounds();
      if (bounds.width > 700) {
        mainWindow.setBounds({
          x: bounds.x,
          y: bounds.y,
          width: 700,
          height: bounds.height
        });
      }
    }
  });
  
  // Detectează mișcarea ferestrei pentru tint-ul dinamic
  mainWindow.on('move', () => {
    if (mainWindow && mainWindow.webContents) {
      const bounds = mainWindow.getBounds();
      mainWindow.webContents.send('window-moved', { 
        x: bounds.x, 
        y: bounds.y, 
        width: bounds.width, 
        height: bounds.height 
      });
    }
  });
}

ipcMain.handle('wifi-get-status', async () => {
  return new Promise((resolve, reject) => {
    exec('nmcli radio wifi', (error, stdout, stderr) => {
      if (error) {
        reject(error);
        return;
      }
      const isEnabled = stdout.trim().toLowerCase().includes('enabled');
      resolve({ enabled: isEnabled });
    });
  });
});

ipcMain.handle('wifi-get-ssid', async () => {
  return new Promise((resolve, reject) => {
    exec("nmcli -t -f active,ssid,security dev wifi | grep '^yes:'", (error, stdout, stderr) => {
      if (error || !stdout.trim()) {
        resolve({ connected: false, ssid: null, security: null });
        return;
      }
      const parts = stdout.trim().split(':');
      const ssid = parts[1] || '';
      const security = parts[2] || '--';
      resolve({ connected: true, ssid: ssid, security: security });
    });
  });
});

ipcMain.handle('wifi-toggle', async (event, enabled) => {
  return new Promise((resolve, reject) => {
    const command = enabled ? 'nmcli radio wifi on' : 'nmcli radio wifi off';
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(error);
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('wifi-get-networks', async () => {
  return new Promise((resolve, reject) => {
    exec("nmcli -t -f SSID,SIGNAL,SECURITY dev wifi list | grep -v '^--' | head -20", (error, stdout, stderr) => {
      if (error) {
        reject(error);
        return;
      }
      const lines = stdout.trim().split('\n').filter(line => line.trim());
      const networks = lines.map(line => {
        const parts = line.split(':');
        return {
          ssid: parts[0] || 'Unknown',
          signal: parts[1] || '0',
          security: parts[2] || '--'
        };
      }).filter(network => network.ssid && network.ssid !== '--');
      resolve({ networks: networks });
    });
  });
});

ipcMain.handle('wifi-connect', async (event, ssid, password) => {
  return new Promise((resolve, reject) => {
    
    const escapedSSID = ssid.replace(/\\/g, '\\\\').replace(/'/g, "'\\''");
    const escapedPassword = password.replace(/\\/g, '\\\\').replace(/'/g, "'\\''");
    
    
    const connectionName = `wifi-${Date.now()}`;
    const addCommand = `nmcli connection add type wifi con-name '${connectionName}' ifname '*' ssid '${escapedSSID}' wifi-sec.key-mgmt wpa-psk wifi-sec.psk '${escapedPassword}'`;
    
    exec(addCommand, { timeout: 10000 }, (addError, addStdout, addStderr) => {
      if (addError) {
        
        const connectCommand = `nmcli device wifi connect '${escapedSSID}' password '${escapedPassword}'`;
        exec(connectCommand, { timeout: 30000 }, (connectError, connectStdout, connectStderr) => {
          if (connectError) {
            reject(new Error(`Failed to connect: ${connectStderr || connectError.message}`));
            return;
          }
          resolve({ success: true });
        });
        return;
      }
      
      
      const activateCommand = `nmcli connection up '${connectionName}'`;
      exec(activateCommand, { timeout: 30000 }, (activateError, activateStdout, activateStderr) => {
        if (activateError) {
          
          exec(`nmcli connection delete '${connectionName}'`, () => {});
          reject(new Error(`Failed to activate connection: ${activateStderr || activateError.message}`));
          return;
        }
        resolve({ success: true });
      });
    });
  });
});

ipcMain.handle('wifi-forget-network', async (event, ssid) => {
  return new Promise((resolve, reject) => {
    const escapedSSID = ssid.replace(/\\/g, '\\\\').replace(/'/g, "'\\''");
    
    exec(`nmcli -t -f NAME,TYPE connection show`, (error, stdout) => {
      if (error || !stdout) {
        reject(new Error('Could not list connections'));
        return;
      }
      
      const lines = stdout.trim().split('\n').filter(line => line.includes(':802-11-wireless:'));
      const connectionPromises = [];
      
      for (const line of lines) {
        const parts = line.split(':');
        if (parts.length >= 2) {
          const connectionName = parts[0];
          const escapedName = connectionName.replace(/'/g, "'\\''");
          
          connectionPromises.push(
            new Promise((resolveCheck) => {
              exec(`nmcli -t -f 802-11-wireless.ssid connection show '${escapedName}' 2>/dev/null`, (showError, showStdout) => {
                if (!showError && showStdout) {
                  const connectionSSID = showStdout.trim();
                  if (connectionSSID === ssid) {
                    resolveCheck(connectionName);
                  } else {
                    resolveCheck(null);
                  }
                } else {
                  resolveCheck(null);
                }
              });
            })
          );
        }
      }
      
      Promise.all(connectionPromises).then((results) => {
        const connectionToDelete = results.find(r => r !== null);
        
        if (connectionToDelete) {
          const escapedName = connectionToDelete.replace(/'/g, "'\\''");
          exec(`nmcli connection delete '${escapedName}'`, (deleteError, deleteStdout, deleteStderr) => {
            if (deleteError) {
              reject(new Error(`Failed to forget network: ${deleteStderr || deleteError.message}`));
              return;
            }
            resolve({ success: true });
          });
        } else {
          reject(new Error('Network not found in saved connections'));
        }
      }).catch((err) => {
        reject(new Error(`Error finding network: ${err.message}`));
      });
    });
  });
});

ipcMain.handle('bluetooth-get-status', async () => {
  return new Promise((resolve, reject) => {
    exec('bluetoothctl show | grep -i "powered:"', (error, stdout, stderr) => {
      if (error) {
        
        exec('rfkill list bluetooth | grep -i "soft blocked"', (rfkillError, rfkillStdout, rfkillStderr) => {
          if (rfkillError) {
            reject(error);
            return;
          }
          const isBlocked = rfkillStdout.toLowerCase().includes('yes');
          resolve({ enabled: !isBlocked });
        });
        return;
      }
      const isEnabled = stdout.toLowerCase().includes('yes');
      resolve({ enabled: isEnabled });
    });
  });
});

ipcMain.handle('bluetooth-toggle', async (event, enabled) => {
  return new Promise((resolve, reject) => {
    if (enabled) {
      
      exec('bluetoothctl power on', (error, stdout, stderr) => {
        if (error) {
          
          exec('rfkill unblock bluetooth', (rfkillError, rfkillStdout, rfkillStderr) => {
            if (rfkillError) {
              reject(rfkillError);
              return;
            }
            resolve({ success: true });
          });
          return;
        }
        resolve({ success: true });
      });
    } else {
      
      exec('bluetoothctl power off', (error, stdout, stderr) => {
        if (error) {
          
          exec('rfkill block bluetooth', (rfkillError, rfkillStdout, rfkillStderr) => {
            if (rfkillError) {
              reject(rfkillError);
              return;
            }
            resolve({ success: true });
          });
          return;
        }
        resolve({ success: true });
      });
    }
  });
});

ipcMain.handle('bluetooth-get-device-name', async () => {
  return new Promise((resolve, reject) => {
    exec('bluetoothctl show | grep -i "name:"', (error, stdout, stderr) => {
      if (error) {
        
        exec('hostname', (hostnameError, hostnameStdout, hostnameStderr) => {
          if (hostnameError) {
            resolve({ name: 'Computer' });
            return;
          }
          resolve({ name: hostnameStdout.trim() || 'Computer' });
        });
        return;
      }
      
      const nameMatch = stdout.match(/name:\s*(.+)/i);
      if (nameMatch && nameMatch[1]) {
        resolve({ name: nameMatch[1].trim() });
      } else {
        
        exec('hostname', (hostnameError, hostnameStdout, hostnameStderr) => {
          if (hostnameError) {
            resolve({ name: 'Computer' });
            return;
          }
          resolve({ name: hostnameStdout.trim() || 'Computer' });
        });
      }
    });
  });
});

ipcMain.handle('bluetooth-get-devices', async () => {
  return new Promise((resolve) => {
    
    exec('dbus-send --system --dest=org.bluez --print-reply / org.freedesktop.DBus.ObjectManager.GetManagedObjects 2>&1', (error, stdout, stderr) => {
      if (error) {
        console.error('Error getting Bluetooth devices via D-Bus:', error);
        resolve({ devices: [] });
        return;
      }
      
      
      const devicePaths = [];
      const lines = stdout.split('\n');
      let currentPath = null;
      let hasDevice1 = false;
      
      for (let i = 0; i < lines.length; i++) {
        const line = lines[i];
        
        
        const pathMatch = line.match(/object path\s+"([^"]+)"/i);
        if (pathMatch) {
          
          if (currentPath && hasDevice1) {
            devicePaths.push(currentPath);
          }
          currentPath = pathMatch[1];
          hasDevice1 = false;
        }
        
        
        if (line.includes('org.bluez.Device1')) {
          hasDevice1 = true;
        }
      }
      
      
      if (currentPath && hasDevice1) {
        devicePaths.push(currentPath);
      }
      
      console.log('Found device paths:', devicePaths.length, devicePaths);
      
      if (devicePaths.length === 0) {
        resolve({ devices: [] });
        return;
      }
      
      const devices = [];
      let processedCount = 0;
      const totalDevices = devicePaths.length;
      
      devicePaths.forEach(devicePath => {
        
        exec(`dbus-send --system --dest=org.bluez --print-reply ${devicePath} org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:Name 2>/dev/null`, (nameError, nameStdout) => {
          let name = 'Unknown Device';
          if (!nameError && nameStdout) {
            const nameMatch = nameStdout.match(/variant\s+string\s+"([^"]+)"/i);
            if (nameMatch) {
              name = nameMatch[1];
            }
          }
          
          
          exec(`dbus-send --system --dest=org.bluez --print-reply ${devicePath} org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:Connected 2>/dev/null`, (connectedError, connectedStdout) => {
            let connected = false;
            if (!connectedError && connectedStdout) {
              const connectedMatch = connectedStdout.match(/variant\s+boolean\s+(true|false)/i);
              if (connectedMatch) {
                connected = connectedMatch[1].toLowerCase() === 'true';
              }
            }
            
            
            exec(`dbus-send --system --dest=org.bluez --print-reply ${devicePath} org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:Icon 2>/dev/null`, (iconError, iconStdout) => {
              let deviceType = 'Device';
              if (!iconError && iconStdout) {
                const iconMatch = iconStdout.match(/variant\s+string\s+"([^"]+)"/i);
                if (iconMatch) {
                  deviceType = iconMatch[1];
                }
              }
              
              
              const macMatch = devicePath.match(/dev_([0-9A-Fa-f_]{17})/);
              const macAddress = macMatch ? macMatch[1].replace(/_/g, ':').toUpperCase() : '';
              
              devices.push({
                macAddress: macAddress,
                name: name,
                connected: connected,
                type: deviceType
              });
              
              processedCount++;
              console.log(`Processed device ${processedCount}/${totalDevices}: ${name} (${connected ? 'connected' : 'not connected'})`);
              
              if (processedCount === totalDevices) {
                console.log('All devices processed:', devices);
                resolve({ devices: devices });
              }
            });
          });
        });
      });
    });
  });
});

ipcMain.handle('bluetooth-scan-devices', async () => {
  return new Promise((resolve) => {
    
    exec('dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0 org.freedesktop.DBus.Properties.Set string:org.bluez.Adapter1 string:Powered variant:boolean:true 2>/dev/null', (powerError) => {
      
      setTimeout(() => {
        
        exec('dbus-send --system --dest=org.bluez --print-reply / org.freedesktop.DBus.ObjectManager.GetManagedObjects 2>&1', (error, stdout, stderr) => {
          if (error) {
            console.error('Error scanning Bluetooth devices:', error);
            resolve({ devices: [] });
            return;
          }
          
          
          const devicePaths = [];
          const lines = stdout.split('\n');
          let currentPath = null;
          let hasDevice1 = false;
          
          for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            
            const pathMatch = line.match(/object path\s+"([^"]+)"/i);
            if (pathMatch) {
              if (currentPath && hasDevice1) {
                devicePaths.push(currentPath);
              }
              currentPath = pathMatch[1];
              hasDevice1 = false;
            }
            
            if (line.includes('org.bluez.Device1')) {
              hasDevice1 = true;
            }
          }
          
          if (currentPath && hasDevice1) {
            devicePaths.push(currentPath);
          }
          
          console.log('Found nearby device paths:', devicePaths.length);
          
          if (devicePaths.length === 0) {
            resolve({ devices: [] });
            return;
          }
          
          const devices = [];
          let processedCount = 0;
          const totalDevices = devicePaths.length;
          
          devicePaths.forEach(devicePath => {
            
            exec(`dbus-send --system --dest=org.bluez --print-reply ${devicePath} org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:Name 2>/dev/null`, (nameError, nameStdout) => {
              let name = 'Unknown Device';
              if (!nameError && nameStdout) {
                const nameMatch = nameStdout.match(/variant\s+string\s+"([^"]+)"/i);
                if (nameMatch) {
                  name = nameMatch[1];
                }
              }
              
              
              exec(`dbus-send --system --dest=org.bluez --print-reply ${devicePath} org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:Connected 2>/dev/null`, (connectedError, connectedStdout) => {
                let connected = false;
                if (!connectedError && connectedStdout) {
                  const connectedMatch = connectedStdout.match(/variant\s+boolean\s+(true|false)/i);
                  if (connectedMatch) {
                    connected = connectedMatch[1].toLowerCase() === 'true';
                  }
                }
                
                
                exec(`dbus-send --system --dest=org.bluez --print-reply ${devicePath} org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:Icon 2>/dev/null`, (iconError, iconStdout) => {
                  let deviceType = 'Device';
                  if (!iconError && iconStdout) {
                    const iconMatch = iconStdout.match(/variant\s+string\s+"([^"]+)"/i);
                    if (iconMatch) {
                      deviceType = iconMatch[1];
                    }
                  }
                  
                  
                  const macMatch = devicePath.match(/dev_([0-9A-Fa-f_]{17})/);
                  const macAddress = macMatch ? macMatch[1].replace(/_/g, ':').toUpperCase() : '';
                  
                  devices.push({
                    macAddress: macAddress,
                    name: name,
                    connected: connected,
                    type: deviceType
                  });
                  
                  processedCount++;
                  if (processedCount === totalDevices) {
                    resolve({ devices: devices });
                  }
                });
              });
            });
          });
        });
      }, 2000); 
    });
  });
});

ipcMain.handle('bluetooth-connect', async (event, macAddress) => {
  return new Promise((resolve, reject) => {
    
    exec(`dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_${macAddress.replace(/:/g, '_')} org.bluez.Device1.Connect 2>/dev/null`, (error, stdout, stderr) => {
      if (error) {
        console.error('Error connecting to device:', error);
        reject(new Error(`Failed to connect: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('bluetooth-disconnect', async (event, macAddress) => {
  return new Promise((resolve, reject) => {
    
    exec(`dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_${macAddress.replace(/:/g, '_')} org.bluez.Device1.Disconnect 2>/dev/null`, (error, stdout, stderr) => {
      if (error) {
        console.error('Error disconnecting from device:', error);
        reject(new Error(`Failed to disconnect: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('get-active-connections', async () => {
  return new Promise((resolve) => {
    exec('nmcli -t -f TYPE,NAME,DEVICE,STATE connection show --active', (error, stdout, stderr) => {
      if (error) {
        console.error('Error getting active connections:', error);
        resolve([]);
        return;
      }
      
      const lines = stdout.trim().split('\n').filter(line => line.trim());
      const connections = [];
      
      lines.forEach(line => {
        const parts = line.split(':');
        if (parts.length >= 4) {
          connections.push({
            type: parts[0].toLowerCase(),
            name: parts[1] || 'Unknown',
            device: parts[2] || 'Unknown',
            status: parts[3] || 'Unknown'
          });
        }
      });
      
      console.log('Parsed active connections:', connections);
      resolve(connections);
    });
  });
});

ipcMain.handle('get-rfkill-status', async () => {
  return new Promise((resolve) => {
    exec('rfkill list', (error, stdout, stderr) => {
      if (error) {
        console.error('Error getting rfkill status:', error);
        resolve([]);
        return;
      }
      
      const lines = stdout.trim().split('\n');
      const devices = [];
      let currentDevice = null;
      
      lines.forEach(line => {
        const idMatch = line.match(/^(\d+):\s+(\S+):\s+(.+)/);
        if (idMatch) {
          if (currentDevice) {
            devices.push(currentDevice);
          }
          const id = idMatch[1];
          const name = idMatch[2];
          const type = idMatch[3];
          currentDevice = { 
            id: id,
            name: name,
            type: type.toLowerCase(),
            softBlocked: false,
            hardBlocked: false
          };
        } else if (currentDevice && line.includes(':')) {
          const parts = line.split(':').map(p => p.trim());
          if (parts[0] === 'Soft blocked') {
            currentDevice.softBlocked = parts[1] === 'yes';
          } else if (parts[0] === 'Hard blocked') {
            currentDevice.hardBlocked = parts[1] === 'yes';
          }
        }
      });
      
      if (currentDevice) {
        devices.push(currentDevice);
      }
      
      console.log('Parsed rfkill devices:', devices);
      resolve(devices);
    });
  });
});

ipcMain.handle('set-rfkill-state', async (event, type, id, enabled) => {
  return new Promise((resolve, reject) => {
    const action = enabled ? 'unblock' : 'block';
    exec(`rfkill ${action} ${id}`, (error, stdout, stderr) => {
      if (error) {
        console.error(`Error ${action}ing rfkill device:`, error);
        reject(new Error(`Failed to ${action} device: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('get-battery-info', async () => {
  return new Promise((resolve) => {
    
    exec('upower -i $(upower -e | grep battery)', (error, stdout, stderr) => {
      if (!error && stdout) {
        const lines = stdout.split('\n');
        const batteryInfo = {
          percentage: 0,
          state: 'Unknown',
          timeToEmpty: null,
          timeToFull: null
        };
        
        lines.forEach(line => {
          const trimmed = line.trim();
          if (trimmed.startsWith('percentage:')) {
            const match = trimmed.match(/percentage:\s*(\d+)/);
            if (match) {
              batteryInfo.percentage = parseInt(match[1]);
            }
          } else if (trimmed.startsWith('state:')) {
            const match = trimmed.match(/state:\s*(.+)/);
            if (match) {
              batteryInfo.state = match[1].trim();
            }
          } else if (trimmed.startsWith('time to empty:')) {
            const match = trimmed.match(/time to empty:\s*(.+)/);
            if (match) {
              batteryInfo.timeToEmpty = match[1].trim();
            }
          } else if (trimmed.startsWith('time to full:')) {
            const match = trimmed.match(/time to full:\s*(.+)/);
            if (match) {
              batteryInfo.timeToFull = match[1].trim();
            }
          }
        });
        
        resolve(batteryInfo);
      } else {
        
        exec('cat /sys/class/power_supply/BAT*/capacity 2>/dev/null | head -1', (capError, capacity) => {
          if (!capError && capacity) {
            const percentage = parseInt(capacity.trim());
            exec('cat /sys/class/power_supply/BAT*/status 2>/dev/null | head -1', (statusError, status) => {
              const state = !statusError && status ? status.trim() : 'Unknown';
              resolve({
                percentage: percentage || 0,
                state: state,
                timeToEmpty: null,
                timeToFull: null
              });
            });
          } else {
            resolve({
              percentage: 0,
              state: 'Unknown',
              timeToEmpty: null,
              timeToFull: null
            });
          }
        });
      }
    });
  });
});

ipcMain.handle('get-low-power-mode', async () => {
  return new Promise((resolve) => {
    
    
    
    exec('cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null | head -1', (error, stdout, stderr) => {
      if (!error && stdout) {
        const governor = stdout.trim();
        
        resolve({ enabled: governor === 'powersave' });
      } else {
        
        exec('tlp-stat -s 2>/dev/null | grep -i "mode" | head -1', (tlpError, tlpStdout) => {
          if (!tlpError && tlpStdout) {
            const enabled = tlpStdout.toLowerCase().includes('powersave');
            resolve({ enabled: enabled });
          } else {
            
            resolve({ enabled: false });
          }
        });
      }
    });
  });
});

ipcMain.handle('set-low-power-mode', async (event, enabled) => {
  return new Promise((resolve, reject) => {
    if (enabled) {
      
      
      exec('echo powersave | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null', (error, stdout, stderr) => {
        if (error) {
          
          exec('sudo tlp start 2>/dev/null', (tlpError) => {
            if (tlpError) {
              console.error('Error enabling low power mode:', tlpError);
              reject(new Error('Failed to enable low power mode'));
              return;
            }
            resolve({ success: true });
          });
        } else {
          resolve({ success: true });
        }
      });
    } else {
      
      
      exec('echo ondemand | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null', (error, stdout, stderr) => {
        if (error) {
          
          exec('sudo tlp stop 2>/dev/null', (tlpError) => {
            if (tlpError) {
              console.error('Error disabling low power mode:', tlpError);
              reject(new Error('Failed to disable low power mode'));
              return;
            }
            resolve({ success: true });
          });
        } else {
          resolve({ success: true });
        }
      });
    }
  });
});

ipcMain.handle('get-battery-health', async () => {
  return new Promise((resolve) => {
    
    exec('upower -i $(upower -e | grep battery)', (error, stdout, stderr) => {
      if (!error && stdout) {
        const lines = stdout.split('\n');
        const batteryHealth = {
          health: null,
          cycleCount: null,
          designCapacity: null,
          fullChargeCapacity: null
        };
        
        lines.forEach(line => {
          const trimmed = line.trim();
          if (trimmed.startsWith('capacity:')) {
            
            const match = trimmed.match(/capacity:\s*(\d+\.?\d*)/);
            if (match) {
              
            }
          } else if (trimmed.startsWith('energy-full-design:')) {
            const match = trimmed.match(/energy-full-design:\s*(\d+\.?\d*)\s*Wh/);
            if (match) {
              
              const wh = parseFloat(match[1]);
              batteryHealth.designCapacity = Math.round(wh * 1000 / 3.7);
            }
          } else if (trimmed.startsWith('energy-full:')) {
            const match = trimmed.match(/energy-full:\s*(\d+\.?\d*)\s*Wh/);
            if (match) {
              const wh = parseFloat(match[1]);
              batteryHealth.fullChargeCapacity = Math.round(wh * 1000 / 3.7);
            }
          }
        });
        
        
        exec('cat /sys/class/power_supply/BAT*/health 2>/dev/null | head -1', (healthError, health) => {
          if (!healthError && health) {
            batteryHealth.health = health.trim();
          }
          
          
          exec('cat /sys/class/power_supply/BAT*/cycle_count 2>/dev/null | head -1', (cycleError, cycleCount) => {
            if (!cycleError && cycleCount) {
              batteryHealth.cycleCount = parseInt(cycleCount.trim());
            }
            resolve(batteryHealth);
          });
        });
      } else {
        
        exec('cat /sys/class/power_supply/BAT*/health 2>/dev/null | head -1', (healthError, health) => {
          const batteryHealth = {
            health: null,
            cycleCount: null,
            designCapacity: null,
            fullChargeCapacity: null
          };
          
          if (!healthError && health) {
            batteryHealth.health = health.trim();
          }
          
          
          exec('cat /sys/class/power_supply/BAT*/cycle_count 2>/dev/null | head -1', (cycleError, cycleCount) => {
            if (!cycleError && cycleCount) {
              batteryHealth.cycleCount = parseInt(cycleCount.trim());
            }
            
            
            exec('cat /sys/class/power_supply/BAT*/energy_full_design 2>/dev/null | head -1', (designError, designCap) => {
              if (!designError && designCap) {
                
                const uwh = parseInt(designCap.trim());
                batteryHealth.designCapacity = Math.round(uwh / 1000 / 3.7);
              }
              
              
              exec('cat /sys/class/power_supply/BAT*/energy_full 2>/dev/null | head -1', (fullError, fullCap) => {
                if (!fullError && fullCap) {
                  const uwh = parseInt(fullCap.trim());
                  batteryHealth.fullChargeCapacity = Math.round(uwh / 1000 / 3.7);
                }
                resolve(batteryHealth);
              });
            });
          });
        });
      }
    });
  });
});

ipcMain.handle('get-system-info', async () => {
  return new Promise((resolve) => {
    const systemInfo = {
      name: 'Unknown',
      chip: 'Unknown',
      memory: 'Unknown',
      serial: 'Unknown',
      coverage: 'Not Available',
      parts: 'Not Available'
    };

    
    exec('hostname', (error, stdout) => {
      if (!error && stdout) {
        systemInfo.name = stdout.trim();
      }

      
      exec('lscpu | grep "Model name" | cut -d":" -f2 | xargs', (error, stdout) => {
        if (!error && stdout) {
          systemInfo.chip = stdout.trim();
        }

        
        exec('free -h --si | grep "Mem:" | awk \'{print $2}\'', (error, stdout) => {
          if (!error && stdout) {
            systemInfo.memory = stdout.trim();
          }

          
          exec('cat /sys/class/dmi/id/product_serial 2>/dev/null || echo "Not Available"', (error, stdout) => {
            if (!error && stdout) {
              const serial = stdout.trim();
              systemInfo.serial = serial && serial !== 'Not Available' && serial !== 'To be filled by a.E.M.' ? serial : 'Not Available';
            }

            
            exec('grep "^PRETTY_NAME=" /etc/os-release 2>/dev/null | cut -d"=" -f2 | tr -d \'"\' || grep DISTRIB_ID /etc/lsb-release 2>/dev/null | cut -d"=" -f2 | tr -d \'"\'', (error, stdout) => {
              if (!error && stdout) {
                systemInfo.osName = stdout.trim();
              } else {
                systemInfo.osName = 'Unknown';
              }

              exec('grep "^VERSION=" /etc/os-release 2>/dev/null | cut -d"=" -f2 | tr -d \'"\' || grep "^VERSION_ID=" /etc/os-release 2>/dev/null | cut -d"=" -f2 | tr -d \'"\' || grep DISTRIB_RELEASE /etc/lsb-release 2>/dev/null | cut -d"=" -f2 | tr -d \'"\'', (error, stdout) => {
                if (!error && stdout) {
                  systemInfo.osVersion = stdout.trim();
                } else {
                  systemInfo.osVersion = 'Unknown';
                }

                exec('grep "^LOGO=" /etc/os-release 2>/dev/null | cut -d"=" -f2 | tr -d \'"\'', (logoError, logoStdout) => {
                  let logoPath = '/usr/share/extras/release_logo.png';
                  
                  if (!logoError && logoStdout) {
                    const logoName = logoStdout.trim();
                    const possiblePaths = [
                      `/usr/share/pixmaps/${logoName}.png`,
                      `/usr/share/pixmaps/${logoName}`,
                      `/usr/share/extras/${logoName}.png`,
                      `/usr/share/extras/release_logo.png`
                    ];
                    
                    for (const path of possiblePaths) {
                      if (fs.existsSync(path)) {
                        logoPath = path;
                        break;
                      }
                    }
                  } else {
                    if (fs.existsSync('/usr/share/extras/release_logo.png')) {
                      logoPath = '/usr/share/extras/release_logo.png';
                    }
                  }
                  
                  systemInfo.osLogo = logoPath;
                  resolve(systemInfo);
                });
              });
            });
          });
        });
      });
    });
  });
});

ipcMain.handle('get-displays', async () => {
  return new Promise((resolve) => {
    exec('xrandr 2>/dev/null', (xrandrError, xrandrStdout) => {
      if (!xrandrError && xrandrStdout) {
        const displays = [];
        const lines = xrandrStdout.split('\n');
        let currentDisplay = null;
        
        lines.forEach(line => {
          const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+(connected|disconnected)/);
          if (displayMatch) {
            if (currentDisplay) {
              displays.push(currentDisplay);
            }
            
            const displayName = displayMatch[1];
            const isConnected = displayMatch[2] === 'connected';
            const isActive = line.includes('*');
            const isPrimary = line.includes('primary');
            currentDisplay = {
              name: displayName,
              resolution: '',
              inches: '',
              isBuiltIn: displays.length === 0,
              isConnected: isConnected,
              isEnabled: isConnected && isActive,
              isPrimary: isPrimary,
              availableResolutions: []
            };
          } else if (currentDisplay && line.match(/\s+[0-9]+x[0-9]+/)) {
            const resolutionMatch = line.match(/([0-9]+x[0-9]+)/);
            if (resolutionMatch) {
              const resolution = resolutionMatch[1];
              const isActive = line.includes('*');
              
              if (!currentDisplay.availableResolutions) {
                currentDisplay.availableResolutions = [];
              }
              
              const [width, height] = resolution.split('x').map(Number);
              const totalPixels = width * height;
              
              const existingRes = currentDisplay.availableResolutions.find(r => r.resolution === resolution);
              if (!existingRes) {
                currentDisplay.availableResolutions.push({
                  resolution: resolution,
                  width: width,
                  height: height,
                  totalPixels: totalPixels,
                  isActive: isActive
                });
              } else if (isActive) {
                existingRes.isActive = true;
              }
              
              if (isActive) {
                currentDisplay.isEnabled = true;
                currentDisplay.resolution = resolution;
                const diagonalPixels = Math.sqrt(width * width + height * height);
                const dpi = 96;
                const diagonalInches = Math.round((diagonalPixels / dpi) * 10) / 10;
                currentDisplay.inches = `${diagonalInches.toString().replace('.', ',')}"`;
              } else if (!currentDisplay.resolution) {
                currentDisplay.resolution = resolution;
                const diagonalPixels = Math.sqrt(width * width + height * height);
                const dpi = 96;
                const diagonalInches = Math.round((diagonalPixels / dpi) * 10) / 10;
                currentDisplay.inches = `${diagonalInches.toString().replace('.', ',')}"`;
              }
            }
          }
            });
            
            if (currentDisplay) {
              if (!currentDisplay.hasOwnProperty('isPrimary')) {
                currentDisplay.isPrimary = false;
              }
              if (!currentDisplay.availableResolutions) {
                currentDisplay.availableResolutions = [];
              }
              if (currentDisplay.availableResolutions.length > 0) {
                currentDisplay.availableResolutions.sort((a, b) => b.totalPixels - a.totalPixels);
              }
              displays.push(currentDisplay);
            }
        
        const connectedDisplays = displays.filter(d => d.isConnected);
        
        if (connectedDisplays.length === 0) {
          resolve([{
            name: 'Display',
            resolution: 'Unknown',
            inches: 'Unknown',
            isBuiltIn: true,
            isConnected: true,
            isEnabled: true
          }]);
        } else {
          resolve(connectedDisplays);
        }
      } else {
        resolve([{
          name: 'Display',
          resolution: 'Unknown',
          inches: 'Unknown',
          isBuiltIn: true,
          isConnected: true,
          isEnabled: true
        }]);
      }
    });
  });
});

ipcMain.handle('get-storage', async () => {
  return new Promise((resolve) => {
    
    exec('df -h 2>/dev/null | grep -E "^/dev/" | grep -v "tmpfs\\|devtmpfs\\|sysfs\\|proc\\|loop"', (error, stdout) => {
      if (!error && stdout) {
        const storages = [];
        const lines = stdout.trim().split('\n');
        const seenDevices = new Set();
        const pendingLabels = [];
        
        lines.forEach(line => {
          const parts = line.trim().split(/\s+/);
          if (parts.length >= 6) {
            const device = parts[0];
            const totalSize = parts[1];
            const availableSize = parts[3];
            const mountPoint = parts[5];
            
            
            const baseDevice = device.replace(/[0-9]+$/, '');
            
            
            
            if (mountPoint === '/') {
              if (!seenDevices.has(baseDevice)) {
                seenDevices.add(baseDevice);
                pendingLabels.push({
                  device: device,
                  total: totalSize,
                  available: availableSize,
                  mountPoint: mountPoint
                });
              }
            } else if (mountPoint === '/home' && !seenDevices.has(baseDevice)) {
              
              const rootDevice = lines.find(l => l.includes(' / '));
              if (rootDevice) {
                const rootParts = rootDevice.trim().split(/\s+/);
                const rootBaseDevice = rootParts[0] ? rootParts[0].replace(/[0-9]+$/, '') : '';
                if (baseDevice !== rootBaseDevice) {
                  seenDevices.add(baseDevice);
                  pendingLabels.push({
                    device: device,
                    total: totalSize,
                    available: availableSize,
                    mountPoint: mountPoint
                  });
                }
              }
            }
          }
        });
        
        
        if (pendingLabels.length === 0) {
          exec('df -h / 2>/dev/null | tail -1', (dfError, dfStdout) => {
            if (!dfError && dfStdout) {
              const parts = dfStdout.trim().split(/\s+/);
              if (parts.length >= 6) {
                exec(`blkid -s LABEL -a value ${parts[0]} 2>/dev/null || lsblk -dno LABEL ${parts[0]} 2>/dev/null || echo ""`, (labelError, labelStdout) => {
                  const label = labelStdout ? labelStdout.trim() : '';
                  
                  const displayName = label || parts[0];
                  resolve([{
                    name: displayName,
                    device: parts[0],
                    total: parts[1],
                    available: parts[3],
                    mountPoint: '/'
                  }]);
                });
              } else {
                resolve([{
                  name: '/dev/sda',
                  device: '/dev/sda',
                  total: 'Unknown',
                  available: 'Unknown',
                  mountPoint: '/'
                }]);
              }
            } else {
              resolve([{
                name: '/dev/sda',
                device: '/dev/sda',
                total: 'Unknown',
                available: 'Unknown',
                mountPoint: '/'
              }]);
            }
          });
        } else {
          
          let completed = 0;
          pendingLabels.forEach(storage => {
            exec(`blkid -s LABEL -a value ${storage.device} 2>/dev/null || lsblk -dno LABEL ${storage.device} 2>/dev/null || echo ""`, (labelError, labelStdout) => {
              const label = labelStdout ? labelStdout.trim() : '';
              
              const displayName = label || storage.device;
              storages.push({
                name: displayName,
                device: storage.device,
                total: storage.total,
                available: storage.available,
                mountPoint: storage.mountPoint
              });
              
              completed++;
              if (completed === pendingLabels.length) {
                resolve(storages);
              }
            });
          });
        }
      } else {
        
        resolve([{
          name: '/dev/sda',
          device: '/dev/sda',
          total: 'Unknown',
          available: 'Unknown',
          mountPoint: '/'
        }]);
      }
    });
  });
});

ipcMain.handle('get-available-updates', async () => {
  return new Promise((resolve) => {
    
    exec('pacman -Qu 2>/dev/null', (error, stdout) => {
      if (error) {
        
        if (error.code === 1) {
          resolve([]);
        } else {
          console.error('Error getting updates:', error);
          resolve([]);
        }
        return;
      }

      if (!stdout || !stdout.trim()) {
        resolve([]);
        return;
      }

      const updates = [];
      const lines = stdout.trim().split('\n');
      
      lines.forEach(line => {
        
        const match = line.match(/^([^\s]+)\s+([^\s]+)\s+->\s+([^\s]+)/);
        if (match) {
          updates.push({
            name: match[1],
            oldVersion: match[2],
            newVersion: match[3]
          });
        }
      });

      resolve(updates);
    });
  });
});

ipcMain.handle('update-system', async (event, password) => {
  return new Promise((resolve, reject) => {
    
    const command = `echo '${password.replace(/'/g, "'\\''")}' | sudo -S pacman -Syu --noconfirm 2>&1`;
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(stderr || error.message));
        return;
      }
      resolve({ success: true, output: stdout });
    });
  });
});

ipcMain.handle('schedule-update', async (event, password) => {
  return new Promise((resolve, reject) => {
    
    const scriptPath = '/tmp/pearos-update.sh';
    const expectScriptPath = '/tmp/pearos-update-expect.sh';
    
    
    const expectScriptContent = `#!/usr/bin/expect -f
set timeout 300
spawn pacman -Syu --noconfirm
expect "password"
send "${password.replace(/"/g, '\\"')}\\r"
expect eof
`;
    
    
    const scriptContent = `#!/bin/bash
${expectScriptPath}
`;
    
    try {
      
      fs.writeFileSync(expectScriptPath, expectScriptContent);
      fs.writeFileSync(scriptPath, scriptContent);
      fs.chmodSync(expectScriptPath, '700');
      fs.chmodSync(scriptPath, '755');

      
      const cronCommand = `(crontab -l 2>/dev/null | grep -v "${scriptPath}"; echo "0 1 * * * ${scriptPath}") | crontab -`;
      exec(cronCommand, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(stderr || error.message));
          return;
        }
        resolve({ success: true });
      });
    } catch (err) {
      reject(new Error(err.message));
    }
  });
});

ipcMain.handle('get-installed-system-info', async () => {
  return new Promise((resolve) => {
    let prettyName = 'pearOS';
    let release = 'Unknown';
    
    
    exec('grep "^PRETTY_NAME=" /etc/os-release 2>/dev/null', (error1, stdout1) => {
      if (!error1 && stdout1) {
        const match = stdout1.match(/PRETTY_NAME="?([^"]+)"?/);
        if (match) {
          prettyName = match[1].trim();
        }
      }
      
      
      exec('grep "^DISTRIB_RELEASE=" /etc/lsb-release 2>/dev/null', (error2, stdout2) => {
        if (!error2 && stdout2) {
          const match = stdout2.match(/DISTRIB_RELEASE="?([^"]+)"?/);
          if (match) {
            release = match[1].trim();
          }
        } else {
          
          exec('grep "^VERSION_ID=" /etc/os-release 2>/dev/null', (error3, stdout3) => {
            if (!error3 && stdout3) {
              const match = stdout3.match(/VERSION_ID="?([^"]+)"?/);
              if (match) {
                release = match[1].trim();
              }
            }
            resolve({ prettyName, release });
          });
        }
        
        if (!error2 && stdout2) {
          resolve({ prettyName, release });
        }
      });
    });
  });
});

ipcMain.handle('get-storage-info', async () => {
  return new Promise((resolve) => {
    
    exec('df -h --si / | tail -1', (error, stdout) => {
      if (error || !stdout) {
        resolve({ name: 'Unknown', device: '/dev/sda1', total: '0', used: '0', available: '0' });
        return;
      }

      const parts = stdout.trim().split(/\s+/);
      const total = parts[1] || '0';
      const used = parts[2] || '0';
      const available = parts[3] || '0';
      const mountPoint = parts[5] || '/';

      
      exec('lsblk -dno NAME,LABEL /dev/sda 2>/dev/null | head -1', (error2, stdout2) => {
        let name = 'Unknown';
        let device = '/dev/sda1';

        if (!error2 && stdout2) {
          const diskInfo = stdout2.trim().split(/\s+/);
          if (diskInfo.length >= 2 && diskInfo[1]) {
            name = diskInfo[1];
          }
          if (diskInfo[0]) {
            device = `/dev/${diskInfo[0]}`;
          }
        }

        
        if (name === 'Unknown' || !name) {
          exec('blkid -s LABEL -a value /dev/sda1 2>/dev/null || echo ""', (error3, stdout3) => {
            if (!error3 && stdout3 && stdout3.trim()) {
              name = stdout3.trim();
            } else {
              name = device;
            }
            resolve({ name, device, total, used, available, mountPoint });
          });
        } else {
          resolve({ name, device, total, used, available, mountPoint });
        }
      });
    });
  });
});

ipcMain.handle('get-storage-analysis', async () => {
  return new Promise((resolve) => {
    
    const analysis = {
      applications: 0,
      documents: 0,
      photos: 0,
      downloads: 0,
      desktop: 0,
      system: 0,
      pearos: 0,
      total: 0
    };

    
    exec('du -sb /usr/share/applications /usr/bin /opt 2>/dev/null | awk \'{sum+=$1} END {print sum/1024/1024/1024}\'', (error1, stdout1) => {
      if (!error1 && stdout1) {
        analysis.applications = parseFloat(stdout1.trim()) || 0;
      }

      
      exec('du -sb /home/*/Documents 2>/dev/null | awk \'{sum+=$1} END {print sum/1024/1024/1024}\'', (error2, stdout2) => {
        if (!error2 && stdout2) {
          analysis.documents = parseFloat(stdout2.trim()) || 0;
        }

        
        exec('du -sb /home/*/Pictures 2>/dev/null | awk \'{sum+=$1} END {print sum/1024/1024/1024}\'', (error3, stdout3) => {
          if (!error3 && stdout3) {
            analysis.photos = parseFloat(stdout3.trim()) || 0;
          }

          
          exec('du -sb /home/*/Downloads 2>/dev/null | awk \'{sum+=$1} END {if (sum) print sum/1024/1024/1024; else print 0}\'', (error4, stdout4) => {
            
            if (stdout4 && stdout4.trim()) {
              const downloadsValue = parseFloat(stdout4.trim());
              if (!isNaN(downloadsValue) && downloadsValue > 0) {
                analysis.downloads = downloadsValue;
              }
            }

            
            exec('du -sb /home/*/Desktop 2>/dev/null | awk \'{sum+=$1} END {if (sum) print sum/1024/1024/1024; else print 0}\'', (error5, stdout5) => {
              
              if (stdout5 && stdout5.trim()) {
                const desktopValue = parseFloat(stdout5.trim());
                if (!isNaN(desktopValue) && desktopValue > 0) {
                  analysis.desktop = desktopValue;
                }
              }

              
              
              exec('du -sb /var /etc /boot /usr/lib /usr/share /usr/include /usr/src 2>/dev/null | awk \'{sum+=$1} END {print sum/1024/1024/1024}\'', (error6, stdout6) => {
                if (!error6 && stdout6) {
                  const systemTotal = parseFloat(stdout6.trim()) || 0;
                  
                  
                  
                  analysis.system = systemTotal;
                } else {
                  analysis.system = 0;
                }

                
                exec('test -d /usr/share/extras && du -sb /usr/share/extras 2>/dev/null | awk \'{print $1}\' || echo 0', (error7, stdout7) => {
                  if (!error7 && stdout7 && stdout7.trim()) {
                    const pearosBytes = parseFloat(stdout7.trim());
                    if (!isNaN(pearosBytes) && pearosBytes > 0) {
                      
                      analysis.pearos = pearosBytes;
                    }
                  }

                  
                  exec('df -B1 / | tail -1 | awk \'{print $3/1024/1024/1024}\'', (error8, stdout8) => {
                    if (!error8 && stdout8) {
                      analysis.total = parseFloat(stdout8.trim()) || 0;
                    } else {
                      
                      analysis.total = analysis.applications + analysis.documents + analysis.photos + analysis.downloads + analysis.desktop + analysis.system;
                    }

                    resolve(analysis);
                  });
                });
              });
            });
          });
        });
      });
    });
  });
});

ipcMain.handle('get-battery-usage-data', async (event, period) => {
  return new Promise((resolve) => {
    
    const is24Hours = period === '24h';
    const dataPoints = is24Hours ? 24 : 10;
    
    
    const batteryLevel = [];
    const screenUsage = [];
    
    for (let i = 0; i < dataPoints; i++) {
      
      const batteryValue = 20 + Math.random() * 80;
      batteryLevel.push({
        time: is24Hours ? `${i}:00` : `Day ${i + 1}`,
        value: Math.round(batteryValue)
      });
      
      
      const usageMinutes = Math.random() * 480;
      screenUsage.push({
        time: is24Hours ? `${i}:00` : `Day ${i + 1}`,
        value: Math.round(usageMinutes)
      });
    }
    
    resolve({
      batteryLevel: batteryLevel,
      screenUsage: screenUsage
    });
  });
});

ipcMain.on('window-close', () => {
  if (mainWindow) {
    mainWindow.close();
  }
});

ipcMain.on('window-minimize', () => {
  if (mainWindow) {
    mainWindow.minimize();
  }
});

let isHeightMaximized = false;
let previousHeight = 600;
let previousY = 0;

ipcMain.on('window-maximize', () => {
  if (mainWindow) {
    if (isHeightMaximized) {
      const bounds = mainWindow.getBounds();
      mainWindow.setBounds({
        x: bounds.x,
        y: previousY,
        width: bounds.width,
        height: previousHeight
      });
      isHeightMaximized = false;
    } else {
      const bounds = mainWindow.getBounds();
      previousHeight = bounds.height;
      previousY = bounds.y;
      
      const primaryDisplay = screen.getPrimaryDisplay();
      const { height: screenHeight } = primaryDisplay.workAreaSize;
      
      mainWindow.setBounds({
        x: bounds.x,
        y: 0,
        width: bounds.width,
        height: screenHeight
      });
      isHeightMaximized = true;
    }
  }
});

ipcMain.handle('get-timezones', async () => {
  return new Promise((resolve, reject) => {
    exec('timedatectl list-timezones', (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting timezones: ${error.message}`));
        return;
      }
      
      
      const timezones = stdout.trim().split('\n').filter(tz => tz.length > 0);
      resolve({ timezones });
    });
  });
});

ipcMain.handle('get-current-timezone', async () => {
  return new Promise((resolve, reject) => {
    exec('timedatectl show', (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting current timezone: ${error.message}`));
        return;
      }
      
      
      const timezoneMatch = stdout.match(/Timezone=([^\n]+)/);
      if (timezoneMatch && timezoneMatch[1]) {
        resolve({ timezone: timezoneMatch[1].trim() });
      } else {
        reject(new Error('Could not parse current timezone'));
      }
    });
  });
});

ipcMain.handle('set-timezone', async (event, timezone) => {
  return new Promise((resolve, reject) => {
    const command = `timedatectl set-timezone "${timezone}"`;
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting timezone: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('set-display-resolution', async (event, displayName, resolutionType) => {
  return new Promise((resolve, reject) => {
    console.log(`Setting resolution for ${displayName} to ${resolutionType}`);
    
    exec('xrandr 2>/dev/null', (error, stdout) => {
      if (error || !stdout) {
        reject(new Error('Could not get display information'));
        return;
      }
      
      const lines = stdout.split('\n');
      let currentDisplay = null;
      const availableResolutions = [];
      
      lines.forEach(line => {
        const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
        if (displayMatch) {
          currentDisplay = displayMatch[1];
        } else if (currentDisplay === displayName && line.match(/\s+[0-9]+x[0-9]+/)) {
          const resolutionMatch = line.match(/([0-9]+x[0-9]+)/);
          if (resolutionMatch) {
            const resolution = resolutionMatch[1];
            const [width, height] = resolution.split('x').map(Number);
            availableResolutions.push({
              resolution: resolution,
              width: width,
              height: height,
              totalPixels: width * height
            });
          }
        }
      });
      
      if (availableResolutions.length === 0) {
        reject(new Error('No resolutions available for this display'));
        return;
      }
      
      availableResolutions.sort((a, b) => b.totalPixels - a.totalPixels);
      
      let selectedResolution = null;
      
      if (resolutionType === 'default') {
        selectedResolution = availableResolutions[0];
      } else if (resolutionType === 'medium') {
        const middleIndex = Math.floor(availableResolutions.length / 2);
        selectedResolution = availableResolutions[middleIndex];
      } else if (resolutionType === 'larger') {
        selectedResolution = availableResolutions[availableResolutions.length - 1];
      }
      
      if (!selectedResolution) {
        reject(new Error('Could not determine resolution'));
        return;
      }
      
      console.log(`Setting resolution to ${selectedResolution.resolution}`);
      exec(`xrandr --output ${displayName} --mode ${selectedResolution.resolution}`, (setError, setStdout, setStderr) => {
        if (setError) {
          console.error(`Error setting resolution: ${setError.message || setStderr}`);
          reject(new Error(`Error setting resolution: ${setError.message || setStderr}`));
          return;
        }
        console.log(`Resolution set successfully to ${selectedResolution.resolution}`);
        resolve({ success: true, resolution: selectedResolution.resolution });
      });
    });
  });
});

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    
    
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

ipcMain.handle('open-kwallet', async () => {
  return new Promise((resolve, reject) => {
    exec('xdg-open kwalletmanager5 || kwalletmanager5', (error, stdout, stderr) => {
      if (error) {
        
        exec('kwalletmanager5', (error2) => {
          if (error2) {
            reject(new Error('KWallet could not be opened'));
            return;
          }
          resolve({ success: true });
        });
        return;
      }
      resolve({ success: true });
    });
  });
});

// Funcții helper pentru cache PearID
function getCachePath() {
  const homeDir = process.env.HOME || process.env.USERPROFILE;
  const cacheDir = path.join(homeDir, '.local', 'share', 'extras', 'org.pearos.systemsettings');
  return path.join(cacheDir, 'userdata.json');
}

function readCache() {
  try {
    const cachePath = getCachePath();
    if (fs.existsSync(cachePath)) {
      const data = fs.readFileSync(cachePath, 'utf8');
      return JSON.parse(data);
    }
  } catch (error) {
    console.error('Error reading cache:', error);
  }
  return null;
}

function writeCache(data) {
  try {
    const cachePath = getCachePath();
    const cacheDir = path.dirname(cachePath);
    
    // Creează directorul dacă nu există
    if (!fs.existsSync(cacheDir)) {
      fs.mkdirSync(cacheDir, { recursive: true });
    }
    
    const cacheData = {
      ...data,
      lastFetched: new Date().toISOString()
    };
    
    fs.writeFileSync(cachePath, JSON.stringify(cacheData, null, 2), 'utf8');
  } catch (error) {
    console.error('Error writing cache:', error);
  }
}

function isCacheValid(cache) {
  if (!cache || !cache.lastFetched) {
    return false;
  }
  
  const lastFetched = new Date(cache.lastFetched);
  const now = new Date();
  const diffInMs = now - lastFetched;
  const diffInDays = diffInMs / (1000 * 60 * 60 * 24);
  
  // Cache-ul este valid dacă nu a trecut mai mult de o zi
  return diffInDays < 1;
}

ipcMain.handle('check-pearid-state', async () => {
  return new Promise((resolve, reject) => {
    const stateScriptPath = path.join(__dirname, 'pearID', 'state.sh');
    exec(`bash "${stateScriptPath}"`, (error, stdout, stderr) => {
      if (error) {
        resolve({ state: 'false' });
        return;
      }
      const result = stdout.trim().toLowerCase();
      resolve({ state: result });
    });
  });
});

ipcMain.handle('check-pearid-lock', async () => {
  return new Promise((resolve) => {
    const lockScriptPath = path.join(__dirname, 'pearID', 'pearid-lock.sh');
    exec(`bash "${lockScriptPath}"`, (error, stdout, stderr) => {
      if (error) {
        resolve({ email: '' });
        return;
      }
      const result = (stdout || '').trim();
      resolve({ email: result });
    });
  });
});

ipcMain.handle('get-user-info', async () => {
  return new Promise((resolve, reject) => {
    // Verifică cache-ul mai întâi
    const cache = readCache();
    if (cache && isCacheValid(cache) && cache.firstName && cache.lastName && cache.email) {
      resolve({ 
        lastName: cache.lastName, 
        firstName: cache.firstName,
        email: cache.email
      });
      return;
    }
    
    // Dacă cache-ul nu este valid, descarcă de pe server
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --last-name --first-name --email`, (error, stdout, stderr) => {
      if (error) {
        // Dacă există cache vechi, folosește-l chiar dacă nu e valid
        if (cache && cache.firstName && cache.lastName) {
          resolve({ 
            lastName: cache.lastName, 
            firstName: cache.firstName,
            email: cache.email || ''
          });
        } else {
          reject(new Error(`Error getting user info: ${error.message}`));
        }
        return;
      }
      const lines = stdout.trim().split('\n').filter(line => line.trim());
      // Scriptul afișează mai întâi first name, apoi last name, apoi email (conform ordinii de procesare)
      const firstName = lines[0] || '';
      const lastName = lines[1] || '';
      const email = lines[2] || '';
      
      // Salvează în cache (inclusiv email-ul)
      writeCache({ firstName, lastName, email });
      
      resolve({ lastName: lastName, firstName: firstName, email: email });
    });
  });
});

ipcMain.handle('get-user-avatar', async () => {
  return new Promise((resolve, reject) => {
    // Verifică cache-ul pentru avatar
    const cache = readCache();
    const homeDir = process.env.HOME || process.env.USERPROFILE;
    const avatarPath = path.join(homeDir, '.pearid_avatars', 'avatar.webp');
    
    // Verifică dacă avatarul există și cache-ul este valid
    // IMPORTANT: Verifică și dacă cache-ul conține email-ul pentru a se asigura că este pentru contul corect
    if (cache && isCacheValid(cache) && fs.existsSync(avatarPath)) {
      // Verifică dacă cache-ul are email (pentru a se asigura că este pentru contul corect)
      if (cache.email) {
        resolve({ avatarPath: avatarPath });
        return;
      } else {
        // Dacă cache-ul nu are email, este posibil să fie vechi, șterge-l
        try {
          if (fs.existsSync(avatarPath)) {
            fs.unlinkSync(avatarPath);
          }
        } catch (err) {
          console.error('Error deleting old avatar:', err);
        }
      }
    }
    
    // Dacă cache-ul nu este valid sau avatarul nu există, descarcă
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --avatar`, (error, stdout, stderr) => {
      if (error) {
        // Nu folosim avatar vechi dacă comanda eșuează - utilizatorul poate nu are avatar
        resolve({ avatarPath: null });
        return;
      }
      
      // Verifică dacă avatarul a fost descărcat
      fs.access(avatarPath, fs.constants.F_OK, (err) => {
        if (err) {
          // Avatarul nu există sau nu a fost descărcat
          resolve({ avatarPath: null });
        } else {
          // Actualizează cache-ul pentru a marca că avatarul a fost descărcat
          const currentCache = readCache() || {};
          writeCache({ ...currentCache, avatarDownloaded: true });
          
          resolve({ avatarPath: avatarPath });
        }
      });
    });
  });
});

ipcMain.handle('get-user-devices', async () => {
  return new Promise((resolve, reject) => {
    // NU folosim cache pentru device-uri, se rulează de fiecare dată
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --devices`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting user devices: ${error.message}`));
        return;
      }
      
      // Parsează output-ul: fiecare linie este un device, device-ul curent are * la sfârșit
      const lines = stdout.trim().split('\n').filter(line => line.trim());
      const devices = lines.map(line => {
        const trimmed = line.trim();
        const isCurrent = trimmed.endsWith(' *');
        const deviceName = isCurrent ? trimmed.slice(0, -2).trim() : trimmed;
        return {
          name: deviceName,
          isCurrent: isCurrent
        };
      });
      
      resolve({ devices: devices });
    });
  });
});

ipcMain.handle('get-user-birthdate', async () => {
  return new Promise((resolve, reject) => {
    // Verifică cache-ul mai întâi
    const cache = readCache();
    if (cache && cache.birthdate) {
      resolve({ birthdate: cache.birthdate });
      return;
    }
    
    // Dacă birthdate nu există în cache, descarcă de pe server
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --birthdate`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting birthdate: ${error.message}`));
        return;
      }
      
      const birthdate = stdout.trim() || '';
      
      // Actualizează cache-ul cu birthdate
      const currentCache = readCache() || {};
      writeCache({ ...currentCache, birthdate: birthdate });
      
      resolve({ birthdate: birthdate });
    });
  });
});

ipcMain.handle('get-user-phone', async () => {
  return new Promise((resolve, reject) => {
    // Verifică cache-ul mai întâi
    const cache = readCache();
    if (cache && cache.phone) {
      resolve({ phone: cache.phone });
      return;
    }
    
    // Dacă phone nu există în cache, descarcă de pe server
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --phone`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting phone: ${error.message}`));
        return;
      }
      
      const phone = stdout.trim() || '';
      
      // Actualizează cache-ul cu phone
      const currentCache = readCache() || {};
      writeCache({ ...currentCache, phone: phone });
      
      resolve({ phone: phone });
    });
  });
});

ipcMain.handle('get-user-billing-address', async () => {
  return new Promise((resolve, reject) => {
    // Verifică cache-ul mai întâi
    const cache = readCache();
    if (cache && cache.billingAddress) {
      resolve({ billingAddress: cache.billingAddress });
      return;
    }
    
    // Dacă billingAddress nu există în cache, descarcă de pe server
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --billing-address`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting billing address: ${error.message}`));
        return;
      }
      
      const billingAddress = stdout.trim() || '';
      
      // Actualizează cache-ul cu billingAddress
      const currentCache = readCache() || {};
      writeCache({ ...currentCache, billingAddress: billingAddress });
      
      resolve({ billingAddress: billingAddress });
    });
  });
});

ipcMain.handle('get-user-apps', async () => {
  return new Promise((resolve, reject) => {
    // Verifică cache-ul mai întâi
    const cache = readCache();
    if (cache && cache.apps && Array.isArray(cache.apps) && cache.apps.length > 0) {
      resolve({ apps: cache.apps });
      return;
    }
    
    // Dacă apps nu există în cache, descarcă de pe server
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --apps`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting apps: ${error.message}`));
        return;
      }
      
      // Parsează output-ul pentru a extrage aplicațiile
      const lines = stdout.split('\n');
      const appsSet = new Set();
      
      // Caută liniile care conțin aplicații (format: • com.archlinux.appname sau com.archlinux.appname)
      for (const line of lines) {
        // Elimină codurile ANSI (culori)
        const cleanLine = line.replace(/\x1b\[[0-9;]*m/g, '').trim();
        
        // Verifică dacă linia conține o aplicație
        // Format: • com.archlinux.appname sau com.archlinux.appname sau ar.xjuan.Cambalache
        if (cleanLine.startsWith('•') || /^[a-z]+\.[a-z0-9.-]+\.[a-z0-9.-]+/.test(cleanLine)) {
          // Extrage numele aplicației
          let appName = cleanLine;
          
          // Elimină bullet point dacă există
          if (appName.startsWith('•')) {
            appName = appName.substring(1).trim();
          }
          
          // Elimină partea "(on X devices) - last synced: ..." dacă există
          const match = appName.match(/^([a-z]+\.[a-z0-9.-]+\.[a-z0-9.-]+(?:\.[a-z0-9.-]+)*)/);
          if (match && match[1]) {
            appsSet.add(match[1]);
          } else {
            // Încearcă să extragă direct numele aplicației
            const parts = appName.split(' ');
            if (parts.length > 0 && /^[a-z]+\.[a-z0-9.-]+/.test(parts[0])) {
              appsSet.add(parts[0]);
            }
          }
        }
      }
      
      // Convertește Set-ul în array sortat (elimină duplicatele automat)
      const apps = Array.from(appsSet).sort();
      
      // Actualizează cache-ul cu apps
      const currentCache = readCache() || {};
      writeCache({ ...currentCache, apps: apps });
      
      resolve({ apps: apps });
    });
  });
});

ipcMain.handle('get-user-country', async () => {
  return new Promise((resolve, reject) => {
    const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
    exec(`bash "${userInfoScriptPath}" --country`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting country: ${error.message}`));
        return;
      }
      
      const country = stdout.trim() || '';
      resolve({ country: country });
    });
  });
});

ipcMain.handle('update-user-phone', async (event, phone) => {
  return new Promise((resolve) => {
    const updateScriptPath = path.join(__dirname, 'pearID', 'update_user_info.sh');
    
    // Escapăm valoarea pentru shell
    const escapedPhone = phone.replace(/"/g, '\\"');
    
    exec(`bash "${updateScriptPath}" --phone "${escapedPhone}"`, (error, stdout, stderr) => {
      if (error) {
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Failed to update phone number.';
        
        if (output.includes('Error') || output.includes('Failed')) {
          const match = output.match(/(Error|Failed):\s*(.+)/);
          if (match && match[2]) {
            errorMessage = match[2];
          }
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Error') || output.includes('Failed')) {
        let errorMessage = 'Failed to update phone number.';
        const match = output.match(/(Error|Failed):\s*(.+)/);
        if (match && match[2]) {
          errorMessage = match[2];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Șterge cache-ul după actualizarea telefonului
      try {
        const cachePath = getCachePath();
        if (fs.existsSync(cachePath)) {
          fs.unlinkSync(cachePath);
          console.log('Cache deleted successfully after phone update');
        }
      } catch (cacheError) {
        console.error('Error deleting cache:', cacheError);
        // Continuă chiar dacă ștergerea cache-ului eșuează
      }
      
      resolve({ success: true });
    });
  });
});

ipcMain.handle('update-user-billing-address', async (event, billingAddress) => {
  return new Promise((resolve) => {
    const updateScriptPath = path.join(__dirname, 'pearID', 'update_user_info.sh');
    
    // Escapăm valoarea pentru shell
    const escapedAddress = billingAddress.replace(/"/g, '\\"').replace(/\$/g, '\\$');
    
    exec(`bash "${updateScriptPath}" --billing-address "${escapedAddress}"`, (error, stdout, stderr) => {
      if (error) {
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Failed to update billing address.';
        
        if (output.includes('Error') || output.includes('Failed')) {
          const match = output.match(/(Error|Failed):\s*(.+)/);
          if (match && match[2]) {
            errorMessage = match[2];
          }
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Error') || output.includes('Failed')) {
        let errorMessage = 'Failed to update billing address.';
        const match = output.match(/(Error|Failed):\s*(.+)/);
        if (match && match[2]) {
          errorMessage = match[2];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Șterge cache-ul după actualizarea adresei
      try {
        const cachePath = getCachePath();
        if (fs.existsSync(cachePath)) {
          fs.unlinkSync(cachePath);
          console.log('Cache deleted successfully after billing address update');
        }
      } catch (cacheError) {
        console.error('Error deleting cache:', cacheError);
        // Continuă chiar dacă ștergerea cache-ului eșuează
      }
      
      resolve({ success: true });
    });
  });
});

ipcMain.handle('verify-sudo-password', async (event, password) => {
  return new Promise((resolve) => {
    // Verifică parola sudo cu o comandă harmless - folosim același model ca la updateSystem
    const command = `echo '${password.replace(/'/g, "'\\''")}' | sudo -S whoami 2>&1`;
    
    exec(command, (error, stdout, stderr) => {
      if (error || stderr.includes('Sorry')) {
        resolve({ success: false, error: 'Incorrect password. Please try again.' });
        return;
      }
      
      // Verifică dacă output-ul conține "root"
      const output = (stdout || '').trim();
      if (output === 'root') {
        resolve({ success: true });
      } else {
        resolve({ success: false, error: 'Incorrect password. Please try again.' });
      }
    });
  });
});

ipcMain.handle('update-user-info', async (event, firstName, lastName) => {
  return new Promise((resolve, reject) => {
    const updateScriptPath = path.join(__dirname, 'pearID', 'update_user_info.sh');
    
    // Escapăm valorile pentru shell
    const escapedFirstName = firstName.replace(/"/g, '\\"');
    const escapedLastName = lastName.replace(/"/g, '\\"');
    
    exec(`bash "${updateScriptPath}" --first-name "${escapedFirstName}" --last-name "${escapedLastName}"`, (error, stdout, stderr) => {
      if (error) {
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Failed to update user information.';
        
        if (output.includes('Error') || output.includes('Failed')) {
          const match = output.match(/(Error|Failed):\s*(.+)/);
          if (match && match[2]) {
            errorMessage = match[2];
          }
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Error') || output.includes('Failed')) {
        let errorMessage = 'Failed to update user information.';
        const match = output.match(/(Error|Failed):\s*(.+)/);
        if (match && match[2]) {
          errorMessage = match[2];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Șterge cache-ul după actualizarea numelui
      try {
        const cachePath = getCachePath();
        if (fs.existsSync(cachePath)) {
          fs.unlinkSync(cachePath);
          console.log('Cache deleted successfully after name update');
        }
      } catch (cacheError) {
        console.error('Error deleting cache:', cacheError);
        // Continuă chiar dacă ștergerea cache-ului eșuează
      }
      
      resolve({ success: true });
    });
  });
});

ipcMain.handle('update-user-avatar', async (event, avatarPath) => {
  return new Promise((resolve) => {
    const updateScriptPath = path.join(__dirname, 'pearID', 'update_user_info.sh');
    
    // Escapăm calea pentru shell
    const escapedAvatarPath = avatarPath.replace(/"/g, '\\"');
    
    exec(`bash "${updateScriptPath}" --avatar "${escapedAvatarPath}"`, (error, stdout, stderr) => {
      if (error) {
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Failed to update avatar.';
        
        if (output.includes('Error') || output.includes('Failed')) {
          const match = output.match(/(Error|Failed):\s*(.+)/);
          if (match && match[2]) {
            errorMessage = match[2];
          }
        } else if (output) {
          errorMessage = output;
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Error') || output.includes('Failed')) {
        let errorMessage = 'Failed to update avatar.';
        const match = output.match(/(Error|Failed):\s*(.+)/);
        if (match && match[2]) {
          errorMessage = match[2];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Șterge cache-ul JSON după actualizarea avatar-ului
      try {
        const cachePath = getCachePath();
        if (fs.existsSync(cachePath)) {
          fs.unlinkSync(cachePath);
          console.log('Cache deleted successfully after avatar update');
        }
      } catch (cacheError) {
        console.error('Error deleting cache:', cacheError);
        // Continuă chiar dacă ștergerea cache-ului eșuează
      }
      
      // Șterge fișierul avatar-ului vechi pentru a forța reîncărcarea
      try {
        const homeDir = process.env.HOME || process.env.USERPROFILE;
        const oldAvatarPath = path.join(homeDir, '.pearid_avatars', 'avatar.webp');
        if (fs.existsSync(oldAvatarPath)) {
          fs.unlinkSync(oldAvatarPath);
          console.log('Old avatar file deleted successfully');
        }
      } catch (avatarError) {
        console.error('Error deleting old avatar file:', avatarError);
        // Continuă chiar dacă ștergerea avatar-ului vechi eșuează
      }
      
      // Reîncarcă avatar-ul din server
      try {
        const userInfoScriptPath = path.join(__dirname, 'pearID', 'get_user_info.sh');
        exec(`bash "${userInfoScriptPath}" --avatar`, (avatarError, avatarStdout, avatarStderr) => {
          if (avatarError) {
            console.error('Error reloading avatar from server:', avatarError);
            // Continuă chiar dacă reîncărcarea eșuează
          } else {
            console.log('Avatar reloaded from server successfully');
          }
          resolve({ success: true });
        });
      } catch (reloadError) {
        console.error('Error in avatar reload process:', reloadError);
        resolve({ success: true }); // Totuși considerăm actualizarea reușită
      }
    });
  });
});

ipcMain.handle('show-open-dialog', async (event, options) => {
  const result = await dialog.showOpenDialog(mainWindow, options);
  return result;
});

ipcMain.handle('update-user-password', async (event, oldPassword, newPassword) => {
  return new Promise((resolve) => {
    const updateScriptPath = path.join(__dirname, 'pearID', 'update_user_info.sh');
    
    // Escapăm valorile pentru shell
    const escapedOldPassword = oldPassword.replace(/"/g, '\\"').replace(/\$/g, '\\$');
    const escapedNewPassword = newPassword.replace(/"/g, '\\"').replace(/\$/g, '\\$');
    
    exec(`bash "${updateScriptPath}" --old-password "${escapedOldPassword}" --new-password "${escapedNewPassword}"`, (error, stdout, stderr) => {
      // Combină stdout și stderr pentru a captura toate mesajele
      const allOutput = (stdout || '').trim() + (stderr || '').trim();
      
      if (error) {
        // Încearcă să extragă mesajul de eroare din output
        let errorMessage = 'Failed to change password.';
        
        // Caută mesaje de eroare comune
        if (allOutput.includes('incorrect password') || allOutput.includes('wrong password') || allOutput.includes('parola gresita')) {
          errorMessage = 'Incorrect old password.';
        } else if (allOutput.includes('password does not meet') || allOutput.includes('parola aleasa nu e buna') || allOutput.includes('BAD PASSWORD')) {
          errorMessage = 'The chosen password does not meet requirements.';
        } else if (allOutput.includes('Error') || allOutput.includes('Failed')) {
          const match = allOutput.match(/(Error|Failed):\s*(.+)/i);
          if (match && match[2]) {
            errorMessage = match[2].trim();
          }
        } else if (stderr && stderr.trim()) {
          errorMessage = stderr.trim();
        } else if (stdout && stdout.trim() && stdout.includes('Error')) {
          errorMessage = stdout.trim();
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      if (allOutput.includes('Error') || allOutput.includes('Failed') || allOutput.includes('incorrect') || allOutput.includes('wrong')) {
        let errorMessage = 'Failed to change password.';
        
        if (allOutput.includes('incorrect password') || allOutput.includes('wrong password') || allOutput.includes('parola gresita')) {
          errorMessage = 'Incorrect old password.';
        } else if (allOutput.includes('password does not meet') || allOutput.includes('parola aleasa nu e buna')) {
          errorMessage = 'The chosen password does not meet requirements.';
        } else {
          const match = allOutput.match(/(Error|Failed):\s*(.+)/i);
          if (match && match[2]) {
            errorMessage = match[2].trim();
          }
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Dacă schimbarea parolei a reușit, rulează login_and_sync.sh cu email-ul din cache și noua parolă
      try {
        const cache = readCache();
        if (cache && cache.email) {
          const loginScriptPath = path.join(__dirname, 'pearID', 'login_and_sync.sh');
          const escapedEmail = cache.email.replace(/"/g, '\\"').replace(/\$/g, '\\$');
          const escapedNewPassword = newPassword.replace(/"/g, '\\"').replace(/\$/g, '\\$');
          
          // Rulează login_and_sync.sh în background (nu așteptăm rezultatul)
          exec(`bash "${loginScriptPath}" "${escapedEmail}" "${escapedNewPassword}"`, (loginError, loginStdout, loginStderr) => {
            if (loginError) {
              console.error('Error running login_and_sync.sh:', loginError);
              // Nu returnăm eroare pentru că schimbarea parolei a reușit
            } else {
              console.log('Login and sync completed successfully');
            }
          });
        } else {
          console.warn('Email not found in cache, skipping login_and_sync.sh');
        }
      } catch (loginError) {
        console.error('Error executing login_and_sync.sh:', loginError);
        // Nu returnăm eroare pentru că schimbarea parolei a reușit
      }
      
      resolve({ success: true });
    });
  });
});

// Handler pentru schimbarea temei KDE (dark/light)
ipcMain.handle('switch-theme', async (event, mode) => {
  return new Promise((resolve, reject) => {
    if (mode !== 'dark' && mode !== 'light') {
      reject(new Error('Invalid theme mode. Use "dark" or "light".'));
      return;
    }
    
    const themeSwitchScriptPath = path.join(__dirname, 'themeswitcher', 'kde-theme-switch.sh');
    exec(`bash "${themeSwitchScriptPath}" --${mode}`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error switching theme: ${error.message}`));
        return;
      }
      resolve({ success: true, mode: mode });
    });
  });
});

// Handler pentru schimbarea accentului KDE
ipcMain.handle('switch-accent', async (event, colorName) => {
  return new Promise((resolve, reject) => {
    // Lista de culori valide
    const validColors = ['azul', 'blue', 'dark-purple', 'green', 'grey', 'lila', 'magenta', 'orange', 'purple', 'yellow'];
    
    if (!validColors.includes(colorName)) {
      reject(new Error(`Invalid accent color. Valid colors: ${validColors.join(', ')}`));
      return;
    }
    
    const themeSwitchScriptPath = path.join(__dirname, 'themeswitcher', 'kde-theme-switch.sh');
    exec(`bash "${themeSwitchScriptPath}" --accent "${colorName}"`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error switching accent: ${error.message}`));
        return;
      }
      resolve({ success: true, colorName: colorName });
    });
  });
});

// Handler pentru citirea accentului curent
ipcMain.handle('get-accent', async () => {
  return new Promise((resolve, reject) => {
    const accentFile = path.join(__dirname, 'themeswitcher', 'accent');
    
    if (!fs.existsSync(accentFile)) {
      resolve({ accent: null });
      return;
    }
    
    fs.readFile(accentFile, 'utf8', (error, data) => {
      if (error) {
        reject(new Error(`Error reading accent file: ${error.message}`));
        return;
      }
      
      const accent = data.trim();
      resolve({ accent: accent || null });
    });
  });
});

// Handler pentru backup-ul setărilor de temă (state și accent)
ipcMain.handle('backup-theme-settings', async () => {
  const backupScriptPath = path.join(__dirname, 'pearID', 'backup_settings.sh');
  const themeSwitcherDir = path.join(__dirname, 'themeswitcher');
  const stateFile = path.join(themeSwitcherDir, 'state');
  const accentFile = path.join(themeSwitcherDir, 'accent');
  
  const filesToBackup = [];
  if (fs.existsSync(stateFile)) {
    filesToBackup.push({ path: stateFile, name: 'state' });
  }
  if (fs.existsSync(accentFile)) {
    filesToBackup.push({ path: accentFile, name: 'accent' });
  }
  
  if (filesToBackup.length === 0) {
    return { success: true, message: 'No theme settings files to backup' };
  }
  
  const backupPromises = filesToBackup.map(file => {
    return new Promise((resolve, reject) => {
      exec(`bash "${backupScriptPath}" "${file.path}"`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Error backing up ${file.name}: ${error.message}`));
        } else {
          resolve({ file: file.name, success: true });
        }
      });
    });
  });
  
  try {
    const results = await Promise.all(backupPromises);
    return { success: true, backedUp: results.length };
  } catch (error) {
    // Dacă unul dintre backup-uri eșuează, încă returnăm success parțial
    // sau putem arunca eroarea dacă vrem să o gestionăm în frontend
    throw error;
  }
});

ipcMain.handle('sign-out', async (event, password) => {
  return new Promise((resolve) => {
    const exitScriptPath = path.join(__dirname, 'pearID', 'exit.sh');
    
    // Rulează exit.sh --exit cu parola
    exec(`bash "${exitScriptPath}" exit "${password}"`, (error, stdout, stderr) => {
      if (error) {
        // Extrage mesajul de eroare din output
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Authentication failed. Please check your password.';
        
        // Încearcă să extragă mesajul de eroare din output
        if (output.includes('Authentication failed')) {
          const match = output.match(/Authentication failed: (.+)/);
          if (match && match[1]) {
            errorMessage = match[1];
          }
        } else if (output.includes('Incorrect password')) {
          errorMessage = 'Incorrect password. Please try again.';
        } else if (output.includes('Password is required')) {
          errorMessage = 'Password is required.';
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Authentication failed') || output.includes('Error')) {
        let errorMessage = 'Authentication failed. Please check your password.';
        const match = output.match(/Authentication failed: (.+)/);
        if (match && match[1]) {
          errorMessage = match[1];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Șterge cache-ul după deconectare
      try {
        const cachePath = getCachePath();
        if (fs.existsSync(cachePath)) {
          fs.unlinkSync(cachePath);
          console.log('Cache deleted successfully');
        }
      } catch (cacheError) {
        console.error('Error deleting cache:', cacheError);
        // Continuă chiar dacă ștergerea cache-ului eșuează
      }
      
      resolve({ success: true });
    });
  });
});

ipcMain.handle('create-account', async (event, accountData) => {
  return new Promise((resolve) => {
    const createAccountScriptPath = path.join(__dirname, 'pearID', 'create_account.sh');
    
    // Construiește comanda cu parametrii necesari
    let command = `bash "${createAccountScriptPath}"`;
    command += ` --first-name "${accountData.firstName.replace(/"/g, '\\"')}"`;
    command += ` --last-name "${accountData.lastName.replace(/"/g, '\\"')}"`;
    command += ` --email "${accountData.email.replace(/"/g, '\\"')}"`;
    command += ` --password "${accountData.password.replace(/"/g, '\\"')}"`;
    
    // Adaugă parametrii opționali dacă există
    if (accountData.birthdate) {
      command += ` --birthdate "${accountData.birthdate}"`;
    }
    if (accountData.phone) {
      command += ` --phone "${accountData.phone.replace(/"/g, '\\"')}"`;
    }
    if (accountData.billingAddress) {
      command += ` --billing-address "${accountData.billingAddress.replace(/"/g, '\\"')}"`;
    }
    
    exec(command, (error, stdout, stderr) => {
      if (error) {
        // Extrage mesajul de eroare din output
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Failed to create account.';
        
        if (output.includes('Error') || output.includes('Failed')) {
          const match = output.match(/(Error|Failed):\s*(.+)/);
          if (match && match[2]) {
            errorMessage = match[2];
          }
        } else if (output) {
          errorMessage = output;
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Error') || output.includes('Failed')) {
        let errorMessage = 'Failed to create account.';
        const match = output.match(/(Error|Failed):\s*(.+)/);
        if (match && match[2]) {
          errorMessage = match[2];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      resolve({ success: true, message: output || 'Account created successfully' });
    });
  });
});

ipcMain.handle('login-and-sync', async (event, email, password) => {
  return new Promise((resolve) => {
    // Șterge cache-ul vechi și avatar-ul vechi înainte de login
    try {
      const cachePath = getCachePath();
      if (fs.existsSync(cachePath)) {
        fs.unlinkSync(cachePath);
        console.log('Old cache deleted before login');
      }
      
      // Șterge avatar-ul vechi
      const homeDir = process.env.HOME || process.env.USERPROFILE;
      const oldAvatarPath = path.join(homeDir, '.pearid_avatars', 'avatar.webp');
      if (fs.existsSync(oldAvatarPath)) {
        fs.unlinkSync(oldAvatarPath);
        console.log('Old avatar deleted before login');
      }
    } catch (cleanupError) {
      console.error('Error cleaning up old cache/avatar before login:', cleanupError);
      // Continuă chiar dacă cleanup-ul eșuează
    }
    
    const loginScriptPath = path.join(__dirname, 'pearID', 'login_and_sync.sh');
    
    // Escapăm valorile pentru shell
    const escapedEmail = email.replace(/"/g, '\\"').replace(/\$/g, '\\$');
    const escapedPassword = password.replace(/"/g, '\\"').replace(/\$/g, '\\$');
    
    exec(`bash "${loginScriptPath}" "${escapedEmail}" "${escapedPassword}"`, (error, stdout, stderr) => {
      if (error) {
        // Extrage mesajul de eroare din output
        const output = (stdout || stderr || '').trim();
        let errorMessage = 'Authentication failed. Please check your credentials.';
        
        if (output.includes('Authentication failed') || output.includes('Error')) {
          const match = output.match(/(Authentication failed|Error):\s*(.+)/);
          if (match && match[2]) {
            errorMessage = match[2];
          } else if (output.includes('Invalid email format')) {
            errorMessage = 'Invalid email format.';
          } else if (output.includes('401')) {
            errorMessage = 'Incorrect email or password.';
          }
        } else if (output) {
          errorMessage = output;
        }
        
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      // Verifică dacă output-ul conține erori (chiar dacă exit code este 0)
      const output = (stdout || '').trim();
      if (output.includes('Authentication failed') || output.includes('Error')) {
        let errorMessage = 'Authentication failed. Please check your credentials.';
        const match = output.match(/(Authentication failed|Error):\s*(.+)/);
        if (match && match[2]) {
          errorMessage = match[2];
        }
        resolve({ success: false, error: errorMessage });
        return;
      }
      
      resolve({ success: true, message: output || 'Login successful' });
    });
  });
});

ipcMain.handle('get-ntp-status', async () => {
  return new Promise((resolve, reject) => {
    exec('timedatectl status', (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting NTP status: ${error.message}`));
        return;
      }
      
      
      const ntpActive = stdout.includes('NTP service: active');
      resolve({ ntpActive });
    });
  });
});

ipcMain.handle('set-ntp-status', async (event, enabled) => {
  return new Promise((resolve, reject) => {
    const command = `timedatectl set-ntp ${enabled ? 'true' : 'false'}`;
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting NTP status: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('set-date-time', async (event, date, time) => {
  return new Promise((resolve, reject) => {
    
    
    
    const dateTimeString = `${date} ${time}:00`;
    const command = `timedatectl set-time "${dateTimeString}"`;
    
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting date and time: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('get-autostart-items', async () => {
  return new Promise((resolve, reject) => {
    const os = require('os');
    const autostartDir = path.join(os.homedir(), '.config', 'autostart');
    
    
    if (!fs.existsSync(autostartDir)) {
      resolve({ items: [] });
      return;
    }
    
    
    fs.readdir(autostartDir, (error, files) => {
      if (error) {
        reject(new Error(`Error reading autostart directory: ${error.message}`));
        return;
      }
      
      const desktopFiles = files.filter(file => file.endsWith('.desktop'));
      const items = [];
      let processed = 0;
      
      if (desktopFiles.length === 0) {
        resolve({ items: [] });
        return;
      }
      
      desktopFiles.forEach(file => {
        const filePath = path.join(autostartDir, file);
        fs.readFile(filePath, 'utf8', (err, content) => {
          if (err) {
            processed++;
            if (processed === desktopFiles.length) {
              resolve({ items });
            }
            return;
          }
          
          
          const nameMatch = content.match(/^Name=(.+)$/m);
          const typeMatch = content.match(/^Type=(.+)$/m);
          const iconMatch = content.match(/^Icon=(.+)$/m);
          
          const name = nameMatch ? nameMatch[1].trim() : file.replace('.desktop', '');
          const type = typeMatch ? typeMatch[1].trim() : 'Application';
          const icon = iconMatch ? iconMatch[1].trim() : null;
          
          items.push({
            name: name,
            type: type,
            icon: icon,
            file: file
          });
          
          processed++;
          if (processed === desktopFiles.length) {
            resolve({ items });
          }
        });
      });
    });
  });
});

ipcMain.handle('remove-autostart-item', async (event, fileName) => {
  return new Promise((resolve, reject) => {
    const os = require('os');
    const filePath = path.join(os.homedir(), '.config', 'autostart', fileName);
    
    
    if (!fs.existsSync(filePath)) {
      reject(new Error('File does not exist'));
      return;
    }
    
    
    fs.unlink(filePath, (error) => {
      if (error) {
        reject(new Error(`Error removing file: ${error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('add-autostart-item', async (event) => {
  return new Promise((resolve, reject) => {
    if (!mainWindow) {
      reject(new Error('Main window not available'));
      return;
    }
    
    dialog.showOpenDialog(mainWindow, {
      title: 'Select .desktop file',
      filters: [
        { name: 'Desktop Files', extensions: ['desktop'] },
        { name: 'All Files', extensions: ['*'] }
      ],
      properties: ['openFile']
    }).then(result => {
      if (result.canceled) {
        resolve({ canceled: true });
        return;
      }
      
      const selectedFile = result.filePaths[0];
      const os = require('os');
      const autostartDir = path.join(os.homedir(), '.config', 'autostart');
      const fileName = path.basename(selectedFile);
      const destPath = path.join(autostartDir, fileName);
      
      
      if (!fs.existsSync(autostartDir)) {
        fs.mkdirSync(autostartDir, { recursive: true });
      }
      
      
      fs.copyFile(selectedFile, destPath, (error) => {
        if (error) {
          reject(new Error(`Error copying file: ${error.message}`));
          return;
        }
        resolve({ success: true, fileName: fileName });
      });
    }).catch(error => {
      reject(error);
    });
  });
});

ipcMain.handle('get-boot-entries', async () => {
  return new Promise((resolve, reject) => {
    exec('efibootmgr', (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error getting boot entries: ${error.message}`));
        return;
      }
      
      
      const lines = stdout.split('\n');
      const bootEntries = [];
      let bootCurrent = null;
      let bootOrder = null;
      
      lines.forEach(line => {
        
        const currentMatch = line.match(/^BootCurrent:\s*(.+)$/);
        if (currentMatch) {
          bootCurrent = currentMatch[1].trim();
        }
        
        
        const orderMatch = line.match(/^BootOrder:\s*(.+)$/);
        if (orderMatch) {
          bootOrder = orderMatch[1].trim();
        }
        
        
        const bootMatch = line.match(/^Boot([0-9A-Fa-f]{4})\*\s+(.+)$/);
        if (bootMatch) {
          const bootId = bootMatch[1];
          const bootDescription = bootMatch[2].trim();
          
          
          
          let entryName = bootDescription.split(/\t/)[0].split(/\s{2,}/)[0];
          
          
          entryName = entryName
            .replace(/\s+PciRoot\([^)]*\)/g, '')
            .replace(/\s+HD\([^)]*\)/g, '')
            .replace(/\s+UsbWwid\([^)]*\)/g, '')
            .replace(/\s+CDROM\([^)]*\)/g, '')
            .replace(/\\[^\\]*$/g, '') 
            .replace(/\s+RC$/g, '') 
            .trim();
          
          
          
          if (entryName.includes('(')) {
            
            const parenMatch = entryName.match(/\(([^)]+)\)/);
            if (parenMatch) {
              const content = parenMatch[1];
              
              if (content.match(/^[0-9a-fA-F-]+$/) || content.includes('\\') || content.includes('/') || content.includes('0x')) {
                entryName = entryName.split('(')[0].trim();
              }
              
            }
          }
          
          if (entryName.length > 50) {
            entryName = entryName.substring(0, 50) + '...';
          }
          
          
          let diskName = 'Unknown';
          
          
          const diskNameMatch = bootDescription.match(/\(([^)]+)\)/);
          if (diskNameMatch && !bootDescription.includes('PciRoot') && !bootDescription.includes('HD(')) {
            
            diskName = diskNameMatch[1];
          } else {
            
            const hdMatch = bootDescription.match(/HD\([^)]+\)/);
            if (hdMatch) {
              
              const pathMatch = bootDescription.match(/PciRoot\([^)]+\)\/Pci\([^)]+\)\/Sata\(([^)]+)\)/);
              if (pathMatch) {
                diskName = `SATA ${pathMatch[1]}`;
              } else {
                
                const uuidMatch = bootDescription.match(/HD\([^,]+,[^,]+,[^,]+,\s*([^,)]+)/);
                if (uuidMatch) {
                  diskName = 'Hard Drive';
                } else {
                  diskName = 'Hard Drive';
                }
              }
            } else if (bootDescription.includes('USB') || bootDescription.includes('UsbWwid')) {
              diskName = 'USB Device';
            } else if (bootDescription.includes('DVD') || bootDescription.includes('CDROM')) {
              diskName = 'DVD/CDROM';
            } else if (bootDescription.includes('Network')) {
              diskName = 'Network';
            } else {
              
              if (bootDescription.includes('HD(')) {
                diskName = 'Hard Drive';
              }
            }
          }
          
          bootEntries.push({
            id: bootId,
            name: entryName,
            description: bootDescription,
            disk: diskName,
            isCurrent: bootCurrent === bootId
          });
        }
      });
      
      resolve({
        bootCurrent: bootCurrent,
        bootOrder: bootOrder,
        entries: bootEntries
      });
    });
  });
});

ipcMain.handle('set-boot-entry', async (event, bootId, password) => {
  return new Promise((resolve, reject) => {
    
    const command = `echo '${password.replace(/'/g, "'\\''")}' | sudo -S efibootmgr -n ${bootId} 2>&1`;
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(stderr || error.message));
        return;
      }
      
      if (stderr && stderr.includes('Permission denied')) {
        reject(new Error('Permission denied. Please check your sudo privileges.'));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('reboot-system', async (event, password) => {
  return new Promise((resolve, reject) => {
    
    const command = `echo '${password.replace(/'/g, "'\\''")}' | sudo -S systemctl reboot 2>&1`;
    exec(command, (error, stdout, stderr) => {
      
      
      resolve({ success: true });
    });
  });
});

ipcMain.handle('get-brightness', async (event, displayName) => {
  return new Promise((resolve, reject) => {
    // Dacă avem un display specific, folosim xrandr
    if (displayName) {
      exec(`xrandr --verbose | grep -A 10 "^${displayName}" | grep -i brightness | head -1 | awk '{print $2}'`, (xrandrError, xrandrStdout) => {
        if (xrandrError || !xrandrStdout.trim()) {
          resolve({ brightness: 100 }); // Default 100% pentru monitoare externe
          return;
        }
        const brightness = parseFloat(xrandrStdout.trim()) * 100;
        resolve({ brightness: Math.round(brightness) || 100 });
      });
      return;
    }
    
    // Obține valoarea curentă și maximă de brightness (pentru display built-in)
    exec('qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement/Actions/BrightnessControl org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightness', (error, stdout, stderr) => {
      if (error) {
        // Fallback la xrandr
        exec('xrandr --verbose | grep -i brightness | head -1 | awk \'{print $2}\'', (xrandrError, xrandrStdout) => {
          if (xrandrError) {
            resolve({ brightness: 50 });
            return;
          }
          const brightness = parseFloat(xrandrStdout.trim()) * 100;
          resolve({ brightness: Math.round(brightness) || 50 });
        });
        return;
      }
      
      const currentBrightness = parseInt(stdout.trim());
      
      // Obține valoarea maximă pentru a calcula procentul
      exec('qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement/Actions/BrightnessControl org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightnessMax', (maxError, maxStdout) => {
        if (maxError || !maxStdout.trim()) {
          // Dacă nu putem obține max, presupunem că valoarea e deja procent
          resolve({ brightness: isNaN(currentBrightness) ? 50 : Math.min(100, currentBrightness) });
          return;
        }
        
        const maxBrightness = parseInt(maxStdout.trim());
        // Convertește valoarea reală în procent (0-100)
        const brightnessPercent = Math.round((currentBrightness / maxBrightness) * 100);
        resolve({ brightness: isNaN(brightnessPercent) ? 50 : brightnessPercent });
      });
    });
  });
});

ipcMain.handle('set-display-enabled', async (event, displayName, enabled, modeId) => {
  console.log(`set-display-enabled called: displayName=${displayName}, enabled=${enabled}, modeId=${modeId}`);
  return new Promise((resolve, reject) => {
    const setDisplayState = () => {
      console.log(`setDisplayState called for ${displayName}, enabled=${enabled}`);
      let command;
      
      if (enabled) {
        exec('xrandr 2>/dev/null', (checkError, checkStdout) => {
          if (checkError || !checkStdout) {
            if (modeId) {
              exec(`xrandr --output ${displayName} --mode ${modeId}`, (setError, setStdout, setStderr) => {
                if (setError) {
                  reject(new Error(`Error enabling display: ${setError.message || setStderr}`));
                  return;
                }
                setTimeout(() => {
                  resolve({ success: true });
                }, 500);
              });
            } else {
              exec(`xrandr --output ${displayName} --auto`, (setError, setStdout, setStderr) => {
                if (setError) {
                  reject(new Error(`Error enabling display: ${setError.message || setStderr}`));
                  return;
                }
                setTimeout(() => {
                  resolve({ success: true });
                }, 500);
              });
            }
            return;
          }
          
          const lines = checkStdout.split('\n');
          let primaryDisplay = null;
          let currentDisplay = null;
          
          lines.forEach(line => {
            const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
            if (displayMatch) {
              currentDisplay = displayMatch[1];
            } else if (currentDisplay && currentDisplay !== displayName && line.match(/\s+[0-9]+x[0-9]+/)) {
              const isActive = line.includes('*');
              if (isActive && !primaryDisplay) {
                primaryDisplay = currentDisplay;
                console.log(`Found primary display: ${primaryDisplay}`);
              }
            }
          });
          
          if (!primaryDisplay) {
            lines.forEach(line => {
              const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
              if (displayMatch && displayMatch[1] !== displayName) {
                primaryDisplay = displayMatch[1];
                console.log(`Using first connected display as primary: ${primaryDisplay}`);
                return;
              }
            });
          }
          
          if (primaryDisplay && primaryDisplay !== displayName) {
            console.log(`Positioning ${displayName} to the right of ${primaryDisplay}`);
            if (modeId) {
              exec(`xrandr --output ${displayName} --mode ${modeId} --right-of ${primaryDisplay}`, (setError, setStdout, setStderr) => {
                if (setError) {
                  console.error(`Error positioning display: ${setError.message || setStderr}`);
                  reject(new Error(`Error enabling display: ${setError.message || setStderr}`));
                  return;
                }
                console.log(`Display ${displayName} positioned successfully`);
                setTimeout(() => {
                  resolve({ success: true });
                }, 500);
              });
            } else {
              exec(`xrandr --output ${displayName} --auto --right-of ${primaryDisplay}`, (setError, setStdout, setStderr) => {
                if (setError) {
                  console.error(`Error positioning display: ${setError.message || setStderr}`);
                  reject(new Error(`Error enabling display: ${setError.message || setStderr}`));
                  return;
                }
                console.log(`Display ${displayName} positioned successfully`);
                setTimeout(() => {
                  resolve({ success: true });
                }, 500);
              });
            }
          } else {
            console.log(`No primary display found or enabling primary, using default positioning`);
            if (modeId) {
              exec(`xrandr --output ${displayName} --mode ${modeId}`, (setError, setStdout, setStderr) => {
                if (setError) {
                  reject(new Error(`Error enabling display: ${setError.message || setStderr}`));
                  return;
                }
                setTimeout(() => {
                  resolve({ success: true });
                }, 500);
              });
            } else {
              exec(`xrandr --output ${displayName} --auto`, (setError, setStdout, setStderr) => {
                if (setError) {
                  reject(new Error(`Error enabling display: ${setError.message || setStderr}`));
                  return;
                }
                setTimeout(() => {
                  resolve({ success: true });
                }, 500);
              });
            }
          }
        });
      } else {
        console.log(`Disabling display: ${displayName}`);
        exec(`xrandr --output ${displayName} --off`, (setError, setStdout, setStderr) => {
          if (setError) {
            console.error(`Error disabling display ${displayName}:`, setError.message || setStderr);
            reject(new Error(`Error disabling display: ${setError.message || setStderr}`));
            return;
          }
          console.log(`Display ${displayName} disabled successfully`);
          setTimeout(() => {
            resolve({ success: true });
          }, 500);
        });
      }
    };
    
    if (!enabled) {
      console.log(`Checking if we can disable display: ${displayName}`);
      exec('xrandr 2>/dev/null', (error, stdout) => {
        if (error || !stdout) {
          console.warn('Could not check display states, proceeding anyway');
          setDisplayState();
          return;
        }
        
        const lines = stdout.split('\n');
        let enabledCount = 0;
        let connectedCount = 0;
        let currentDisplay = null;
        
        lines.forEach(line => {
          const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
          if (displayMatch) {
            currentDisplay = displayMatch[1];
            if (currentDisplay !== displayName) {
              connectedCount++;
              console.log(`Found connected display: ${currentDisplay}`);
            }
          } else if (currentDisplay && currentDisplay !== displayName && line.match(/\s+[0-9]+x[0-9]+/)) {
            const isActive = line.includes('*');
            if (isActive) {
              enabledCount++;
              console.log(`Found enabled display: ${currentDisplay} with active mode`);
            }
          }
        });
        
        console.log(`Total connected displays (excluding ${displayName}): ${connectedCount}`);
        console.log(`Total enabled displays (excluding ${displayName}): ${enabledCount}`);
        
        if (connectedCount === 0 && enabledCount === 0) {
          console.error('Cannot disable all displays. At least one display must be enabled.');
          reject(new Error('Cannot disable all displays. At least one display must be enabled.'));
          return;
        }
        
        console.log(`Proceeding to disable ${displayName}`);
        setDisplayState();
      });
    } else {
      setDisplayState();
    }
  });
});

ipcMain.handle('set-display-primary', async (event, displayName) => {
  return new Promise((resolve, reject) => {
    console.log(`Setting ${displayName} as primary display`);
    
    exec('xrandr 2>/dev/null', (checkError, checkStdout) => {
      if (checkError || !checkStdout) {
        exec(`xrandr --output ${displayName} --primary`, (error, stdout, stderr) => {
          if (error) {
            console.error(`Error setting primary display: ${error.message || stderr}`);
            reject(new Error(`Error setting primary display: ${error.message || stderr}`));
            return;
          }
          console.log(`Display ${displayName} set as primary successfully`);
          setTimeout(() => {
            resolve({ success: true });
          }, 500);
        });
        return;
      }
      
      const lines = checkStdout.split('\n');
      let currentPrimary = null;
      let currentDisplay = null;
      
      lines.forEach(line => {
        const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
        if (displayMatch) {
          currentDisplay = displayMatch[1];
          if (line.includes('primary')) {
            currentPrimary = currentDisplay;
          }
        }
      });
      
      if (currentPrimary && currentPrimary !== displayName) {
        console.log(`Temporarily disabling current primary ${currentPrimary} to set ${displayName} as primary`);
        exec(`xrandr --output ${currentPrimary} --off --output ${displayName} --auto --primary`, (error, stdout, stderr) => {
          if (error) {
            console.error(`Error setting primary display: ${error.message || stderr}`);
            reject(new Error(`Error setting primary display: ${error.message || stderr}`));
            return;
          }
          
          console.log(`Display ${displayName} set as primary, re-enabling ${currentPrimary} to the right...`);
          setTimeout(() => {
            exec(`xrandr --output ${currentPrimary} --auto --right-of ${displayName}`, (reenableError, reenableStdout, reenableStderr) => {
              if (reenableError) {
                console.warn(`Warning: Could not re-enable ${currentPrimary}: ${reenableError.message || reenableStderr}`);
              } else {
                console.log(`Display ${currentPrimary} re-enabled successfully to the right of ${displayName}`);
              }
              
              setTimeout(() => {
                exec('xrandr 2>/dev/null', (verifyError, verifyStdout) => {
                  if (!verifyError && verifyStdout) {
                    const verifyLines = verifyStdout.split('\n');
                    let isActuallyPrimary = false;
                    
                    verifyLines.forEach(line => {
                      const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
                      if (displayMatch && displayMatch[1] === displayName && line.includes('primary')) {
                        isActuallyPrimary = true;
                      }
                    });
                    
                    if (isActuallyPrimary) {
                      console.log(`Display ${displayName} verified as primary`);
                      resolve({ success: true });
                    } else {
                      console.warn(`Display ${displayName} was not set as primary`);
                      resolve({ success: true });
                    }
                  } else {
                    resolve({ success: true });
                  }
                });
              }, 200);
            });
          }, 300);
        });
      } else {
        exec(`xrandr --output ${displayName} --primary`, (error, stdout, stderr) => {
          if (error) {
            console.error(`Error setting primary display: ${error.message || stderr}`);
            reject(new Error(`Error setting primary display: ${error.message || stderr}`));
            return;
          }
          
          setTimeout(() => {
            exec('xrandr 2>/dev/null', (verifyError, verifyStdout) => {
              if (!verifyError && verifyStdout) {
                const verifyLines = verifyStdout.split('\n');
                let isActuallyPrimary = false;
                
                verifyLines.forEach(line => {
                  const displayMatch = line.match(/^([A-Za-z0-9-]+)\s+connected/);
                  if (displayMatch && displayMatch[1] === displayName && line.includes('primary')) {
                    isActuallyPrimary = true;
                  }
                });
                
                if (isActuallyPrimary) {
                  console.log(`Display ${displayName} verified as primary`);
                  resolve({ success: true });
                } else {
                  console.warn(`Display ${displayName} was not set as primary`);
                  resolve({ success: true });
                }
              } else {
                resolve({ success: true });
              }
            });
          }, 200);
        });
      }
    });
  });
});



ipcMain.handle('get-current-wallpaper', async () => {
  return new Promise((resolve) => {

    exec('grep "^[[:space:]]*Image=" ~/.config/plasma-org.kde.plasma.desktop-appletsrc | sed "s/^.*Image=//" | head -n 1',
      (error, stdout) => {

      if (error || !stdout || !stdout.trim()) {
        resolve({ path: null, name: null });
        return;
      }

      let wallpaperPath = stdout.trim();

      // ---------- Detect KDE wallpaper folder ----------
      if (wallpaperPath.endsWith("/")) {
        const imgDir = path.join(wallpaperPath, "contents/images");

        try {
          const files = fs.readdirSync(imgDir);

          // găsim png/jpg/jpeg
          const candidates = files.filter(f =>
            f.toLowerCase().endsWith(".png") ||
            f.toLowerCase().endsWith(".jpg") ||
            f.toLowerCase().endsWith(".jpeg")
          );

          if (candidates.length > 0) {
            // ia primul wallpaper real
            wallpaperPath = path.join(imgDir, candidates[0]);
          }
        } catch (e) {
          console.error("Wallpaper folder parse error:", e);
        }
      }

      const wallpaperName = wallpaperPath.split('/').pop();

      resolve({
        path: wallpaperPath,
        name: wallpaperName
      });
    });
  });
});


//ipcMain.handle('get-current-wallpaper', async () => {
//  return new Promise((resolve) => {
//    //exec('grep "Image=" ~/.config/plasma-org.kde.plasma.desktop-appletsrc 2>/dev/null | grep -v "Image=true" | sed \'s/.*Image=//\' | tail -n 1', (error, stdout) => {
//    exec('grep "^[[:space:]]*Image=" ~/.config/plasma-org.kde.plasma.desktop-appletsrc | sed "s/.*Image=//" | head -n 1', (error, stdout) => {
//
//      if (error || !stdout || !stdout.trim()) {
//        resolve({ path: null, name: null });
//        return;
//      }
//      
//      const wallpaperPath = stdout.trim();
//      const wallpaperName = wallpaperPath.split('/').pop();
//      
//      resolve({
//        path: wallpaperPath,
//        name: wallpaperName
//      });
//    });
//  });
//});

ipcMain.handle('get-wallpapers', async () => {
  return new Promise((resolve) => {
    exec('find /usr/share/extras/wallpapers -type f \\( -name "*.jpg" -o -name "*.jpeg" -o -name "*.png" -o -name "*.bmp" \\) 2>/dev/null', (error, stdout) => {
      if (error || !stdout) {
        resolve([]);
        return;
      }
      
      const wallpapers = stdout.trim().split('\n')
        .filter(path => path.trim())
        .map(path => {
          const fullPath = path.trim();
          return {
            path: fullPath,
            name: fullPath.split('/').pop()
          };
        });
      
      resolve(wallpapers);
    });
  });
});

ipcMain.handle('set-wallpaper', async (event, wallpaperPath) => {
  return new Promise((resolve, reject) => {
    exec(`plasma-apply-wallpaperimage "${wallpaperPath}"`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting wallpaper: ${error.message || stderr}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('browse-wallpaper', async () => {
  const { dialog } = require('electron');
  const homedir = require('os').homedir();
  
  const result = await dialog.showOpenDialog(mainWindow, {
    title: 'Select Wallpaper',
    defaultPath: homedir,
    filters: [
      { name: 'Images', extensions: ['jpg', 'jpeg', 'png', 'bmp', 'webp'] }
    ],
    properties: ['openFile']
  });
  
  if (result.canceled || !result.filePaths || result.filePaths.length === 0) {
    return null;
  }
  
  return result.filePaths[0];
});

ipcMain.handle('set-wallpaper-fill-mode', async (event, fillMode) => {
  return new Promise((resolve, reject) => {
    
    const kdeFillModes = {
      'Fill screen': '6', 
      'Fit to screen': '1', 
      'Stretch to Fill Screen': '2', 
      'Center': '0', 
      'Tile': '3' 
    };
    
    const kdeMode = kdeFillModes[fillMode] || '6';
    
    exec(`kwriteconfig5 --file ~/.config/plasma-org.kde.plasma.desktop-appletsrc --group Containments --group 1 --group Wallpaper --group org.kde.image --group General --key FillMode ${kdeMode} && qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript "var allDesktops = desktops();for (i=0;i<allDesktops.length;i++) {d = allDesktops[i];d.wallpaperPlugin = 'org.kde.image';d.currentConfigGroup = Array('Wallpaper', 'org.kde.image', 'General');d.writeConfig('FillMode', ${kdeMode})}"`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting fill mode: ${error.message || stderr}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

// Handler pentru extragerea culorii din wallpaper la o anumită poziție
ipcMain.handle('get-wallpaper-color-at-position', async (event, windowX, windowY, windowWidth, windowHeight) => {
  return new Promise((resolve, reject) => {
    // Obține wallpaper-ul curent

exec(
  'grep "^[[:space:]]*Image=" ~/.config/plasma-org.kde.plasma.desktop-appletsrc | sed "s/^.*Image=//" | head -n 1',
  (error, stdout) => {

    if (error || !stdout || !stdout.trim()) {
      reject(new Error('Could not get wallpaper path'));
      return;
    }

    let wallpaperPath = stdout.trim();

    // ---------- Detect KDE wallpaper folder ----------
    if (wallpaperPath.endsWith("/")) {
      const imgDir = path.join(wallpaperPath, "contents/images");

      try {
        const files = fs.readdirSync(imgDir);

        // găsim png/jpg/jpeg
        const candidates = files.filter(f =>
          f.toLowerCase().endsWith(".png") ||
          f.toLowerCase().endsWith(".jpg") ||
          f.toLowerCase().endsWith(".jpeg")
        );

        if (candidates.length > 0) {
          // ia primul wallpaper real
          wallpaperPath = path.join(imgDir, candidates[0]);
        }
      } catch (e) {
        console.error("Wallpaper folder parse error:", e);
      }
    }

    // Elimină prefixul file:// dacă există
    if (wallpaperPath.startsWith('file://')) {
      wallpaperPath = wallpaperPath.substring(7);
    }

    // continuă restul codului tău pentru fs.existsSync și extragerea culorii


//    exec('grep "Image=" ~/.config/plasma-org.kde.plasma.desktop-appletsrc 2>/dev/null | grep -v "Image=true" | sed \'s/.*Image=//\' | tail -n 1', (error, stdout) => {
//      if (error || !stdout || !stdout.trim()) {
//        reject(new Error('Could not get wallpaper path'));
//        return;
//      }
//      
//      let wallpaperPath = stdout.trim();
//      
//      // Elimină prefixul file:// dacă există
//      if (wallpaperPath.startsWith('file://')) {
//        wallpaperPath = wallpaperPath.substring(7); // Elimină 'file://'
//      }
      
      // Verifică dacă fișierul există
      if (!fs.existsSync(wallpaperPath)) {
        reject(new Error(`Wallpaper file does not exist: ${wallpaperPath}`));
        return;
      }
      

      // Obține dimensiunile ecranului și ale wallpaper-ului
      const primaryDisplay = screen.getPrimaryDisplay();
      const screenWidth = primaryDisplay.workAreaSize.width;
      const screenHeight = primaryDisplay.workAreaSize.height;
      
      // Calculează poziția centrului ferestrei relativ la ecran
      const windowCenterX = windowX + windowWidth / 2;
      const windowCenterY = windowY + windowHeight / 2;
      
      // Obține dimensiunile imaginii wallpaper
      exec(`identify -format "%wx%h" "${wallpaperPath}"`, (identifyError, identifyStdout) => {
        if (identifyError || !identifyStdout) {
          reject(new Error('Could not get wallpaper dimensions'));
          return;
        }
        
        const [wallpaperWidth, wallpaperHeight] = identifyStdout.trim().split('x').map(Number);
        
        // Calculează poziția pe wallpaper (presupunem fill mode "Fill screen" - cea mai comună)
        // Poziția este calculată ca procent din dimensiunile ecranului, apoi măsurat pe wallpaper
        const percentX = windowCenterX / screenWidth;
        const percentY = windowCenterY / screenHeight;
        
        // Calculăm poziția pe wallpaper (presupunem că wallpaper-ul este scalat pentru a umple ecranul)
        let wallpaperX, wallpaperY;
        
        const aspectScreen = screenWidth / screenHeight;
        const aspectWallpaper = wallpaperWidth / wallpaperHeight;
        
        if (aspectWallpaper > aspectScreen) {
          // Wallpaper-ul este mai lat - se scală pe înălțime
          const scaledWidth = wallpaperHeight * aspectScreen;
          const offsetX = (wallpaperWidth - scaledWidth) / 2;
          wallpaperX = offsetX + (percentX * scaledWidth);
          wallpaperY = percentY * wallpaperHeight;
        } else {
          // Wallpaper-ul este mai înalt - se scală pe lățime
          const scaledHeight = wallpaperWidth / aspectScreen;
          const offsetY = (wallpaperHeight - scaledHeight) / 2;
          wallpaperX = percentX * wallpaperWidth;
          wallpaperY = offsetY + (percentY * scaledHeight);
        }
        
        // Limitează coordonatele la dimensiunile imaginii
        wallpaperX = Math.max(0, Math.min(wallpaperWidth - 1, Math.floor(wallpaperX)));
        wallpaperY = Math.max(0, Math.min(wallpaperHeight - 1, Math.floor(wallpaperY)));
        
        // Extrage culoarea de la poziția calculată folosind ImageMagick
        // Folosim o singură comandă care returnează toate valorile RGB într-un format standard
        exec(`convert "${wallpaperPath}" -crop 1x1+${wallpaperX}+${wallpaperY} -format "%[fx:int(255*r)],%[fx:int(255*g)],%[fx:int(255*b)]" info:`, (convertError, convertStdout) => {
          if (convertError || !convertStdout) {
            reject(new Error(`Could not extract color from wallpaper: ${convertError ? convertError.message : 'No output'}`));
            return;
          }
          
          const output = convertStdout.trim();
          
          // Output-ul ar trebui să fie în format: "valR,valG,valB" (ex: "72,72,72")
          const colorParts = output.split(',');
          
          if (colorParts.length !== 3) {
            // Încearcă parsing alternativ pentru formatul cu procente (rgb(x%,y%,z%))
            const percentMatch = output.match(/rgb\(([\d.]+)%,([\d.]+)%,([\d.]+)%\)/i);
            if (percentMatch) {
              const r = Math.round(parseFloat(percentMatch[1]) * 255 / 100);
              const g = Math.round(parseFloat(percentMatch[2]) * 255 / 100);
              const b = Math.round(parseFloat(percentMatch[3]) * 255 / 100);
              
              const hex = `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
              resolve({ color: hex, rgb: { r, g, b } });
              return;
            }
            
            reject(new Error(`Could not parse color from wallpaper. Output: "${output}"`));
            return;
          }
          
          const r = parseInt(colorParts[0], 10);
          const g = parseInt(colorParts[1], 10);
          const b = parseInt(colorParts[2], 10);
          
          if (isNaN(r) || isNaN(g) || isNaN(b)) {
            reject(new Error(`Invalid color values. Output: "${output}"`));
            return;
          }
          
          // Convertește în hex
          const hex = `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
          
          resolve({ color: hex, rgb: { r, g, b } });
        });
      });
    });
  });
});

// Handler pentru obținerea poziției ferestrei
ipcMain.handle('get-window-position', async () => {
  if (!mainWindow) {
    return { x: 0, y: 0, width: 0, height: 0 };
  }
  
  const bounds = mainWindow.getBounds();
  return { x: bounds.x, y: bounds.y, width: bounds.width, height: bounds.height };
});

// Funcții helper pentru salvarea/încărcarea setărilor de appearance
function getAppearanceOptionsPath() {
  const homeDir = process.env.HOME || process.env.USERPROFILE;
  const optionsDir = path.join(homeDir, '.local', 'share', 'extras', 'org.pearos.systemsettings', 'appearance');
  return path.join(optionsDir, 'options.json');
}

function readAppearanceOptions() {
  try {
    const optionsPath = getAppearanceOptionsPath();
    if (fs.existsSync(optionsPath)) {
      const data = fs.readFileSync(optionsPath, 'utf8');
      return JSON.parse(data);
    }
  } catch (error) {
    console.error('Error reading appearance options:', error);
  }
  return null;
}

function writeAppearanceOptions(data) {
  try {
    const optionsPath = getAppearanceOptionsPath();
    const optionsDir = path.dirname(optionsPath);
    
    // Creează directorul dacă nu există
    if (!fs.existsSync(optionsDir)) {
      fs.mkdirSync(optionsDir, { recursive: true });
    }
    
    // Citește opțiunile existente sau creează un obiect nou
    let options = {};
    if (fs.existsSync(optionsPath)) {
      try {
        const existingData = fs.readFileSync(optionsPath, 'utf8');
        options = JSON.parse(existingData);
      } catch (error) {
        console.error('Error reading existing options:', error);
      }
    }
    
    // Actualizează opțiunile
    Object.assign(options, data);
    
    // Scrie opțiunile actualizate
    fs.writeFileSync(optionsPath, JSON.stringify(options, null, 2), 'utf8');
    return true;
  } catch (error) {
    console.error('Error writing appearance options:', error);
    return false;
  }
}

// Handler pentru salvarea setării de tint window background
ipcMain.handle('save-tint-window-setting', async (event, enabled) => {
  return new Promise((resolve) => {
    const success = writeAppearanceOptions({ tintWindowBackground: enabled });
    resolve({ success });
  });
});

// Handler pentru încărcarea setării de tint window background
ipcMain.handle('load-tint-window-setting', async () => {
  return new Promise((resolve) => {
    const options = readAppearanceOptions();
    // Default: ON (true) dacă nu există sau lipsesc datele
    const enabled = options && typeof options.tintWindowBackground === 'boolean' 
      ? options.tintWindowBackground 
      : true;
    resolve({ enabled });
  });
});

ipcMain.handle('get-audio-outputs', async () => {
  return new Promise((resolve) => {
    exec('pactl list sinks 2>/dev/null', (error, stdout) => {
      if (error || !stdout) {
        resolve([]);
        return;
      }
      
      const outputs = [];
      const lines = stdout.split('\n');
      let currentSink = null;
      
      lines.forEach(line => {
        const sinkMatch = line.match(/^Sink #(\d+)/);
        if (sinkMatch) {
          if (currentSink) {
            outputs.push(currentSink);
          }
          currentSink = {
            index: sinkMatch[1],
            name: null,
            description: null
          };
        } else if (currentSink) {
          const nameMatch = line.match(/^\s+Name:\s+(.+)$/);
          if (nameMatch) {
            currentSink.name = nameMatch[1].trim();
          }
          
          const descMatch = line.match(/^\s+Description:\s+(.+)$/);
          if (descMatch) {
            currentSink.description = descMatch[1].trim();
          }
        }
      });
      
      if (currentSink) {
        outputs.push(currentSink);
      }
      
      resolve(outputs);
    });
  });
});

ipcMain.handle('get-audio-inputs', async () => {
  return new Promise((resolve) => {
    exec('pactl list sources 2>/dev/null', (error, stdout) => {
      if (error || !stdout) {
        resolve([]);
        return;
      }
      
      const inputs = [];
      const lines = stdout.split('\n');
      let currentSource = null;
      
      lines.forEach(line => {
        
        if (line.includes('.monitor')) {
          return;
        }
        
        const sourceMatch = line.match(/^Source #(\d+)/);
        if (sourceMatch) {
          if (currentSource) {
            inputs.push(currentSource);
          }
          currentSource = {
            index: sourceMatch[1],
            name: null,
            description: null
          };
        } else if (currentSource) {
          const nameMatch = line.match(/^\s+Name:\s+(.+)$/);
          if (nameMatch && !nameMatch[1].includes('.monitor')) {
            currentSource.name = nameMatch[1].trim();
          }
          
          const descMatch = line.match(/^\s+Description:\s+(.+)$/);
          if (descMatch) {
            currentSource.description = descMatch[1].trim();
          }
        }
      });
      
      if (currentSource) {
        inputs.push(currentSource);
      }
      
      resolve(inputs);
    });
  });
});

ipcMain.handle('get-output-volume', async () => {
  return new Promise((resolve) => {
    exec('pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | head -1', (error, stdout) => {
      if (error || !stdout) {
        resolve({ volume: 50 });
        return;
      }
      
      const match = stdout.match(/(\d+)%/);
      if (match) {
        resolve({ volume: parseInt(match[1]) });
      } else {
        resolve({ volume: 50 });
      }
    });
  });
});

ipcMain.handle('get-input-volume', async () => {
  return new Promise((resolve) => {
    exec('pactl get-source-volume @DEFAULT_SOURCE@ 2>/dev/null | head -1', (error, stdout) => {
      if (error || !stdout) {
        resolve({ volume: 50 });
        return;
      }
      
      const match = stdout.match(/(\d+)%/);
      if (match) {
        resolve({ volume: parseInt(match[1]) });
      } else {
        resolve({ volume: 50 });
      }
    });
  });
});

ipcMain.handle('set-output-volume', async (event, volume) => {
  return new Promise((resolve, reject) => {
    const volumeValue = Math.max(0, Math.min(100, parseInt(volume)));
    exec(`pactl set-sink-volume @DEFAULT_SINK@ ${volumeValue}%`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting output volume: ${error.message || stderr}`));
        return;
      }
      resolve({ success: true, volume: volumeValue });
    });
  });
});

ipcMain.handle('set-input-volume', async (event, volume) => {
  return new Promise((resolve, reject) => {
    const volumeValue = Math.max(0, Math.min(100, parseInt(volume)));
    exec(`pactl set-source-volume @DEFAULT_SOURCE@ ${volumeValue}%`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting input volume: ${error.message || stderr}`));
        return;
      }
      resolve({ success: true, volume: volumeValue });
    });
  });
});

ipcMain.handle('set-output-device', async (event, deviceIndex) => {
  return new Promise((resolve, reject) => {
    
    exec(`pactl list sinks | grep -A 10 "Sink #${deviceIndex}" | grep "Name:" | head -1 | awk '{print $2}'`, (nameError, deviceName) => {
      if (nameError || !deviceName) {
        reject(new Error(`Error getting device name: ${nameError?.message || 'Device not found'}`));
        return;
      }
      
      exec(`pactl set-default-sink ${deviceName.trim()}`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Error setting output device: ${error.message || stderr}`));
          return;
        }
        resolve({ success: true });
      });
    });
  });
});

ipcMain.handle('set-input-device', async (event, deviceIndex) => {
  return new Promise((resolve, reject) => {
    
    exec(`pactl list sources | grep -A 10 "Source #${deviceIndex}" | grep "Name:" | head -1 | awk '{print $2}'`, (nameError, deviceName) => {
      if (nameError || !deviceName || deviceName.includes('.monitor')) {
        reject(new Error(`Error getting device name: ${nameError?.message || 'Device not found'}`));
        return;
      }
      
      exec(`pactl set-default-source ${deviceName.trim()}`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Error setting input device: ${error.message || stderr}`));
          return;
        }
        resolve({ success: true });
      });
    });
  });
});

ipcMain.handle('get-current-output-device', async () => {
  return new Promise((resolve) => {
    exec('pactl info | grep "Default Sink:" | awk \'{print $3}\'', (error, stdout) => {
      if (error || !stdout) {
        resolve({ name: null });
        return;
      }
      resolve({ name: stdout.trim() });
    });
  });
});

ipcMain.handle('get-current-input-device', async () => {
  return new Promise((resolve) => {
    exec('pactl info | grep "Default Source:" | awk \'{print $3}\'', (error, stdout) => {
      if (error || !stdout) {
        resolve({ name: null });
        return;
      }
      resolve({ name: stdout.trim() });
    });
  });
});

ipcMain.handle('get-battery-timeout', async () => {
  return new Promise((resolve) => {
    
    exec('kreadconfig5 --file ~/.config/powerdevilrc --group "Battery" --group "DPMSControl" --key "idleTime" 2>/dev/null', (error, stdout) => {
      if (!error && stdout && stdout.trim()) {
        const timeout = parseInt(stdout.trim());
        if (!isNaN(timeout)) {
          resolve({ timeout: timeout });
          return;
        }
      }
      
      
      exec('xset q 2>/dev/null | grep "DPMS is"', (xsetError, xsetStdout) => {
        if (!xsetError && xsetStdout && xsetStdout.includes('enabled')) {
          
          exec('xset q 2>/dev/null | grep -E "Standby|Suspend|Off" | head -1', (timeoutError, timeoutStdout) => {
            if (!timeoutError && timeoutStdout) {
              const match = timeoutStdout.match(/(\d+)/);
              if (match) {
                resolve({ timeout: parseInt(match[1]) });
                return;
              }
            }
            resolve({ timeout: 600 }); 
          });
        } else {
          resolve({ timeout: 0 }); 
        }
      });
    });
  });
});

ipcMain.handle('get-ac-timeout', async () => {
  return new Promise((resolve) => {
    
    exec('kreadconfig5 --file ~/.config/powerdevilrc --group "AC" --group "DPMSControl" --key "idleTime" 2>/dev/null', (error, stdout) => {
      if (!error && stdout && stdout.trim()) {
        const timeout = parseInt(stdout.trim());
        if (!isNaN(timeout)) {
          resolve({ timeout: timeout });
          return;
        }
      }
      
      
      exec('xset q 2>/dev/null | grep "DPMS is"', (xsetError, xsetStdout) => {
        if (!xsetError && xsetStdout && xsetStdout.includes('enabled')) {
          
          exec('xset q 2>/dev/null | grep -E "Standby|Suspend|Off" | head -1', (timeoutError, timeoutStdout) => {
            if (!timeoutError && timeoutStdout) {
              const match = timeoutStdout.match(/(\d+)/);
              if (match) {
                resolve({ timeout: parseInt(match[1]) });
                return;
              }
            }
            resolve({ timeout: 600 }); 
          });
        } else {
          resolve({ timeout: 0 }); 
        }
      });
    });
  });
});

ipcMain.handle('get-lock-delay', async () => {
  return new Promise((resolve) => {
    exec('kreadconfig5 --file ~/.config/kscreenlockerrc --group "Daemon" --key "Timeout" 2>/dev/null', (error, stdout) => {
      if (error || !stdout || !stdout.trim()) {
        resolve({ delay: 0 }); 
        return;
      }
      const delay = parseInt(stdout.trim());
      
      if (delay === 0) {
        exec('kreadconfig5 --file ~/.config/kscreenlockerrc --group "Daemon" --key "LockGrace" 2>/dev/null', (graceError, graceStdout) => {
          if (!graceError && graceStdout && graceStdout.trim() === '0') {
            resolve({ delay: -1 }); 
          } else {
            resolve({ delay: 0 }); 
          }
        });
      } else {
        resolve({ delay: isNaN(delay) ? 0 : delay });
      }
    });
  });
});

ipcMain.handle('set-battery-timeout', async (event, timeoutSeconds) => {
  return new Promise((resolve, reject) => {
    const timeout = parseInt(timeoutSeconds);
    
    
    const commands = [
      `kwriteconfig5 --file ~/.config/powerdevilrc --group "Battery" --group "DPMSControl" --key "idleTime" ${timeout}`,
      timeout > 0 ? `xset dpms ${timeout} ${timeout} ${timeout}` : `xset -dpms`
    ].join(' && ');
    
    exec(commands, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting battery timeout: ${error.message || stderr}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('set-ac-timeout', async (event, timeoutSeconds) => {
  return new Promise((resolve, reject) => {
    const timeout = parseInt(timeoutSeconds);
    
    
    const commands = [
      `kwriteconfig5 --file ~/.config/powerdevilrc --group "AC" --group "DPMSControl" --key "idleTime" ${timeout}`,
      timeout > 0 ? `xset dpms ${timeout} ${timeout} ${timeout}` : `xset -dpms`
    ].join(' && ');
    
    exec(commands, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error setting AC timeout: ${error.message || stderr}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('set-lock-delay', async (event, delaySeconds) => {
  return new Promise((resolve, reject) => {
    const delay = parseInt(delaySeconds);
    
    if (delay === -1) {
      exec(`kwriteconfig5 --file ~/.config/kscreenlockerrc --group "Daemon" --key "Timeout" 0 && kwriteconfig5 --file ~/.config/kscreenlockerrc --group "Daemon" --key "LockGrace" 0`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Error setting lock delay: ${error.message || stderr}`));
          return;
        }
        resolve({ success: true });
      });
    } else {
      exec(`kwriteconfig5 --file ~/.config/kscreenlockerrc --group "Daemon" --key "Timeout" ${delay}`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Error setting lock delay: ${error.message || stderr}`));
          return;
        }
        resolve({ success: true });
      });
    }
  });
});

// Cache pentru brightness max (evită apeluri repetate)
let cachedBrightnessMax = null;

ipcMain.handle('set-brightness', async (event, brightness, displayName) => {
  return new Promise((resolve, reject) => {
    const brightnessPercent = Math.max(0, Math.min(100, parseInt(brightness)));
    
    // Pentru display-uri built-in (eDP), folosim qdbus care afișează OSD
    if (displayName && displayName.startsWith('eDP')) {
      // Folosește qdbus pentru laptop display (afișează OSD)
      const setBrightnessViaQdbus = (maxBrightness) => {
        const actualBrightness = Math.round((brightnessPercent / 100) * maxBrightness);
        exec(`qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement/Actions/BrightnessControl org.kde.Solid.PowerManagement.Actions.BrightnessControl.setBrightness ${actualBrightness}`, (error) => {
          if (error) {
            resolve({ success: false, brightness: brightnessPercent, message: error.message });
            return;
          }
          resolve({ success: true, brightness: brightnessPercent });
        });
      };
      
      if (cachedBrightnessMax !== null) {
        setBrightnessViaQdbus(cachedBrightnessMax);
      } else {
        exec('qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement/Actions/BrightnessControl org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightnessMax', (maxError, maxStdout) => {
          if (maxError || !maxStdout.trim()) {
            resolve({ success: false, brightness: brightnessPercent, message: 'Could not get max brightness' });
            return;
          }
          cachedBrightnessMax = parseInt(maxStdout.trim());
          setBrightnessViaQdbus(cachedBrightnessMax);
        });
      }
      return;
    }
    
    // Pentru monitoare externe, folosim xrandr direct (fără verificare suplimentară)
    if (displayName) {
      exec(`xrandr --output ${displayName} --brightness ${(brightnessPercent / 100).toFixed(2)}`, (xrandrError) => {
        if (xrandrError) {
          resolve({ success: false, brightness: brightnessPercent, message: xrandrError.message });
          return;
        }
        resolve({ success: true, brightness: brightnessPercent });
      });
      return;
    }
    
    // Fallback: fără display specificat, folosim qdbus
    const setBrightnessViaQdbus = (maxBrightness) => {
      const actualBrightness = Math.round((brightnessPercent / 100) * maxBrightness);
      exec(`qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement/Actions/BrightnessControl org.kde.Solid.PowerManagement.Actions.BrightnessControl.setBrightness ${actualBrightness}`, (error) => {
        if (error) {
          exec(`xrandr --output $(xrandr | grep " connected" | head -1 | cut -d" " -f1) --brightness ${(brightnessPercent / 100).toFixed(2)}`, (xrandrError) => {
            if (xrandrError) {
              reject(new Error(`Error setting brightness: ${error.message || xrandrError.message}`));
              return;
            }
            resolve({ success: true, brightness: brightnessPercent });
          });
          return;
        }
        resolve({ success: true, brightness: brightnessPercent });
      });
    };
    
    if (cachedBrightnessMax !== null) {
      setBrightnessViaQdbus(cachedBrightnessMax);
    } else {
      exec('qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement/Actions/BrightnessControl org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightnessMax', (maxError, maxStdout) => {
        if (maxError || !maxStdout.trim()) {
          exec(`xrandr --output $(xrandr | grep " connected" | head -1 | cut -d" " -f1) --brightness ${(brightnessPercent / 100).toFixed(2)}`, (xrandrError) => {
            if (xrandrError) {
              reject(new Error(`Error setting brightness: ${xrandrError.message}`));
              return;
            }
            resolve({ success: true, brightness: brightnessPercent });
          });
          return;
        }
        cachedBrightnessMax = parseInt(maxStdout.trim());
        setBrightnessViaQdbus(cachedBrightnessMax);
      });
    }
  });
});

ipcMain.handle('get-require-apps', async () => {
  return new Promise((resolve) => {
    exec('find /usr/share/extras -name "*.require" -type f 2>/dev/null', (error, stdout) => {
      if (error || !stdout || !stdout.trim()) {
        resolve([]);
        return;
      }
      
      const files = stdout.trim().split('\n').filter(f => f.trim());
      const apps = [];
      
      files.forEach(filePath => {
        try {
          const content = fs.readFileSync(filePath, 'utf8');
          const appData = JSON.parse(content);
          apps.push({
            file: filePath,
            icon: appData.icon || '',
            title: appData.title || 'Unknown App',
            description: appData.description || '',
            developer: appData.developer || '',
            website: appData.website || '',
            allowAction: appData.allowAction || ''
          });
        } catch (err) {
          console.error(`Error reading ${filePath}:`, err);
        }
      });
      
      resolve(apps);
    });
  });
});

ipcMain.handle('allow-require-app', async (event, filePath, allowAction) => {
  return new Promise((resolve, reject) => {
    
    const deleteCommand = `pkexec rm -f "${filePath}"`;
    exec(deleteCommand, (deleteError, deleteStdout, deleteStderr) => {
      if (deleteError) {
        reject(new Error(`Error deleting file: ${deleteStderr || deleteError.message}`));
        return;
      }
      
      
      if (allowAction) {
        exec(allowAction, (error, stdout, stderr) => {
          if (error) {
            
            console.error(`Error executing allowAction: ${error.message}`);
          }
          resolve({ success: true });
        });
      } else {
        resolve({ success: true });
      }
    });
  });
});

ipcMain.handle('deny-require-app', async (event, filePath) => {
  return new Promise((resolve, reject) => {
    
    const deleteCommand = `pkexec rm -f "${filePath}"`;
    exec(deleteCommand, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Error deleting file: ${stderr || error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('open-external-link', async (event, url) => {
  return new Promise((resolve) => {
    shell.openExternal(url);
    resolve({ success: true });
  });
});

ipcMain.handle('change-password', async (event, oldPassword, newPassword) => {
  return new Promise((resolve) => {
    const os = require('os');
    const username = os.userInfo().username;
    
    
    const verifyCommand = `echo '${oldPassword.replace(/'/g, "'\\''")}' | sudo -S true 2>&1`;
    exec(verifyCommand, (verifyError, verifyStdout, verifyStderr) => {
      if (verifyError || verifyStderr.includes('Sorry')) {
        resolve({ success: false, error: 'Incorrect old password.' });
        return;
      }
      
      
      const changeCommand = `echo '${oldPassword.replace(/'/g, "'\\''")}' | sudo -S sh -c 'echo "${username}:${newPassword.replace(/'/g, "'\\''")}" | chpasswd' 2>&1`;
      exec(changeCommand, (changeError, changeStdout, changeStderr) => {
        if (changeError || changeStderr.includes('Sorry') || changeStderr.includes('chpasswd')) {
          let errorMessage = 'Failed to change password.';
          if (changeStderr.includes('Sorry')) {
            errorMessage = 'Incorrect old password.';
          } else if (changeStderr.includes('BAD PASSWORD') || changeStderr.includes('password')) {
            errorMessage = 'Password does not meet requirements.';
          }
          resolve({ success: false, error: errorMessage });
          return;
        }
        resolve({ success: true });
      });
    });
  });
});

ipcMain.handle('check-fingerprint-device', async () => {
  return new Promise((resolve) => {
    exec('lsusb | grep -i -e finger -e fprint -e biom', (error, stdout) => {
      if (error || !stdout || !stdout.trim()) {
        resolve({ hasDevice: false });
        return;
      }
      resolve({ hasDevice: true });
    });
  });
});

ipcMain.handle('list-fingerprints', async () => {
  return new Promise((resolve) => {
    const os = require('os');
    const username = os.userInfo().username;
    
    exec(`fprintd-list ${username}`, (error, stdout) => {
      if (error || !stdout || !stdout.trim()) {
        resolve({ fingerprints: [] });
        return;
      }
      
      
      const lines = stdout.trim().split('\n');
      const fingerprints = [];
      
      lines.forEach(line => {
        
        const match1 = line.match(/^\s*-\s*#\d+:\s*(\S+)$/);
        if (match1) {
          fingerprints.push({
            username: username,
            finger: match1[1]
          });
        } else {
          
          const match2 = line.match(/(\w+), (\w+)/);
          if (match2) {
            fingerprints.push({
              username: match2[1],
              finger: match2[2]
            });
          }
        }
      });
      
      resolve({ fingerprints });
    });
  });
});

ipcMain.handle('delete-fingerprint', async (event, username, finger) => {
  return new Promise((resolve, reject) => {
    
    exec(`fprintd-delete ${username} -f ${finger}`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Failed to delete fingerprint: ${stderr || error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('enroll-fingerprint', async (event, finger) => {
  return new Promise((resolve, reject) => {
    const os = require('os');
    const username = os.userInfo().username;
    const fingerName = finger || 'left-index-finger';
    
    
    const enrollCommand = `fprintd-enroll ${username} ${fingerName}`;
    
    console.log(`Executing: ${enrollCommand}`);
    
    
    const childProcess = exec(enrollCommand, { timeout: 300000 }, (error, stdout, stderr) => {
      console.log(`fprintd-enroll stdout: ${stdout}`);
      console.log(`fprintd-enroll stderr: ${stderr}`);
      
      if (error) {
        console.log(`fprintd-enroll error: ${error.message}, code: ${error.code}, signal: ${error.signal}`);
        
        if (error.signal === 'SIGTERM' || error.signal === 'SIGINT' || error.code === 1) {
          resolve({ success: false, cancelled: true });
          return;
        }
        reject(new Error(`Failed to enroll fingerprint: ${stderr || error.message}`));
        return;
      }
      
      
      console.log('fprintd-enroll completed successfully');
      resolve({ success: true, message: stdout || stderr });
    });
    
    
    childProcess.stdout.on('data', (data) => {
      const output = data.toString();
      console.log(`fprintd-enroll stdout chunk: ${output}`);
      event.sender.send('enroll-fingerprint-output', output);
    });
    
    childProcess.stderr.on('data', (data) => {
      const output = data.toString();
      console.log(`fprintd-enroll stderr chunk: ${output}`);
      event.sender.send('enroll-fingerprint-output', output);
    });
  });
});

ipcMain.handle('get-users', async () => {
  return new Promise((resolve, reject) => {
    const os = require('os');
    const fs = require('fs');
    
    
    exec("awk -F: '$3>=1000 && $3<65534 {print $1}' /etc/passwd", (error, stdout) => {
      if (error) {
        reject(new Error(`Failed to get users: ${error.message}`));
        return;
      }
      
      const usernames = stdout.trim().split('\n').filter(u => u);
      const users = [];
      
      
      let processed = 0;
      if (usernames.length === 0) {
        resolve({ users: [] });
        return;
      }
      
      usernames.forEach(username => {
        
        exec(`groups ${username}`, (error, stdout) => {
          processed++;
          const isAdmin = !error && stdout && (stdout.includes('sudo') || stdout.includes('wheel'));
          
          
          const currentUser = os.userInfo().username;
          let faceIconPath;
          
          if (username === currentUser) {
            faceIconPath = `${os.homedir()}/.face.icon`;
          } else {
            
            exec(`getent passwd ${username} | cut -d: -f6`, (error, homedir) => {
              if (!error && homedir && homedir.trim()) {
                faceIconPath = `${homedir.trim()}/.face.icon`;
              } else {
                faceIconPath = `/home/${username}/.face.icon`;
              }
              
              const faceIcon = fs.existsSync(faceIconPath) ? `file://${faceIconPath}` : null;
              
              users.push({
                username: username,
                faceIcon: faceIcon,
                isAdmin: isAdmin
              });
              
              
              if (processed === usernames.length) {
                resolve({ users });
              }
            });
            return;
          }
          
          const faceIcon = fs.existsSync(faceIconPath) ? `file://${faceIconPath}` : null;
          
          users.push({
            username: username,
            faceIcon: faceIcon,
            isAdmin: isAdmin
          });
          
          
          if (processed === usernames.length) {
            resolve({ users });
          }
        });
      });
    });
  });
});

ipcMain.handle('get-autologin-status', async () => {
  return new Promise((resolve) => {
    const fs = require('fs');
    const autologinPath = '/etc/sddm.conf.d/autologin.conf';
    
    if (fs.existsSync(autologinPath)) {
      const content = fs.readFileSync(autologinPath, 'utf8');
      
      if (content.includes('[Autologin]') && content.includes('User=')) {
        resolve({ enabled: true });
        return;
      }
    }
    
    resolve({ enabled: false });
  });
});

ipcMain.handle('set-autologin', async (event, enabled) => {
  return new Promise((resolve, reject) => {
    const os = require('os');
    const username = os.userInfo().username;
    
    if (enabled) {
      
      exec(`pkexec bash -c 'echo -e "[Autologin]\\nUser=${username}\\nSession=plasma.desktop" > /etc/sddm.conf.d/autologin.conf'`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Failed to enable autologin: ${stderr || error.message}`));
          return;
        }
        resolve({ success: true });
      });
    } else {
      
      exec(`pkexec rm -f /etc/sddm.conf.d/autologin.conf`, (error, stdout, stderr) => {
        if (error) {
          reject(new Error(`Failed to disable autologin: ${stderr || error.message}`));
          return;
        }
        resolve({ success: true });
      });
    }
  });
});

ipcMain.handle('get-keyboard-settings', async () => {
  return new Promise((resolve, reject) => {
    
    exec('xset -q | grep "auto repeat:"', (error, stdout) => {
      if (error) {
        reject(new Error(`Failed to get keyboard settings: ${error.message}`));
        return;
      }
      
      const repeatEnabled = stdout.includes('on');
      
      
      exec('xset -q | grep "auto repeat delay:"', (error, stdout) => {
        if (error) {
          reject(new Error(`Failed to get keyboard settings: ${error.message}`));
          return;
        }
        
        const delayMatch = stdout.match(/delay:\s+(\d+)/);
        const rateMatch = stdout.match(/rate:\s+(\d+)/);
        const delay = delayMatch ? parseInt(delayMatch[1]) : 600;
        const rate = rateMatch ? parseInt(rateMatch[1]) : 25;
        
        
        exec('setxkbmap -query | grep layout', (error, stdout) => {
          if (error) {
            resolve({
              repeatEnabled,
              delay,
              rate,
              layout: 'us'
            });
            return;
          }
          
          const layoutMatch = stdout.match(/layout:\s+(\S+)/);
          const layout = layoutMatch ? layoutMatch[1] : 'us';
          
          resolve({
            repeatEnabled,
            delay,
            rate,
            layout
          });
        });
      });
    });
  });
});

ipcMain.handle('set-key-repeat', async (event, enabled) => {
  return new Promise((resolve, reject) => {
    const command = enabled ? 'xset r on' : 'xset r off';
    exec(command, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Failed to set key repeat: ${stderr || error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('set-key-repeat-rate', async (event, delay, rate) => {
  return new Promise((resolve, reject) => {
    exec(`xset r rate ${delay} ${rate}`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Failed to set key repeat rate: ${stderr || error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('set-keyboard-layout', async (event, layout) => {
  return new Promise((resolve, reject) => {
    exec(`setxkbmap -layout ${layout}`, (error, stdout, stderr) => {
      if (error) {
        reject(new Error(`Failed to set keyboard layout: ${stderr || error.message}`));
        return;
      }
      resolve({ success: true });
    });
  });
});

ipcMain.handle('get-trackpad-settings', async () => {
  return new Promise((resolve) => {
    
    exec('synclient -l 2>/dev/null', (error, stdout) => {
      if (!error && stdout) {
        
        const tapToClick = stdout.includes('TapButton1') && stdout.match(/TapButton1\s*=\s*(\d+)/)?.[1] === '1';
        const twoFingerScroll = stdout.includes('VertTwoFingerScroll') && stdout.match(/VertTwoFingerScroll\s*=\s*(\d+)/)?.[1] === '1';
        const naturalScrolling = stdout.includes('VertScrollDelta') && parseInt(stdout.match(/VertScrollDelta\s*=\s*(-?\d+)/)?.[1] || '0') < 0;
        const speedMatch = stdout.match(/FingerHigh\s*=\s*(\d+)/);
        const speed = speedMatch ? parseFloat(speedMatch[1]) / 50 : 1.0;
        
        resolve({
          tapToClick: tapToClick || false,
          naturalScrolling: naturalScrolling || false,
          twoFingerScrolling: twoFingerScroll || false,
          speed: speed
        });
        return;
      }
      
      
      exec('xinput list | grep -i touchpad | head -1', (error, stdout) => {
        if (error || !stdout) {
          
          resolve({
            tapToClick: false,
            naturalScrolling: false,
            twoFingerScrolling: true,
            speed: 1.0
          });
          return;
        }
        
        const deviceIdMatch = stdout.match(/id=(\d+)/);
        if (deviceIdMatch) {
          const deviceId = deviceIdMatch[1];
          exec(`xinput list-props ${deviceId} 2>/dev/null`, (error, props) => {
            if (error || !props) {
              resolve({
                tapToClick: false,
                naturalScrolling: false,
                twoFingerScrolling: true,
                speed: 1.0
              });
              return;
            }
            
            
            const tapToClick = props.includes('Tapping Enabled') && props.match(/Tapping Enabled.*\((\d+)\)/)?.[1] === '1';
            const naturalScrolling = props.includes('Natural Scrolling Enabled') && props.match(/Natural Scrolling Enabled.*\((\d+)\)/)?.[1] === '1';
            const twoFingerScroll = props.includes('Two-Finger Scrolling Enabled') && props.match(/Two-Finger Scrolling Enabled.*\((\d+)\)/)?.[1] === '1';
            const speedMatch = props.match(/libinput Accel Speed.*\(([-\d.]+)\)/);
            const speed = speedMatch ? parseFloat(speedMatch[1]) + 1.0 : 1.0;
            
            resolve({
              tapToClick: tapToClick || false,
              naturalScrolling: naturalScrolling || false,
              twoFingerScrolling: twoFingerScroll !== false,
              speed: speed
            });
          });
        } else {
          resolve({
            tapToClick: false,
            naturalScrolling: false,
            twoFingerScrolling: true,
            speed: 1.0
          });
        }
      });
    });
  });
});

ipcMain.handle('set-tap-to-click', async (event, enabled) => {
  return new Promise((resolve) => {
    
    exec(`synclient TapButton1=${enabled ? 1 : 0} 2>/dev/null`, (error) => {
      if (!error) {
        resolve({ success: true });
        return;
      }
      
      
      exec('xinput list 2>/dev/null | grep -i touchpad | head -1', (error, stdout) => {
        if (error || !stdout) {
          console.log('Trackpad not found, skipping tap to click setting');
          resolve({ success: true, skipped: true });
          return;
        }
        
        const deviceIdMatch = stdout.match(/id=(\d+)/);
        if (deviceIdMatch) {
          const deviceId = deviceIdMatch[1];
          exec(`xinput set-prop ${deviceId} "libinput Tapping Enabled" ${enabled ? 1 : 0} 2>/dev/null`, (error) => {
            if (error) {
              console.log(`Failed to set tap to click: ${error.message}`);
              resolve({ success: true, skipped: true });
              return;
            }
            resolve({ success: true });
          });
        } else {
          console.log('Could not find trackpad device ID');
          resolve({ success: true, skipped: true });
        }
      });
    });
  });
});

ipcMain.handle('set-natural-scrolling', async (event, enabled) => {
  return new Promise((resolve) => {
    
    exec(`synclient VertScrollDelta=${enabled ? -1 : 1} 2>/dev/null`, (error) => {
      if (!error) {
        resolve({ success: true });
        return;
      }
      
      
      exec('xinput list 2>/dev/null | grep -i touchpad | head -1', (error, stdout) => {
        if (error || !stdout) {
          console.log('Trackpad not found, skipping natural scrolling setting');
          resolve({ success: true, skipped: true });
          return;
        }
        
        const deviceIdMatch = stdout.match(/id=(\d+)/);
        if (deviceIdMatch) {
          const deviceId = deviceIdMatch[1];
          exec(`xinput set-prop ${deviceId} "libinput Natural Scrolling Enabled" ${enabled ? 1 : 0} 2>/dev/null`, (error) => {
            if (error) {
              console.log(`Failed to set natural scrolling: ${error.message}`);
              resolve({ success: true, skipped: true });
              return;
            }
            resolve({ success: true });
          });
        } else {
          console.log('Could not find trackpad device ID');
          resolve({ success: true, skipped: true });
        }
      });
    });
  });
});

ipcMain.handle('set-two-finger-scrolling', async (event, enabled) => {
  return new Promise((resolve) => {
    
    exec(`synclient VertTwoFingerScroll=${enabled ? 1 : 0} HorizTwoFingerScroll=${enabled ? 1 : 0} 2>/dev/null`, (error) => {
      if (!error) {
        resolve({ success: true });
        return;
      }
      
      
      exec('xinput list 2>/dev/null | grep -i touchpad | head -1', (error, stdout) => {
        if (error || !stdout) {
          console.log('Trackpad not found, skipping two-finger scrolling setting');
          resolve({ success: true, skipped: true });
          return;
        }
        
        const deviceIdMatch = stdout.match(/id=(\d+)/);
        if (deviceIdMatch) {
          const deviceId = deviceIdMatch[1];
          exec(`xinput set-prop ${deviceId} "libinput Scroll Method Enabled" 0, 0, ${enabled ? 1 : 0} 2>/dev/null`, (error) => {
            if (error) {
              console.log(`Failed to set two-finger scrolling: ${error.message}`);
              resolve({ success: true, skipped: true });
              return;
            }
            resolve({ success: true });
          });
        } else {
          console.log('Could not find trackpad device ID');
          resolve({ success: true, skipped: true });
        }
      });
    });
  });
});

ipcMain.handle('set-trackpad-speed', async (event, speed) => {
  return new Promise((resolve) => {
    
    const fingerHigh = Math.round(speed * 50);
    exec(`synclient FingerHigh=${fingerHigh} 2>/dev/null`, (error) => {
      if (!error) {
        resolve({ success: true });
        return;
      }
      
      
      exec('xinput list 2>/dev/null | grep -i touchpad | head -1', (error, stdout) => {
        if (error || !stdout) {
          
          console.log('Trackpad not found, skipping speed setting');
          resolve({ success: true, skipped: true });
          return;
        }
        
        const deviceIdMatch = stdout.match(/id=(\d+)/);
        if (deviceIdMatch) {
          const deviceId = deviceIdMatch[1];
          const libinputSpeed = speed - 1.0; 
          exec(`xinput set-prop ${deviceId} "libinput Accel Speed" ${libinputSpeed} 2>/dev/null`, (error) => {
            if (error) {
              console.log(`Failed to set trackpad speed: ${error.message}`);
              resolve({ success: true, skipped: true });
              return;
            }
            resolve({ success: true });
          });
        } else {
          console.log('Could not find trackpad device ID');
          resolve({ success: true, skipped: true });
        }
      });
    });
  });
});

ipcMain.handle('modules-list', async () => {
  return new Promise((resolve, reject) => {
    const modulesDir = path.join(__dirname, 'modules');
    
    
    if (!fs.existsSync(modulesDir)) {
      resolve({ modules: [] });
      return;
    }
    
    fs.readdir(modulesDir, { withFileTypes: true }, (error, entries) => {
      if (error) {
        reject(error);
        return;
      }
      
      const modules = [];
      
      
      entries.forEach(entry => {
        if (entry.isDirectory()) {
          const moduleDir = path.join(modulesDir, entry.name);
          const manifestPath = path.join(moduleDir, 'manifest.module');
          
          
          if (fs.existsSync(manifestPath)) {
            try {
              const manifestContent = fs.readFileSync(manifestPath, 'utf-8');
              modules.push({
                id: entry.name,
                path: moduleDir,
                manifest: manifestContent
              });
            } catch (readError) {
              console.error(`Error reading manifest for module ${entry.name}:`, readError);
              
            }
          }
        }
      });
      
      resolve({ modules: modules });
    });
  });
});

ipcMain.handle('module-read-file', async (event, moduleId, filePath) => {
  return new Promise((resolve, reject) => {
    const modulesDir = path.join(__dirname, 'modules');
    const moduleDir = path.join(modulesDir, moduleId);
    const fullPath = path.join(moduleDir, filePath);
    
    
    if (!fullPath.startsWith(moduleDir + path.sep) && fullPath !== moduleDir) {
      reject(new Error('Access denied: File outside module directory'));
      return;
    }
    
    if (!fs.existsSync(fullPath)) {
      reject(new Error('File not found'));
      return;
    }
    
    fs.readFile(fullPath, 'utf-8', (error, data) => {
      if (error) {
        reject(error);
        return;
      }
      resolve({ content: data });
    });
  });
});

app.on('window-all-closed', () => {
  
  
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

