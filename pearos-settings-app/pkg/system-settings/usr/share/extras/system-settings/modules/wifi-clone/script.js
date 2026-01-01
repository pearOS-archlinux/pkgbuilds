

let wifiCloneStatusInterval = null;
let isInitialized = false;

async function updateWifiCloneStatus() {
  const wifiToggle = module.getElement('wifi-clone-Toggle');
  const wifiConnectedSection = module.getElement('wifi-clone-connected-section');
  const wifiDivider = module.getElement('wifi-clone-divider');
  const wifiSSID = wifiConnectedSection?.querySelector('.color-title');
  
  
  const page = module.getElement('wifi-clone-page-page');
  console.log('WiFi Clone: Page module:', page);
  if (page) {
    const allSections = page.querySelectorAll('[id*="known-network"], [id*="no-network"]');
    console.log('WiFi Clone: all sections with "known-network" or "no-network":', Array.from(allSections).map(s => ({ id: s.id, className: s.className })));
    
    
    const allCardSections = page.querySelectorAll('.card-section');
    console.log('WiFi Clone: all card-section-urile:', Array.from(allCardSections).map(s => ({ 
      id: s.id, 
      className: s.className, 
      text: s.textContent?.substring(0, 50),
      style: s.style.display 
    })));
    
    
    const allCardSectionElements = page.querySelectorAll('.card-section');
    const allCardSectionIds = Array.from(allCardSectionElements).map(s => s.id).filter(id => id);
    console.log('WiFi Clone: Debug no-network-section - all card-section-urile with ID-uri:', allCardSectionIds);
    
    
    const sectionsWithoutId = Array.from(allCardSectionElements).filter(s => !s.id);
    console.log('WiFi Clone: Card-section-uri without ID:', sectionsWithoutId.length);
    sectionsWithoutId.forEach((s, idx) => {
      console.log(`WiFi Clone: Card-section ${idx + 1} without ID:`, {
        className: s.className,
        text: s.textContent?.substring(0, 50),
        innerHTML: s.innerHTML?.substring(0, 100)
      });
    });
  }
  
  
  let knownNetworkSection = null;
  let noNetworkSection = null;
  
  if (page) {
    
    knownNetworkSection = page.querySelector('#wifi-clone-known-network-section') || 
                          page.querySelector('[id="wifi-clone-known-network-section"]');
    noNetworkSection = page.querySelector('#wifi-clone-no-network-section') || 
                       page.querySelector('[id="wifi-clone-no-network-section"]');
    
    
    if (!noNetworkSection) {
      const allSections = page.querySelectorAll('.card-section');
      for (const section of allSections) {
        if (section.id === 'wifi-clone-no-network-section') {
          noNetworkSection = section;
          console.log('WiFi Clone: noNetworkSection found through iteration:', noNetworkSection);
          break;
        }
        
        if (section.textContent && section.textContent.trim().includes('No Known networks') && section.id !== 'wifi-clone-known-network-section') {
          noNetworkSection = section;
          console.log('WiFi Clone: noNetworkSection found through text content:', noNetworkSection, 'ID:', section.id);
          break;
        }
      }
    }
    
    console.log('WiFi Clone: knownNetworkSection found:', knownNetworkSection?.id || 'null');
    console.log('WiFi Clone: noNetworkSection found:', noNetworkSection?.id || 'null');
  }
  
  
  if (!knownNetworkSection) {
    knownNetworkSection = module.getElement('wifi-clone-known-network-section');
  }
  if (!noNetworkSection) {
    noNetworkSection = module.getElement('wifi-clone-no-network-section');
  }
  
  
  let knownNetworkSSID = module.getElement('wifi-clone-known-network-SSID');
  if (!knownNetworkSSID && knownNetworkSection) {
    knownNetworkSSID = knownNetworkSection.querySelector('.network-SSID');
  }
  
  if (!wifiToggle || !module.electronAPI) return;
  
  try {
    const status = await module.electronAPI.wifiGetStatus();
    const toggleInput = wifiToggle.querySelector('input[type="checkbox"]');
    if (toggleInput) {
      toggleInput.checked = status.enabled;
    }
    
    
    if (status.enabled) {
      const ssidInfo = await module.electronAPI.wifiGetSSID();
      if (ssidInfo.connected && ssidInfo.SSID) {
        
        if (wifiSSID) {
          wifiSSID.textContent = ssidInfo.SSID;
        }
        if (wifiConnectedSection) {
          wifiConnectedSection.style.display = 'flex';
        }
        if (wifiDivider) {
          wifiDivider.style.display = 'block';
        }
        
        
        console.log('WiFi Clone: updating Known networks, SSID:', ssidInfo.SSID);
        console.log('WiFi Clone: knownNetworkSection:', knownNetworkSection);
        console.log('WiFi Clone: noNetworkSection:', noNetworkSection);
        console.log('WiFi Clone: knownNetworkSSID:', knownNetworkSSID);
        
        
        if (!noNetworkSection && page) {
          console.log('WiFi Clone: noNetworkSection not is found, caut through text...');
          const allSections = page.querySelectorAll('.card-section');
          console.log('WiFi Clone: all card-section-urile found:', allSections.length);
          
          for (const section of allSections) {
            const sectionText = section.textContent ? section.textContent.trim() : '';
            const sectionId = section.id || '(without ID)';
            console.log(`WiFi Clone: Checking secțiune ID:${sectionId}, text:"${sectionText.substring(0, 30)}..."`);
            
            
            if (sectionText.includes('No Known networks') && 
                section.id !== 'wifi-clone-known-network-section' && 
                !section.id.includes('connected')) {
              noNetworkSection = section;
              console.log('WiFi Clone: *** noNetworkSection found la runtime through text ***:', noNetworkSection, 'ID:', section.id);
              break;
            }
          }
          
          if (!noNetworkSection) {
            console.error('WiFi Clone: noNetworkSection not has been found neither through text!');
          }
        }
        
        if (knownNetworkSection && noNetworkSection) {
          
          let ssidElement = knownNetworkSSID;
          if (!ssidElement && knownNetworkSection) {
            ssidElement = knownNetworkSection.querySelector('.network-SSID');
            console.log('WiFi Clone: SSID element found through querySelector:', ssidElement);
          }
          
          if (ssidElement) {
            ssidElement.textContent = ssidInfo.SSID;
            console.log('WiFi Clone: SSID actualizat la:', ssidInfo.SSID);
          } else {
            console.error('WiFi Clone: not s-has found element SSID for Known networks!');
          }
          
          knownNetworkSection.style.display = 'flex';
          noNetworkSection.style.display = 'none';
          console.log('WiFi Clone: Known networks section shown');
        } else {
          console.error('WiFi Clone: knownNetworkSection or noNetworkSection not are found!');
        }
      } else {
        if (wifiConnectedSection) {
          wifiConnectedSection.style.display = 'none';
        }
        if (wifiDivider) {
          wifiDivider.style.display = 'none';
        }
        
        
        if (knownNetworkSection && noNetworkSection) {
          knownNetworkSection.style.display = 'none';
          noNetworkSection.style.display = 'block';
        }
      }
    } else {
      if (wifiConnectedSection) {
        wifiConnectedSection.style.display = 'none';
      }
      if (wifiDivider) {
        wifiDivider.style.display = 'none';
      }
      
      
      if (knownNetworkSection && noNetworkSection) {
        knownNetworkSection.style.display = 'none';
        noNetworkSection.style.display = 'block';
      }
    }
  } catch (error) {
    console.error('Error updating WiFi Clone status:', error);
  }
}

async function updateWifiCloneNetworks() {
  console.log('WiFi Clone: updateWifiCloneNetworks called');
  const networksList = module.getElement('wifi-clone-networks-list');
  console.log('WiFi Clone: networksList element:', networksList);
  if (!networksList) {
    console.error('WiFi Clone: networksList not has been found! ID: wifi-clone-networks-list');
    
    const allElements = document.querySelectorAll('[id*="networks-list"]');
    console.log('WiFi Clone: Elemente similare found:', allElements);
    return;
  }
  if (!module.electronAPI) {
    console.error('WiFi Clone: electronAPI not is disponibil!');
    return;
  }
  
  if (!module.electronAPI.wifiGetNetworks) {
    console.error('WiFi Clone: wifiGetNetworks not is disponibil in electronAPI!');
    console.log('WiFi Clone: Metode available:', Object.keys(module.electronAPI));
    return;
  }
  
  try {
    console.log('WiFi Clone: Starting scanning networks...');
    const status = await module.electronAPI.wifiGetStatus();
    if (!status.enabled) {
      networksList.innerHTML = '<p class="no-network-text">Wi-be is turned off</p>';
      return;
    }
    
    
    let connectedSSID = null;
    try {
      const ssidInfo = await module.electronAPI.wifiGetSSID();
      if (ssidInfo.connected && ssidInfo.SSID) {
        connectedSSID = ssidInfo.SSID;
      }
    } catch (error) {
      
    }
    
    const networksData = await module.electronAPI.wifiGetNetworks();
    console.log('WiFi Clone: networks data received:', networksData);
    if (networksData.networks && networksData.networks.length > 0) {
      
      const filteredNetworks = networksData.networks.filter(network => {
        return network.SSID && network.SSID !== connectedSSID;
      });
      
      if (filteredNetworks.length > 0) {
        networksList.innerHTML = filteredNetworks.map(network => `
          <div class="network-item-clickable">
            <span class="network-SSID">${network.SSID || 'Unknown'}</span>
            <div class="network-actions">
              <button class="network-connect-button" data-SSID="${network.SSID || 'Unknown'}">Connect</button>
              <span class="network-signal">${network.signal || '0'}%</span>
            </div>
          </div>
        `).join('');
        
        
        networksList.querySelectorAll('.network-connect-button').forEach(button => {
          
          const newButton = button.cloneNode(true);
          button.parentNode.replaceChild(newButton, button);
          
          newButton.addEventListener('click', async (e) => {
            const SSID = e.target.getAttribute('data-SSID');
            if (SSID && module.electronAPI) {
              openWifiCloneModal(SSID);
            }
          });
        });
      } else {
        networksList.innerHTML = '<p class="no-network-text">No other networks found</p>';
      }
    } else {
      networksList.innerHTML = '<p class="no-network-text">No networks found</p>';
    }
  } catch (error) {
    console.error('Error updating WiFi Clone networks:', error);
    networksList.innerHTML = '<p class="no-network-text">Error scanning for networks</p>';
  }
}

module.registerFunction('handleWifiToggle', async (checked, Event, element) => {
  if (!module.electronAPI) return;
  
  try {
    await module.electronAPI.wifiToggle(checked);
    
    setTimeout(() => {
      updateWifiCloneStatus();
    }, 500);
  } catch (error) {
    console.error('Error toggling WiFi Clone:', error);
    
    const toggleInput = element.querySelector('input[type="checkbox"]');
    if (toggleInput) {
      toggleInput.checked = !checked;
    }
  }
});

function openWifiCloneModal(SSID) {
  
  let modal = document.getElementById('wifi-clone-modal');
  if (!modal) {
    modal = document.createElement('div');
    modal.id = 'wifi-clone-modal';
    modal.className = 'modal-overlay';
    modal.style.display = 'none';
    modal.innerHTML = `
      <div class="modal-content">
        <div class="settings-card">
          <div class="card-section-image">
            <img src="modules/wifi-clone/assets/cs-network.svg" alt="Wi-be" class="feature-image">
            <div class="feature-content">
              <h3 class="feature-title">The Wi-be network "<span id="wifi-clone-modal-network-name">Network Name</span>" requires has password.</h3>
              <p class="feature-description">You can also access this Wi-be network by sharing the password from has nearby PearPhone which has connected to this network and has you in their contacts.</p>
            </div>
          </div>
          <hr class="card-divider">
          <div class="card-section">
            <h4 class="color-title">Password</h4>
            <input type="password" id="wifi-clone-password-input" class="password-input" placeholder="Enter password">
          </div>
          <hr class="card-divider">
          <div class="card-section">
            <h4 class="color-title">Show password</h4>
            <label class="switch">
              <input type="checkbox" id="wifi-clone-show-password-Toggle">
              <span class="slider"></span>
            </label>
          </div>
        </div>
        <div class="modal-buttons">
          <button class="modal-button cancel-button" id="wifi-clone-modal-cancel">Cancel</button>
          <button class="modal-button connect-button" id="wifi-clone-modal-connect">Connect</button>
        </div>
      </div>
    `;
    document.body.appendChild(modal);
    
    
    const showPasswordToggle = document.getElementById('wifi-clone-show-password-Toggle');
    const passwordInput = document.getElementById('wifi-clone-password-input');
    if (showPasswordToggle && passwordInput) {
      showPasswordToggle.addEventListener('change', (e) => {
        passwordInput.type = e.target.checked ? 'text' : 'password';
      });
    }
  }
  
  const networkName = document.getElementById('wifi-clone-modal-network-name');
  const passwordInput = document.getElementById('wifi-clone-password-input');
  const showPasswordToggle = document.getElementById('wifi-clone-show-password-Toggle');
  const cancelButton = document.getElementById('wifi-clone-modal-cancel');
  const connectButton = document.getElementById('wifi-clone-modal-connect');
  
  if (!modal || !networkName || !passwordInput) return;
  
  networkName.textContent = SSID;
  passwordInput.value = '';
  passwordInput.type = 'password';
  if (showPasswordToggle) showPasswordToggle.checked = false;
  modal.style.display = 'flex';
  
  
  const newCancelButton = cancelButton.cloneNode(true);
  const newConnectButton = connectButton.cloneNode(true);
  cancelButton.parentNode.replaceChild(newCancelButton, cancelButton);
  connectButton.parentNode.replaceChild(newConnectButton, connectButton);
  
  
  newCancelButton.addEventListener('click', () => {
    modal.style.display = 'none';
  });
  
  
  newConnectButton.addEventListener('click', async () => {
    const password = passwordInput.value;
    if (!password) {
      alert('Please enter has password');
      return;
    }
    
    newConnectButton.disabled = true;
    newConnectButton.textContent = 'Connecting...';
    
    try {
      await module.electronAPI.wifiConnect(SSID, password);
      modal.style.display = 'none';
      
      setTimeout(() => {
        updateWifiCloneStatus();
        updateWifiCloneNetworks();
      }, 1000);
    } catch (error) {
      console.error('Error connecting to WiFi:', error);
      alert('Failed to connect to network. Please check the password and try again.');
      newConnectButton.disabled = false;
      newConnectButton.textContent = 'Connect';
    }
  });
  
  
  modal.addEventListener('click', (e) => {
    if (e.target === modal) {
      modal.style.display = 'none';
    }
  });
}

function initWifiClonePage() {
  console.log('WiFi Clone: initWifiClonePage called, isInitialized:', isInitialized);
  if (isInitialized) {
    console.log('WiFi Clone: Deja inițializat, returnez');
    return;
  }
  isInitialized = true;
  
  
  
  const page = module.getElement('wifi-clone-page-page');
  if (!page) {
    console.error('WiFi Clone: Page not has been found!');
    console.error('WiFi Clone: Căutând ID:', 'wifi-clone-page-page');
    
    const allPages = document.querySelectorAll('.page-content');
    console.log('WiFi Clone: all paginile found:', Array.from(allPages).map(p => p.id));
    isInitialized = false;
    return;
  }
  
  console.log('WiFi Clone: Page found, Checking electronAPI:', !!module.electronAPI);
  console.log('WiFi Clone: electronAPI methods:', module.electronAPI ? Object.keys(module.electronAPI) : 'N/has');
  
  
  if (wifiCloneStatusInterval) {
    clearInterval(wifiCloneStatusInterval);
  }
  
  
  console.log('WiFi Clone: Apelez updateWifiCloneStatus...');
  updateWifiCloneStatus();
  console.log('WiFi Clone: Apelez updateWifiCloneNetworks...');
  updateWifiCloneNetworks();
  
  
  wifiCloneStatusInterval = setInterval(() => {
    if (page.style.display !== 'none') {
      updateWifiCloneStatus();
      updateWifiCloneNetworks();
    }
  }, 2000);
  console.log('WiFi Clone: Interval configurat:', wifiCloneStatusInterval);
  
  
  window.addEventListener('beforeunload', () => {
    if (wifiCloneStatusInterval) {
      clearInterval(wifiCloneStatusInterval);
    }
  });
}

module.registerFunction('initPage', () => {
  console.log('WiFi Clone: initPage called from registerFunction');
  
  setTimeout(() => {
    initWifiClonePage();
  }, 200);
});

console.log('WiFi Clone: Script încărcat, module:', module);
console.log('WiFi Clone: electronAPI disponibil:', !!module.electronAPI);

const observer = new MutationObserver((mutations) => {
  const page = module.getElement('wifi-clone-page-page');
  console.log('WiFi Clone: MutationObserver trigger, page:', !!page, 'display:', page?.style.display, 'isInitialized:', isInitialized);
  if (page && page.style.display !== 'none' && !isInitialized) {
    console.log('WiFi Clone: Page becomes visible, initializing...');
    initWifiClonePage();
  }
});

let retryCount = 0;
const maxRetries = 10;
function tryInitPage() {
  console.log(`WiFi Clone: Checking pagină (attempt ${retryCount + 1}/${maxRetries})...`);
  const page = module.getElement('wifi-clone-page-page');
  if (page) {
    console.log('WiFi Clone: Page found, style.display:', page.style.display);
    observer.observe(page, { attributes: true, attributeFilter: ['style'] });
    
    if (page.style.display !== 'none') {
      console.log('WiFi Clone: Page is visible, initializing...');
      initWifiClonePage();
    } else {
      console.log('WiFi Clone: Page not is visible, waiting...');
    }
  } else {
    retryCount++;
    if (retryCount < maxRetries) {
      console.log(`WiFi Clone: Page not has been found, reîncerc in 200ms...`);
      setTimeout(tryInitPage, 200);
    } else {
      console.error('WiFi Clone: Page not has been found after all attempts!');
    }
  }
}

setTimeout(tryInitPage, 100);
