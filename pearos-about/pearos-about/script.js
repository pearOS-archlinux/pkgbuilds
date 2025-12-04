//feather.replace();

document.addEventListener('DOMContentLoaded', () => {
  const redDot = document.querySelector('.traffic-lights .dot.red');
  const yellowDot = document.querySelector('.traffic-lights .dot.yellow');
  const greenDot = document.querySelector('.traffic-lights .dot.green');

  if (window.electronAPI) {
    if (redDot) {
      redDot.addEventListener('click', () => {
        window.electronAPI.closeWindow();
      });
    }

    if (yellowDot) {
      yellowDot.addEventListener('click', () => {
        window.electronAPI.minimizeWindow();
      });
    }

    if (greenDot) {
      greenDot.addEventListener('click', () => {
        window.electronAPI.maximizeWindow();
      });
    }
  }

  // Dark mode automat bazat pe sistem
  function getSystemTheme() {
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
      return 'dark';
    }
    return 'light';
  }

  function applyTheme() {
    const root = document.documentElement;
    const systemTheme = getSystemTheme();
    root.setAttribute('data-theme', systemTheme);
  }

  // Inițializează tema
  applyTheme();

  // Ascultă schimbările de temă ale sistemului
  const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
  mediaQuery.addEventListener('change', () => {
    applyTheme();
  });

  // Inițializează pagina About
  initAboutPage();

  // Setează anul curent în copyright
  const currentYearElement = document.getElementById('current-year');
  if (currentYearElement) {
    currentYearElement.textContent = new Date().getFullYear();
  }

  // Buton More Info
  const moreInfoButton = document.getElementById('more-info-button');
  if (moreInfoButton) {
    moreInfoButton.addEventListener('click', () => {
      if (window.electronAPI && window.electronAPI.openSystemSettingsAbout) {
        window.electronAPI.openSystemSettingsAbout().catch((error) => {
          console.error('Error opening system settings about:', error);
        });
      }
    });
  }

  // Funcție pentru a printea dimensiunile ferestrei
  function printWindowSize() {
    const windowSize = {
      innerWidth: window.innerWidth,
      innerHeight: window.innerHeight,
      outerWidth: window.outerWidth,
      outerHeight: window.outerHeight,
      clientWidth: document.documentElement.clientWidth,
      clientHeight: document.documentElement.clientHeight,
      screenWidth: window.screen.width,
      screenHeight: window.screen.height
    };
    console.log('=== Dimensiuni Fereastră ===');
    console.log('Inner (viewport):', windowSize.innerWidth + 'x' + windowSize.innerHeight);
    console.log('Outer (fereastră completă):', windowSize.outerWidth + 'x' + windowSize.outerHeight);
    console.log('Client (document):', windowSize.clientWidth + 'x' + windowSize.clientHeight);
    console.log('Screen (ecran):', windowSize.screenWidth + 'x' + windowSize.screenHeight);
    console.log('===========================');
    return windowSize;
  }

  // Printează dimensiunile la încărcare
  printWindowSize();

  // Printează dimensiunile când fereastra se redimensionează
  window.addEventListener('resize', () => {
    printWindowSize();
  });
});

async function initAboutPage() {
  if (!window.electronAPI) return;

  try {
    const systemInfo = await window.electronAPI.getSystemInfo();
    
    // Actualizează chip
    const chipElement = document.getElementById('about-chip');
    if (chipElement) {
      chipElement.textContent = systemInfo.chip || 'Unknown';
    }

    // Actualizează memory
    const memoryElement = document.getElementById('about-memory');
    if (memoryElement) {
      memoryElement.textContent = systemInfo.memory || 'Unknown';
    }

    // Actualizează serial number
    const serialElement = document.getElementById('about-serial');
    if (serialElement) {
      serialElement.textContent = systemInfo.serial || 'Unknown';
    }

    // Verifică dacă are baterie
    let hasBattery = false;
    try {
      const batteryInfo = await window.electronAPI.getBatteryInfo();
      hasBattery = batteryInfo && batteryInfo.percentage !== undefined;
    } catch (batteryError) {
      hasBattery = false;
    }

    // Actualizează imaginea și tipul dispozitivului
    const deviceImage = document.getElementById('about-device-image');
    const deviceType = document.getElementById('about-device-type');
    
    if (hasBattery) {
      if (deviceImage) deviceImage.src = './assets/laptop.svg';
      if (deviceType) deviceType.textContent = 'Laptop';
    } else {
      if (deviceImage) deviceImage.src = './assets/computer.svg';
      if (deviceType) deviceType.textContent = 'Computer';
    }

    // Actualizează OS name și versiune
    const osNameElement = document.getElementById('os-name');
    if (osNameElement) {
      osNameElement.textContent = systemInfo.osName || 'pearOS';
    }

    const osVersionElement = document.getElementById('os-version');
    if (osVersionElement) {
      osVersionElement.textContent = systemInfo.osVersion || 'Unknown';
    }
  } catch (error) {
    console.error('Error loading system info:', error);
    
    // Setează valori default în caz de eroare
    const chipElement = document.getElementById('about-chip');
    if (chipElement) chipElement.textContent = 'Error loading info';
    
    const memoryElement = document.getElementById('about-memory');
    if (memoryElement) memoryElement.textContent = 'Error loading info';
    
    const serialElement = document.getElementById('about-serial');
    if (serialElement) serialElement.textContent = 'Error loading info';
    
    const deviceImage = document.getElementById('about-device-image');
    const deviceType = document.getElementById('about-device-type');
    if (deviceImage) deviceImage.src = './assets/computer.svg';
    if (deviceType) deviceType.textContent = 'Unknown';
  }
}
