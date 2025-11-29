

module.registerFunction('handleDevModeToggle', (checked, event, element) => {
  console.log('Developer Mode toggled:', checked);
  
  
  if (module.electronAPI) {
    
    localStorage.setItem(`module-${module.moduleId}-dev-mode`, checked);
  }
  
  
  const label = document.querySelector(`label[for="${element.id}"]`) || 
                element.closest('.card-section')?.querySelector('.color-title');
  if (label) {
    label.style.color = checked ? 'var(--accent-color)' : 'var(--text-primary)';
  }
});

module.registerFunction('handleLogLevelChange', (value, event, element) => {
  console.log('Logging level changed to:', value);
  
  
  if (module.electronAPI) {
    localStorage.setItem(`module-${module.moduleId}-log-level`, value);
  }
  
  
  console.log(`[${value.toUpperCase()}] Logging level changed`);
});

module.registerFunction('handleResetButton', (event, element) => {
  console.log('Reset button clicked');
  
  
  if (confirm('Are you sure you want to reset all settings?')) {
    
    localStorage.removeItem(`module-${module.moduleId}-dev-mode`);
    localStorage.removeItem(`module-${module.moduleId}-log-level`);
    
    
    const devModeToggle = module.getElement('test-dev-mode');
    if (devModeToggle) {
      devModeToggle.checked = false;
    }
    
    
    const logLevelSelect = module.getElement('test-log-level');
    if (logLevelSelect) {
      logLevelSelect.value = 'debug';
    }
    
    console.log('Settings reset completed');
    
    
    alert('Settings have been reset!');
  }
});

module.registerFunction('navigateToAdvancedPage', (event, element) => {
  console.log('Navigating to advanced page');
  
  
  const currentPage = document.getElementById('test-page-page');
  if (currentPage) {
    currentPage.style.display = 'none';
  }
  
  
  const advancedPage = document.getElementById('test-advanced-page-page');
  if (advancedPage) {
    advancedPage.style.display = 'block';
    
    
    const pageTitle = document.querySelector('.page-title');
    if (pageTitle) {
      pageTitle.textContent = 'Advanced Settings';
    }
    
    
    feather.replace();
  } else {
    console.error('Advanced page not found!');
  }
});

module.registerFunction('navigateToMainPage', (event, element) => {
  console.log('Navigating back to main page');
  
  
  const advancedPage = document.getElementById('test-advanced-page-page');
  if (advancedPage) {
    advancedPage.style.display = 'none';
  }
  
  
  const mainPage = document.getElementById('test-page-page');
  if (mainPage) {
    mainPage.style.display = 'block';
    
    
    const pageTitle = document.querySelector('.page-title');
    if (pageTitle) {
      pageTitle.textContent = 'Developer Settings';
    }
    
    
    feather.replace();
  } else {
    console.error('Main page not found!');
  }
});

