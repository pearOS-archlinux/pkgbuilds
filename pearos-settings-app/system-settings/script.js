feather.replace();

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

    // Verifică starea PearID la lansare
    checkPearIdStateAndUpdateUI();
    
    // Încarcă setarea de tint window background la pornirea aplicației
    initTintWindowSetting();
  }

  
  let navigationHistory = [];
  let currentHistoryIndex = -1;
  
  
  let dateTimeInterval = null;

  
  function updateNavigationButtons() {
    const navBack = document.getElementById('nav-back');
    const navForward = document.getElementById('nav-forward');
    
    if (navBack) {
      navBack.disabled = currentHistoryIndex <= 0;
    }
    
    if (navForward) {
      navForward.disabled = currentHistoryIndex >= navigationHistory.length - 1;
    }
  }

  
  function addToHistory(pageName) {
    
    if (currentHistoryIndex < navigationHistory.length - 1) {
      navigationHistory = navigationHistory.slice(0, currentHistoryIndex + 1);
    }
    
    
    navigationHistory.push(pageName);
    currentHistoryIndex = navigationHistory.length - 1;
    
    
    updateNavigationButtons();
  }

  
  function updatePageTitleAndSidebar(pageName) {
    const pageTitle = document.querySelector('.page-title');
    if (pageTitle) {
      
      if (pageName === 'Battey' || pageName === 'Battery') {
        updateBatteryTitle();
      } else if (pageName === 'General') {
        
        pageTitle.textContent = '';
        
        const subtitle = pageTitle.querySelector('.battery-subtitle');
        if (subtitle) {
          subtitle.remove();
        }
      } else if (pageName === 'User Profile') {
        // Pentru pagina de profil, afișează "Pear Account" ca titlu
        pageTitle.textContent = 'Pear Account';
        
        const subtitle = pageTitle.querySelector('.battery-subtitle');
        if (subtitle) {
          subtitle.remove();
        }
      } else if (pageName === 'PearID Sign In') {
        // Pentru pagina de sign in, afișează "Pear Account" ca titlu
        pageTitle.textContent = 'Pear Account';
        
        const subtitle = pageTitle.querySelector('.battery-subtitle');
        if (subtitle) {
          subtitle.remove();
        }
      } else {
        pageTitle.textContent = pageName;
        
        const subtitle = pageTitle.querySelector('.battery-subtitle');
        if (subtitle) {
          subtitle.remove();
        }
      }
    }

    // Pentru pagina User Profile și sub-paginile sale, selectează user-signin-section în sidebar
    const pearAccountSubPages = ['User Profile', 'Personal Information', 'Sign In & Security', 'Payment & Shipping', 'Pear Cloud', 'Media & Purchases', 'Sign in with Pear', 'Device Info', 'PearID Sign In'];
    if (pearAccountSubPages.includes(pageName)) {
      const sidebarItems = document.querySelectorAll('.sidenav .item');
      sidebarItems.forEach(i => i.classList.remove('active'));
      
      const userSigninSection = document.querySelector('.user-signin-section');
      if (userSigninSection) {
        userSigninSection.classList.add('active');
      }
    } else {
      // Pentru celelalte pagini, selectează item-ul corespunzător
      const sidebarItems = document.querySelectorAll('.sidenav .item');
      // Elimină clasa active de la user-signin-section când se selectează alte pagini
      const userSigninSection = document.querySelector('.user-signin-section');
      if (userSigninSection) {
        userSigninSection.classList.remove('active');
      }
      
      sidebarItems.forEach(item => {
        const itemName = item.querySelector('.name');
        if (itemName && itemName.textContent.trim() === pageName) {
          sidebarItems.forEach(i => i.classList.remove('active'));
          item.classList.add('active');
        }
      });
    }
  }

  
  const navBack = document.getElementById('nav-back');
  const navForward = document.getElementById('nav-forward');
  
  
  if (navBack) {
    navBack.addEventListener('click', () => {
      if (currentHistoryIndex > 0) {
        currentHistoryIndex--;
        const previousPage = navigationHistory[currentHistoryIndex];
        updatePageTitleAndSidebar(previousPage);
        showPage(previousPage, false); 
        updateNavigationButtons();
      }
    });
  }
  
  if (navForward) {
    navForward.addEventListener('click', () => {
      if (currentHistoryIndex < navigationHistory.length - 1) {
        currentHistoryIndex++;
        const nextPage = navigationHistory[currentHistoryIndex];
        updatePageTitleAndSidebar(nextPage);
        showPage(nextPage, false); 
        updateNavigationButtons();
      }
    });
  }

  
  updateNavigationButtons();

  
  addToHistory('General');

  // Funcție pentru verificarea stării PearID și actualizarea UI-ului
  async function checkPearIdStateAndUpdateUI() {
    if (!window.electronAPI || !window.electronAPI.checkPearIdState) {
      return;
    }

    try {
      const stateResult = await window.electronAPI.checkPearIdState();
      const state = stateResult.state ? stateResult.state.trim().toLowerCase() : 'false';

      if (state === 'true') {
        // Utilizatorul este autentificat, obține informațiile
        try {
          const userInfo = await window.electronAPI.getUserInfo();
          const firstName = userInfo.firstName || '';
          const lastName = userInfo.lastName || '';
          const fullName = `${firstName} ${lastName}`.trim() || 'User';

          // Obține avatarul utilizatorului
          try {
            const avatarResult = await window.electronAPI.getUserAvatar();
            const avatarPath = avatarResult.avatarPath;

            // Actualizează UI-ul
            const userAvatar = document.querySelector('.user-signin-section .user-avatar');
            const userSigninTitle = document.querySelector('.user-signin-title');
            const userSigninDescription = document.querySelector('.user-signin-description');

            if (userAvatar && avatarPath) {
              // Înlocuiește poza cu avatarul utilizatorului
              // Folosim file:// pentru a accesa fișierul local
              userAvatar.src = `file://${avatarPath}`;
            }

            if (userSigninTitle) {
              // Înlocuiește "Sign in" cu numele utilizatorului
              userSigninTitle.textContent = fullName;
            }

            if (userSigninDescription) {
              // Înlocuiește "with your pear account" cu "Pear Account"
              userSigninDescription.textContent = 'Pear Account';
            }
          } catch (avatarError) {
            console.error('Error getting user avatar:', avatarError);
            // Continuă fără avatar, doar actualizează textul
            const userSigninTitle = document.querySelector('.user-signin-title');
            const userSigninDescription = document.querySelector('.user-signin-description');

            if (userSigninTitle) {
              userSigninTitle.textContent = fullName;
            }

            if (userSigninDescription) {
              userSigninDescription.textContent = 'Pear Account';
            }
          }
        } catch (error) {
          console.error('Error getting user info:', error);
        }
      }
      // Dacă state este 'false', UI-ul rămâne neschimbat (comportamentul default)
    } catch (error) {
      console.error('Error checking PearID state:', error);
    }
  }

  // Funcție pentru inițializarea paginii de sign in PearID
  async function initPearIdSignInPage() {
    // Verifică dacă device-ul este blocat (Activation Lock)
    let lockEmail = '';
    if (window.electronAPI && window.electronAPI.checkPearIdLock) {
      try {
        const lockResult = await window.electronAPI.checkPearIdLock();
        lockEmail = lockResult.email || '';
      } catch (error) {
        console.error('Error checking PearID lock:', error);
      }
    }
    
    // Actualizează UI-ul în funcție de starea de lock
    const titleElement = document.getElementById('pearid-signin-title');
    const descriptionElement = document.getElementById('pearid-signin-description');
    const lockEmailElement = document.getElementById('pearid-signin-lock-email');
    
    if (lockEmail && lockEmail.trim() !== '') {
      // Device-ul este blocat - afișează Activation Lock
      if (titleElement) {
        titleElement.textContent = 'Activation Lock';
      }
      if (descriptionElement) {
        descriptionElement.textContent = 'This computer is linked to a Pear ID. Enter the Pear ID and password that was previously used with this Computer.';
      }
      if (lockEmailElement) {
        lockEmailElement.textContent = lockEmail;
        lockEmailElement.style.display = 'block';
      }
    } else {
      // Device-ul nu este blocat - afișează mesajul normal
      if (titleElement) {
        titleElement.textContent = 'One account for everything Pear.';
      }
      if (descriptionElement) {
        descriptionElement.textContent = 'Sign in to securely access your personal data stored in PearCloud, set up the Pear Store, and use all Pear services on this Computer.';
      }
      if (lockEmailElement) {
        lockEmailElement.style.display = 'none';
      }
    }
    
    const emailInput = document.getElementById('pearid-signin-email-input');
    const continueButton = document.getElementById('pearid-signin-continue-button');
    
    if (!emailInput || !continueButton) {
      console.error('PearID sign in page elements not found');
      return;
    }
    
    // Elimină event listener-urile vechi dacă există
    const newEmailInput = emailInput.cloneNode(true);
    emailInput.parentNode.replaceChild(newEmailInput, emailInput);
    
    const newContinueButton = continueButton.cloneNode(true);
    continueButton.parentNode.replaceChild(newContinueButton, continueButton);
    
    // Obține referințele noi
    const emailInputNew = document.getElementById('pearid-signin-email-input');
    const continueButtonNew = document.getElementById('pearid-signin-continue-button');
    
    // Resetează input-ul și butonul
    emailInputNew.value = '';
    continueButtonNew.disabled = true;
    continueButtonNew.setAttribute('disabled', 'disabled');
    
    // Resetează și ascunde câmpul de parolă
    const passwordSection = document.getElementById('pearid-signin-password-section');
    const passwordInput = document.getElementById('pearid-signin-password-input');
    if (passwordSection) {
      passwordSection.style.display = 'none';
      passwordSection.style.opacity = '';
      passwordSection.style.transform = '';
      passwordSection.style.transition = '';
    }
    if (passwordInput) {
      passwordInput.value = '';
    }
    
    // Event listener pentru input - activează/dezactivează butonul Continue
    emailInputNew.addEventListener('input', () => {
      const value = emailInputNew.value.trim();
      if (value === '') {
        continueButtonNew.disabled = true;
        continueButtonNew.setAttribute('disabled', 'disabled');
      } else {
        continueButtonNew.disabled = false;
        continueButtonNew.removeAttribute('disabled');
      }
    });
    
    // Event listener pentru Enter key
    emailInputNew.addEventListener('keypress', (e) => {
      if (e.key === 'Enter' && !continueButtonNew.disabled) {
        continueButtonNew.click();
      }
    });

    // Funcție pentru gestionarea login-ului
    async function handleLogin() {
      const emailInput = document.getElementById('pearid-signin-email-input');
      const passwordInput = document.getElementById('pearid-signin-password-input');
      
      if (!emailInput || !passwordInput) return;
      
      const email = emailInput.value.trim();
      const password = passwordInput.value;
      
      if (!email || !password) {
        alert('Please fill in both email and password.');
        return;
      }

      // Dezactivează butonul Continue în timpul login-ului
      continueButtonNew.disabled = true;
      continueButtonNew.setAttribute('disabled', 'disabled');
      continueButtonNew.textContent = 'Signing in...';

      try {
        const result = await window.electronAPI.loginAndSync(email, password);
        
        if (result.success) {
          // Reîncarcă starea PearID și navighează la pagina Pear Account
          await checkPearIdStateAndUpdateUI();
          await initUserProfilePage();
          showPage('User Profile');
        } else {
          alert(`Error: ${result.error || 'Failed to sign in'}`);
          continueButtonNew.disabled = false;
          continueButtonNew.removeAttribute('disabled');
          continueButtonNew.textContent = 'Continue';
        }
      } catch (error) {
        console.error('Error signing in:', error);
        alert(`Error: ${error.message || 'Failed to sign in'}`);
        continueButtonNew.disabled = false;
        continueButtonNew.removeAttribute('disabled');
        continueButtonNew.textContent = 'Continue';
      }
    }

    // Event listener pentru butonul Continue
    continueButtonNew.addEventListener('click', async () => {
      const emailValue = emailInputNew.value.trim();
      if (emailValue === '') return;

      // Verifică dacă câmpul de parolă există deja
      const passwordSection = document.getElementById('pearid-signin-password-section');
      if (passwordSection && passwordSection.style.display === 'none') {
        // Afișează animat câmpul de parolă
        passwordSection.style.display = 'block';
        passwordSection.style.opacity = '0';
        passwordSection.style.transform = 'translateY(-10px)';
        passwordSection.style.transition = 'opacity 0.3s ease, transform 0.3s ease';
        
        // Animație
        setTimeout(() => {
          passwordSection.style.opacity = '1';
          passwordSection.style.transform = 'translateY(0)';
        }, 10);

        // Focus pe câmpul de parolă
        const passwordInput = document.getElementById('pearid-signin-password-input');
        if (passwordInput) {
          setTimeout(() => {
            passwordInput.focus();
          }, 300);
        }

        // Actualizează butonul Continue pentru a verifica și parola
        const passwordInputNew = document.getElementById('pearid-signin-password-input');
        if (passwordInputNew) {
          // Verifică dacă ambele câmpuri sunt completate
          const checkFields = () => {
            const email = emailInputNew.value.trim();
            const password = passwordInputNew.value.trim();
            if (email !== '' && password !== '') {
              continueButtonNew.disabled = false;
              continueButtonNew.removeAttribute('disabled');
            } else {
              continueButtonNew.disabled = true;
              continueButtonNew.setAttribute('disabled', 'disabled');
            }
          };

          passwordInputNew.addEventListener('input', checkFields);
          
          // Event listener pentru Enter key pe parolă
          passwordInputNew.addEventListener('keypress', async (e) => {
            if (e.key === 'Enter' && !continueButtonNew.disabled) {
              await handleLogin();
            }
          });
        }
      } else {
        // Dacă câmpul de parolă este deja vizibil, înseamnă că ambele câmpuri sunt completate
        // Apelează login
        await handleLogin();
      }
    });
    
    // Event listener pentru butonul "Don't Have an Account?"
    const noAccountButton = document.getElementById('pearid-signin-no-account-button');
    if (noAccountButton) {
      noAccountButton.addEventListener('click', () => {
        openPearIdNoAccountModal();
      });
    }
  }

  // Funcție pentru deschiderea modalului "Don't Have an Account?"
  function openPearIdNoAccountModal() {
    const modal = document.getElementById('pearid-no-account-modal');
    if (!modal) return;

    // Schimbă culoarea red dot
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Afișează modalul - începe cu register form
    modal.style.display = 'flex';
    const loginForm = document.getElementById('loginForm');
    const registerForm = document.getElementById('registerForm');
    if (loginForm) loginForm.style.display = 'none';
    if (registerForm) registerForm.style.display = 'block';

    // Funcție pentru cleanup
    const cleanup = () => {
      modal.style.display = 'none';
      restoreRedDot();
    };

    // Event listener pentru click în afara modalului
    const overlayHandler = (e) => {
      if (e.target === modal) {
        cleanup();
        modal.removeEventListener('click', overlayHandler);
      }
    };
    modal.addEventListener('click', overlayHandler);

    // Event listeners pentru butoanele Cancel
    const cancelButton = document.getElementById('pearid-no-account-cancel');
    const cancelButtonRegister = document.getElementById('pearid-no-account-cancel-register');
    
    if (cancelButton) {
      cancelButton.onclick = cleanup;
    }
    if (cancelButtonRegister) {
      cancelButtonRegister.onclick = cleanup;
    }

    // Event listeners pentru switch între login și register
    const showRegister = document.getElementById('showRegister');
    const showLogin = document.getElementById('showLogin');

    if (showRegister) {
      const registerHandler = (e) => {
        e.preventDefault();
        if (loginForm) loginForm.style.display = 'none';
        if (registerForm) registerForm.style.display = 'block';
        showRegister.removeEventListener('click', registerHandler);
      };
      showRegister.addEventListener('click', registerHandler);
    }

    if (showLogin) {
      const loginHandler = (e) => {
        e.preventDefault();
        if (registerForm) registerForm.style.display = 'none';
        if (loginForm) loginForm.style.display = 'block';
        showLogin.removeEventListener('click', loginHandler);
      };
      showLogin.addEventListener('click', loginHandler);
    }

    // Event listener pentru butonul Sign In din modal
    const loginSubmitBtn = document.getElementById('loginSubmitBtn');
    if (loginSubmitBtn && !loginSubmitBtn.hasAttribute('data-listener-added')) {
      loginSubmitBtn.setAttribute('data-listener-added', 'true');
      loginSubmitBtn.addEventListener('click', async (e) => {
        e.preventDefault();
        
        const email = document.getElementById('loginEmail').value.trim();
        const password = document.getElementById('loginPassword').value;

        if (!email || !password) {
          alert('Please enter both email and password.');
          return;
        }

        try {
          const result = await window.electronAPI.loginAndSync(email, password);
          
          if (result.success) {
            alert('Signed in successfully!');
            cleanup();
            // Reîncarcă pagina pentru a actualiza starea
            location.reload();
          } else {
            alert(`Error: ${result.error || 'Failed to sign in'}`);
          }
        } catch (error) {
          console.error('Error during login:', error);
          alert(`Error: ${error.message || 'Failed to sign in'}`);
        }
      });
    }

    // Event listener pentru butonul Create Account
    const registerSubmitBtn = document.getElementById('registerSubmitBtn');
    if (registerSubmitBtn && !registerSubmitBtn.hasAttribute('data-listener-added')) {
      registerSubmitBtn.setAttribute('data-listener-added', 'true');
      registerSubmitBtn.addEventListener('click', async (e) => {
        e.preventDefault();
        
        const firstName = document.getElementById('firstName').value.trim();
        const lastName = document.getElementById('lastName').value.trim();
        const email = document.getElementById('registerEmail').value.trim();
        const password = document.getElementById('registerPassword').value;
        const birthdate = document.getElementById('birthdate').value;
        const phone = document.getElementById('phoneNumber').value.trim();
        const billingAddress = document.getElementById('billingAddress').value.trim();

        // Verifică doar câmpurile obligatorii
        if (!firstName || !lastName || !email || !password) {
          alert('Please fill in all required fields (First Name, Last Name, Email, Password).');
          return;
        }

        if (password.length < 8) {
          alert('Password must be at least 8 characters long.');
          return;
        }

        try {
          const accountData = {
            firstName,
            lastName,
            email,
            password
          };

          // Adaugă câmpurile opționale doar dacă sunt completate
          if (birthdate) {
            accountData.birthdate = birthdate;
          }
          if (phone) {
            accountData.phone = phone;
          }
          if (billingAddress) {
            accountData.billingAddress = billingAddress;
          }

          const result = await window.electronAPI.createAccount(accountData);
          
          if (result.success) {
            alert('Account created successfully!');
            cleanup();
            // Reîncarcă pagina pentru a actualiza starea
            location.reload();
          } else {
            alert(`Error: ${result.error || 'Failed to create account'}`);
          }
        } catch (error) {
          console.error('Error creating account:', error);
          alert(`Error: ${error.message || 'Failed to create account'}`);
        }
      });
    }
  }

  // Funcție pentru inițializarea paginii de profil utilizator
  async function initUserProfilePage() {
    if (!window.electronAPI) {
      console.error('electronAPI not available');
      return;
    }

    try {
      console.log('Loading user profile page...');
      const userInfo = await window.electronAPI.getUserInfo();
      console.log('User info received:', userInfo);
      
      const firstName = userInfo.firstName || '';
      const lastName = userInfo.lastName || '';
      const email = userInfo.email || '';
      const fullName = `${firstName} ${lastName}`.trim() || 'User';

      console.log('Full name:', fullName, 'Email:', email);

      // Obține avatarul
      let avatarPath = null;
      try {
        const avatarResult = await window.electronAPI.getUserAvatar();
        avatarPath = avatarResult.avatarPath;
        console.log('Avatar path:', avatarPath);
      } catch (avatarError) {
        console.error('Error getting avatar for profile:', avatarError);
      }

      // Actualizează pagina de profil
      const profileAvatar = document.getElementById('user-profile-avatar');
      const profileName = document.getElementById('user-profile-name');
      const profileEmail = document.getElementById('user-profile-email');

      console.log('Profile elements:', { profileAvatar, profileName, profileEmail });

      if (profileAvatar) {
        if (avatarPath) {
          profileAvatar.src = `file://${avatarPath}`;
        } else {
          profileAvatar.src = './assets/cs-user.svg';
        }
      }

      if (profileName) {
        profileName.textContent = fullName;
        console.log('Set name to:', fullName);
      }

      if (profileEmail) {
        profileEmail.textContent = email || 'No email available';
        console.log('Set email to:', email || 'No email available');
      }

      // Încarcă device-urile (fără cache, se rulează de fiecare dată)
      await loadUserDevices();

      // Adaugă event listener pe avatar pentru click (file chooser)
      const avatarContainer = document.getElementById('user-profile-avatar-container');
      if (avatarContainer && !avatarContainer.hasAttribute('data-listener-added')) {
        avatarContainer.setAttribute('data-listener-added', 'true');
        avatarContainer.addEventListener('click', async () => {
          await handleAvatarClick();
        });
      }

      // Adaugă event listener pe butonul Sign Out
      const signOutButton = document.getElementById('sign-out-button');
      if (signOutButton) {
        signOutButton.addEventListener('click', async () => {
          await handleSignOut();
        });
      }

      // Adaugă event listener pe Personal Information
      const personalInformationItem = document.getElementById('personal-information-item');
      if (personalInformationItem && !personalInformationItem.hasAttribute('data-listener-added')) {
        personalInformationItem.setAttribute('data-listener-added', 'true');
        personalInformationItem.addEventListener('click', async () => {
          await initPersonalInformationPage();
          showPage('Personal Information');
        });
      }

      // Event listener pentru Payment & Shipping
      const paymentShippingItem = document.getElementById('payment-shipping-item');
      if (paymentShippingItem && !paymentShippingItem.hasAttribute('data-listener-added')) {
        paymentShippingItem.setAttribute('data-listener-added', 'true');
        paymentShippingItem.addEventListener('click', async () => {
          await initPaymentShippingPage();
          showPage('Payment & Shipping');
        });
      }

      // Event listener pentru Pear Cloud
      const pearCloudItem = document.getElementById('pear-cloud-item');
      if (pearCloudItem && !pearCloudItem.hasAttribute('data-listener-added')) {
        pearCloudItem.setAttribute('data-listener-added', 'true');
        pearCloudItem.addEventListener('click', () => {
          showPage('Pear Cloud');
        });
      }

      // Event listener pentru Media & Purchases
      const mediaPurchasesItem = document.getElementById('media-purchases-item');
      if (mediaPurchasesItem && !mediaPurchasesItem.hasAttribute('data-listener-added')) {
        mediaPurchasesItem.setAttribute('data-listener-added', 'true');
        mediaPurchasesItem.addEventListener('click', async () => {
          await initMediaPurchasesPage();
          showPage('Media & Purchases');
        });
      }

      // Event listener pentru Sign in with Pear
      const signinPearItem = document.getElementById('signin-pear-item');
      if (signinPearItem && !signinPearItem.hasAttribute('data-listener-added')) {
        signinPearItem.setAttribute('data-listener-added', 'true');
        signinPearItem.addEventListener('click', () => {
          showPage('Sign in with Pear');
        });
      }

      // Adaugă event listener pe Sign In & Security
      const signinSecurityItem = document.getElementById('signin-security-item');
      if (signinSecurityItem && !signinSecurityItem.hasAttribute('data-listener-added')) {
        signinSecurityItem.setAttribute('data-listener-added', 'true');
        signinSecurityItem.addEventListener('click', async () => {
          await initSignInSecurityPage();
          showPage('Sign In & Security');
        });
      }
    } catch (error) {
      console.error('Error initializing user profile page:', error);
    }
  }

  // Funcție pentru inițializarea paginii Sign In & Security
  // Funcție pentru inițializarea paginii Media & Purchases
  async function initMediaPurchasesPage() {
    if (!window.electronAPI) {
      console.error('electronAPI not available');
      return;
    }

    try {
      // Obține numele și prenumele din cache
      const userInfo = await window.electronAPI.getUserInfo();
      const fullName = `${userInfo.firstName || ''} ${userInfo.lastName || ''}`.trim() || 'Not set';

      // Actualizează pagina
      const accountNameElement = document.getElementById('media-purchases-account-name');
      if (accountNameElement) {
        accountNameElement.textContent = fullName;
      }

      // Obține aplicațiile
      const appsResult = await window.electronAPI.getUserApps();
      const apps = appsResult.apps || [];

      // Actualizează lista de aplicații
      const appsListElement = document.getElementById('media-purchases-apps-list');
      if (appsListElement) {
        appsListElement.innerHTML = '';
        
        if (apps.length === 0) {
          const emptyItem = document.createElement('div');
          emptyItem.className = 'general-menu-item';
          emptyItem.innerHTML = '<span style="font-size: 13px; color: var(--text-secondary);">No applications found</span>';
          appsListElement.appendChild(emptyItem);
        } else {
          apps.forEach((app) => {
            const item = document.createElement('div');
            item.className = 'general-menu-item';
            item.innerHTML = `
              <span style="font-size: 13px; color: var(--text-primary);">${app}</span>
            `;
            appsListElement.appendChild(item);
          });
        }
      }
    } catch (error) {
      console.error('Error loading media & purchases page:', error);
      const accountNameElement = document.getElementById('media-purchases-account-name');
      if (accountNameElement) {
        accountNameElement.textContent = 'Error loading name';
      }
      const appsListElement = document.getElementById('media-purchases-apps-list');
      if (appsListElement) {
        appsListElement.innerHTML = '<div class="general-menu-item"><span style="font-size: 13px; color: var(--text-secondary);">Error loading applications</span></div>';
      }
    }
  }

  // Funcție pentru inițializarea paginii Payment & Shipping
  async function initPaymentShippingPage() {
    if (!window.electronAPI) {
      console.error('electronAPI not available');
      return;
    }

    try {
      // Obține adresa de facturare
      const billingAddressResult = await window.electronAPI.getUserBillingAddress();
      const billingAddress = billingAddressResult.billingAddress || 'Not set';

      // Actualizează pagina
      const addressElement = document.getElementById('payment-shipping-address');
      if (addressElement) {
        addressElement.textContent = billingAddress;
      }

      // Adaugă event listener pe butonul Change Shipping Address
      const changeShippingAddressButton = document.getElementById('change-shipping-address-button');
      if (changeShippingAddressButton && !changeShippingAddressButton.hasAttribute('data-listener-added')) {
        changeShippingAddressButton.setAttribute('data-listener-added', 'true');
        changeShippingAddressButton.addEventListener('click', () => {
          openChangeShippingAddressModal(billingAddress);
        });
      }
    } catch (error) {
      console.error('Error loading billing address:', error);
      const addressElement = document.getElementById('payment-shipping-address');
      if (addressElement) {
        addressElement.textContent = 'Error loading address';
      }
    }
  }

  // Funcție pentru deschiderea modalului de schimbare a adresei de livrare
  function openChangeShippingAddressModal(currentAddress) {
    const modal = document.getElementById('change-shipping-address-modal');
    const addressInput = document.getElementById('shipping-address-input');
    const continueButton = document.getElementById('change-shipping-address-continue');
    const cancelButton = document.getElementById('change-shipping-address-cancel');

    if (!modal || !addressInput || !continueButton || !cancelButton) {
      console.error('Change shipping address modal elements not found');
      return;
    }

    // Schimbă culoarea butonului roșu când se deschide modalul
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Setează adresa curentă în input
    addressInput.value = currentAddress || '';
    
    // Afișează modalul
    modal.style.display = 'flex';
    addressInput.focus();

    // Funcție pentru închiderea modalului
    const closeModal = () => {
      modal.style.display = 'none';
      addressInput.value = '';
      restoreRedDot();
    };

    // Funcție pentru cleanup
    const cleanup = () => {
      continueButton.removeEventListener('click', handleContinue);
      cancelButton.removeEventListener('click', handleCancel);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru Continue
    const handleContinue = async () => {
      const newAddress = addressInput.value.trim();

      if (!newAddress) {
        alert('Please enter an address.');
        return;
      }

      try {
        const result = await window.electronAPI.updateUserBillingAddress(newAddress);
        
        if (!result.success) {
          alert(result.error || 'Failed to update shipping address. Please try again.');
          return;
        }

        // Actualizează UI-ul
        const addressElement = document.getElementById('payment-shipping-address');
        if (addressElement) {
          addressElement.textContent = newAddress;
        }

        cleanup();
        closeModal();
        alert('Shipping address updated successfully');
      } catch (error) {
        console.error('Error updating shipping address:', error);
        alert('Error updating shipping address: ' + (error.message || 'Unknown error'));
      }
    };

    // Event listener pentru Cancel
    const handleCancel = () => {
      cleanup();
      closeModal();
    };

    // Event listener pentru click în afara modalului
    const handleModalClick = (e) => {
      if (e.target === modal) {
        cleanup();
        closeModal();
      }
    };

    // Adaugă event listeners
    continueButton.addEventListener('click', handleContinue);
    cancelButton.addEventListener('click', handleCancel);
    modal.addEventListener('click', handleModalClick);
  }

  async function initSignInSecurityPage() {
    if (!window.electronAPI) {
      console.error('electronAPI not available');
      return;
    }

    try {
      // Obține email-ul
      const userInfo = await window.electronAPI.getUserInfo();
      const email = userInfo.email || 'Not set';

      // Obține numărul de telefon (doar dacă nu există în cache)
      let phone = 'Not set';
      try {
        const phoneResult = await window.electronAPI.getUserPhone();
        phone = phoneResult.phone || 'Not set';
      } catch (phoneError) {
        console.error('Error getting phone:', phoneError);
      }

      // Actualizează pagina
      const emailElement = document.getElementById('signin-security-email');
      const phoneElement = document.getElementById('signin-security-phone');

      if (emailElement) {
        emailElement.textContent = email;
        
        // Adaugă event listener pe email pentru a deschide modalul
        const emailItem = emailElement.closest('.general-menu-item');
        if (emailItem && !emailItem.hasAttribute('data-email-listener-added')) {
          emailItem.setAttribute('data-email-listener-added', 'true');
          emailItem.style.cursor = 'pointer';
          emailItem.addEventListener('click', () => {
            openViewEmailModal(email);
          });
        }
      }

      if (phoneElement) {
        phoneElement.textContent = phone;
        
        // Adaugă event listener pe numărul de telefon pentru a deschide modalul
        const phoneItem = phoneElement.closest('.general-menu-item');
        if (phoneItem && !phoneItem.hasAttribute('data-phone-listener-added')) {
          phoneItem.setAttribute('data-phone-listener-added', 'true');
          phoneItem.style.cursor = 'pointer';
          phoneItem.addEventListener('click', () => {
            openViewPhoneModal(phone);
          });
        }
      }

      // Adaugă event listener pe butonul Add Phone Number
      const addPhoneButton = document.getElementById('add-phone-number-button');
      if (addPhoneButton && !addPhoneButton.hasAttribute('data-listener-added')) {
        addPhoneButton.setAttribute('data-listener-added', 'true');
        addPhoneButton.addEventListener('click', async () => {
          await openAddPhoneModal();
        });
      }

      // Adaugă event listener pe butonul Change Password
      const changePasswordButton = document.getElementById('signin-security-change-password-button');
      if (changePasswordButton && !changePasswordButton.hasAttribute('data-listener-added')) {
        changePasswordButton.setAttribute('data-listener-added', 'true');
        changePasswordButton.addEventListener('click', async () => {
          await requestSudoPasswordAndChangePassword();
        });
      }
    } catch (error) {
      console.error('Error initializing sign in & security page:', error);
    }
  }

  // Funcție pentru cererea parolei sudo și deschiderea modalului pentru noua parolă
  async function requestSudoPasswordAndChangePassword() {
    while (true) {
      const password = await requestPassword(false, true);
      
      if (!password) {
        return; // Utilizatorul a anulat
      }

      // Verifică parola sudo folosind același model ca la software update
      try {
        const result = await window.electronAPI.verifySudoPassword(password);
        
        if (!result.success) {
          alert(result.error || 'Incorrect password. Please try again.');
          // Continuă loop-ul pentru a permite încercări repetate
          continue;
        }

        // Parola sudo este corectă, deschide modalul pentru noua parolă
        openChangePasswordNewModal(password);
        break;
      } catch (error) {
        console.error('Error verifying sudo password:', error);
        alert('Error verifying password: ' + (error.message || 'Unknown error'));
        // Continuă loop-ul pentru a permite încercări repetate
        continue;
      }
    }
  }

  // Funcție pentru deschiderea modalului de nouă parolă
  function openChangePasswordNewModal(sudoPassword) {
    const modal = document.getElementById('change-password-new-modal');
    const oldPasswordInput = document.getElementById('change-password-old-input');
    const newPasswordInput = document.getElementById('change-password-new-input');
    const retypePasswordInput = document.getElementById('change-password-retype-input');
    const continueButton = document.getElementById('change-password-new-continue');
    const cancelButton = document.getElementById('change-password-new-cancel');

    if (!modal || !oldPasswordInput || !newPasswordInput || !retypePasswordInput || !continueButton || !cancelButton) {
      console.error('Change password new modal elements not found');
      return;
    }

    // Schimbă culoarea butonului roșu când se deschide modalul
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Resetează input-urile
    oldPasswordInput.value = '';
    newPasswordInput.value = '';
    retypePasswordInput.value = '';
    
    // Obține elemente pentru mesajele de validare (acum sunt în HTML)
    const newPasswordError = document.getElementById('new-password-error');
    const retypePasswordError = document.getElementById('retype-password-error');
    
    // Resetează mesajele de eroare
    if (newPasswordError) {
      newPasswordError.textContent = '';
      newPasswordError.style.display = 'none';
    }
    if (retypePasswordError) {
      retypePasswordError.textContent = '';
      retypePasswordError.style.display = 'none';
    }
    
    // Dezactivează butonul Continue inițial - FORȚAT
    continueButton.disabled = true;
    continueButton.setAttribute('disabled', 'disabled');
    continueButton.style.opacity = '0.5';
    continueButton.style.cursor = 'not-allowed';
    
    // Afișează modalul
    modal.style.display = 'flex';
    oldPasswordInput.focus();

    // Funcție pentru validarea parolei
    const validatePassword = (password) => {
      if (password.length < 8) {
        return 'Password must be at least 8 characters long.';
      }
      if (!/\d/.test(password)) {
        return 'Password must include a number.';
      }
      if (!/[A-Z]/.test(password)) {
        return 'Password must include an uppercase letter.';
      }
      if (!/[a-z]/.test(password)) {
        return 'Password must include a lowercase letter.';
      }
      if (!/[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]/.test(password)) {
        return 'Password must include a special character.';
      }
      return null;
    };

    // Funcție pentru verificarea dacă toate condițiile sunt îndeplinite
    const checkAllConditions = () => {
      const newPassword = newPasswordInput.value;
      const retypePassword = retypePasswordInput.value;
      const oldPassword = oldPasswordInput.value;
      
      // Verifică dacă toate câmpurile sunt completate
      if (!oldPassword || !newPassword || !retypePassword) {
        continueButton.disabled = true;
        continueButton.setAttribute('disabled', 'disabled');
        continueButton.style.opacity = '0.5';
        continueButton.style.cursor = 'not-allowed';
        return;
      }
      
      // Verifică dacă parola respectă toate cerințele
      const passwordError = validatePassword(newPassword);
      if (passwordError) {
        continueButton.disabled = true;
        continueButton.setAttribute('disabled', 'disabled');
        continueButton.style.opacity = '0.5';
        continueButton.style.cursor = 'not-allowed';
        return;
      }
      
      // Verifică dacă parolele coincid
      if (newPassword !== retypePassword) {
        continueButton.disabled = true;
        continueButton.setAttribute('disabled', 'disabled');
        continueButton.style.opacity = '0.5';
        continueButton.style.cursor = 'not-allowed';
        return;
      }
      
      // Toate condițiile sunt îndeplinite
      continueButton.disabled = false;
      continueButton.removeAttribute('disabled');
      continueButton.style.opacity = '1';
      continueButton.style.cursor = 'pointer';
    };

    // Validare live pentru New Password
    const validateNewPasswordLive = () => {
      const password = newPasswordInput.value;
      
      if (password.length === 0) {
        newPasswordError.textContent = '';
        newPasswordError.style.display = 'none';
        checkAllConditions();
        return;
      }
      
      newPasswordError.style.display = 'block';
      const error = validatePassword(password);
      if (error) {
        newPasswordError.textContent = error;
        newPasswordError.style.color = '#fe5b51';
      } else {
        newPasswordError.textContent = '✓ Password meets requirements';
        newPasswordError.style.color = '#4caf50';
      }
      
      // Dacă retype password are deja o valoare, verifică din nou coincidența
      if (retypePasswordInput.value.length > 0) {
        validateRetypePasswordLive();
      } else {
        checkAllConditions();
      }
    };

    // Validare live pentru Retype Password
    const validateRetypePasswordLive = () => {
      const newPassword = newPasswordInput.value;
      const retypePassword = retypePasswordInput.value;
      
      if (retypePassword.length === 0) {
        retypePasswordError.textContent = '';
        retypePasswordError.style.display = 'none';
        checkAllConditions();
        return;
      }
      
      retypePasswordError.style.display = 'block';
      if (newPassword !== retypePassword) {
        retypePasswordError.textContent = '✗ Passwords do not match';
        retypePasswordError.style.color = '#fe5b51';
      } else {
        retypePasswordError.textContent = '✓ Passwords match';
        retypePasswordError.style.color = '#4caf50';
      }
      
      checkAllConditions();
    };

    // Adaugă event listeners pentru validare live
    oldPasswordInput.addEventListener('input', checkAllConditions);
    newPasswordInput.addEventListener('input', validateNewPasswordLive);
    retypePasswordInput.addEventListener('input', validateRetypePasswordLive);

    // Funcție pentru închiderea modalului
    const closeModal = () => {
      modal.style.display = 'none';
      oldPasswordInput.value = '';
      newPasswordInput.value = '';
      retypePasswordInput.value = '';
      restoreRedDot();
    };

    // Funcție pentru cleanup
    const cleanup = () => {
      continueButton.removeEventListener('click', handleContinue);
      cancelButton.removeEventListener('click', handleCancel);
      modal.removeEventListener('click', handleModalClick);
      oldPasswordInput.removeEventListener('input', checkAllConditions);
      newPasswordInput.removeEventListener('input', validateNewPasswordLive);
      retypePasswordInput.removeEventListener('input', validateRetypePasswordLive);
      // Resetează mesajele de eroare
      if (newPasswordError) {
        newPasswordError.textContent = '';
        newPasswordError.style.display = 'none';
      }
      if (retypePasswordError) {
        retypePasswordError.textContent = '';
        retypePasswordError.style.display = 'none';
      }
      // Resetează starea butonului
      continueButton.disabled = true;
      continueButton.setAttribute('disabled', 'disabled');
      continueButton.style.opacity = '0.5';
      continueButton.style.cursor = 'not-allowed';
    };

    // Event listener pentru Continue
    const handleContinue = async () => {
      const oldPassword = oldPasswordInput.value;
      const newPassword = newPasswordInput.value;
      const retypePassword = retypePasswordInput.value;

      if (!oldPassword || !newPassword || !retypePassword) {
        alert('Please fill in all fields.');
        return;
      }

      // Mai întâi validează că noua parolă respectă cerințele
      const validationError = validatePassword(newPassword);
      if (validationError) {
        alert(validationError);
        newPasswordInput.value = '';
        retypePasswordInput.value = '';
        newPasswordInput.focus();
        return;
      }

      // Apoi verifică că Retype Password este identic cu New Password
      if (newPassword !== retypePassword) {
        alert('New passwords do not match. Please make sure both password fields are identical.');
        retypePasswordInput.value = '';
        retypePasswordInput.focus();
        return;
      }

      try {
        // Folosim newPassword din retypePassword (după validare, ele sunt identice)
        const result = await window.electronAPI.updateUserPassword(oldPassword, retypePassword);
        
        if (!result.success) {
          alert(result.error || 'Failed to change password. Please try again.');
          oldPasswordInput.value = '';
          oldPasswordInput.focus();
          return;
        }

        cleanup();
        closeModal();
        alert('Password changed successfully');
      } catch (error) {
        console.error('Error changing password:', error);
        alert('Error changing password: ' + (error.message || 'Unknown error'));
      }
    };

    // Event listener pentru Cancel
    const handleCancel = () => {
      cleanup();
      closeModal();
    };

    // Event listener pentru click în afara modalului
    const handleModalClick = (e) => {
      if (e.target === modal) {
        cleanup();
        closeModal();
      }
    };

    // Adaugă event listeners
    continueButton.addEventListener('click', handleContinue);
    cancelButton.addEventListener('click', handleCancel);
    modal.addEventListener('click', handleModalClick);
  }

  // Lista de țări cu prefixe telefonice
  const countryPhonePrefixes = {
    'Romania': '+40',
    'United States': '+1',
    'United Kingdom': '+44',
    'Germany': '+49',
    'France': '+33',
    'Italy': '+39',
    'Spain': '+34',
    'Poland': '+48',
    'Netherlands': '+31',
    'Belgium': '+32',
    'Greece': '+30',
    'Portugal': '+351',
    'Czech Republic': '+420',
    'Hungary': '+36',
    'Sweden': '+46',
    'Norway': '+47',
    'Denmark': '+45',
    'Finland': '+358',
    'Austria': '+43',
    'Switzerland': '+41',
    'Ireland': '+353',
    'Canada': '+1',
    'Australia': '+61',
    'New Zealand': '+64',
    'Japan': '+81',
    'China': '+86',
    'India': '+91',
    'Brazil': '+55',
    'Mexico': '+52',
    'Argentina': '+54',
    'Chile': '+56',
    'Colombia': '+57',
    'Peru': '+51',
    'Venezuela': '+58',
    'Ecuador': '+593',
    'Uruguay': '+598',
    'Paraguay': '+595',
    'Bolivia': '+591',
    'Russia': '+7',
    'Ukraine': '+380',
    'Turkey': '+90',
    'Israel': '+972',
    'Saudi Arabia': '+966',
    'United Arab Emirates': '+971',
    'Egypt': '+20',
    'South Africa': '+27',
    'Nigeria': '+234',
    'Kenya': '+254',
    'Morocco': '+212',
    'Algeria': '+213',
    'Tunisia': '+216',
    'Ghana': '+233',
    'Ethiopia': '+251',
    'Tanzania': '+255',
    'Uganda': '+256',
    'Senegal': '+221',
    'Ivory Coast': '+225',
    'Cameroon': '+237',
    'Angola': '+244',
    'Mozambique': '+258',
    'Madagascar': '+261',
    'Mali': '+223',
    'Burkina Faso': '+226',
    'Niger': '+227',
    'Chad': '+235',
    'Sudan': '+249',
    'Malawi': '+265',
    'Zambia': '+260',
    'Zimbabwe': '+263',
    'Botswana': '+267',
    'Namibia': '+264',
    'Lesotho': '+266',
    'Swaziland': '+268',
    'Mauritius': '+230',
    'Seychelles': '+248',
    'Comoros': '+269',
    'Djibouti': '+253',
    'Eritrea': '+291',
    'Somalia': '+252',
    'Rwanda': '+250',
    'Burundi': '+257',
    'South Korea': '+82',
    'Thailand': '+66',
    'Vietnam': '+84',
    'Indonesia': '+62',
    'Malaysia': '+60',
    'Singapore': '+65',
    'Philippines': '+63',
    'Myanmar': '+95',
    'Cambodia': '+855',
    'Laos': '+856',
    'Bangladesh': '+880',
    'Pakistan': '+92',
    'Afghanistan': '+93',
    'Iran': '+98',
    'Iraq': '+964',
    'Syria': '+963',
    'Lebanon': '+961',
    'Jordan': '+962',
    'Kuwait': '+965',
    'Qatar': '+974',
    'Bahrain': '+973',
    'Oman': '+968',
    'Yemen': '+967',
    'Libya': '+218',
    'Mauritania': '+222',
    'Guinea': '+224',
    'Sierra Leone': '+232',
    'Liberia': '+231',
    'Gambia': '+220',
    'Guinea-Bissau': '+245',
    'Cape Verde': '+238',
    'São Tomé and Príncipe': '+239',
    'Equatorial Guinea': '+240',
    'Gabon': '+241',
    'Republic of the Congo': '+242',
    'Democratic Republic of the Congo': '+243',
    'Central African Republic': '+236',
    'Togo': '+228',
    'Benin': '+229'
  };

  // Funcție pentru deschiderea modalului Add Phone Number
  async function openAddPhoneModal() {
    const modal = document.getElementById('add-phone-modal');
    const countrySelect = document.getElementById('phone-country-select');
    const phoneInput = document.getElementById('phone-number-input');
    const cancelButton = document.getElementById('add-phone-cancel-button');
    const continueButton = document.getElementById('add-phone-continue-button');

    if (!modal || !countrySelect || !phoneInput || !cancelButton || !continueButton) {
      console.error('Add phone modal elements not found');
      return;
    }

    // Obține țara utilizatorului
    let userCountry = 'Romania'; // Default
    let defaultPrefix = '+40'; // Default pentru Romania
    try {
      const countryResult = await window.electronAPI.getUserCountry();
      userCountry = countryResult.country || 'Romania';
      // Găsește prefixul pentru țara utilizatorului
      if (countryPhonePrefixes[userCountry]) {
        defaultPrefix = countryPhonePrefixes[userCountry];
      }
    } catch (error) {
      console.error('Error getting country:', error);
    }

    // Populează dropdown-ul cu țări
    countrySelect.innerHTML = '';
    const sortedCountries = Object.keys(countryPhonePrefixes).sort();
    sortedCountries.forEach(country => {
      const option = document.createElement('option');
      const prefix = countryPhonePrefixes[country];
      option.value = prefix;
      option.textContent = `${prefix} (${country})`;
      if (prefix === defaultPrefix) {
        option.selected = true;
      }
      countrySelect.appendChild(option);
    });

    // Resetează input-ul
    phoneInput.value = '';

    // Schimbă culoarea butonului roșu când se deschide modalul
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Afișează modalul
    modal.style.display = 'flex';

    // Funcție pentru închiderea modalului
    const closeModal = () => {
      modal.style.display = 'none';
      restoreRedDot();
    };

    // Event listener pentru Cancel
    const handleCancel = () => {
      closeModal();
      cancelButton.removeEventListener('click', handleCancel);
      continueButton.removeEventListener('click', handleContinue);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru Continue
    const handleContinue = async () => {
      const selectedPrefix = countrySelect.value;
      const phoneNumber = phoneInput.value.trim();

      if (!phoneNumber) {
        alert('Please enter a phone number.');
        return;
      }

      // Combină prefixul cu numărul
      const fullPhoneNumber = selectedPrefix + phoneNumber;

      try {
        const result = await window.electronAPI.updateUserPhone(fullPhoneNumber);
        
        if (!result.success) {
          alert(result.error || 'Failed to update phone number. Please try again.');
          return;
        }

        // Actualizează UI-ul
        const phoneElement = document.getElementById('signin-security-phone');
        if (phoneElement) {
          phoneElement.textContent = fullPhoneNumber;
        }

        closeModal();
        alert('Phone number updated successfully');
      } catch (error) {
        console.error('Error updating phone number:', error);
        alert('Error updating phone number: ' + (error.message || 'Unknown error'));
      }

      cancelButton.removeEventListener('click', handleCancel);
      continueButton.removeEventListener('click', handleContinue);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru click în afara modalului
    const handleModalClick = (e) => {
      if (e.target === modal) {
        closeModal();
        cancelButton.removeEventListener('click', handleCancel);
        continueButton.removeEventListener('click', handleContinue);
        modal.removeEventListener('click', handleModalClick);
      }
    };

    // Adaugă event listeners
    cancelButton.addEventListener('click', handleCancel);
    continueButton.addEventListener('click', handleContinue);
    modal.addEventListener('click', handleModalClick);
  }

  // Funcție pentru deschiderea modalului de vizualizare a numărului de telefon
  function openViewPhoneModal(phoneNumber) {
    const modal = document.getElementById('view-phone-modal');
    const titleElement = document.getElementById('view-phone-title');
    const doneButton = document.getElementById('view-phone-done-button');

    if (!modal || !titleElement || !doneButton) {
      console.error('View phone modal elements not found');
      return;
    }

    // Schimbă culoarea butonului roșu când se deschide modalul
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Setează numărul de telefon ca titlu
    titleElement.textContent = phoneNumber || 'Phone Number';

    // Afișează modalul
    modal.style.display = 'flex';

    // Funcție pentru închiderea modalului
    const closeModal = () => {
      modal.style.display = 'none';
      restoreRedDot();
    };

    // Event listener pentru Done
    const handleDone = () => {
      closeModal();
      doneButton.removeEventListener('click', handleDone);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru click în afara modalului
    const handleModalClick = (e) => {
      if (e.target === modal) {
        closeModal();
        doneButton.removeEventListener('click', handleDone);
        modal.removeEventListener('click', handleModalClick);
      }
    };

    // Adaugă event listeners
    doneButton.addEventListener('click', handleDone);
    modal.addEventListener('click', handleModalClick);
  }

  // Funcție pentru deschiderea modalului de vizualizare a email-ului
  function openViewEmailModal(emailAddress) {
    const modal = document.getElementById('view-email-modal');
    const titleElement = document.getElementById('view-email-title');
    const doneButton = document.getElementById('view-email-done-button');

    if (!modal || !titleElement || !doneButton) {
      console.error('View email modal elements not found');
      return;
    }

    // Schimbă culoarea butonului roșu când se deschide modalul
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Setează email-ul ca titlu
    titleElement.textContent = emailAddress || 'Email';

    // Afișează modalul
    modal.style.display = 'flex';

    // Funcție pentru închiderea modalului
    const closeModal = () => {
      modal.style.display = 'none';
      restoreRedDot();
    };

    // Event listener pentru Done
    const handleDone = () => {
      closeModal();
      doneButton.removeEventListener('click', handleDone);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru click în afara modalului
    const handleModalClick = (e) => {
      if (e.target === modal) {
        closeModal();
        doneButton.removeEventListener('click', handleDone);
        modal.removeEventListener('click', handleModalClick);
      }
    };

    // Adaugă event listeners
    doneButton.addEventListener('click', handleDone);
    modal.addEventListener('click', handleModalClick);
  }

  // Funcție pentru inițializarea paginii Personal Information
  async function initPersonalInformationPage() {
    if (!window.electronAPI) {
      console.error('electronAPI not available');
      return;
    }

    try {
      // Obține numele utilizatorului
      const userInfo = await window.electronAPI.getUserInfo();
      const firstName = userInfo.firstName || '';
      const lastName = userInfo.lastName || '';
      const fullName = `${firstName} ${lastName}`.trim() || 'Not set';

      // Obține data nașterii (doar dacă nu există în cache)
      let birthdate = 'Not set';
      try {
        const birthdateResult = await window.electronAPI.getUserBirthdate();
        birthdate = birthdateResult.birthdate || 'Not set';
      } catch (birthdateError) {
        console.error('Error getting birthdate:', birthdateError);
      }

      // Actualizează pagina
      const nameElement = document.getElementById('personal-info-name');
      const birthdateElement = document.getElementById('personal-info-birthdate');

      if (nameElement) {
        nameElement.textContent = fullName;
      }

      if (birthdateElement) {
        birthdateElement.textContent = birthdate;
      }

      // Adaugă event listener pe secțiunea numelui pentru a deschide modalul
      const nameSection = document.getElementById('personal-info-name-section');
      if (nameSection && !nameSection.hasAttribute('data-listener-added')) {
        nameSection.setAttribute('data-listener-added', 'true');
        nameSection.addEventListener('click', () => {
          openEditNameModal(firstName, lastName);
        });
      }
    } catch (error) {
      console.error('Error initializing personal information page:', error);
    }
  }

  // Funcție pentru deschiderea modalului de editare a numelui
  function openEditNameModal(currentFirstName, currentLastName) {
    const modal = document.getElementById('edit-name-modal');
    const firstNameInput = document.getElementById('edit-first-name-input');
    const lastNameInput = document.getElementById('edit-last-name-input');
    const cancelButton = document.getElementById('edit-name-cancel-button');
    const doneButton = document.getElementById('edit-name-done-button');

    if (!modal || !firstNameInput || !lastNameInput || !cancelButton || !doneButton) {
      console.error('Edit name modal elements not found');
      return;
    }

    // Setează valorile curente
    firstNameInput.value = currentFirstName || '';
    lastNameInput.value = currentLastName || '';

    // Schimbă culoarea butonului roșu când se deschide modalul
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Afișează modalul
    modal.style.display = 'flex';

    // Funcție pentru închiderea modalului
    const closeModal = () => {
      modal.style.display = 'none';
      restoreRedDot();
    };

    // Event listener pentru Cancel
    const handleCancel = () => {
      closeModal();
      cancelButton.removeEventListener('click', handleCancel);
      doneButton.removeEventListener('click', handleDone);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru Done
    const handleDone = async () => {
      const newFirstName = firstNameInput.value.trim();
      const newLastName = lastNameInput.value.trim();

      if (!newFirstName && !newLastName) {
        alert('Please enter at least a first name or last name.');
        return;
      }

      try {
        const result = await window.electronAPI.updateUserInfo(newFirstName, newLastName);
        
        if (!result.success) {
          alert(result.error || 'Failed to update name. Please try again.');
          return;
        }

        // Actualizează UI-ul
        const nameElement = document.getElementById('personal-info-name');
        if (nameElement) {
          const fullName = `${newFirstName} ${newLastName}`.trim() || 'Not set';
          nameElement.textContent = fullName;
        }

        // Actualizează și în pagina de profil dacă este deschisă
        const userProfileName = document.getElementById('user-profile-name');
        if (userProfileName) {
          const fullName = `${newFirstName} ${newLastName}`.trim() || 'Not set';
          userProfileName.textContent = fullName;
        }

        // Actualizează și în sidebar dacă este deschis
        const userSigninTitle = document.querySelector('.user-signin-title');
        if (userSigninTitle) {
          const fullName = `${newFirstName} ${newLastName}`.trim() || 'Sign in';
          userSigninTitle.textContent = fullName;
        }

        closeModal();
        alert('Name updated successfully');
      } catch (error) {
        console.error('Error updating name:', error);
        alert('Error updating name: ' + (error.message || 'Unknown error'));
      }

      cancelButton.removeEventListener('click', handleCancel);
      doneButton.removeEventListener('click', handleDone);
      modal.removeEventListener('click', handleModalClick);
    };

    // Event listener pentru click în afara modalului
    const handleModalClick = (e) => {
      if (e.target === modal) {
        closeModal();
        cancelButton.removeEventListener('click', handleCancel);
        doneButton.removeEventListener('click', handleDone);
        modal.removeEventListener('click', handleModalClick);
      }
    };

    // Adaugă event listeners
    cancelButton.addEventListener('click', handleCancel);
    doneButton.addEventListener('click', handleDone);
    modal.addEventListener('click', handleModalClick);
  }

  // Funcție pentru gestionarea click-ului pe avatar
  async function handleAvatarClick() {
    if (!window.electronAPI || !window.electronAPI.showOpenDialog) {
      console.error('electronAPI or showOpenDialog not available');
      return;
    }

    try {
      // Deschide file chooser
      const result = await window.electronAPI.showOpenDialog({
        properties: ['openFile'],
        filters: [
          { name: 'Images', extensions: ['jpg', 'jpeg', 'png', 'gif', 'webp'] },
          { name: 'All Files', extensions: ['*'] }
        ],
        title: 'Select Avatar Image'
      });

      if (result.canceled || !result.filePaths || result.filePaths.length === 0) {
        return; // Utilizatorul a anulat
      }

      const selectedFilePath = result.filePaths[0];
      console.log('Selected avatar file:', selectedFilePath);

      // Actualizează avatar-ul
      const updateResult = await window.electronAPI.updateUserAvatar(selectedFilePath);
      
      if (updateResult.success) {
        // Reîncarcă avatar-ul din cache/server
        try {
          const avatarResult = await window.electronAPI.getUserAvatar();
          const profileAvatar = document.getElementById('user-profile-avatar');
          if (profileAvatar && avatarResult.avatarPath) {
            profileAvatar.src = `file://${avatarResult.avatarPath}`;
          }
          
          // Actualizează și avatar-ul din sidebar
          const sidebarAvatar = document.querySelector('.user-avatar');
          if (sidebarAvatar && avatarResult.avatarPath) {
            sidebarAvatar.src = `file://${avatarResult.avatarPath}`;
          }
          
          alert('Avatar updated successfully!');
        } catch (avatarError) {
          console.error('Error reloading avatar:', avatarError);
          alert('Avatar updated, but there was an error reloading it. Please refresh the page.');
        }
      } else {
        alert(`Error updating avatar: ${updateResult.error || 'Unknown error'}`);
      }
    } catch (error) {
      console.error('Error in handleAvatarClick:', error);
      alert(`Error: ${error.message || 'Unknown error'}`);
    }
  }

  // Funcție pentru gestionarea Sign Out
  async function handleSignOut() {
    if (!window.electronAPI || !window.electronAPI.signOut) {
      console.error('signOut not available');
      return;
    }

    // Cere parola într-un modal (pentru Sign Out)
    const password = await requestPassword(true);
    if (!password) {
      return; // Utilizatorul a anulat
    }

    try {
      // Rulează comanda de sign out
      const result = await window.electronAPI.signOut(password);
      
      if (!result.success) {
        // Afișează mesajul de eroare dacă sign out a eșuat
        alert(result.error || 'Failed to sign out. Please check your password.');
        return;
      }
      
      // După deconectare, resetează UI-ul
      const userAvatar = document.querySelector('.user-signin-section .user-avatar');
      const userSigninTitle = document.querySelector('.user-signin-title');
      const userSigninDescription = document.querySelector('.user-signin-description');

      if (userAvatar) {
        userAvatar.src = './assets/cs-user.svg';
      }

      if (userSigninTitle) {
        userSigninTitle.textContent = 'Sign in';
      }

      if (userSigninDescription) {
        userSigninDescription.textContent = 'with your pear account';
      }

      // Navighează la pagina de sign in
      showPage('PearID Sign In');
      
      // alert('Signed out successfully');
    } catch (error) {
      console.error('Error signing out:', error);
      alert('Error signing out: ' + (error.message || 'Unknown error'));
    }
  }

  // Funcție pentru afișarea paginii de detalii device
  function showDeviceDetailsPage(deviceName, isCurrentDevice = false) {
    // Actualizează numele device-ului în pagină
    const deviceNameElement = document.getElementById('device-details-name');
    const deviceNameValueElement = document.getElementById('device-details-name-value');
    const thisDeviceLabel = document.getElementById('device-details-this-device');
    const pearIdLockCard = document.getElementById('pearid-lock-card');
    
    if (deviceNameElement) {
      deviceNameElement.textContent = deviceName;
    }
    if (deviceNameValueElement) {
      deviceNameValueElement.textContent = deviceName;
    }
    
    // Afișează "(This Device)" sub numele device-ului dacă este "This Device"
    if (thisDeviceLabel) {
      thisDeviceLabel.style.display = isCurrentDevice ? 'block' : 'none';
    }
    
    // Afișează cardul "pearID Lock" doar pentru "This Device"
    if (pearIdLockCard) {
      pearIdLockCard.style.display = isCurrentDevice ? 'block' : 'none';
      
      // Adaugă event listener pentru click pe card (doar pentru "This Device")
      if (isCurrentDevice) {
        pearIdLockCard.onclick = () => {
          openPearIdLockModal();
        };
      }
    }
    
    // Ascunde cardul "Remove from Account" dacă este "This Device"
    const removeDeviceCard = document.querySelector('#device-details-page .settings-card:last-of-type');
    if (removeDeviceCard) {
      removeDeviceCard.style.display = isCurrentDevice ? 'none' : 'block';
    }
    
    // Adaugă event listener pe link-ul "change your password" (doar dacă cardul este vizibil)
    if (!isCurrentDevice) {
      const changePasswordLink = document.getElementById('change-password-link');
      if (changePasswordLink && !changePasswordLink.hasAttribute('data-listener-added')) {
        changePasswordLink.setAttribute('data-listener-added', 'true');
        changePasswordLink.addEventListener('click', async (e) => {
          e.preventDefault();
          await initSignInSecurityPage();
          showPage('Sign In & Security');
        });
      }
    }
    
    // Afișează pagina
    showPage('Device Info');
  }

  // Funcție pentru deschiderea modalului pearID Lock
  function openPearIdLockModal() {
    const modal = document.getElementById('pearid-lock-modal');
    if (!modal) return;

    // Schimbă culoarea red dot
    const redDot = document.querySelector('.traffic-lights .dot.red');
    const originalColor = '#fe5b51';
    
    if (redDot) {
      redDot.style.backgroundColor = '#6f6f6f';
    }

    const restoreRedDot = () => {
      if (redDot) {
        redDot.style.backgroundColor = originalColor;
      }
    };

    // Afișează modalul
    modal.style.display = 'flex';

    // Funcție pentru cleanup
    const cleanup = () => {
      modal.style.display = 'none';
      restoreRedDot();
    };

    // Event listener pentru Done
    const doneButton = document.getElementById('pearid-lock-done');
    if (doneButton) {
      const doneHandler = () => {
        cleanup();
        doneButton.removeEventListener('click', doneHandler);
      };
      doneButton.addEventListener('click', doneHandler);
    }

    // Event listener pentru click în afara modalului
    const overlayHandler = (e) => {
      if (e.target === modal) {
        cleanup();
        modal.removeEventListener('click', overlayHandler);
      }
    };
    modal.addEventListener('click', overlayHandler);
  }

  // Funcție pentru încărcarea device-urilor
  async function loadUserDevices() {
    if (!window.electronAPI || !window.electronAPI.getUserDevices) {
      console.error('getUserDevices not available');
      return;
    }

    const devicesList = document.getElementById('devices-list');
    if (!devicesList) {
      console.error('devices-list element not found');
      return;
    }

    try {
      devicesList.innerHTML = '<div style="color: var(--text-secondary); padding: 20px; text-align: center;">Loading devices...</div>';
      
      const devicesResult = await window.electronAPI.getUserDevices();
      const devices = devicesResult.devices || [];

      if (devices.length === 0) {
        devicesList.innerHTML = '<div style="color: var(--text-secondary); padding: 20px; text-align: center;">No devices found</div>';
        return;
      }

      // Afișează device-urile
      devicesList.innerHTML = devices.map((device, index) => {
        const isCurrentDevice = device.isCurrent;
        const deviceName = device.name;
        return `
        <div class="general-menu-item" data-device-name="${deviceName}" data-device-index="${index}" data-is-current="${isCurrentDevice}" style="cursor: default !important;">
          <div class="general-menu-item-content">
            <img src="./assets/preferences-other.svg" alt="Device" class="general-menu-icon">
            <span class="general-menu-label">${deviceName}${isCurrentDevice ? ' (This device)' : ''}</span>
          </div>
          <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-chevron-right general-menu-arrow"><polyline points="9 18 15 12 9 6"></polyline></svg>
        </div>
      `;
      }).join('');

      // Adaugă event listeners pentru toate device-urile
      devices.forEach((device, index) => {
        const deviceElement = devicesList.querySelector(`[data-device-index="${index}"]`);
        if (deviceElement) {
          // Pentru "This Device", păstrăm event listener-ul dar cursorul rămâne default
          deviceElement.addEventListener('click', () => {
            showDeviceDetailsPage(device.name, device.isCurrent);
          });
        }
      });
    } catch (error) {
      console.error('Error loading devices:', error);
      devicesList.innerHTML = '<div style="color: var(--text-secondary); padding: 20px; text-align: center;">Error loading devices</div>';
    }
  }

  
  feather.replace();

  
  const pageTitle = document.querySelector('.page-title');
  const sidebarItems = document.querySelectorAll('.sidenav .item');
  
  sidebarItems.forEach(item => {
    item.addEventListener('click', () => {
      const itemName = item.querySelector('.name');
      if (itemName && pageTitle) {
        const pageName = itemName.textContent.trim();
        
        
        sidebarItems.forEach(i => i.classList.remove('active'));
        
        item.classList.add('active');
        
        
        if (pageName === 'Battey' || pageName === 'Battery') {
          updateBatteryTitle();
        } else if (pageName === 'General') {
          
          pageTitle.textContent = '';
          
          const subtitle = pageTitle.querySelector('.battery-subtitle');
          if (subtitle) {
            subtitle.remove();
          }
        } else {
          pageTitle.textContent = pageName;
          
          const subtitle = pageTitle.querySelector('.battery-subtitle');
          if (subtitle) {
            subtitle.remove();
          }
        }
        
        
        showPage(pageName);
      }
    });
  });

  
  const generalMenuItems = document.querySelectorAll('.general-menu-item');
  generalMenuItems.forEach(item => {
    item.addEventListener('click', () => {
      const label = item.querySelector('.general-menu-label');
      if (label) {
        const itemName = label.textContent.trim();
        
        
        const pageTitle = document.querySelector('.page-title');
        if (pageTitle) {
          pageTitle.textContent = itemName;
        }
        
        
        if (itemName === 'About') {
          showPage('About');
        } else if (itemName === 'Software Update') {
          showPage('Software Update');
        } else if (itemName === 'Storage') {
          showPage('Storage');
        } else if (itemName === 'PearCare & Warranty') {
          showPage('PearCare & Warranty');
        } else if (itemName === 'PearDrop & Handoff') {
          showPage('PearDrop & Handoff');
        } else if (itemName === 'AutoFill & Passwords') {
          showPage('AutoFill & Passwords');
        } else if (itemName === 'Date & Time') {
          showPage('Date & Time');
        } else if (itemName === 'Login Items & Extensions') {
          showPage('Login Items & Extensions');
        } else if (itemName === 'Sharing') {
          showPage('Sharing');
        } else if (itemName === 'Startup Disk') {
          showPage('Startup Disk');
        } else if (itemName === 'Device Management') {
          showPage('Device Management');
        } else if (itemName === 'Time Machine') {
          showPage('Time Machine');
        } else if (itemName === 'Transfer or Reset') {
          showPage('Transfer or Reset');
        }
        
      }
    });
  });

  
  const storageSettingsButton = document.querySelector('.storage-settings-button');
  if (storageSettingsButton) {
    storageSettingsButton.addEventListener('click', () => {
      showPage('Storage');
    });
  }

  const displaySettingsButton = document.querySelector('.display-settings-button');
  if (displaySettingsButton) {
    displaySettingsButton.addEventListener('click', () => {
      showPage('Displays');
    });
  }

  
  async function updateSoftwareUpdateNotification() {
    if (!window.electronAPI) return;

    const notification = document.getElementById('software-update-notification');
    const badge = document.getElementById('update-badge');
    
    if (!notification || !badge) return;

    try {
      const updates = await window.electronAPI.getAvailableUpdates();
      const updateCount = updates && updates.length ? updates.length : 0;
      
      if (updateCount > 0) {
        badge.textContent = updateCount;
        notification.style.display = 'flex';
        
        
        if (!notification.hasAttribute('data-listener-added')) {
          notification.setAttribute('data-listener-added', 'true');
          notification.addEventListener('click', () => {
            showPage('General');
            
            setTimeout(() => {
              const generalMenuItems = document.querySelectorAll('.general-menu-item');
              generalMenuItems.forEach(item => {
                const label = item.querySelector('.general-menu-label');
                if (label && label.textContent.trim() === 'Software Update') {
                  item.click();
                }
              });
            }, 100);
          });
        }
      } else {
        // Ascunde itemul dacă numărul de updates este 0
        badge.textContent = '0';
        notification.style.display = 'none';
      }
    } catch (error) {
      console.error('Error checking for updates:', error);
      notification.style.display = 'none';
    }
  }

  
  if (window.electronAPI) {
    updateSoftwareUpdateNotification();
    
    setInterval(updateSoftwareUpdateNotification, 5 * 60 * 1000);
  }

  // Adaugă event listener pe user-signin-section la inițializare
  const userSigninSection = document.querySelector('.user-signin-section');
  if (userSigninSection) {
    userSigninSection.style.cursor = 'default';
    console.log('Adding click listener to user-signin-section');
    userSigninSection.addEventListener('click', async () => {
      console.log('User signin section clicked');
      
      // Afișează spinner-ul
      const spinner = document.getElementById('content-spinner');
      if (spinner) {
        spinner.style.display = 'flex';
      }
      
      // Ascunde toate paginile
      const allPages = document.querySelectorAll('.page-content');
      allPages.forEach(page => {
        page.style.display = 'none';
      });
      
      // Verifică dacă utilizatorul este autentificat
      if (window.electronAPI && window.electronAPI.checkPearIdState) {
        try {
          const stateResult = await window.electronAPI.checkPearIdState();
          const state = stateResult.state ? stateResult.state.trim().toLowerCase() : 'false';
          console.log('PearID state:', state);
          
          if (state === 'true') {
            console.log('User is authenticated, opening profile page');
            await initUserProfilePage();
            showPage('User Profile');
          } else {
            console.log('User is not authenticated, showing sign in page');
            showPage('PearID Sign In');
          }
        } catch (error) {
          console.error('Error checking PearID state:', error);
        } finally {
          // Ascunde spinner-ul
          if (spinner) {
            spinner.style.display = 'none';
          }
        }
      } else {
        console.error('electronAPI or checkPearIdState not available');
        // Ascunde spinner-ul
        if (spinner) {
          spinner.style.display = 'none';
        }
      }
    });
  } else {
    console.error('user-signin-section not found');
  }

  
  loadModules();
  
  
  showPage('General', false);

  const searchInput = document.querySelector('.sidebar-search');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      const searchTerm = e.target.value.toLowerCase().trim();
      const allItems = document.querySelectorAll('.sidenav .item');
      const allNavSections = document.querySelectorAll('.sidenav .nav-section');
      const allGeneralMenuItems = document.querySelectorAll('.general-menu-item');
      
      if (!searchTerm) {
        allItems.forEach(item => {
          item.style.display = '';
        });
        allNavSections.forEach(section => {
          section.style.display = '';
        });
        allGeneralMenuItems.forEach(menuItem => {
          menuItem.style.display = '';
        });
        const generalPage = document.getElementById('general-page');
        if (generalPage) {
          generalPage.style.display = 'none';
        }
        return;
      }
      
      let foundInGeneralMenu = false;
      
      allGeneralMenuItems.forEach(menuItem => {
        const menuLabel = menuItem.querySelector('.general-menu-label');
        if (menuLabel) {
          const menuText = menuLabel.textContent.toLowerCase();
          if (menuText.includes(searchTerm)) {
            foundInGeneralMenu = true;
            menuItem.style.display = '';
          } else {
            menuItem.style.display = 'none';
          }
        } else {
          menuItem.style.display = 'none';
        }
      });
      
      if (foundInGeneralMenu) {
        const generalItem = Array.from(allItems).find(item => {
          const itemName = item.querySelector('.name');
          return itemName && itemName.textContent.trim() === 'General';
        });
        if (generalItem) {
          generalItem.style.display = '';
          const generalNavSection = generalItem.closest('.nav-section');
          if (generalNavSection) {
            generalNavSection.style.display = '';
          }
        }
      }
      
      allItems.forEach(item => {
        const itemName = item.querySelector('.name');
        if (itemName) {
          const itemText = itemName.textContent.toLowerCase();
          if (itemText.includes(searchTerm)) {
            item.style.display = '';
          } else if (itemText === 'general' && !foundInGeneralMenu) {
            item.style.display = 'none';
          } else if (itemText !== 'general') {
            item.style.display = 'none';
          }
        } else {
          item.style.display = 'none';
        }
      });
      
      allNavSections.forEach(section => {
        const visibleItemsInSection = Array.from(section.querySelectorAll('.item')).some(item => 
          item.style.display !== 'none'
        );
        if (visibleItemsInSection) {
          section.style.display = '';
        } else {
          section.style.display = 'none';
        }
      });
    });
    
    searchInput.addEventListener('keydown', (e) => {
      if (e.key === 'Escape') {
        searchInput.value = '';
        searchInput.dispatchEvent(new Event('input'));
        searchInput.blur();
      } else if (e.key === 'Enter') {
        const searchTerm = searchInput.value.toLowerCase().trim();
        if (searchTerm) {
          const generalMenuItems = document.querySelectorAll('.general-menu-item');
          for (const menuItem of generalMenuItems) {
            if (menuItem.style.display !== 'none') {
              const menuLabel = menuItem.querySelector('.general-menu-label');
              if (menuLabel) {
                const itemName = menuLabel.textContent.trim();
                showPage(itemName);
                searchInput.blur();
                break;
              }
            }
          }
          
          const visibleItems = Array.from(document.querySelectorAll('.sidenav .item')).filter(item => 
            item.style.display !== 'none'
          );
          if (visibleItems.length > 0 && visibleItems[0] !== Array.from(document.querySelectorAll('.sidenav .item')).find(item => {
            const name = item.querySelector('.name');
            return name && name.textContent.trim() === 'General';
          })) {
            const itemName = visibleItems[0].querySelector('.name');
            if (itemName) {
              showPage(itemName.textContent.trim());
              searchInput.blur();
            }
          }
        }
      }
    });
  }

  const contentArea = document.querySelector('.main-content .content-area');
  if (contentArea) {
    let scrollTimeout;
    let isScrolling = false;
    
    contentArea.addEventListener('scroll', () => {
      
      if (!isScrolling) {
        contentArea.classList.add('scrolling');
        isScrolling = true;
      }
      
      
      clearTimeout(scrollTimeout);
      
      
      
      scrollTimeout = setTimeout(() => {
        contentArea.classList.remove('scrolling');
        isScrolling = false;
      }, 800);
    });
  }

  
  const sidenav = document.querySelector('.sidenav');
  if (sidenav) {
    let sidenavScrollTimeout;
    let isSidenavScrolling = false;
    
    sidenav.addEventListener('scroll', () => {
      
      if (!isSidenavScrolling) {
        sidenav.classList.add('scrolling');
        isSidenavScrolling = true;
      }
      
      
      clearTimeout(sidenavScrollTimeout);
      
      
      sidenavScrollTimeout = setTimeout(() => {
        sidenav.classList.remove('scrolling');
        isSidenavScrolling = false;
      }, 800);
    });
  }
  
  
  async function updateBatteryTitle() {
    const pageTitle = document.querySelector('.page-title');
    if (!pageTitle || !window.electronAPI) return;
    
    try {
      const batteryInfo = await window.electronAPI.getBatteryInfo();
      pageTitle.innerHTML = 'Battery';
      
      
      const existingSubtitle = pageTitle.querySelector('.battery-subtitle');
      if (existingSubtitle) {
        existingSubtitle.remove();
      }
      
      
      const subtitle = document.createElement('span');
      subtitle.className = 'battery-subtitle';
      if (batteryInfo.percentage === 100) {
        subtitle.textContent = 'Fully Charged';
      } else {
        subtitle.textContent = `${batteryInfo.percentage}%`;
      }
      pageTitle.appendChild(subtitle);
    } catch (error) {
      console.error('Error updating battery title:', error);
      pageTitle.textContent = 'Battery';
    }
  }
  
  
  
  
  let loadedModules = {};
  
  
  let moduleFunctionRegistry = {};
  
  
  function removeQuotes(str) {
    str = str.trim();
    if ((str.startsWith('"') && str.endsWith('"')) || (str.startsWith("'") && str.endsWith("'"))) {
      return str.substring(1, str.length - 1);
    }
    return str;
  }
  
  
  function getIndentLevel(line) {
    let indent = 0;
    for (let i = 0; i < line.length; i++) {
      if (line[i] === ' ' || line[i] === '\t') {
        indent++;
      } else {
        break;
      }
    }
    return indent;
  }
  
  
  function parseModuleManifest(manifestContent, moduleId) {
    try {
      const lines = manifestContent.split('\n');
      const result = {
        main: {},
        item: {},
        pages: [] 
      };
      
      let currentSection = null;
      let currentContext = null; 
      let contextStack = []; 
      let currentPage = null; 
      
      for (let i = 0; i < lines.length; i++) {
        const originalLine = lines[i];
        const line = originalLine.trim();
        const indentLevel = getIndentLevel(originalLine);
        
        
        if (!line) continue;
        
        
        if (line === '__main') {
          currentSection = 'main';
          currentContext = null;
          contextStack = [];
          continue;
        } else if (line === '__item') {
          currentSection = 'item';
          currentContext = null;
          contextStack = [];
          continue;
        } else if (line === '__page') {
          
          if (currentPage) {
            result.pages.push(currentPage);
          }
          currentPage = {
            id: '',
            content: []
          };
          currentSection = 'page';
          currentContext = null;
          contextStack = [];
          continue;
        }
        
        
        if (currentSection === 'main') {
          const colonIndex = line.indexOf(':');
          if (colonIndex > 0) {
            const key = line.substring(0, colonIndex).trim();
            let value = line.substring(colonIndex + 1).trim();
            value = removeQuotes(value);
            result.main[key] = value;
          }
        } else if (currentSection === 'item') {
          const colonIndex = line.indexOf(':');
          if (colonIndex > 0) {
            const key = line.substring(0, colonIndex).trim();
            let value = line.substring(colonIndex + 1).trim();
            value = removeQuotes(value);
            result.item[key] = value;
          }
        } else if (currentSection === 'page') {
          
          if (line.startsWith('id:')) {
            let idValue = line.substring(3).trim();
            if (currentPage) {
              currentPage.id = removeQuotes(idValue);
            }
            continue;
          }
          
          
          if (line === '}') {
            if (contextStack.length > 0) {
              contextStack.pop();
              if (contextStack.length > 0) {
                currentContext = contextStack[contextStack.length - 1];
              } else {
                currentContext = null;
              }
            }
            continue;
          }
          
          
          const trimmedLine = line.trim();
          
          
          if (trimmedLine === 'section-title:' || trimmedLine.startsWith('section-title:')) {
            let value = trimmedLine.includes(':') ? trimmedLine.substring(trimmedLine.indexOf(':') + 1).trim() : '';
            if (!value && i + 1 < lines.length) {
              value = lines[i + 1].trim();
              i++;
            }
            if (currentPage) {
              currentPage.content.push({ type: 'section-title', text: removeQuotes(value) });
            }
            continue;
          }
          
          
          if (trimmedLine.startsWith('settings-card')) {
            const card = { type: 'settings-card', sections: [] };
            if (currentPage) {
              currentPage.content.push(card);
            }
            currentContext = card;
            contextStack.push(card);
            continue;
          }
          
          
          if (trimmedLine.startsWith('card-section') && !trimmedLine.startsWith('card-section-vertical') && !trimmedLine.startsWith('card-section-image') && !trimmedLine.startsWith('card-section-info')) {
            const parts = trimmedLine.split(/\s+/);
            const sectionId = parts.length > 1 ? removeQuotes(parts[1]) : null;
            const section = { type: 'card-section', items: [], id: sectionId, style: null };
            
            
            console.log('Parser: Found card-section, ID:', sectionId, 'linia:', trimmedLine);
            
            
            if (sectionId === 'wifi-clone-no-network-section' || trimmedLine.includes('wifi-clone-no-network-section')) {
              console.log('Parser: *** Found card-section wifi-clone-no-network-section ***, ID:', sectionId, 'parts:', parts);
            }
            
            
            if (i + 1 < lines.length) {
              const nextLineIndent = getIndentLevel(lines[i + 1]);
              const nextLineTrimmed = lines[i + 1].trim();
              if (nextLineIndent > indentLevel && nextLineTrimmed.startsWith('style:')) {
                section.style = removeQuotes(nextLineTrimmed.substring(6).trim());
                i++; 
              }
            }
            
            if (currentContext && currentContext.sections) {
              currentContext.sections.push(section);
            } else if (currentContext && currentContext.type === 'settings-card') {
              currentContext.sections.push(section);
            }
            currentContext = section;
            contextStack.push(section);
            
            
            if (sectionId === 'wifi-clone-no-network-section') {
              console.log('Parser: *** Set currentContext for wifi-clone-no-network-section ***, next line:', i + 1 < lines.length ? lines[i + 1].trim() : 'N/A');
            }
            
            continue;
          }
          
          
          if (trimmedLine.startsWith('card-section-vertical')) {
            const parts = trimmedLine.split(/\s+/);
            const sectionId = parts.length > 1 ? removeQuotes(parts[1]) : null;
            const section = { type: 'card-section-vertical', items: [], id: sectionId };
            if (currentContext && currentContext.sections) {
              currentContext.sections.push(section);
            } else if (currentContext && currentContext.type === 'settings-card') {
              currentContext.sections.push(section);
            }
            currentContext = section;
            contextStack.push(section);
            continue;
          }
          
          
          if (trimmedLine.startsWith('card-section-image')) {
            const parts = trimmedLine.split(/\s+/);
            const sectionId = parts.length > 1 ? removeQuotes(parts[1]) : null;
            const section = { type: 'card-section-image', items: [], id: sectionId };
            if (currentContext && currentContext.sections) {
              currentContext.sections.push(section);
            } else if (currentContext && currentContext.type === 'settings-card') {
              currentContext.sections.push(section);
            }
            currentContext = section;
            contextStack.push(section);
            continue;
          }
          
          
          if (trimmedLine.startsWith('card-section-info')) {
            const parts = trimmedLine.split(/\s+/);
            const sectionId = parts.length > 1 ? removeQuotes(parts[1]) : null;
            const section = { type: 'card-section-info', items: [], id: sectionId };
            if (currentContext && currentContext.sections) {
              currentContext.sections.push(section);
            } else if (currentContext && currentContext.type === 'settings-card') {
              currentContext.sections.push(section);
            }
            currentContext = section;
            contextStack.push(section);
            continue;
          }
          
          
          if (trimmedLine.startsWith('divider') || trimmedLine.startsWith('hr')) {
            const parts = trimmedLine.split(/\s+/);
            const dividerId = parts.length > 1 ? removeQuotes(parts[1]) : null;
            const divider = { type: 'divider', id: dividerId, style: null };
            
            
            if (i + 1 < lines.length) {
              const nextLineIndent = getIndentLevel(lines[i + 1]);
              const nextLineTrimmed = lines[i + 1].trim();
              if (nextLineIndent > indentLevel && nextLineTrimmed.startsWith('style:')) {
                divider.style = removeQuotes(nextLineTrimmed.substring(6).trim());
                i++; 
              }
            }
            
            if (currentContext && currentContext.sections) {
              currentContext.sections.push(divider);
            } else if (currentContext && currentContext.items) {
              currentContext.items.push(divider);
            }
            continue;
          }
          
          
          if (currentContext) {
            
            if (trimmedLine.startsWith('title:')) {
              let value = trimmedLine.substring(6).trim();
              if (!value && i + 1 < lines.length) {
                value = lines[i + 1].trim();
                i++;
              }
              const item = { type: 'title', text: removeQuotes(value) };
              if (currentContext.items) currentContext.items.push(item);
              else if (currentContext.sections) {
                
                currentContext.sections.push({ type: 'card-section', items: [item] });
              }
              continue;
            }
            
            
            if (trimmedLine.startsWith('label:')) {
              let value = trimmedLine.substring(6).trim();
              if (!value && i + 1 < lines.length) {
                value = lines[i + 1].trim();
                i++;
              }
              const item = { type: 'label', text: removeQuotes(value) };
              if (currentContext.items) currentContext.items.push(item);
              continue;
            }
            
            
            if (trimmedLine.startsWith('toggle') || trimmedLine.startsWith('switch')) {
              const parts = trimmedLine.split(/\s+/);
              const item = { 
                type: 'toggle',
                id: parts[1] ? removeQuotes(parts[1]) : null,
                checked: trimmedLine.includes('checked'),
                disabled: trimmedLine.includes('disabled'),
                onChange: null
              };
              
              
              let j = i + 1;
              while (j < lines.length && (getIndentLevel(lines[j]) > indentLevel || lines[j].trim() === '')) {
                const nextLine = lines[j].trim();
                if (nextLine.startsWith('on-change:')) {
                  item.onChange = removeQuotes(nextLine.substring(10).trim());
                  j++;
                  break;
                } else if (nextLine && !nextLine.includes(':') && !nextLine.startsWith('toggle') && !nextLine.startsWith('switch')) {
                  break;
                }
                j++;
              }
              
              if (currentContext.items) currentContext.items.push(item);
              i = j - 1;
              continue;
            }
            
            
            if (trimmedLine.startsWith('select')) {
              const parts = trimmedLine.split(/\s+/);
              const item = {
                type: 'select',
                id: parts[1] ? removeQuotes(parts[1]) : null,
                options: [],
                disabled: trimmedLine.includes('disabled'),
                onChange: null
              };
              if (currentContext.items) currentContext.items.push(item);
              
              let optionContext = item;
              continue;
            }
            
            
            if (trimmedLine.startsWith('option:')) {
              let value = trimmedLine.substring(7).trim();
              const parts = value.split('|');
              const option = {
                value: removeQuotes(parts[0] || ''),
                text: removeQuotes(parts[1] || parts[0] || ''),
                selected: trimmedLine.includes('selected')
              };
              
              let lastSelect = null;
              
              
              if (currentContext && currentContext.items) {
                for (let j = currentContext.items.length - 1; j >= 0; j--) {
                  if (currentContext.items[j].type === 'select') {
                    lastSelect = currentContext.items[j];
                    break;
                  }
                }
              }
              
              
              if (!lastSelect && currentContext && contextStack.length > 1) {
                const parentContext = contextStack[contextStack.length - 2];
                if (parentContext && parentContext.items) {
                  for (let j = parentContext.items.length - 1; j >= 0; j--) {
                    if (parentContext.items[j].type === 'select') {
                      lastSelect = parentContext.items[j];
                      break;
                    }
                  }
                }
              }
              
              if (lastSelect) {
                lastSelect.options.push(option);
              }
              continue;
            }
            
            
            if (trimmedLine.startsWith('on-change:')) {
              
              let lastSelect = null;
              if (currentContext && currentContext.items) {
                for (let j = currentContext.items.length - 1; j >= 0; j--) {
                  if (currentContext.items[j].type === 'select') {
                    lastSelect = currentContext.items[j];
                    break;
                  }
                }
              }
              if (lastSelect) {
                lastSelect.onChange = removeQuotes(trimmedLine.substring(10).trim());
              }
              continue;
            }
            
            
            if (trimmedLine.startsWith('button')) {
              const parts = trimmedLine.split(/\s+/);
              let text = '';
              let buttonId = null;
              let buttonClass = 'default';
              
              
              for (let p = 1; p < parts.length; p++) {
                const part = parts[p];
                if (part !== 'primary' && part !== 'delete' && part !== 'default') {
                  buttonId = removeQuotes(part);
                } else {
                  buttonClass = part;
                }
              }
              
              
              let j = i + 1;
              while (j < lines.length && (getIndentLevel(lines[j]) > indentLevel || lines[j].trim() === '')) {
                const nextLine = lines[j].trim();
                if (nextLine && !nextLine.includes(':')) {
                  text = removeQuotes(nextLine);
                  j++;
                  break;
                }
                j++;
              }
              
              
              let onClick = null;
              while (j < lines.length && (getIndentLevel(lines[j]) > indentLevel || lines[j].trim() === '')) {
                const nextLine = lines[j].trim();
                if (nextLine.startsWith('on-click:')) {
                  onClick = removeQuotes(nextLine.substring(9).trim());
                  j++;
                  break;
                } else if (nextLine && !nextLine.includes(':')) {
                  break;
                }
                j++;
              }
              
              const item = {
                type: 'button',
                text: text,
                id: buttonId,
                class: trimmedLine.includes('primary') ? 'primary' : 
                       trimmedLine.includes('delete') ? 'delete' : buttonClass,
                onClick: onClick
              };
              if (currentContext.items) currentContext.items.push(item);
              i = j - 1;
              continue;
            }
            
            
            if (trimmedLine.startsWith('input')) {
              const parts = trimmedLine.split(/\s+/);
              const item = {
                type: 'input',
                inputType: trimmedLine.includes('password') ? 'password' : 'text',
                id: parts[1] ? removeQuotes(parts[1]) : null,
                placeholder: ''
              };
              
              if (trimmedLine.includes('placeholder:')) {
                const placeholderMatch = trimmedLine.match(/placeholder:["']?([^"']+)["']?/);
                if (placeholderMatch) {
                  item.placeholder = removeQuotes(placeholderMatch[1]);
                }
              }
              if (currentContext.items) currentContext.items.push(item);
              continue;
            }
            
            
            if (trimmedLine.startsWith('text:') || trimmedLine.startsWith('description:')) {
              let value = trimmedLine.substring(trimmedLine.indexOf(':') + 1).trim();
              if (!value && i + 1 < lines.length) {
                value = lines[i + 1].trim();
                i++;
              }
              const item = { type: 'text', text: removeQuotes(value) };
              
              
              if (value.includes('No known networks') || trimmedLine.includes('No known networks')) {
                console.log('Parser: *** Found text: "No known networks" ***, currentContext:', currentContext?.type, 'currentContext.id:', currentContext?.id, 'has items:', !!currentContext?.items);
              }
              
              if (currentContext.items) {
                currentContext.items.push(item);
                console.log('Parser: Text added to items, currentContext.id:', currentContext.id, 'items count:', currentContext.items.length);
              } else {
                console.error('Parser: currentContext.items does not exist! currentContext:', currentContext?.type, 'currentContext.id:', currentContext?.id);
              }
              continue;
            }
            
            
            if (trimmedLine.startsWith('info:')) {
              let value = trimmedLine.substring(5).trim();
              if (!value && i + 1 < lines.length) {
                value = lines[i + 1].trim();
                i++;
              }
              const item = { 
                type: 'info',
                text: removeQuotes(value),
                link: null,
                linkText: null
              };
              
              const linkMatch = value.match(/\[([^\]]+)\]\(([^)]+)\)/);
              if (linkMatch) {
                item.linkText = linkMatch[1];
                item.link = linkMatch[2];
                item.text = value.replace(/\[([^\]]+)\]\(([^)]+)\)/, linkMatch[1]);
              }
              if (currentContext.items) currentContext.items.push(item);
              continue;
            }
            
            
            if (trimmedLine.startsWith('card-item') || trimmedLine.startsWith('item')) {
              const item = {
                type: 'card-item',
                icon: '',
                title: '',
                subtitle: '',
                arrow: false
              };
              if (currentContext.items) {
                currentContext.items.push(item);
                const itemContext = item;
                
                let j = i + 1;
                while (j < lines.length && getIndentLevel(lines[j]) > indentLevel) {
                  const propLine = lines[j].trim();
                  if (propLine.startsWith('icon:')) {
                    itemContext.icon = removeQuotes(propLine.substring(5).trim());
                  } else if (propLine.startsWith('title:')) {
                    itemContext.title = removeQuotes(propLine.substring(6).trim());
                  } else if (propLine.startsWith('subtitle:')) {
                    itemContext.subtitle = removeQuotes(propLine.substring(9).trim());
                  } else if (propLine.startsWith('arrow:')) {
                    itemContext.arrow = removeQuotes(propLine.substring(6).trim()).toLowerCase() === 'true';
                  }
                  j++;
                }
                i = j - 1;
              }
              continue;
            }
            
            
            if (trimmedLine.startsWith('radio-group')) {
              const group = {
                type: 'radio-group',
                name: '',
                items: []
              };
              if (currentContext.items) {
                currentContext.items.push(group);
                const groupContext = group;
                let j = i + 1;
                const groupIndent = getIndentLevel(lines[i]);
                while (j < lines.length) {
                  const nextLine = lines[j];
                  const nextIndent = getIndentLevel(nextLine);
                  const propLine = nextLine.trim();
                  
                  
                  if (nextIndent <= groupIndent && propLine && !propLine.startsWith('name:') && !propLine.startsWith('radio:')) {
                    break;
                  }
                  
                  if (!propLine) {
                    j++;
                    continue;
                  }
                  
                  if (propLine.startsWith('name:')) {
                    groupContext.name = removeQuotes(propLine.substring(5).trim());
                  } else if (propLine.startsWith('radio:')) {
                    const radioParts = propLine.substring(6).trim().split('|');
                    const radio = {
                      value: removeQuotes(radioParts[0] || ''),
                      text: removeQuotes(radioParts[1] || radioParts[0] || ''),
                      checked: propLine.includes('checked')
                    };
                    groupContext.items.push(radio);
                  }
                  j++;
                }
                i = j - 1;
              }
              continue;
            }
            
            
            if (trimmedLine.startsWith('icon:')) {
              let value = trimmedLine.substring(5).trim();
              value = removeQuotes(value);
              const item = { type: 'icon', src: value };
              if (currentContext.items) currentContext.items.push(item);
              continue;
            }
            
            
            if (trimmedLine.startsWith('image:')) {
              let value = trimmedLine.substring(6).trim();
              value = removeQuotes(value);
              const item = { type: 'image', src: value, alt: '' };
              if (trimmedLine.includes('alt:')) {
                const altMatch = trimmedLine.match(/alt:["']?([^"']+)["']?/);
                if (altMatch) {
                  item.alt = removeQuotes(altMatch[1]);
                }
              }
              if (currentContext.items) currentContext.items.push(item);
              continue;
            }
            
            
            if (trimmedLine.startsWith('network-item')) {
              const parts = trimmedLine.split(/\s+/);
              const itemId = parts.length > 1 ? removeQuotes(parts[1]) : null;
              const item = {
                type: 'network-item',
                id: itemId,
                ssid: '',
                ssidId: null
              };
              
              
              let j = i + 1;
              while (j < lines.length && (getIndentLevel(lines[j]) > indentLevel || lines[j].trim() === '')) {
                const propLine = lines[j].trim();
                if (propLine.startsWith('ssid:')) {
                  item.ssid = removeQuotes(propLine.substring(5).trim());
                } else if (propLine.startsWith('ssid-id:')) {
                  item.ssidId = removeQuotes(propLine.substring(8).trim());
                }
                j++;
              }
              
              if (currentContext.items) currentContext.items.push(item);
              i = j - 1;
              continue;
            }
          }
        }
      }
      
      
      if (currentPage) {
        result.pages.push(currentPage);
      }
      
      
      
      if (result.pages.length > 0) {
        result.page = result.pages[0];
      } else {
        result.page = { id: '', content: [] };
      }
      
      return result;
    } catch (error) {
      console.error(`Error parsing manifest for module ${moduleId}:`, error);
      console.error(error.stack);
      return null;
    }
  }
  
  
  function validateModule(moduleData, moduleId) {
    const errors = [];
    
    
    if (!moduleData.manifest) {
      errors.push('Manifest is missing');
      return { valid: false, errors };
    }
    
    
    const parsed = parseModuleManifest(moduleData.manifest, moduleId);
    if (!parsed) {
      errors.push('Could not parse manifest');
      return { valid: false, errors };
    }
    
    
    if (!parsed.main.name || !parsed.main.description) {
      errors.push('The __main section must contain at least name and description');
    }
    
    
    if (!parsed.item.label || !parsed.item.icon || !parsed.item['page-id']) {
      errors.push('The __item section must contain label, icon and page-id');
    }
    
    
    if (!parsed.pages || parsed.pages.length === 0) {
      errors.push('The module must contain at least one __page section with id');
    } else {
      
      for (let i = 0; i < parsed.pages.length; i++) {
        if (!parsed.pages[i].id) {
          errors.push(`Page ${i + 1} must contain id`);
        }
      }
      
      
      if (parsed.item['page-id']) {
        const pageIdExists = parsed.pages.some(page => page.id === parsed.item['page-id']);
        if (!pageIdExists) {
          errors.push(`page-id "${parsed.item['page-id']}" from __item does not match any id from __page`);
        }
      }
    }
    
    if (errors.length > 0) {
      return { valid: false, errors, parsed };
    }
    
    return { valid: true, errors: [], parsed };
  }
  
  
  async function loadModuleScript(moduleId) {
    if (!window.electronAPI || !window.electronAPI.moduleReadFile) {
      console.warn('API for reading module files is not available');
      return;
    }
    
    try {
      const response = await window.electronAPI.moduleReadFile(moduleId, 'script.js');
      if (response && response.content) {
        
        const moduleContext = {
          moduleId: moduleId,
          registerFunction: (functionName, func) => {
            if (!moduleFunctionRegistry[moduleId]) {
              moduleFunctionRegistry[moduleId] = {};
            }
            moduleFunctionRegistry[moduleId][functionName] = func;
            console.log(`Function ${functionName} registered for module ${moduleId}`);
          },
          getElement: (id) => {
            return document.getElementById(id);
          },
          electronAPI: window.electronAPI 
        };
        
        
        const moduleScript = new Function('module', response.content);
        moduleScript(moduleContext);
      }
    } catch (error) {
      
      
    }
  }
  
  
  function callModuleFunction(moduleId, functionName, ...args) {
    if (moduleFunctionRegistry[moduleId] && moduleFunctionRegistry[moduleId][functionName]) {
      try {
        return moduleFunctionRegistry[moduleId][functionName](...args);
      } catch (error) {
        console.error(`Error calling function ${functionName} from module ${moduleId}:`, error);
      }
    } else {
      console.warn(`Function ${functionName} is not registered for module ${moduleId}`);
    }
  }
  
  
  async function loadModules() {
    if (!window.electronAPI || !window.electronAPI.modulesList) {
      console.warn('API for module is not available');
      return;
    }
    
    try {
      const response = await window.electronAPI.modulesList();
      const modules = response.modules || [];
      
      
      for (const moduleData of modules) {
        const moduleId = moduleData.id;
        const validation = validateModule(moduleData, moduleId);
        
        if (validation.valid && validation.parsed) {
          
          const moduleInfo = {
            id: moduleId,
            main: validation.parsed.main,
            item: validation.parsed.item,
            page: validation.parsed.page, 
            pages: validation.parsed.pages, 
            path: moduleData.path
          };
          loadedModules[validation.parsed.item['page-id']] = moduleInfo;
          
          
          addModuleToSidebar(validation.parsed.item, moduleId);
          
          
          if (validation.parsed.pages && validation.parsed.pages.length > 0) {
            for (const page of validation.parsed.pages) {
              createModulePage(page, moduleId);
            }
          } else {
            
            createModulePage(validation.parsed.page, moduleId);
          }
          
          
          setTimeout(() => {
            loadModuleScript(moduleId);
          }, 100);
        } else {
          console.error(`Modulul ${moduleId} is not valid:`, validation.errors);
        }
      }
    } catch (error) {
      console.error('Error loading modules:', error);
    }
  }
  
  
  let modulesNavSection = null;
  
  
  function addModuleToSidebar(item, moduleId) {
    
    if (!modulesNavSection) {
      
      const printersScannersItem = document.querySelector('.item[data-page="Printers & Scanners"]');
      if (!printersScannersItem) {
        console.error('Not found item Printers & Scanners');
        return;
      }
      
      
      const parentNavSection = printersScannersItem.closest('.nav-section');
      if (!parentNavSection) {
        console.error('Not found section de navigare');
        return;
      }
      
      
      modulesNavSection = document.createElement('div');
      modulesNavSection.className = 'nav-section';
      
      const itemsContainer = document.createElement('div');
      itemsContainer.className = 'items';
      modulesNavSection.appendChild(itemsContainer);
      
      
      parentNavSection.parentNode.insertBefore(modulesNavSection, parentNavSection.nextSibling);
    }
    
    const itemsContainer = modulesNavSection.querySelector('.items');
    if (!itemsContainer) {
      console.error('Container for items not found');
      return;
    }
    
    
    const newItem = document.createElement('div');
    newItem.className = 'item';
    newItem.setAttribute('data-page', item.label);
    
    
    let iconPath = item.icon;
    if (iconPath.startsWith('./')) {
      iconPath = iconPath.substring(2);
    }
    
    newItem.innerHTML = `
      <div class="content">
        <img src="modules/${moduleId}/${iconPath}" class="icon gray" alt="${item.label}"/>
        <p class="name">${item.label}</p>
      </div>
    `;
    
    
    newItem.addEventListener('click', () => {
      const itemName = newItem.querySelector('.name');
      if (itemName) {
        showPage(itemName.textContent.trim());
      }
    });
    
    
    itemsContainer.appendChild(newItem);
  }
  
  
  function resolveModulePath(path, moduleId) {
    if (!path) return '';
    if (path.startsWith('./')) {
      path = path.substring(2);
    }
    return `modules/${moduleId}/${path}`;
  }
  
  
  function renderElement(element, moduleId) {
    let html = '';
    
    switch (element.type) {
      case 'section-title':
        html = `<h3 class="section-title">${escapeHtml(element.text || '')}</h3>`;
        break;
        
      case 'settings-card':
        html = '<div class="settings-card">';
        if (element.sections) {
          for (const section of element.sections) {
            html += renderSection(section, moduleId);
          }
        }
        html += '</div>';
        break;
        
      case 'divider':
        const dividerId = element.id ? ` id="${escapeHtml(element.id)}"` : '';
        const dividerStyle = element.style ? ` style="${escapeHtml(element.style)}"` : '';
        html = `<hr class="card-divider"${dividerId}${dividerStyle}>`;
        break;
        
      case 'title':
        html = `<h3 class="card-title">${escapeHtml(element.text || '')}</h3>`;
        break;
        
      case 'label':
        html = `<h4 class="color-title">${escapeHtml(element.text || '')}</h4>`;
        break;
        
      case 'text':
      case 'description':
        html = `<p style="font-size: 14px; color: var(--text-secondary); line-height: 1.5; margin: 0;">${escapeHtml(element.text || '')}</p>`;
        break;
        
      case 'toggle':
      case 'switch':
        const toggleId = element.id || `toggle-${Math.random().toString(36).substr(2, 9)}`;
        html = `<label class="switch">
          <input type="checkbox" id="${toggleId}" ${element.checked ? 'checked' : ''} ${element.disabled ? 'disabled' : ''}>
          <span class="slider"></span>
        </label>`;
        break;
        
      case 'select':
        const selectId = element.id || `select-${Math.random().toString(36).substr(2, 9)}`;
        html = `<select id="${selectId}" class="size-dropdown" ${element.disabled ? 'disabled' : ''}>`;
        if (element.options) {
          for (const option of element.options) {
            html += `<option value="${escapeHtml(option.value)}" ${option.selected ? 'selected' : ''}>${escapeHtml(option.text || option.value)}</option>`;
          }
        }
        html += '</select>';
        break;
        
      case 'button':
        const buttonId = element.id || `button-${Math.random().toString(36).substr(2, 9)}`;
        let buttonClass = 'update-button';
        if (element.class === 'delete') buttonClass = 'delete-button';
        else if (element.class === 'primary') buttonClass = 'update-button';
        html = `<button id="${buttonId}" class="${buttonClass}">${escapeHtml(element.text || '')}</button>`;
        break;
        
      case 'input':
        const inputId = element.id || `input-${Math.random().toString(36).substr(2, 9)}`;
        html = `<input type="${element.inputType || 'text'}" id="${inputId}" class="password-input" placeholder="${escapeHtml(element.placeholder || '')}">`;
        break;
        
      case 'info':
        html = '<div class="card-section-info">';
        html += `<p class="info-text">${escapeHtml(element.text || '')}`;
        if (element.link && element.linkText) {
          html += ` <a href="${escapeHtml(element.link)}" class="info-link">${escapeHtml(element.linkText)}</a>`;
        }
        html += '</p></div>';
        break;
        
      case 'card-item':
        html = '<div class="settings-card-item" style="cursor: pointer; display: flex; align-items: center; padding: 16px; border-bottom: 1px solid var(--border-color);">';
        if (element.icon) {
          html += `<img src="${resolveModulePath(element.icon, moduleId)}" alt="${escapeHtml(element.title || '')}" style="width: 48px; height: 48px; margin-right: 16px; opacity: 0.8;"/>`;
        }
        html += '<div style="flex: 1;">';
        if (element.title) {
          html += `<h3 style="font-size: 16px; font-weight: 500; color: var(--text-primary); margin: 0 0 4px 0;">${escapeHtml(element.title)}</h3>`;
        }
        if (element.subtitle) {
          html += `<p style="font-size: 14px; color: var(--text-secondary); margin: 0;">${escapeHtml(element.subtitle)}</p>`;
        }
        html += '</div>';
        if (element.arrow) {
          html += '<i data-feather="chevron-right" style="width: 20px; height: 20px; color: var(--text-secondary);"></i>';
        }
        html += '</div>';
        break;
        
      case 'radio-group':
        html = '<div class="radio-group">';
        if (element.items) {
          for (const radio of element.items) {
            const radioId = `radio-${Math.random().toString(36).substr(2, 9)}`;
            html += `<label class="radio-label">
              <input type="radio" name="${escapeHtml(element.name || 'radio')}" value="${escapeHtml(radio.value)}" id="${radioId}" ${radio.checked ? 'checked' : ''}>
              <span class="radio-text">${escapeHtml(radio.text || radio.value)}</span>
            </label>`;
          }
        }
        html += '</div>';
        break;
        
      case 'icon':
        html = `<img src="${resolveModulePath(element.src, moduleId)}" alt="" style="width: 48px; height: 48px; opacity: 0.8;">`;
        break;
        
      case 'image':
        html = `<img src="${resolveModulePath(element.src, moduleId)}" alt="${escapeHtml(element.alt || '')}" style="width: 48px; height: 48px; opacity: 0.8;">`;
        break;
        
      case 'network-item':
        const networkItemId = element.id ? ` id="${escapeHtml(element.id)}"` : '';
        const ssidId = element.ssidId ? ` id="${escapeHtml(element.ssidId)}"` : '';
        html = `<div class="network-item"${networkItemId}>
          <span class="network-checkmark">✓</span>
          <span class="network-ssid"${ssidId}>${escapeHtml(element.ssid || 'Network Name')}</span>
        </div>`;
        break;
    }
    
    return html;
  }
  
  
  function renderSection(section, moduleId) {
    if (!section || !section.type) return '';
    
    let html = '';
    
    switch (section.type) {
      case 'card-section':
        const sectionId = section.id ? ` id="${escapeHtml(section.id)}"` : '';
        const sectionStyle = section.style ? ` style="${escapeHtml(section.style)}"` : '';
        
        
        if (section.id === 'wifi-clone-no-network-section' || section.id?.includes('no-network')) {
          console.log('Render: *** Generating card-section wifi-clone-no-network-section ***, ID:', section.id, 'items:', section.items?.length || 0, 'section:', section);
        }
        
        html = `<div class="card-section"${sectionId}${sectionStyle}>`;
        if (section.items) {
          for (const item of section.items) {
            html += renderElement(item, moduleId);
          }
        }
        html += '</div>';
        break;
        
      case 'card-section-vertical':
        const verticalSectionId = section.id ? ` id="${escapeHtml(section.id)}"` : '';
        html = `<div class="card-section-vertical"${verticalSectionId}>`;
        if (section.items) {
          for (const item of section.items) {
            html += renderElement(item, moduleId);
          }
        }
        html += '</div>';
        break;
        
      case 'card-section-image':
        const imageSectionId = section.id ? ` id="${escapeHtml(section.id)}"` : '';
        html = `<div class="card-section-image"${imageSectionId}>`;
        
        
        let image = null;
        let title = null;
        let description = null;
        let toggle = null;
        
        if (section.items) {
          for (const item of section.items) {
            if (item.type === 'image' || item.type === 'icon') {
              image = item;
            } else if (item.type === 'title') {
              title = item;
            } else if (item.type === 'text' || item.type === 'description') {
              description = item;
            } else if (item.type === 'toggle') {
              toggle = item;
            }
          }
        }
        
        
        if (image) {
          const imgSrc = image.type === 'image' ? resolveModulePath(image.src, moduleId) : resolveModulePath(image.src, moduleId);
          const imgAlt = image.alt || '';
          html += `<img src="${imgSrc}" alt="${escapeHtml(imgAlt)}" class="feature-image">`;
        }
        
        
        if (title || description) {
          html += '<div class="feature-content">';
          if (title) {
            html += `<h3 class="feature-title">${escapeHtml(title.text || '')}</h3>`;
          }
          if (description) {
            html += `<p class="feature-description">${escapeHtml(description.text || '')}</p>`;
          }
          html += '</div>';
        }
        
        
        if (toggle) {
          const toggleId = toggle.id || `toggle-${Math.random().toString(36).substr(2, 9)}`;
          html += `<label class="switch">
            <input type="checkbox" id="${toggleId}" ${toggle.checked ? 'checked' : ''} ${toggle.disabled ? 'disabled' : ''}>
            <span class="slider"></span>
          </label>`;
        }
        
        html += '</div>';
        break;
        
      case 'card-section-info':
        html = '<div class="card-section-info">';
        if (section.items) {
          for (const item of section.items) {
            html += renderElement(item, moduleId);
          }
        }
        html += '</div>';
        break;
        
      case 'divider':
        html = '<hr class="card-divider">';
        break;
    }
    
    return html;
  }
  
  
  function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
  }
  
  
  function attachEventListeners(element, moduleId, pageData) {
    if (!element || !element.id) return;
    
    const domElement = document.getElementById(element.id);
    if (!domElement) return;
    
    
    if (element.type === 'button' && element.onClick) {
      domElement.addEventListener('click', (e) => {
        callModuleFunction(moduleId, element.onClick, e, domElement);
      });
    }
    
    
    if (element.type === 'toggle' && element.onChange) {
      domElement.addEventListener('change', (e) => {
        callModuleFunction(moduleId, element.onChange, e.target.checked, e, domElement);
      });
    }
    
    
    if (element.type === 'select' && element.onChange) {
      domElement.addEventListener('change', (e) => {
        callModuleFunction(moduleId, element.onChange, e.target.value, e, domElement);
      });
    }
  }
  
  
  function findElementsWithHandlers(items, moduleId, handlers) {
    if (!items) return;
    
    for (const item of items) {
      
      if (item.type === 'button' && item.onClick) {
        handlers.push({ element: item, handler: 'onClick' });
      } else if (item.type === 'toggle' && item.onChange) {
        handlers.push({ element: item, handler: 'onChange' });
      } else if (item.type === 'select' && item.onChange) {
        handlers.push({ element: item, handler: 'onChange' });
      }
      
      
      if (item.sections && Array.isArray(item.sections)) {
        for (const section of item.sections) {
          if (section.items && Array.isArray(section.items)) {
            findElementsWithHandlers(section.items, moduleId, handlers);
          }
        }
      } else if (item.items && Array.isArray(item.items)) {
        findElementsWithHandlers(item.items, moduleId, handlers);
      }
    }
  }
  
  
  function createModulePage(page, moduleId) {
    const contentArea = document.querySelector('.content-area');
    if (!contentArea) {
      console.error('Not found content area');
      return;
    }
    
    
    const pageDiv = document.createElement('div');
    pageDiv.className = 'page-content';
    pageDiv.id = `${page.id}-page`;
    pageDiv.style.display = 'none';
    
    
    let pageHTML = '';
    
    
    for (const contentItem of page.content) {
      pageHTML += renderElement(contentItem, moduleId);
    }
    
    pageDiv.innerHTML = pageHTML;
    contentArea.appendChild(pageDiv);
    
    
    feather.replace();
    
    
    setTimeout(() => {
      const handlers = [];
      findElementsWithHandlers(page.content, moduleId, handlers);
      
      for (const handlerInfo of handlers) {
        attachEventListeners(handlerInfo.element, moduleId, page);
      }
      
      
      if (moduleFunctionRegistry[moduleId]) {
        
        const initFunction = moduleFunctionRegistry[moduleId]['initPage'] ||
                             moduleFunctionRegistry[moduleId]['init'];
        if (initFunction && typeof initFunction === 'function') {
          try {
            console.log(`Calling initialization function for module ${moduleId}`);
            initFunction();
          } catch (error) {
            console.error(`Error initializing module ${moduleId}:`, error);
          }
        }
      }
    }, 100);
  }
  
  
  
  
  function showPage(pageName, addToHistoryFlag = true) {
    // Ascunde spinner-ul când se afișează o pagină
    const spinner = document.getElementById('content-spinner');
    if (spinner) {
      spinner.style.display = 'none';
    }
    
    if (dateTimeInterval) {
      clearInterval(dateTimeInterval);
      dateTimeInterval = null;
    }
    
    
    if (addToHistoryFlag) {
      addToHistory(pageName);
    }
    
    
    updatePageTitleAndSidebar(pageName);
    
    
    const allPages = document.querySelectorAll('.page-content');
    allPages.forEach(page => {
      page.style.display = 'none';
    });
    
    
    if (pageName === 'Appearance') {
      const appearancePage = document.getElementById('appearance-page');
      if (appearancePage) {
        appearancePage.style.display = 'block';
        initAppearancePage();
      }
    } else if (pageName === 'Pear Intelligence and Piri' || pageName === 'Pear Piri') {
      const pearPiriPage = document.getElementById('pear-piri-page');
      if (pearPiriPage) {
        pearPiriPage.style.display = 'block';
      }
    } else if (pageName === 'Wi-Fi') {
      const wifiPage = document.getElementById('wifi-page');
      if (wifiPage) {
        wifiPage.style.display = 'block';
        initWiFiPage();
      }
    } else if (pageName === 'Bluetooth') {
      const bluetoothPage = document.getElementById('bluetooth-page');
      if (bluetoothPage) {
        bluetoothPage.style.display = 'block';
        initBluetoothPage();
      }
    } else if (pageName === 'Network') {
      const networkPage = document.getElementById('network-page');
      if (networkPage) {
        networkPage.style.display = 'block';
        initNetworkPage();
      }
    } else if (pageName === 'Battey' || pageName === 'Battery') {
      const batteryPage = document.getElementById('battery-page');
      if (batteryPage) {
        batteryPage.style.display = 'block';
        initBatteryPage();
      }
    } else if (pageName === 'General') {
      const generalPage = document.getElementById('general-page');
      if (generalPage) {
        generalPage.style.display = 'block';
      }
      const allGeneralMenuItems = document.querySelectorAll('.general-menu-item');
      allGeneralMenuItems.forEach(menuItem => {
        menuItem.style.display = '';
      });
      const searchInput = document.querySelector('.sidebar-search');
      if (searchInput) {
        searchInput.value = '';
        const allItems = document.querySelectorAll('.sidenav .item');
        const allNavSections = document.querySelectorAll('.sidenav .nav-section');
        allItems.forEach(item => {
          item.style.display = '';
        });
        allNavSections.forEach(section => {
          section.style.display = '';
        });
      }
      // Ascunde notificarea imediat, apoi verifică starea pentru a o afișa doar dacă este necesar
      const notification = document.getElementById('software-update-notification');
      if (notification) {
        notification.style.display = 'none';
      }
      // Verifică din nou starea notificării de software update după resetarea display-ului
      // pentru a o afișa doar dacă numărul de updates este > 0
      if (window.electronAPI) {
        updateSoftwareUpdateNotification();
      }
    } else if (pageName === 'About') {
      const aboutPage = document.getElementById('about-page');
      if (aboutPage) {
        aboutPage.style.display = 'block';
        initAboutPage();
      }
    } else if (pageName === 'PearCare & Warranty') {
      const pearcarePage = document.getElementById('pearcare-page');
      if (pearcarePage) {
        pearcarePage.style.display = 'block';
      }
    } else if (pageName === 'PearDrop & Handoff') {
      const peardropPage = document.getElementById('peardrop-page');
      if (peardropPage) {
        peardropPage.style.display = 'block';
      }
    } else if (pageName === 'AutoFill & Passwords') {
      const autofillPage = document.getElementById('autofill-page');
      if (autofillPage) {
        autofillPage.style.display = 'block';
        initAutofillPage();
      }
    } else if (pageName === 'Software Update') {
      const softwareUpdatePage = document.getElementById('software-update-page');
      if (softwareUpdatePage) {
        softwareUpdatePage.style.display = 'block';
        initSoftwareUpdatePage();
      }
    } else if (pageName === 'Storage') {
      const storagePage = document.getElementById('storage-page');
      if (storagePage) {
        storagePage.style.display = 'block';
        initStoragePage();
      }
    } else if (pageName === 'Date & Time') {
      const dateTimePage = document.getElementById('date-time-page');
      if (dateTimePage) {
        dateTimePage.style.display = 'block';
        initDateTimePage();
      }
    } else if (pageName === 'Login Items & Extensions') {
      const loginItemsPage = document.getElementById('login-items-page');
      if (loginItemsPage) {
        loginItemsPage.style.display = 'block';
        initLoginItemsPage();
      }
    } else if (pageName === 'Sharing') {
      const sharingPage = document.getElementById('sharing-page');
      if (sharingPage) {
        sharingPage.style.display = 'block';
      }
    } else if (pageName === 'Startup Disk') {
      const startupDiskPage = document.getElementById('startup-disk-page');
      if (startupDiskPage) {
        startupDiskPage.style.display = 'block';
        initStartupDiskPage();
      }
    } else if (pageName === 'Device Management') {
      const deviceManagementPage = document.getElementById('device-management-page');
      if (deviceManagementPage) {
        deviceManagementPage.style.display = 'block';
      }
    } else if (pageName === 'Time Machine') {
      const timeMachinePage = document.getElementById('time-machine-page');
      if (timeMachinePage) {
        timeMachinePage.style.display = 'block';
      }
    } else if (pageName === 'Transfer or Reset') {
      const transferResetPage = document.getElementById('transfer-reset-page');
      if (transferResetPage) {
        transferResetPage.style.display = 'block';
      }
    } else if (pageName === 'Accessibility') {
      const accessibilityPage = document.getElementById('accessibility-page');
      if (accessibilityPage) {
        accessibilityPage.style.display = 'block';
      }
    } else if (pageName === 'Menu Bar') {
      const menuBarPage = document.getElementById('menu-bar-page');
      if (menuBarPage) {
        menuBarPage.style.display = 'block';
      }
    } else if (pageName === 'Spotlight') {
      const spotlightPage = document.getElementById('spotlight-page');
      if (spotlightPage) {
        spotlightPage.style.display = 'block';
      }
    } else if (pageName === 'Desktop and Dock') {
      const desktopDockPage = document.getElementById('desktop-dock-page');
      if (desktopDockPage) {
        desktopDockPage.style.display = 'block';
      }
    } else if (pageName === 'Displays') {
      const displaysPage = document.getElementById('displays-page');
      if (displaysPage) {
        displaysPage.style.display = 'block';
        initDisplaysPage();
      }
    } else if (pageName === 'Wallpaper') {
      const wallpaperPage = document.getElementById('wallpaper-page');
      if (wallpaperPage) {
        wallpaperPage.style.display = 'block';
        initWallpaperPage();
      }
    } else if (pageName === 'Notifications') {
      const notificationsPage = document.getElementById('notifications-page');
      if (notificationsPage) {
        notificationsPage.style.display = 'block';
      }
    } else if (pageName === 'Sound') {
      const soundPage = document.getElementById('sound-page');
      if (soundPage) {
        soundPage.style.display = 'block';
        initSoundPage();
      }
    } else if (pageName === 'Focus') {
      const focusPage = document.getElementById('focus-page');
      if (focusPage) {
        focusPage.style.display = 'block';
      }
    } else if (pageName === 'Lock Screen') {
      const lockScreenPage = document.getElementById('lock-screen-page');
      if (lockScreenPage) {
        lockScreenPage.style.display = 'block';
        initLockScreenPage();
      }
    } else if (pageName === 'Privacy and Security') {
      const privacySecurityPage = document.getElementById('privacy-security-page');
      if (privacySecurityPage) {
        privacySecurityPage.style.display = 'block';
        initPrivacySecurityPage();
      }
    } else if (pageName === 'Touch ID and Password') {
      const touchIdPasswordPage = document.getElementById('touch-id-password-page');
      if (touchIdPasswordPage) {
        touchIdPasswordPage.style.display = 'block';
        initTouchIdPasswordPage();
      }
    } else if (pageName === 'Personal Information') {
      const personalInformationPage = document.getElementById('personal-information-page');
      if (personalInformationPage) {
        personalInformationPage.style.display = 'block';
        setTimeout(async () => {
          await initPersonalInformationPage();
        }, 100);
      }
    } else if (pageName === 'Sign In & Security') {
      const signinSecurityPage = document.getElementById('signin-security-page');
      if (signinSecurityPage) {
        signinSecurityPage.style.display = 'block';
        setTimeout(async () => {
          await initSignInSecurityPage();
        }, 100);
      }
    } else if (pageName === 'Payment & Shipping') {
      const paymentShippingPage = document.getElementById('payment-shipping-page');
      if (paymentShippingPage) {
        paymentShippingPage.style.display = 'block';
        setTimeout(async () => {
          await initPaymentShippingPage();
        }, 100);
      }
    } else if (pageName === 'Pear Cloud') {
      const pearCloudPage = document.getElementById('pear-cloud-page');
      if (pearCloudPage) {
        pearCloudPage.style.display = 'block';
      }
    } else if (pageName === 'Media & Purchases') {
      const mediaPurchasesPage = document.getElementById('media-purchases-page');
      if (mediaPurchasesPage) {
        mediaPurchasesPage.style.display = 'block';
        setTimeout(async () => {
          await initMediaPurchasesPage();
        }, 100);
      }
    } else if (pageName === 'Sign in with Pear') {
      const signinPearPage = document.getElementById('signin-pear-page');
      if (signinPearPage) {
        signinPearPage.style.display = 'block';
      }
    } else if (pageName === 'Device Info') {
      const deviceDetailsPage = document.getElementById('device-details-page');
      if (deviceDetailsPage) {
        deviceDetailsPage.style.display = 'block';
      }
    } else if (pageName === 'PearID Sign In') {
      const pearIdSignInPage = document.getElementById('pearid-signin-page');
      if (pearIdSignInPage) {
        console.log('Showing PearID sign in page');
        pearIdSignInPage.style.display = 'block';
        // Apelează initPearIdSignInPage după un mic delay pentru a se asigura că pagina este vizibilă
        setTimeout(() => {
          initPearIdSignInPage();
        }, 100);
      } else {
        console.error('pearid-signin-page not found');
      }
    } else if (pageName === 'User Profile') {
      const userProfilePage = document.getElementById('user-profile-page');
      if (userProfilePage) {
        console.log('Showing user profile page');
        userProfilePage.style.display = 'block';
        // Apelează initUserProfilePage după un mic delay pentru a se asigura că pagina este vizibilă
        setTimeout(async () => {
          await initUserProfilePage();
        }, 100);
      } else {
        console.error('user-profile-page not found');
      }
    } else if (pageName === 'Users & Groups') {
      const usersGroupsPage = document.getElementById('users-groups-page');
      if (usersGroupsPage) {
        usersGroupsPage.style.display = 'block';
        initUsersGroupsPage();
      }
    } else if (pageName === 'Internet Accounts') {
      const internetAccountsPage = document.getElementById('internet-accounts-page');
      if (internetAccountsPage) {
        internetAccountsPage.style.display = 'block';
      }
    } else if (pageName === 'Game Center') {
      const gameCenterPage = document.getElementById('game-center-page');
      if (gameCenterPage) {
        gameCenterPage.style.display = 'block';
      }
    } else if (pageName === 'pCloud') {
      const pcloudPage = document.getElementById('pcloud-page');
      if (pcloudPage) {
        pcloudPage.style.display = 'block';
      }
    } else if (pageName === 'Wallet & Pear Pay') {
      const walletPearPayPage = document.getElementById('wallet-pear-pay-page');
      if (walletPearPayPage) {
        walletPearPayPage.style.display = 'block';
      }
    } else if (pageName === 'Keyboard') {
      const keyboardPage = document.getElementById('keyboard-page');
      if (keyboardPage) {
        keyboardPage.style.display = 'block';
        initKeyboardPage();
      }
    } else if (pageName === 'Trackpad') {
      const trackpadPage = document.getElementById('trackpad-page');
      if (trackpadPage) {
        trackpadPage.style.display = 'block';
        initTrackpadPage();
      }
    } else if (pageName === 'Printers & Scanners') {
      const printersScannersPage = document.getElementById('printers-scanners-page');
      if (printersScannersPage) {
        printersScannersPage.style.display = 'block';
      }
    } else if (pageName === 'Screen Time') {
      const screenTimePage = document.getElementById('screen-time-page');
      if (screenTimePage) {
        screenTimePage.style.display = 'block';
      }
    } else {
      
      let isModulePage = false;
      for (const pageId in loadedModules) {
        const moduleInfo = loadedModules[pageId];
        if (moduleInfo.item.label === pageName) {
          const modulePage = document.getElementById(`${moduleInfo.page.id}-page`);
          if (modulePage) {
            modulePage.style.display = 'block';
            isModulePage = true;
            
            feather.replace();
            
            
            if (moduleFunctionRegistry[moduleInfo.id]) {
              
              const initFunction = moduleFunctionRegistry[moduleInfo.id]['initPage'] ||
                                   moduleFunctionRegistry[moduleInfo.id]['init'];
              if (initFunction && typeof initFunction === 'function') {
                try {
                  initFunction();
                } catch (error) {
                  console.error(`Error initializing module ${moduleInfo.id}:`, error);
                }
              }
            }
            
            break;
          }
        }
      }
      
      
      if (!isModulePage) {
      const defaultPage = document.getElementById('default-page');
      if (defaultPage) {
        defaultPage.style.display = 'block';
        }
      }
    }
  }
  
  
  
  function getSystemTheme() {
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
      return 'dark';
    }
    return 'light';
  }

  
  // Variabilă globală pentru accentul curent
  let currentAccentColor = 'blue'; // Default
  
  function applyTheme(theme) {
    const root = document.documentElement;
    if (theme === 'auto') {
      const systemTheme = getSystemTheme();
      root.setAttribute('data-theme', systemTheme);
    } else {
      root.setAttribute('data-theme', theme);
    }
    
    localStorage.setItem('theme-preference', theme);
    
    // Reaplică culorile accentului când se schimbă tema
    updateAccentColors(currentAccentColor);
  }

  
  function initTheme() {
    const savedTheme = localStorage.getItem('theme-preference') || 'auto';
    applyTheme(savedTheme);
    
    
    const themeButtons = document.querySelectorAll('.theme-button');
    themeButtons.forEach(button => {
      const buttonTheme = button.getAttribute('data-theme');
      const wrapper = button.closest('.theme-button-wrapper');
      if (buttonTheme === savedTheme) {
        button.classList.add('active');
        if (wrapper) wrapper.classList.add('active');
      } else {
        button.classList.remove('active');
        if (wrapper) wrapper.classList.remove('active');
      }
    });

    
    if (savedTheme === 'auto') {
      const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
      mediaQuery.addEventListener('change', () => {
        applyTheme('auto');
      });
    }
  }

  
  initTheme();
  
  // Inițializează culoarea accentului la încărcarea paginii
  // Se va actualiza când se încarcă pagina Appearance, dar setăm o valoare default
  if (window.electronAPI && window.electronAPI.getAccent) {
    window.electronAPI.getAccent().then(accentResult => {
      if (accentResult && accentResult.accent) {
        const accentColor = accentResult.accent.trim();
        updateAccentColors(accentColor);
        currentAccentColor = accentColor;
      } else {
        // Dacă nu există accent, folosește default (blue)
        updateAccentColors('blue');
      }
    }).catch(error => {
      console.error('Error loading accent color on init:', error);
      // Dacă nu poate citi accentul, folosește default (blue)
      updateAccentColors('blue');
    });
  } else {
    // Dacă electronAPI nu este disponibil, folosește default (blue)
    updateAccentColors('blue');
  }

  
  const themeButtons = document.querySelectorAll('.theme-button');
  themeButtons.forEach(button => {
    button.addEventListener('click', async () => {
      const theme = button.getAttribute('data-theme');
      
      // Ignoră butonul "auto" - acesta nu execută kde-theme-switch.sh
      if (theme === 'auto') {
        // Elimină active de la toate butoanele
        themeButtons.forEach(btn => {
          btn.classList.remove('active');
          const wrapper = btn.closest('.theme-button-wrapper');
          if (wrapper) wrapper.classList.remove('active');
        });
        
        // Adaugă active la butonul curent
        button.classList.add('active');
        const wrapper = button.closest('.theme-button-wrapper');
        if (wrapper) wrapper.classList.add('active');
        
        // Aplică tema
        applyTheme(theme);
        return;
      }
      
      // Pentru dark și light, execută kde-theme-switch.sh
      if (theme === 'dark' || theme === 'light') {
        // Verifică dacă electronAPI este disponibil
        if (!window.electronAPI || !window.electronAPI.switchTheme) {
          console.error('electronAPI.switchTheme not available');
          // Aplică tema local chiar dacă scriptul nu este disponibil
          applyTheme(theme);
          return;
        }
        
        try {
          // Execută kde-theme-switch.sh
          const result = await window.electronAPI.switchTheme(theme);
          
          if (result && result.success) {
            // Schimbă tema aplicației
            applyTheme(theme);
            
            // Elimină active de la toate butoanele
            themeButtons.forEach(btn => {
              btn.classList.remove('active');
              const wrapper = btn.closest('.theme-button-wrapper');
              if (wrapper) wrapper.classList.remove('active');
            });
            
            // Adaugă active la butonul curent
            button.classList.add('active');
            const wrapper = button.closest('.theme-button-wrapper');
            if (wrapper) wrapper.classList.add('active');
            
            // Verifică dacă utilizatorul este logat cu PearID și face backup
            if (window.electronAPI && window.electronAPI.checkPearIdState) {
              try {
                const pearIdState = await window.electronAPI.checkPearIdState();
                if (pearIdState && pearIdState.state === 'true') {
                  // Utilizatorul este logat, face backup pentru state și accent
                  if (window.electronAPI.backupThemeSettings) {
                    try {
                      await window.electronAPI.backupThemeSettings();
                      console.log('Theme settings backed up successfully');
                    } catch (backupError) {
                      console.error('Error backing up theme settings:', backupError);
                      // Nu aruncăm eroare - backup-ul este opțional
                    }
                  }
                }
              } catch (pearIdError) {
                console.error('Error checking PearID state:', pearIdError);
                // Nu aruncăm eroare - verificarea este opțională
              }
            }
          } else {
            console.error('Failed to switch theme');
          }
        } catch (error) {
          console.error('Error switching theme:', error);
          // Chiar dacă scriptul eșuează, aplică tema local
          applyTheme(theme);
          
          // Elimină active de la toate butoanele
          themeButtons.forEach(btn => {
            btn.classList.remove('active');
            const wrapper = btn.closest('.theme-button-wrapper');
            if (wrapper) wrapper.classList.remove('active');
          });
          
          // Adaugă active la butonul curent
          button.classList.add('active');
          const wrapper = button.closest('.theme-button-wrapper');
          if (wrapper) wrapper.classList.add('active');
        }
      } else {
        // Pentru alte teme, aplică doar local
        applyTheme(theme);
        
        // Elimină active de la toate butoanele
        themeButtons.forEach(btn => {
          btn.classList.remove('active');
          const wrapper = btn.closest('.theme-button-wrapper');
          if (wrapper) wrapper.classList.remove('active');
        });
        
        // Adaugă active la butonul curent
        button.classList.add('active');
        const wrapper = button.closest('.theme-button-wrapper');
        if (wrapper) wrapper.classList.add('active');
      }
    });
  });

  // Funcție pentru actualizarea variabilelor CSS --active-bg și --hover-bg în funcție de accent
  function updateAccentColors(accentColorName) {
    // Maparea culorilor de accent
    const accentColors = {
      'purple': '#8B5CF6',
      'magenta': '#EC4899',
      'orange': '#F97316',
      'yellow': '#EAB308',
      'green': '#22C55E',
      'azul': '#06B6D4',
      'blue': '#3B82F6',
      'lila': '#A855F7',
      'dark-purple': '#6B21A8',
      'grey': '#6B7280'
    };
    
    const colorHex = accentColors[accentColorName] || accentColors['blue']; // Default la blue
    
    // Convertesc hex în RGB
    const hex = colorHex.replace('#', '');
    const r = parseInt(hex.substr(0, 2), 16);
    const g = parseInt(hex.substr(2, 2), 16);
    const b = parseInt(hex.substr(4, 2), 16);
    
    // Actualizez variabilele CSS pentru hover și active
    // Verificăm dacă suntem în mod dark sau light
    const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
    const root = document.documentElement;
    
    // Setează culoarea accentului pentru toate utilizările (inclusiv border-uri și box-shadow)
    root.style.setProperty('--accent-color', colorHex);
    
    if (isDark) {
      // Pentru dark theme, folosim culoarea accentului cu transparență mai mică
      root.style.setProperty('--hover-bg', `rgba(${r}, ${g}, ${b}, 0.15)`);
      root.style.setProperty('--active-bg', `rgba(${r}, ${g}, ${b}, 0.2)`);
    } else {
      // Pentru light theme, folosim culoarea accentului cu transparență mai mică
      root.style.setProperty('--hover-bg', `rgba(${r}, ${g}, ${b}, 0.1)`);
      root.style.setProperty('--active-bg', `rgba(${r}, ${g}, ${b}, 0.15)`);
    }
    
    // Setează box-shadow pentru color-dot folosind culoarea accentului
    root.style.setProperty('--accent-shadow', `rgba(${r}, ${g}, ${b}, 0.2)`);
  }

  // Funcție pentru inițializarea paginii Appearance
  // Variabile globale pentru tint window background
  let globalTintEnabled = false;
  let globalWindowMoveListener = null;
  let globalTintUpdateTimeout = null;
  let isUpdatingTint = false;
  let pendingTintUpdate = false;
  
  // Funcție pentru inițializarea setării de tint la pornirea aplicației
  async function initTintWindowSetting() {
    if (!window.electronAPI || !window.electronAPI.loadTintWindowSetting) {
      return;
    }
    
    try {
      const setting = await window.electronAPI.loadTintWindowSetting();
      const enabled = setting && typeof setting.enabled === 'boolean' ? setting.enabled : true; // Default ON
      globalTintEnabled = enabled;
      
      // Activează tint-ul dacă este setat
      if (enabled) {
        if (window.electronAPI && window.electronAPI.onWindowMoved) {
          // Înregistrează listener pentru mișcarea ferestrei
          // Folosim throttle în loc de debounce pentru actualizare în timp real
          globalWindowMoveListener = (position) => {
            if (!isUpdatingTint) {
              // Actualizează imediat dacă nu este în proces de actualizare
              updateGlobalTint();
            } else {
              // Marchează că trebuie să actualizăm din nou după ce se termină actualizarea curentă
              pendingTintUpdate = true;
            }
          };
          
          window.electronAPI.onWindowMoved(globalWindowMoveListener);
          
          // Actualizează tint-ul inițial imediat
          setTimeout(() => {
            updateGlobalTint();
          }, 50);
        }
      }
    } catch (error) {
      console.error('Error loading tint window setting:', error);
      // Default ON în caz de eroare
      globalTintEnabled = true;
      if (window.electronAPI && window.electronAPI.onWindowMoved) {
        globalWindowMoveListener = (position) => {
          if (!isUpdatingTint) {
            updateGlobalTint();
          } else {
            pendingTintUpdate = true;
          }
        };
        window.electronAPI.onWindowMoved(globalWindowMoveListener);
        setTimeout(() => {
          updateGlobalTint();
        }, 50);
      }
    }
  }
  
  // Funcție pentru actualizarea înălțimii tint-ului în containere scrollable
  function updateScrollableTintHeight() {
    // Găsește toate containerele scrollable
    const scrollableElements = [
      document.querySelector('.window .inner-wrapper .sidenav'),
      document.querySelector('.window .inner-wrapper .main-content .content-area')
    ];
    
    scrollableElements.forEach(element => {
      if (element) {
        // Obține înălțimea reală a conținutului scrollable
        const scrollHeight = element.scrollHeight;
        const clientHeight = element.clientHeight;
        // Folosește înălțimea scrollable sau cel puțin înălțimea viewport-ului
        const height = Math.max(scrollHeight, clientHeight);
        
        // Creează sau actualizează un pseudo-element pentru a acoperi întreaga zonă
        // Folosim un selector CSS custom pentru a seta înălțimea dinamic
        const styleId = `tint-height-${element.classList[element.classList.length - 1] || 'element'}`;
        let styleElement = document.getElementById(styleId);
        
        if (!styleElement) {
          styleElement = document.createElement('style');
          styleElement.id = styleId;
          document.head.appendChild(styleElement);
        }
        
        // Generează selectorul CSS unic pentru acest element
        const uniqueSelector = `.window .inner-wrapper .sidenav::before,
                                .window .inner-wrapper .main-content .content-area::before`;
        
        styleElement.textContent = `
          .window .inner-wrapper .sidenav::before,
          .window .inner-wrapper .main-content .content-area::before {
            height: ${height}px !important;
            min-height: 100% !important;
          }
        `;
      }
    });
  }
  
  // Funcție globală pentru actualizarea tint-ului
  async function updateGlobalTint() {
    if (!globalTintEnabled || !window.electronAPI || !window.electronAPI.getWindowPosition || !window.electronAPI.getWallpaperColorAtPosition) {
      return;
    }
    
    // Dacă este deja în proces de actualizare, marchează pentru re-înnoire
    if (isUpdatingTint) {
      pendingTintUpdate = true;
      return;
    }
    
    isUpdatingTint = true;
    
    try {
      const position = await window.electronAPI.getWindowPosition();
      if (!position || position.width === 0 || position.height === 0) {
        isUpdatingTint = false;
        if (pendingTintUpdate) {
          pendingTintUpdate = false;
          // Reîncepe actualizarea dacă există o cerere pending
          setTimeout(() => updateGlobalTint(), 0);
        }
        return;
      }
      
      const color = await window.electronAPI.getWallpaperColorAtPosition(
        position.x,
        position.y,
        position.width,
        position.height
      );
      
      if (color && color.rgb) {
        const { r, g, b } = color.rgb;
        const tintOpacity = 0.05;
        const tintColor = `rgba(${r}, ${g}, ${b}, ${tintOpacity})`;
        document.documentElement.style.setProperty('--window-tint', tintColor);
        
        // Actualizează înălțimea pentru pseudo-elementele din containere scrollable
        // pentru a acoperi întreaga zonă scrollable
        updateScrollableTintHeight();
      }
    } catch (error) {
      console.error('Error updating tint:', error);
    } finally {
      isUpdatingTint = false;
      
      // Dacă există o cerere pending, actualizează din nou
      if (pendingTintUpdate) {
        pendingTintUpdate = false;
        // Folosim requestAnimationFrame pentru actualizare mai fluentă
        requestAnimationFrame(() => {
          updateGlobalTint();
          updateScrollableTintHeight();
        });
      }
    }
  }
  
  async function initAppearancePage() {
    // Inițializează selecția culoare accent la încărcarea paginii
    let currentAccent = null;
    if (window.electronAPI && window.electronAPI.getAccent) {
      try {
        const accentResult = await window.electronAPI.getAccent();
        if (accentResult && accentResult.accent) {
          currentAccent = accentResult.accent.trim();
          // Actualizează variabila globală
          currentAccentColor = currentAccent;
          // Selectează radio button-ul corespunzător
          const colorRadio = document.querySelector(`input[name="color"][value="${currentAccent}"]`);
          if (colorRadio) {
            colorRadio.checked = true;
          }
        }
      } catch (error) {
        console.error('Error loading accent color:', error);
      }
    }
    
    // Actualizează culorile accentului (sau folosește blue ca default)
    if (currentAccent) {
      currentAccentColor = currentAccent;
    }
    updateAccentColors(currentAccentColor);
    
    // Adaugă event listener pentru radio buttons de culori
    const colorRadios = document.querySelectorAll('input[name="color"].color-radio');
    colorRadios.forEach(radio => {
      radio.addEventListener('change', async (e) => {
        if (e.target.checked) {
          const colorName = e.target.value;
          
          // Verifică dacă electronAPI este disponibil
          if (!window.electronAPI || !window.electronAPI.switchAccent) {
            console.error('electronAPI.switchAccent not available');
            return;
          }
          
          try {
            // Execută kde-theme-switch.sh --accent colorname
            const result = await window.electronAPI.switchAccent(colorName);
            
            if (result && result.success) {
              console.log(`Accent color switched to: ${colorName}`);
              
              // Actualizează variabila globală
              currentAccentColor = colorName;
              
              // Actualizează culorile accentului
              updateAccentColors(colorName);
              
              // Verifică dacă utilizatorul este logat cu PearID și face backup
              if (window.electronAPI && window.electronAPI.checkPearIdState) {
                try {
                  const pearIdState = await window.electronAPI.checkPearIdState();
                  if (pearIdState && pearIdState.state === 'true') {
                    // Utilizatorul este logat, face backup pentru state și accent
                    if (window.electronAPI.backupThemeSettings) {
                      try {
                        await window.electronAPI.backupThemeSettings();
                        console.log('Theme settings backed up successfully after accent change');
                      } catch (backupError) {
                        console.error('Error backing up theme settings:', backupError);
                        // Nu aruncăm eroare - backup-ul este opțional
                      }
                    }
                  }
                } catch (pearIdError) {
                  console.error('Error checking PearID state:', pearIdError);
                  // Nu aruncăm eroare - verificarea este opțională
                }
              }
            } else {
              console.error('Failed to switch accent color');
            }
          } catch (error) {
            console.error('Error switching accent color:', error);
          }
        }
      });
    });
    
    // Inițializează funcționalitatea de tint pentru window background
    const tintCheckbox = document.getElementById('tint-window-checkbox');
    if (tintCheckbox && window.electronAPI) {
      // Sincronizează checkbox-ul cu setarea globală
      tintCheckbox.checked = globalTintEnabled;
      
      // Funcție pentru eliminarea tint-ului
      const removeTint = () => {
        document.documentElement.style.removeProperty('--window-tint');
      };
      
      // Event listener pentru checkbox
      tintCheckbox.addEventListener('change', async (e) => {
        globalTintEnabled = e.target.checked;
        
        // Salvează setarea
        if (window.electronAPI && window.electronAPI.saveTintWindowSetting) {
          try {
            await window.electronAPI.saveTintWindowSetting(globalTintEnabled);
          } catch (error) {
            console.error('Error saving tint window setting:', error);
          }
        }
        
        if (globalTintEnabled) {
          // Activează tint-ul
          if (window.electronAPI && window.electronAPI.onWindowMoved) {
            // Înregistrează listener pentru mișcarea ferestrei dacă nu există deja
            if (!globalWindowMoveListener) {
              globalWindowMoveListener = (position) => {
                if (!isUpdatingTint) {
                  // Actualizează imediat dacă nu este în proces de actualizare
                  updateGlobalTint();
                } else {
                  // Marchează că trebuie să actualizăm din nou după ce se termină actualizarea curentă
                  pendingTintUpdate = true;
                }
              };
              
              window.electronAPI.onWindowMoved(globalWindowMoveListener);
            }
            
            // Actualizează tint-ul inițial imediat
            setTimeout(() => {
              updateGlobalTint();
            }, 50);
          }
        } else {
          // Dezactivează tint-ul
          if (window.electronAPI && window.electronAPI.removeWindowMovedListener) {
            window.electronAPI.removeWindowMovedListener();
            globalWindowMoveListener = null;
          }
          
          if (globalTintUpdateTimeout) {
            clearTimeout(globalTintUpdateTimeout);
            globalTintUpdateTimeout = null;
          }
          
          removeTint();
        }
      });
    }
  }

  
  function initWiFiPage() {
    const wifiToggle = document.getElementById('wifi-toggle');
    const wifiConnectedSection = document.getElementById('wifi-connected-section');
    const wifiDivider = document.getElementById('wifi-divider');
    const wifiSSID = document.getElementById('wifi-ssid');

    if (!wifiToggle || !window.electronAPI) return;

    
    async function updateWiFiStatus() {
      try {
        const status = await window.electronAPI.wifiGetStatus();
        wifiToggle.checked = status.enabled;

        
        if (status.enabled) {
          const ssidInfo = await window.electronAPI.wifiGetSSID();
          if (ssidInfo.connected && ssidInfo.ssid) {
            wifiSSID.textContent = ssidInfo.ssid;
            wifiConnectedSection.style.display = 'flex';
            wifiDivider.style.display = 'block';
            
            const linkIcon = document.getElementById('wifi-link-icon');
            const lockIcon = document.getElementById('wifi-lock-icon');
            
            if (linkIcon) {
              const ssidLower = ssidInfo.ssid.toLowerCase();
              const showLink = ssidLower.includes('phone') || 
                              ssidLower.includes('hotspot') || 
                              ssidLower.includes('android');
              linkIcon.style.display = showLink ? 'block' : 'none';
            }
            
            if (lockIcon && ssidInfo.security && ssidInfo.security !== '--') {
              lockIcon.style.display = 'block';
            } else if (lockIcon) {
              lockIcon.style.display = 'none';
            }
            
            const knownNetworkSection = document.getElementById('known-network-section');
            const noNetworkSection = document.getElementById('no-network-section');
            const knownNetworkSSID = document.getElementById('known-network-ssid');
            const knownNetworkSignal = document.getElementById('known-network-signal');
            const knownNetworkLockIcon = document.getElementById('known-network-lock-icon');
            
            if (knownNetworkSection && knownNetworkSSID && noNetworkSection) {
              knownNetworkSSID.textContent = ssidInfo.ssid;
              knownNetworkSection.style.display = 'flex';
              noNetworkSection.style.display = 'none';
              
              if (knownNetworkLockIcon) {
                if (ssidInfo.security && ssidInfo.security !== '--') {
                  knownNetworkLockIcon.style.display = 'block';
                } else {
                  knownNetworkLockIcon.style.display = 'none';
                }
              }
              
              try {
                const networksData = await window.electronAPI.wifiGetNetworks();
                if (networksData.networks && networksData.networks.length > 0) {
                  const connectedNetwork = networksData.networks.find(network => network.ssid === ssidInfo.ssid);
                  if (connectedNetwork && connectedNetwork.signal && knownNetworkSignal) {
                    knownNetworkSignal.textContent = `${connectedNetwork.signal}%`;
                  } else if (knownNetworkSignal) {
                    knownNetworkSignal.textContent = '0%';
                  }
                } else if (knownNetworkSignal) {
                  knownNetworkSignal.textContent = '0%';
                }
              } catch (error) {
                console.error('Error getting network signal:', error);
                if (knownNetworkSignal) {
                  knownNetworkSignal.textContent = '0%';
                }
              }
            }
          } else {
            wifiConnectedSection.style.display = 'none';
            wifiDivider.style.display = 'none';
            
            
            const knownNetworkSection = document.getElementById('known-network-section');
            const noNetworkSection = document.getElementById('no-network-section');
            if (knownNetworkSection && noNetworkSection) {
              knownNetworkSection.style.display = 'none';
              noNetworkSection.style.display = 'block';
            }
          }
        } else {
          wifiConnectedSection.style.display = 'none';
          wifiDivider.style.display = 'none';
          
          
          const knownNetworkSection = document.getElementById('known-network-section');
          const noNetworkSection = document.getElementById('no-network-section');
          if (knownNetworkSection && noNetworkSection) {
            knownNetworkSection.style.display = 'none';
            noNetworkSection.style.display = 'block';
          }
        }
      } catch (error) {
        console.error('Error updating WiFi status:', error);
      }
    }

    
    async function updateWiFiNetworks() {
      const networksList = document.getElementById('wifi-networks-list');
      if (!networksList || !window.electronAPI) return;

      try {
        const status = await window.electronAPI.wifiGetStatus();
        if (!status.enabled) {
          networksList.innerHTML = '<p class="no-network-text">Wi-Fi is turned off</p>';
          return;
        }

        
        let connectedSSID = null;
        try {
          const ssidInfo = await window.electronAPI.wifiGetSSID();
          if (ssidInfo.connected && ssidInfo.ssid) {
            connectedSSID = ssidInfo.ssid;
          }
        } catch (error) {
          
        }

        const networksData = await window.electronAPI.wifiGetNetworks();
        if (networksData.networks && networksData.networks.length > 0) {
          
          const filteredNetworks = networksData.networks.filter(network => {
            return network.ssid && network.ssid !== connectedSSID;
          });

          if (filteredNetworks.length > 0) {
            networksList.innerHTML = filteredNetworks.map(network => {
              const hasPassword = network.security && network.security !== '--';
              return `
              <div class="network-item-clickable">
                <span class="network-ssid">${network.ssid || 'Unknown'}</span>
                <div class="network-actions">
                  <button class="network-connect-button" data-ssid="${network.ssid || 'Unknown'}">Connect</button>
                  <div style="display: flex; align-items: center; gap: 6px;">
                    ${hasPassword ? `<img src="./assets/lock.svg" alt="Lock" style="width: 12px; height: 12px; opacity: 0.7;" class="wifi-icon-theme">` : ''}
                  <span class="network-signal">${network.signal || '0'}%</span>
                </div>
              </div>
              </div>
            `;
            }).join('');
          } else {
            networksList.innerHTML = '<p class="no-network-text">No other networks found</p>';
          }
        } else {
          networksList.innerHTML = '<p class="no-network-text">No networks found</p>';
        }
      } catch (error) {
        console.error('Error updating WiFi networks:', error);
        networksList.innerHTML = '<p class="no-network-text">Error scanning for networks</p>';
      }
    }

    
    updateWiFiStatus();
    updateWiFiNetworks();

    
    const statusInterval = setInterval(() => {
      updateWiFiStatus();
      updateWiFiNetworks();
    }, 2000);

    
    wifiToggle.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.wifiToggle(e.target.checked);
        
        setTimeout(updateWiFiStatus, 500);
      } catch (error) {
        console.error('Error toggling WiFi:', error);
        
        e.target.checked = !e.target.checked;
      }
    });

    
    window.addEventListener('beforeunload', () => {
      clearInterval(statusInterval);
    });

    
    document.addEventListener('click', async (e) => {
      if (e.target.classList.contains('network-connect-button')) {
        const ssid = e.target.getAttribute('data-ssid');
        if (ssid && window.electronAPI) {
          openWiFiModal(ssid);
        }
      }
      
      if (e.target.classList.contains('select-button') && e.target.textContent === 'Details...') {
        const ssidInfo = await window.electronAPI.wifiGetSSID();
        if (ssidInfo.connected && ssidInfo.ssid) {
          openWiFiDetailsModal(ssidInfo.ssid);
        }
      }
    });
    
    const detailsModalCancel = document.getElementById('wifi-details-modal-cancel');
    const detailsModalForget = document.getElementById('wifi-details-modal-forget');
    
    if (detailsModalCancel) {
      detailsModalCancel.addEventListener('click', () => {
        const modal = document.getElementById('wifi-details-modal');
        if (modal) {
          modal.style.display = 'none';
      }
    });
  }

    if (detailsModalForget) {
      detailsModalForget.addEventListener('click', async () => {
        const modal = document.getElementById('wifi-details-modal');
        const networkName = document.getElementById('details-modal-network-name');
        if (modal && networkName) {
          const ssid = networkName.textContent;
          try {
            await window.electronAPI.wifiForgetNetwork(ssid);
            modal.style.display = 'none';
            setTimeout(() => {
              updateWiFiStatus();
              updateWiFiNetworks();
            }, 500);
          } catch (error) {
            console.error('Error forgetting network:', error);
            alert('Could not forget network. Please check your system permissions.');
          }
        }
      });
    }
  }
  
  async function openWiFiDetailsModal(ssid) {
    const modal = document.getElementById('wifi-details-modal');
    const networkName = document.getElementById('details-modal-network-name');
    const security = document.getElementById('details-modal-security');
    const signal = document.getElementById('details-modal-signal');
    const status = document.getElementById('details-modal-status');
    
    if (!modal || !networkName) return;
    
    networkName.textContent = ssid;
    
    try {
      const ssidInfo = await window.electronAPI.wifiGetSSID();
      const networksData = await window.electronAPI.wifiGetNetworks();
      
      if (ssidInfo.connected && ssidInfo.ssid === ssid) {
        if (status) {
          status.textContent = 'Connected';
        }
        if (security) {
          security.textContent = ssidInfo.security && ssidInfo.security !== '--' ? ssidInfo.security : 'None';
        }
        
        if (networksData.networks && networksData.networks.length > 0) {
          const connectedNetwork = networksData.networks.find(network => network.ssid === ssid);
          if (connectedNetwork && connectedNetwork.signal && signal) {
            signal.textContent = `${connectedNetwork.signal}%`;
          } else if (signal) {
            signal.textContent = '0%';
          }
        } else if (signal) {
          signal.textContent = '0%';
        }
      } else {
        if (status) {
          status.textContent = 'Not connected';
        }
        if (networksData.networks && networksData.networks.length > 0) {
          const network = networksData.networks.find(n => n.ssid === ssid);
          if (network) {
            if (security) {
              security.textContent = network.security && network.security !== '--' ? network.security : 'None';
            }
            if (signal) {
              signal.textContent = `${network.signal || '0'}%`;
            }
          }
        }
      }
    } catch (error) {
      console.error('Error loading network details:', error);
    }
    
    modal.style.display = 'flex';
  }

  
  function openWiFiModal(ssid) {
    const modal = document.getElementById('wifi-modal');
    const networkName = document.getElementById('modal-network-name');
    const passwordInput = document.getElementById('wifi-password-input');
    const showPasswordToggle = document.getElementById('show-password-toggle');
    const cancelButton = document.getElementById('wifi-modal-cancel');
    const connectButton = document.getElementById('wifi-modal-connect');

    if (!modal || !networkName || !passwordInput) return;

    networkName.textContent = ssid;
    passwordInput.value = '';
    passwordInput.type = 'password';
    showPasswordToggle.checked = false;
    modal.style.display = 'flex';

    
    showPasswordToggle.addEventListener('change', (e) => {
      passwordInput.type = e.target.checked ? 'text' : 'password';
    });

    
    const cancelHandler = () => {
      modal.style.display = 'none';
      cancelButton.removeEventListener('click', cancelHandler);
      connectButton.removeEventListener('click', connectHandler);
    };
    cancelButton.addEventListener('click', cancelHandler);

    
    const connectHandler = async () => {
      const password = passwordInput.value;
      if (!password) {
        alert('Please enter a password');
        return;
      }

      connectButton.disabled = true;
      connectButton.textContent = 'Connecting...';

      try {
        await window.electronAPI.wifiConnect(ssid, password);
        modal.style.display = 'none';
        
        setTimeout(() => {
          const wifiPage = document.getElementById('wifi-page');
          if (wifiPage && wifiPage.style.display !== 'none') {
            initWiFiPage();
          }
        }, 1000);
      } catch (error) {
        console.error('Error connecting to WiFi:', error);
        alert('Failed to connect to network. Please check the password and try again.');
        connectButton.disabled = false;
        connectButton.textContent = 'Connect';
      }

      cancelButton.removeEventListener('click', cancelHandler);
      connectButton.removeEventListener('click', connectHandler);
    };
    connectButton.addEventListener('click', connectHandler);

    
    modal.addEventListener('click', (e) => {
      if (e.target === modal) {
        cancelHandler();
      }
    });
  }

  
  function initNetworkPage() {
    updateActiveConnections();
    updateRfkillControls();
    
    
    const networkInterval = setInterval(() => {
      if (document.getElementById('network-page') && document.getElementById('network-page').style.display !== 'none') {
        updateActiveConnections();
        updateRfkillControls();
      } else {
        clearInterval(networkInterval);
      }
    }, 3000);
  }

  
  async function updateActiveConnections() {
    const connectionsList = document.getElementById('active-connections-list');
    if (!connectionsList || !window.electronAPI) return;

    try {
      const connections = await window.electronAPI.getActiveConnections();
      
      console.log('Active connections received:', connections);
      
      if (connections && connections.length > 0) {
        connectionsList.innerHTML = connections.map(conn => {
          
          let icon = '📡';
          let displayType = conn.type.toUpperCase();
          if (conn.type.includes('wireless') || conn.type === 'wifi' || conn.type === '802-11-wireless') {
            icon = '📶';
            displayType = 'Wi-Fi';
          } else if (conn.type === 'ethernet' || conn.type === '802-3-ethernet') {
            icon = '🔌';
            displayType = 'Ethernet';
          } else if (conn.type === 'loopback') {
            icon = '🔁';
            displayType = 'Loopback';
          }
          
          return `
            <div class="connection-item">
              <div class="connection-info">
                <div class="connection-icon">${icon}</div>
                <div class="connection-details">
                  <div class="connection-name">${conn.name || 'Unknown Connection'}</div>
                  <div class="connection-status">${conn.device} • ${conn.status || 'Unknown'}</div>
                </div>
              </div>
              <div class="connection-actions">
                <span class="connection-type">${displayType}</span>
              </div>
            </div>
          `;
        }).join('');
      } else {
        connectionsList.innerHTML = '<p class="no-network-text">No active connections</p>';
      }
    } catch (error) {
      console.error('Error updating active connections:', error);
      connectionsList.innerHTML = '<p class="no-network-text">Error loading connections</p>';
    }
  }

  
  async function updateRfkillControls() {
    const rfkillControls = document.getElementById('rfkill-controls');
    if (!rfkillControls || !window.electronAPI) return;

    try {
      const rfkillStatus = await window.electronAPI.getRfkillStatus();
      
      if (rfkillStatus && rfkillStatus.length > 0) {
        rfkillControls.innerHTML = rfkillStatus.map(device => `
          <div class="card-section">
            <div class="rfkill-info">
              <span class="rfkill-name">${device.name || 'Unknown Device'}</span>
              <span class="rfkill-type">${device.type || 'Unknown'}</span>
            </div>
            <label class="switch">
              <input type="checkbox" ${device.softBlocked ? '' : 'checked'} data-type="${device.type}" data-id="${device.id}">
              <span class="slider"></span>
            </label>
          </div>
        `).join('');
        
        
        rfkillControls.querySelectorAll('input[type="checkbox"]').forEach(checkbox => {
          checkbox.addEventListener('change', async (e) => {
            const type = e.target.getAttribute('data-type');
            const id = e.target.getAttribute('data-id');
            const enabled = e.target.checked;
            
            try {
              await window.electronAPI.setRfkillState(type, id, enabled);
              
              updateRfkillControls();
            } catch (error) {
              console.error('Error setting rfkill state:', error);
              
              e.target.checked = !enabled;
            }
          });
        });
      } else {
        rfkillControls.innerHTML = '<p class="no-network-text">No radio devices found</p>';
      }
    } catch (error) {
      console.error('Error updating rfkill controls:', error);
      rfkillControls.innerHTML = '<p class="no-network-text">Error loading radio controls</p>';
    }
  }

  
  function initBatteryPage() {
    updateBatteryInfo();
    updateBatteryTitle();
    updateLowPowerMode();
    updateBatteryHealth();
    initBatteryCharts();
    
    
    const lowPowerModeToggle = document.getElementById('low-power-mode-toggle');
    if (lowPowerModeToggle && window.electronAPI) {
      lowPowerModeToggle.addEventListener('change', async (e) => {
        try {
          await window.electronAPI.setLowPowerMode(e.target.checked);
        } catch (error) {
          console.error('Error setting low power mode:', error);
          
          e.target.checked = !e.target.checked;
        }
      });
    }
    
    
    const batteryTabs = document.querySelectorAll('.battery-tab');
    batteryTabs.forEach(tab => {
      tab.addEventListener('click', () => {
        batteryTabs.forEach(t => t.classList.remove('active'));
        tab.classList.add('active');
        const period = tab.getAttribute('data-period');
        updateBatteryCharts(period);
      });
    });
    
    
    const batteryInterval = setInterval(() => {
      if (document.getElementById('battery-page') && document.getElementById('battery-page').style.display !== 'none') {
        updateBatteryInfo();
        updateBatteryTitle();
        updateLowPowerMode();
        updateBatteryHealth();
        const activeTab = document.querySelector('.battery-tab.active');
        if (activeTab) {
          updateBatteryCharts(activeTab.getAttribute('data-period'));
        }
      } else {
        clearInterval(batteryInterval);
      }
    }, 5000);
  }

  
  function initBatteryCharts() {
    const activeTab = document.querySelector('.battery-tab.active');
    const period = activeTab ? activeTab.getAttribute('data-period') : '24h';
    updateBatteryCharts(period);
  }

  
  async function updateBatteryCharts(period) {
    if (!window.electronAPI) return;

    try {
      const chartData = await window.electronAPI.getBatteryUsageData(period);
      drawBatteryLevelChart(chartData.batteryLevel);
      drawScreenUsageChart(chartData.screenUsage);
    } catch (error) {
      console.error('Error updating battery charts:', error);
    }
  }

  
  function drawBatteryLevelChart(data) {
    const canvas = document.getElementById('battery-level-chart');
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;
    
    
    ctx.clearRect(0, 0, width, height);
    
    if (!data || data.length === 0) {
      ctx.fillStyle = '#666';
      ctx.font = '12px sans-serif';
      ctx.textAlign = 'center';
      ctx.fillText('No data available', width / 2, height / 2);
      return;
    }

    
    const rect = canvas.getBoundingClientRect();
    canvas.width = rect.width;
    canvas.height = 150;
    
    const padding = 40;
    const chartWidth = canvas.width - padding * 2;
    const chartHeight = canvas.height - padding * 2;
    
    
    ctx.strokeStyle = '#e0e0e0';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(padding, padding);
    ctx.lineTo(padding, canvas.height - padding);
    ctx.lineTo(canvas.width - padding, canvas.height - padding);
    ctx.stroke();
    
    
    ctx.strokeStyle = '#007AFF';
    ctx.lineWidth = 2;
    ctx.beginPath();
    
    const stepX = chartWidth / (data.length - 1 || 1);
    const maxValue = 100; 
    
    data.forEach((point, index) => {
      const x = padding + (index * stepX);
      const y = canvas.height - padding - ((point.value / maxValue) * chartHeight);
      
      if (index === 0) {
        ctx.moveTo(x, y);
      } else {
        ctx.lineTo(x, y);
      }
    });
    
    ctx.stroke();
    
    
    ctx.fillStyle = '#007AFF';
    data.forEach((point, index) => {
      const x = padding + (index * stepX);
      const y = canvas.height - padding - ((point.value / maxValue) * chartHeight);
      ctx.beginPath();
      ctx.arc(x, y, 3, 0, 2 * Math.PI);
      ctx.fill();
    });
    
    
    ctx.fillStyle = '#666';
    ctx.font = '10px sans-serif';
    ctx.textAlign = 'right';
    for (let i = 0; i <= 4; i++) {
      const value = (4 - i) * 25;
      const y = padding + (i * chartHeight / 4);
      ctx.fillText(value + '%', padding - 10, y + 4);
    }
  }

  
  function drawScreenUsageChart(data) {
    const canvas = document.getElementById('screen-usage-chart');
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    
    
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    if (!data || data.length === 0) {
      ctx.fillStyle = '#666';
      ctx.font = '12px sans-serif';
      ctx.textAlign = 'center';
      ctx.fillText('No data available', canvas.width / 2, canvas.height / 2);
      return;
    }

    
    const rect = canvas.getBoundingClientRect();
    canvas.width = rect.width;
    canvas.height = 150;
    
    const padding = 40;
    const chartWidth = canvas.width - padding * 2;
    const chartHeight = canvas.height - padding * 2;
    
    
    ctx.strokeStyle = '#e0e0e0';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(padding, padding);
    ctx.lineTo(padding, canvas.height - padding);
    ctx.lineTo(canvas.width - padding, canvas.height - padding);
    ctx.stroke();
    
    
    const barWidth = chartWidth / data.length;
    const maxValue = Math.max(...data.map(d => d.value), 1);
    
    data.forEach((point, index) => {
      const x = padding + (index * barWidth);
      const barHeight = (point.value / maxValue) * chartHeight;
      const y = canvas.height - padding - barHeight;
      
      
      ctx.fillStyle = '#007AFF';
      ctx.fillRect(x + 2, y, barWidth - 4, barHeight);
    });
    
    
    ctx.fillStyle = '#666';
    ctx.font = '10px sans-serif';
    ctx.textAlign = 'right';
    const maxHours = Math.ceil(maxValue / 60);
    for (let i = 0; i <= 4; i++) {
      const value = (4 - i) * (maxHours / 4);
      const y = padding + (i * chartHeight / 4);
      ctx.fillText(Math.round(value) + 'h', padding - 10, y + 4);
    }
  }

  
  async function updateLowPowerMode() {
    const lowPowerModeToggle = document.getElementById('low-power-mode-toggle');
    if (!lowPowerModeToggle || !window.electronAPI) return;

    try {
      const lowPowerModeStatus = await window.electronAPI.getLowPowerMode();
      lowPowerModeToggle.checked = lowPowerModeStatus.enabled;
    } catch (error) {
      console.error('Error getting low power mode status:', error);
    }
  }

  
  async function updateBatteryHealth() {
    const batteryHealthDiv = document.getElementById('battery-health-info');
    if (!batteryHealthDiv || !window.electronAPI) return;

    try {
      const batteryHealth = await window.electronAPI.getBatteryHealth();
      
      if (batteryHealth) {
        let healthStatus = batteryHealth.health || 'Unknown';
        let healthColor = '#666';
        
        
        if (healthStatus.toLowerCase() === 'good') {
          healthColor = '#22C55E';
        } else if (healthStatus.toLowerCase() === 'fair') {
          healthColor = '#EAB308';
        } else if (healthStatus.toLowerCase() === 'poor' || healthStatus.toLowerCase() === 'bad') {
          healthColor = '#EF4444';
        }
        
        batteryHealthDiv.innerHTML = `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Health Status</span>
              <span class="battery-value" style="color: ${healthColor};">${healthStatus}</span>
            </div>
          </div>
          ${batteryHealth.cycleCount !== null && batteryHealth.cycleCount !== undefined ? `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Cycle Count</span>
              <span class="battery-value">${batteryHealth.cycleCount}</span>
            </div>
          </div>
          ` : ''}
          ${batteryHealth.designCapacity !== null && batteryHealth.designCapacity !== undefined ? `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Design Capacity</span>
              <span class="battery-value">${batteryHealth.designCapacity} mAh</span>
            </div>
          </div>
          ` : ''}
          ${batteryHealth.fullChargeCapacity !== null && batteryHealth.fullChargeCapacity !== undefined ? `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Full Charge Capacity</span>
              <span class="battery-value">${batteryHealth.fullChargeCapacity} mAh</span>
            </div>
          </div>
          ` : ''}
        `;
      } else {
        batteryHealthDiv.innerHTML = '<p class="no-network-text">No battery health information available</p>';
      }
    } catch (error) {
      console.error('Error updating battery health:', error);
      batteryHealthDiv.innerHTML = '<p class="no-network-text">Error loading battery health</p>';
    }
  }

  
  async function updateBatteryInfo() {
    const batteryInfoDiv = document.getElementById('battery-info');
    if (!batteryInfoDiv || !window.electronAPI) return;

    try {
      const batteryInfo = await window.electronAPI.getBatteryInfo();
      
      if (batteryInfo) {
        batteryInfoDiv.innerHTML = `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Status</span>
              <span class="battery-value">${batteryInfo.state || 'Unknown'}</span>
            </div>
          </div>
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Percentage</span>
              <span class="battery-value">${batteryInfo.percentage}%</span>
            </div>
          </div>
          ${batteryInfo.timeToEmpty ? `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Time to Empty</span>
              <span class="battery-value">${batteryInfo.timeToEmpty}</span>
            </div>
          </div>
          ` : ''}
          ${batteryInfo.timeToFull ? `
          <div class="card-section">
            <div class="battery-info-item">
              <span class="battery-label">Time to Full</span>
              <span class="battery-value">${batteryInfo.timeToFull}</span>
            </div>
          </div>
          ` : ''}
        `;
      } else {
        batteryInfoDiv.innerHTML = '<p class="no-network-text">No battery information available</p>';
      }
    } catch (error) {
      console.error('Error updating battery info:', error);
      batteryInfoDiv.innerHTML = '<p class="no-network-text">Error loading battery information</p>';
    }
  }

  
  function initBluetoothPage() {
    const bluetoothToggle = document.getElementById('bluetooth-toggle');
    const bluetoothDeviceName = document.getElementById('bluetooth-device-name');

    if (!bluetoothToggle || !window.electronAPI) return;

    
    async function updateBluetoothStatus() {
      try {
        const status = await window.electronAPI.bluetoothGetStatus();
        bluetoothToggle.checked = status.enabled;
      } catch (error) {
        console.error('Error updating Bluetooth status:', error);
      }
    }

    
    async function updateBluetoothDeviceName() {
      if (!bluetoothDeviceName) return;
      try {
        const deviceName = await window.electronAPI.bluetoothGetDeviceName();
        if (deviceName && deviceName.name) {
          bluetoothDeviceName.textContent = deviceName.name;
        }
      } catch (error) {
        console.error('Error updating Bluetooth device name:', error);
        bluetoothDeviceName.textContent = 'Computer';
      }
    }

    
    async function updateBluetoothDevices() {
      const devicesList = document.getElementById('bluetooth-devices-list');
      if (!devicesList || !window.electronAPI) return;

      try {
        const status = await window.electronAPI.bluetoothGetStatus();
        if (!status.enabled) {
          devicesList.innerHTML = '<p class="no-network-text">Bluetooth is turned off</p>';
          return;
        }

        console.log('Fetching Bluetooth devices...');
        const devicesData = await window.electronAPI.bluetoothGetDevices();
        console.log('Received devices data:', devicesData);
        
        if (devicesData.devices && devicesData.devices.length > 0) {
          devicesList.innerHTML = devicesData.devices.map(device => `
            <div class="network-item-clickable" data-mac="${device.macAddress}">
              <div class="device-info">
                <span class="device-status ${device.connected ? 'connected' : 'disconnected'}">${device.connected ? '●' : '○'}</span>
                <span class="network-ssid">${device.name || 'Unknown Device'}</span>
              </div>
              <div class="network-actions">
                <button class="bluetooth-connect-button ${device.connected ? 'disconnect' : 'connect'}" data-mac="${device.macAddress}" data-connected="${device.connected}">
                  ${device.connected ? 'Disconnect' : 'Connect'}
                </button>
                <span class="device-type">${device.type || 'Device'}</span>
              </div>
            </div>
          `).join('');
          
          
          devicesList.querySelectorAll('.bluetooth-connect-button').forEach(button => {
            button.addEventListener('click', async (e) => {
              e.stopPropagation();
              const macAddress = button.getAttribute('data-mac');
              const isConnected = button.getAttribute('data-connected') === 'true';
              
              try {
                if (isConnected) {
                  await window.electronAPI.bluetoothDisconnect(macAddress);
                } else {
                  await window.electronAPI.bluetoothConnect(macAddress);
                }
                
                updateBluetoothDevices();
                updateNearbyBluetoothDevices();
              } catch (error) {
                console.error('Error connecting/disconnecting device:', error);
              }
            });
          });
        } else {
          console.log('No devices found in response');
          devicesList.innerHTML = '<p class="no-network-text">No devices found</p>';
        }
      } catch (error) {
        console.error('Error updating Bluetooth devices:', error);
        devicesList.innerHTML = '<p class="no-network-text">Error scanning for devices</p>';
      }
    }

    
    async function updateNearbyBluetoothDevices() {
      const nearbyDevicesList = document.getElementById('bluetooth-nearby-devices-list');
      const spinner = document.getElementById('bluetooth-scanning-spinner');
      if (!nearbyDevicesList || !window.electronAPI) return;

      try {
        const status = await window.electronAPI.bluetoothGetStatus();
        if (!status.enabled) {
          if (spinner) spinner.classList.add('hidden');
          nearbyDevicesList.innerHTML = '<p class="no-network-text">Bluetooth is turned off</p>';
          return;
        }

        
        let myDevicesMacs = new Set();
        try {
          const myDevicesData = await window.electronAPI.bluetoothGetDevices();
          if (myDevicesData.devices && myDevicesData.devices.length > 0) {
            myDevicesMacs = new Set(myDevicesData.devices.map(device => device.macAddress.toUpperCase()));
          }
        } catch (error) {
          console.error('Error getting my devices for filtering:', error);
        }

        
        if (spinner) spinner.classList.remove('hidden');

        console.log('Scanning for nearby Bluetooth devices...');
        const devicesData = await window.electronAPI.bluetoothScanDevices();
        console.log('Received nearby devices data:', devicesData);
        
        
        if (spinner) spinner.classList.add('hidden');
        
        
        const filteredDevices = devicesData.devices && devicesData.devices.length > 0
          ? devicesData.devices.filter(device => !myDevicesMacs.has(device.macAddress.toUpperCase()))
          : [];
        
        
        if (filteredDevices.length > 0) {
          nearbyDevicesList.innerHTML = filteredDevices.map(device => `
            <div class="network-item-clickable" data-mac="${device.macAddress}">
              <div class="device-info">
                <span class="device-status ${device.connected ? 'connected' : 'disconnected'}">${device.connected ? '●' : '○'}</span>
                <span class="network-ssid">${device.name || 'Unknown Device'}</span>
              </div>
              <div class="network-actions">
                <button class="bluetooth-connect-button ${device.connected ? 'disconnect' : 'connect'}" data-mac="${device.macAddress}" data-connected="${device.connected}">
                  ${device.connected ? 'Disconnect' : 'Connect'}
                </button>
                <span class="device-type">${device.type || 'Device'}</span>
              </div>
            </div>
          `).join('');
          
          
          nearbyDevicesList.querySelectorAll('.bluetooth-connect-button').forEach(button => {
            button.addEventListener('click', async (e) => {
              e.stopPropagation();
              const macAddress = button.getAttribute('data-mac');
              const isConnected = button.getAttribute('data-connected') === 'true';
              
              try {
                if (isConnected) {
                  await window.electronAPI.bluetoothDisconnect(macAddress);
                } else {
                  await window.electronAPI.bluetoothConnect(macAddress);
                }
                
                updateBluetoothDevices();
                updateNearbyBluetoothDevices();
              } catch (error) {
                console.error('Error connecting/disconnecting device:', error);
              }
            });
          });
        } else {
          nearbyDevicesList.innerHTML = '<p class="no-network-text">No nearby devices found</p>';
        }
      } catch (error) {
        console.error('Error updating nearby Bluetooth devices:', error);
        if (spinner) spinner.classList.add('hidden');
        nearbyDevicesList.innerHTML = '<p class="no-network-text">Error scanning for devices</p>';
      }
    }

    
    updateBluetoothStatus();
    updateBluetoothDeviceName();
    updateBluetoothDevices();
    updateNearbyBluetoothDevices();

    
    const statusInterval = setInterval(() => {
      updateBluetoothStatus();
      updateBluetoothDeviceName();
      updateBluetoothDevices();
    }, 2000);

    
    const nearbyInterval = setInterval(() => {
      updateNearbyBluetoothDevices();
    }, 5000);

    
    bluetoothToggle.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.bluetoothToggle(e.target.checked);
        
        setTimeout(updateBluetoothStatus, 500);
      } catch (error) {
        console.error('Error toggling Bluetooth:', error);
        
        e.target.checked = !e.target.checked;
      }
    });

    
    window.addEventListener('beforeunload', () => {
      clearInterval(statusInterval);
    });
  }

  
  async function initAboutPage() {
    if (!window.electronAPI) return;

    try {
      const systemInfo = await window.electronAPI.getSystemInfo();
      
      document.getElementById('about-name').textContent = systemInfo.name || 'Unknown';
      document.getElementById('about-chip').textContent = systemInfo.chip || 'Unknown';
      document.getElementById('about-memory').textContent = systemInfo.memory || 'Unknown';
      document.getElementById('about-serial').textContent = systemInfo.serial || 'Unknown';
      document.getElementById('about-coverage').textContent = systemInfo.coverage || 'Unknown';
      document.getElementById('about-parts').textContent = systemInfo.parts || 'Unknown';

      
      let hasBattery = false;
      try {
        const batteryInfo = await window.electronAPI.getBatteryInfo();
        hasBattery = batteryInfo && batteryInfo.percentage !== undefined;
      } catch (batteryError) {
        
        hasBattery = false;
      }

      
      const deviceImage = document.getElementById('about-device-image');
      const deviceType = document.getElementById('about-device-type');
      
      if (hasBattery) {
        deviceImage.src = './assets/laptop.svg';
        deviceType.textContent = 'Laptop';
      } else {
        deviceImage.src = './assets/computer.svg';
        deviceType.textContent = 'Pearintosh';
      }

      
      document.getElementById('os-name').textContent = systemInfo.osName || 'Unknown';
      const osVersionSpan = document.getElementById('os-version');
      if (osVersionSpan) {
        osVersionSpan.textContent = systemInfo.osVersion || 'Unknown';
      }
      
      
      const osLogo = document.getElementById('os-logo');
      if (osLogo) {
        osLogo.src = 'file:///usr/share/extras/release_logo.png';
      }

      
      await initDisplays(hasBattery);
      
      
      await initStorage();
      
      
      const currentYearElement = document.getElementById('current-year');
      if (currentYearElement) {
        currentYearElement.textContent = new Date().getFullYear();
      }
    } catch (error) {
      console.error('Error loading system info:', error);
      document.getElementById('about-name').textContent = 'Error loading info';
      document.getElementById('about-chip').textContent = 'Error loading info';
      document.getElementById('about-memory').textContent = 'Error loading info';
      document.getElementById('about-serial').textContent = 'Error loading info';
      document.getElementById('about-coverage').textContent = 'Error loading info';
      document.getElementById('about-parts').textContent = 'Error loading info';
      
      
      const deviceImage = document.getElementById('about-device-image');
      const deviceType = document.getElementById('about-device-type');
      if (deviceImage) deviceImage.src = './assets/computer.svg';
      if (deviceType) deviceType.textContent = 'Unknown';
    }
  }

  
  async function initDisplays(hasBattery) {
    if (!window.electronAPI) return;

    try {
      const displays = await window.electronAPI.getDisplays();
      const displaysList = document.getElementById('displays-list');
      if (!displaysList || !displays || displays.length === 0) return;

      
      const existingItems = displaysList.querySelectorAll('.display-item');
      existingItems.forEach(item => item.remove());

      
      const connectedDisplays = displays.filter(display => 
        display.isConnected || display.isEnabled || display.isBuiltIn
      );
      
      if (connectedDisplays.length === 0) {
        displaysList.innerHTML = '<p class="no-network-text">No displays connected</p>';
        return;
      }
      
      connectedDisplays.forEach((display, index) => {
        const displayItem = document.createElement('div');
        displayItem.className = 'display-item';
        
        const displayNameText = hasBattery && display.isBuiltIn ? 'Built-in Display' : display.name || 'Display';
        let displaySpecsText = 'Unknown';
        
        if (display.inches && display.inches !== 'Unknown' && display.resolution && display.resolution !== 'Unknown') {
          
          const inches = display.inches.replace('"', '');
          const [width, height] = display.resolution.split('x');
          displaySpecsText = `${inches}-inch (${width} x ${height})`;
        }
        
        displayItem.innerHTML = `
          <div class="display-info">
            <img src="${hasBattery ? './assets/laptop.svg' : './assets/computer.svg'}" alt="Display" class="display-icon">
            <div class="display-details">
              <div class="display-name-row">
                <div class="display-name">${displayNameText}</div>
                <div class="display-specs">${displaySpecsText}</div>
              </div>
            </div>
          </div>
        `;
        
        displaysList.insertBefore(displayItem, displaysList.querySelector('.display-settings-button-container'));
      });
    } catch (error) {
      console.error('Error loading displays:', error);
      const displayName = document.getElementById('display-name');
      const displaySpecs = document.getElementById('display-specs');
      if (displayName) displayName.textContent = 'Error loading display';
      if (displaySpecs) displaySpecs.textContent = 'Unknown';
    }
  }

  
  async function initStorage() {
    if (!window.electronAPI) return;

    try {
      const storages = await window.electronAPI.getStorage();
      const storageList = document.getElementById('storage-list');
      if (!storageList || !storages || storages.length === 0) return;

      
      const existingItems = storageList.querySelectorAll('.storage-item');
      existingItems.forEach(item => item.remove());

      
      storages.forEach((storage) => {
        const storageItem = document.createElement('div');
        storageItem.className = 'storage-item';
        
        const storageNameText = storage.name || 'Storage';
        const storageSpaceText = storage.available && storage.total && 
          storage.available !== 'Unknown' && storage.total !== 'Unknown' 
          ? `${storage.available} available of ${storage.total}`
          : 'Unknown';
        
        storageItem.innerHTML = `
          <div class="storage-info">
            <img src="./assets/preferences-system-disks.svg" alt="Storage" class="storage-icon">
            <div class="storage-details">
              <div class="storage-name-row">
                <div class="storage-name">${storageNameText}</div>
                <div class="storage-space">${storageSpaceText}</div>
              </div>
            </div>
          </div>
        `;
        
        storageList.insertBefore(storageItem, storageList.querySelector('.storage-settings-button-container'));
      });
    } catch (error) {
      console.error('Error loading storage:', error);
      const storageName = document.getElementById('storage-name');
      const storageSpace = document.getElementById('storage-space');
      if (storageName) storageName.textContent = 'Error loading storage';
      if (storageSpace) storageSpace.textContent = 'Unknown';
    }
  }

  
  async function initSoftwareUpdatePage() {
    if (!window.electronAPI) return;

    try {
      const updates = await window.electronAPI.getAvailableUpdates();
      const updatesContainer = document.getElementById('updates-container');
      
      if (!updatesContainer) return;

      // Actualizează notificarea din sidebar când se accesează pagina
      await updateSoftwareUpdateNotification();

      if (!updates || updates.length === 0) {
        updatesContainer.innerHTML = '<div class="no-updates-message">Your system is up to date.</div>';
        
        const updateActions = document.querySelector('.update-actions');
        if (updateActions) {
          updateActions.style.display = 'none';
        }
        
        await loadInstalledSystemInfo();
        return;
      }

      
      updatesContainer.innerHTML = updates.map(update => `
        <div class="update-card">
          <div class="update-info">
            <div class="update-name">${update.name}</div>
            <div class="update-version">${update.oldVersion} → ${update.newVersion}</div>
          </div>
        </div>
      `).join('');

      
      const updateActions = document.querySelector('.update-actions');
      if (updateActions) {
        updateActions.style.display = 'flex';
      }

      
      const updateTonightButton = document.querySelector('.update-tonight-button');
      const updateNowButton = document.querySelector('.update-now-button');

      if (updateTonightButton) {
        updateTonightButton.addEventListener('click', async () => {
          
          const confirmed = confirm('This will schedule updates for 1:00 AM. Continue?');
          if (confirmed) {
            await requestPasswordAndScheduleUpdate();
          }
        });
      }

      if (updateNowButton) {
        updateNowButton.addEventListener('click', async () => {
          await requestPasswordAndUpdate();
        });
      }

      
      await loadInstalledSystemInfo();
    } catch (error) {
      console.error('Error loading updates:', error);
      const updatesContainer = document.getElementById('updates-container');
      if (updatesContainer) {
        updatesContainer.innerHTML = '<div class="updates-error">Error loading updates</div>';
      }
    }
  }

  
  async function loadInstalledSystemInfo() {
    if (!window.electronAPI) return;

    try {
      const systemInfo = await window.electronAPI.getInstalledSystemInfo();
      const installedSystemInfo = document.getElementById('installed-system-info');
      
      if (installedSystemInfo && systemInfo) {
        installedSystemInfo.textContent = `${systemInfo.prettyName} ${systemInfo.release}`;
      }
    } catch (error) {
      console.error('Error loading installed system info:', error);
      const installedSystemInfo = document.getElementById('installed-system-info');
      if (installedSystemInfo) {
        installedSystemInfo.textContent = 'Unknown';
      }
    }
  }

  
  async function requestPasswordAndScheduleUpdate() {
    const password = await requestPassword();
    if (password) {
      try {
        await window.electronAPI.scheduleUpdate(password);
        alert('Updates scheduled for 1:00 AM');
        
        // Actualizează notificarea din sidebar
        await updateSoftwareUpdateNotification();
      } catch (error) {
        console.error('Error scheduling update:', error);
        alert('Error scheduling update: ' + error.message);
        
        // Actualizează notificarea chiar și în caz de eroare
        await updateSoftwareUpdateNotification();
      }
    }
  }

  
  async function requestPasswordAndUpdate() {
    const password = await requestPassword();
    if (password) {
      try {
        const updatesContainer = document.getElementById('updates-container');
        if (updatesContainer) {
          updatesContainer.innerHTML = '<div class="updates-loading">Updating system...</div>';
        }
        
        await window.electronAPI.updateSystem(password);
        alert('System updated successfully');
        
        // Actualizează notificarea din sidebar după actualizare
        await updateSoftwareUpdateNotification();
        
        await initSoftwareUpdatePage();
      } catch (error) {
        console.error('Error updating system:', error);
        alert('Error updating system: ' + error.message);
        
        // Actualizează notificarea chiar și în caz de eroare
        await updateSoftwareUpdateNotification();
        
        await initSoftwareUpdatePage();
      }
    }
  }

  
  function requestPassword(forSignOut = false, forSudo = false) {
    return new Promise((resolve) => {
      const modal = document.getElementById('password-modal');
      const passwordInput = document.getElementById('password-input');
      const confirmButton = document.getElementById('password-modal-confirm');
      const cancelButton = document.getElementById('password-modal-cancel');
      const modalTitle = document.querySelector('#password-modal .modal-title');
      const modalDescription = document.querySelector('#password-modal .modal-description');

      if (!modal || !passwordInput || !confirmButton || !cancelButton) {
        resolve(null);
        return;
      }

      // Actualizează textul modalului în funcție de context
      if (forSignOut) {
        if (modalTitle) {
          modalTitle.textContent = 'Enter Password';
        }
        if (modalDescription) {
          modalDescription.textContent = 'Enter your Pear Account password to sign out.';
        }
      } else if (forSudo) {
        if (modalTitle) {
          modalTitle.textContent = 'Enter Password';
        }
        if (modalDescription) {
          modalDescription.textContent = 'Administrator password required to change password.';
        }
      } else {
        if (modalTitle) {
          modalTitle.textContent = 'Enter Password';
        }
        if (modalDescription) {
          modalDescription.textContent = 'Administrator password required to update software.';
        }
      }

      // Schimbă culoarea butonului roșu când se deschide modalul
      const redDot = document.querySelector('.traffic-lights .dot.red');
      const originalColor = '#fe5b51'; // Culoarea originală din CSS
      
      if (redDot) {
        // Setează culoarea gri
        redDot.style.backgroundColor = '#6f6f6f';
      }

      const restoreRedDot = () => {
        if (redDot) {
          // Restaurează culoarea originală - folosim valoarea hex direct
          redDot.style.backgroundColor = originalColor;
          console.log('Restored red dot color to:', originalColor);
        }
      };

      
      passwordInput.value = '';
      modal.style.display = 'flex';
      passwordInput.focus();

      const confirmHandler = () => {
        const password = passwordInput.value;
        modal.style.display = 'none';
        passwordInput.value = '';
        restoreRedDot();
        confirmButton.removeEventListener('click', confirmHandler);
        cancelButton.removeEventListener('click', cancelHandler);
        passwordInput.removeEventListener('keypress', enterHandler);
        modal.removeEventListener('click', modalClickHandler);
        resolve(password);
      };

      const cancelHandler = () => {
        modal.style.display = 'none';
        passwordInput.value = '';
        restoreRedDot();
        confirmButton.removeEventListener('click', confirmHandler);
        cancelButton.removeEventListener('click', cancelHandler);
        passwordInput.removeEventListener('keypress', enterHandler);
        modal.removeEventListener('click', modalClickHandler);
        resolve(null);
      };

      const enterHandler = (e) => {
        if (e.key === 'Enter') {
          confirmHandler();
        }
      };

      const modalClickHandler = (e) => {
        if (e.target === modal) {
          cancelHandler();
        }
      };

      confirmButton.addEventListener('click', confirmHandler);
      cancelButton.addEventListener('click', cancelHandler);
      passwordInput.addEventListener('keypress', enterHandler);
      modal.addEventListener('click', modalClickHandler);
    });
  }

  
  async function initStoragePage() {
    if (!window.electronAPI) return;

    try {
      
      const storageInfo = await window.electronAPI.getStorageInfo();
      const diskName = document.getElementById('storage-disk-name');
      const usageText = document.getElementById('storage-usage-text');
      
      if (storageInfo && diskName && usageText) {
        diskName.textContent = storageInfo.name || storageInfo.device || 'Unknown';
        usageText.textContent = `${storageInfo.used} GB of ${storageInfo.total} GB used`;
      }

      
      const storageAnalysis = await window.electronAPI.getStorageAnalysis();
      const spinner = document.querySelector('.storage-loading-spinner');
      const storageBarWrapper = document.getElementById('storage-bar-wrapper');
      const storageLegend = document.getElementById('storage-legend');
      
      if (spinner) {
        spinner.style.display = 'none';
      }

      if (storageAnalysis) {
        if (!storageBarWrapper) {
          console.error('storageBarWrapper not found!');
          return;
        }
        if (!storageLegend) {
          console.error('storageLegend not found!');
          return;
        }
        
        const storageInfo = await window.electronAPI.getStorageInfo();
        const totalStorage = storageInfo ? (parseFloat(storageInfo.total) || 1) : (storageAnalysis.total || 1);
        const usedStorage = storageInfo ? (parseFloat(storageInfo.used) || 0) : 0;
        
        
        const knownCategories = (storageAnalysis.applications || 0) + (storageAnalysis.documents || 0) + (storageAnalysis.photos || 0) + (storageAnalysis.system || 0);
        
        
        
        
        
        
        const applicationsPercent = totalStorage > 0 ? ((storageAnalysis.applications || 0) / totalStorage) * 100 : 0;
        const documentsPercent = totalStorage > 0 ? ((storageAnalysis.documents || 0) / totalStorage) * 100 : 0;
        const photosPercent = totalStorage > 0 ? ((storageAnalysis.photos || 0) / totalStorage) * 100 : 0;
        const downloadsPercent = totalStorage > 0 ? ((storageAnalysis.downloads || 0) / totalStorage) * 100 : 0;
        const desktopPercent = totalStorage > 0 ? ((storageAnalysis.desktop || 0) / totalStorage) * 100 : 0;
        
        
        let systemDataGB = storageAnalysis.system || 0;
        
        if (systemDataGB > usedStorage * 0.8) {
          
          systemDataGB = Math.min(systemDataGB, usedStorage * 0.3);
        }
        const systemPercent = totalStorage > 0 ? (systemDataGB / totalStorage) * 100 : 0;

        console.log('Storage analysis:', storageAnalysis);
        console.log('Total storage:', totalStorage);
        console.log('Percentages:', { applicationsPercent, documentsPercent, photosPercent, downloadsPercent, desktopPercent, systemPercent });

        
        const legendItemsForTooltips = {
          applications: { gb: storageAnalysis.applications || 0 },
          documents: { gb: storageAnalysis.documents || 0 },
          photos: { gb: storageAnalysis.photos || 0 },
          downloads: { gb: storageAnalysis.downloads || 0 },
          desktop: { gb: storageAnalysis.desktop || 0 }
        };

        
        const categories = [
          { name: 'applications', percent: applicationsPercent, element: document.getElementById('storage-segment-applications') },
          { name: 'documents', percent: documentsPercent, element: document.getElementById('storage-segment-documents') },
          { name: 'photos', percent: photosPercent, element: document.getElementById('storage-segment-photos') },
          { name: 'downloads', percent: downloadsPercent, element: document.getElementById('storage-segment-downloads') },
          { name: 'desktop', percent: desktopPercent, element: document.getElementById('storage-segment-desktop') }
        ];

        
        categories.sort((a, b) => b.percent - a.percent);

        
        let currentLeft = 0;
        categories.forEach(category => {
          if (category.element && category.percent > 0) {
            category.element.style.left = `${currentLeft}%`;
            category.element.style.width = `${Math.max(category.percent, 0.1)}%`;
            console.log(`${category.name} segment:`, { left: `${currentLeft}%`, width: `${category.percent}%` });
            
            
            const gbValue = legendItemsForTooltips[category.name]?.gb || 0;
            const displayName = category.name.charAt(0).toUpperCase() + category.name.slice(1);
            setupSegmentTooltip(category.element, displayName, gbValue, currentLeft, category.percent);
            
            currentLeft += category.percent;
          } else if (category.element) {
            category.element.style.width = '0%';
            category.element.style.left = '0%';
          }
        });

        
        const segmentSystem = document.getElementById('storage-segment-system');
        if (segmentSystem && systemPercent > 0) {
          segmentSystem.style.left = `${currentLeft}%`;
          segmentSystem.style.width = `${Math.max(systemPercent, 0.1)}%`;
          console.log('System segment:', { left: `${currentLeft}%`, width: `${systemPercent}%`, systemDataGB });
          
          
          setupSegmentTooltip(segmentSystem, 'System Data', systemDataGB, currentLeft, systemPercent);
          
          currentLeft += systemPercent;
        } else if (segmentSystem) {
          segmentSystem.style.width = '0%';
          segmentSystem.style.left = '0%';
        }
        
        
        const storageBarBase = document.getElementById('storage-bar-base');
        const availableGB = totalStorage - usedStorage;
        const usedPercentTotal = (usedStorage / totalStorage) * 100;
        if (storageBarBase && availableGB > 0) {
          setupAvailableTooltip(storageBarBase, availableGB, usedPercentTotal, totalStorage);
        }

        
        const legendItems = {
          applications: { gb: storageAnalysis.applications || 0, element: document.getElementById('legend-applications'), parent: document.querySelector('.legend-item:has(#legend-applications)') },
          documents: { gb: storageAnalysis.documents || 0, element: document.getElementById('legend-documents'), parent: document.querySelector('.legend-item:has(#legend-documents)') },
          photos: { gb: storageAnalysis.photos || 0, element: document.getElementById('legend-photos'), parent: document.querySelector('.legend-item:has(#legend-photos)') },
          downloads: { gb: storageAnalysis.downloads || 0, element: document.getElementById('legend-downloads'), parent: document.querySelector('.legend-item:has(#legend-downloads)') },
          desktop: { gb: storageAnalysis.desktop || 0, element: document.getElementById('legend-desktop'), parent: document.querySelector('.legend-item:has(#legend-desktop)') }
        };

        
        if (storageLegend) {
          
          categories.forEach(category => {
            const item = legendItems[category.name];
            if (item && item.parent) {
              item.parent.remove();
            }
          });

          
          categories.forEach(category => {
            const item = legendItems[category.name];
            if (item && item.parent && category.percent > 0 && item.gb > 0) {
              storageLegend.appendChild(item.parent);
              if (item.element) {
                item.element.textContent = `${item.gb.toFixed(2)} GB`;
              }
            } else if (item && item.parent) {
              
              item.parent.style.display = 'none';
            }
          });

          
          const systemLegendItem = document.querySelector('.legend-item:has(#legend-system)');
          if (systemLegendItem) {
            if (systemDataGB > 0) {
              storageLegend.appendChild(systemLegendItem);
              systemLegendItem.style.display = 'flex';
              const legendSystem = document.getElementById('legend-system');
              if (legendSystem) {
                legendSystem.textContent = `${systemDataGB.toFixed(2)} GB`;
              }
            } else {
              systemLegendItem.style.display = 'none';
            }
          }
        } else {
          
          if (legendItems.applications.element) legendItems.applications.element.textContent = `${legendItems.applications.gb.toFixed(2)} GB`;
          if (legendItems.documents.element) legendItems.documents.element.textContent = `${legendItems.documents.gb.toFixed(2)} GB`;
          if (legendItems.photos.element) legendItems.photos.element.textContent = `${legendItems.photos.gb.toFixed(2)} GB`;
          if (legendItems.downloads.element) legendItems.downloads.element.textContent = `${legendItems.downloads.gb.toFixed(2)} GB`;
          if (legendItems.desktop.element) legendItems.desktop.element.textContent = `${legendItems.desktop.gb.toFixed(2)} GB`;
          const legendSystem = document.getElementById('legend-system');
          if (legendSystem) {
            legendSystem.textContent = `${systemDataGB.toFixed(2)} GB`;
          }
        }

        
        if (storageBarWrapper) {
          storageBarWrapper.style.display = 'block';
          console.log('Storage bar wrapper displayed');
        } else {
          console.error('Storage bar wrapper not found!');
        }
        if (storageLegend) {
          storageLegend.style.display = 'flex';
        }

        
        const formatSize = (value, isBytes = false) => {
          let bytes = value;
          if (!isBytes) {
            
            bytes = value * 1024 * 1024 * 1024;
          }
          
          if (bytes === 0) {
            return '0 MB';
          }
          
          
          const gb = bytes / (1024 * 1024 * 1024);
          const mb = bytes / (1024 * 1024);
          const kb = bytes / 1024;
          
          if (gb >= 1) {
            return `${gb.toFixed(2)} GB`;
          } else if (mb >= 1) {
            return `${mb.toFixed(2)} MB`;
          } else if (kb >= 1) {
            return `${kb.toFixed(2)} KB`;
          } else {
            return `${bytes} B`;
          }
        };

        const applicationsSizeEl = document.getElementById('storage-category-applications-size');
        const documentsSizeEl = document.getElementById('storage-category-documents-size');
        const photosSizeEl = document.getElementById('storage-category-photos-size');
        const downloadsSizeEl = document.getElementById('storage-category-downloads-size');
        const desktopSizeEl = document.getElementById('storage-category-desktop-size');

        if (applicationsSizeEl) {
          applicationsSizeEl.textContent = formatSize(storageAnalysis.applications || 0, false);
        }
        if (documentsSizeEl) {
          documentsSizeEl.textContent = formatSize(storageAnalysis.documents || 0, false);
        }
        if (photosSizeEl) {
          photosSizeEl.textContent = formatSize(storageAnalysis.photos || 0, false);
        }
        if (downloadsSizeEl) {
          downloadsSizeEl.textContent = formatSize(storageAnalysis.downloads || 0, false);
        }
        if (desktopSizeEl) {
          desktopSizeEl.textContent = formatSize(storageAnalysis.desktop || 0, false);
        }

        
        const pearosItem = document.getElementById('storage-pearos-item');
        const pearosSizeEl = document.getElementById('storage-pearos-size');
        const systemDataSizeEl = document.getElementById('storage-system-data-size');

        
        
        if (storageAnalysis.pearos && storageAnalysis.pearos > 0) {
          if (pearosItem) {
            pearosItem.style.display = 'flex';
          }
          if (pearosSizeEl) {
            pearosSizeEl.textContent = formatSize(storageAnalysis.pearos, true);
          }
        } else {
          if (pearosItem) {
            pearosItem.style.display = 'none';
          }
        }

        
        if (systemDataSizeEl) {
          systemDataSizeEl.textContent = formatSize(systemDataGB, false);
        }
      }
    } catch (error) {
      console.error('Error loading storage:', error);
      const spinner = document.querySelector('.storage-loading-spinner');
      if (spinner) {
        spinner.innerHTML = '<span>Error loading storage information</span>';
      }
    }
  }

  
  function setupSegmentTooltip(element, name, gb, leftPercent, widthPercent) {
    if (!element) return;
    
    const tooltip = document.getElementById('storage-tooltip');
    const tooltipTitle = document.getElementById('tooltip-title');
    const tooltipSize = document.getElementById('tooltip-size');
    const storageBarWrapper = document.getElementById('storage-bar-wrapper');
    
    if (!tooltip || !tooltipTitle || !tooltipSize || !storageBarWrapper) return;
    
    element.addEventListener('mouseenter', (e) => {
      const rect = storageBarWrapper.getBoundingClientRect();
      const segmentLeft = (rect.width * leftPercent) / 100;
      const segmentWidth = (rect.width * widthPercent) / 100;
      const tooltipLeft = segmentLeft + (segmentWidth / 2);
      
      tooltipTitle.textContent = name;
      tooltipSize.textContent = `${gb.toFixed(2)} GB`;
      tooltip.style.left = `${tooltipLeft}px`;
      tooltip.style.display = 'block';
    });
    
    element.addEventListener('mouseleave', () => {
      tooltip.style.display = 'none';
    });
    
    element.addEventListener('mousemove', (e) => {
      const rect = storageBarWrapper.getBoundingClientRect();
      const segmentLeft = (rect.width * leftPercent) / 100;
      const segmentWidth = (rect.width * widthPercent) / 100;
      const tooltipLeft = segmentLeft + (segmentWidth / 2);
      
      tooltip.style.left = `${tooltipLeft}px`;
    });
  }

  
  function setupAvailableTooltip(element, availableGB, usedPercent, totalStorage) {
    if (!element) return;
    
    const tooltip = document.getElementById('storage-tooltip');
    const tooltipTitle = document.getElementById('tooltip-title');
    const tooltipSize = document.getElementById('tooltip-size');
    const storageBarWrapper = document.getElementById('storage-bar-wrapper');
    
    if (!tooltip || !tooltipTitle || !tooltipSize || !storageBarWrapper) return;
    
    element.addEventListener('mousemove', (e) => {
      const rect = storageBarWrapper.getBoundingClientRect();
      const mouseX = e.clientX - rect.left;
      const usedWidth = (rect.width * usedPercent) / 100;
      
      
      if (mouseX > usedWidth) {
        const availableLeft = usedWidth;
        const availableWidth = rect.width - usedWidth;
        const tooltipLeft = availableLeft + (availableWidth / 2);
        
        tooltipTitle.textContent = 'Available';
        tooltipSize.textContent = `${availableGB.toFixed(2)} GB`;
        tooltip.style.left = `${tooltipLeft}px`;
        tooltip.style.display = 'block';
      } else {
        tooltip.style.display = 'none';
      }
    });
    
    element.addEventListener('mouseleave', () => {
      tooltip.style.display = 'none';
    });
  }

  
  function initAutofillPage() {
    const openKWalletButton = document.getElementById('open-kwallet-button');
    if (openKWalletButton && window.electronAPI) {
      openKWalletButton.addEventListener('click', async () => {
        try {
          await window.electronAPI.openKWallet();
        } catch (error) {
          console.error('Error opening KWallet:', error);
          alert('Could not open KWallet. Please make sure it is installed.');
        }
      });
    }
  }

  
  function initDateTimePage() {
    
    if (dateTimeInterval) {
      clearInterval(dateTimeInterval);
      dateTimeInterval = null;
    }
    
    const dateTimeDisplay = document.getElementById('date-time-display');
    const autoTimeDateToggle = document.getElementById('auto-time-date-toggle');
    const setTimeButton = document.getElementById('set-time-button');
    const hour24Toggle = document.getElementById('24-hour-time-toggle');
    
    if (!dateTimeDisplay) return;
    
    let is24Hour = hour24Toggle ? hour24Toggle.checked : false;
    
    
    function formatDateTime() {
      if (!dateTimeDisplay) return;
      
      const now = new Date();
      const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
      const month = months[now.getMonth()];
      const day = now.getDate();
      const year = now.getFullYear();
      
      let hours = now.getHours();
      const minutes = String(now.getMinutes()).padStart(2, '0');
      const seconds = String(now.getSeconds()).padStart(2, '0');
      
      if (is24Hour) {
        hours = String(hours).padStart(2, '0');
        dateTimeDisplay.textContent = `${month} ${day}, ${year} at ${hours}:${minutes}:${seconds}`;
      } else {
        const ampm = hours >= 12 ? 'PM' : 'AM';
        hours = hours % 12;
        hours = hours ? hours : 12; 
        hours = String(hours).padStart(2, '0');
        dateTimeDisplay.textContent = `${month} ${day}, ${year} at ${hours}:${minutes}:${seconds} ${ampm}`;
      }
    }
    
    
    formatDateTime();
    dateTimeInterval = setInterval(formatDateTime, 1000);
    
    
    if (autoTimeDateToggle && window.electronAPI) {
      
      window.electronAPI.getNtpStatus()
        .then((result) => {
          if (result && result.ntpActive !== undefined) {
            autoTimeDateToggle.checked = result.ntpActive;
            
            if (setTimeButton) {
              setTimeButton.style.display = result.ntpActive ? 'none' : 'inline-block';
            }
          }
        })
        .catch((error) => {
          console.error('Error getting NTP status:', error);
        });
      
      
      autoTimeDateToggle.addEventListener('change', async () => {
        const isChecked = autoTimeDateToggle.checked;
        
        
        if (setTimeButton) {
          setTimeButton.style.display = isChecked ? 'none' : 'inline-block';
        }
        
        
        if (window.electronAPI) {
          try {
            await window.electronAPI.setNtpStatus(isChecked);
          } catch (error) {
            console.error('Error setting NTP status:', error);
            
            autoTimeDateToggle.checked = !isChecked;
            if (setTimeButton) {
              setTimeButton.style.display = !isChecked ? 'none' : 'inline-block';
            }
            alert('Could not set NTP status. Please check your system permissions.');
          }
        }
      });
    }
    
    
    if (hour24Toggle) {
      hour24Toggle.addEventListener('change', () => {
        is24Hour = hour24Toggle.checked;
        formatDateTime();
        
        updateTimeLabelInModal(is24Hour);
      });
    }
    
    
    if (setTimeButton) {
      setTimeButton.addEventListener('click', () => {
        openDateTimeModal();
      });
    }
    
    
    initTimezoneDropdown();
  }
  
  
  function initTimezoneDropdown() {
    const timezoneSelect = document.getElementById('timezone-select');
    
    if (!timezoneSelect || !window.electronAPI) return;
    
    
    Promise.all([
      window.electronAPI.getTimezones(),
      window.electronAPI.getCurrentTimezone()
    ])
      .then(([timezonesResult, currentTimezoneResult]) => {
        const timezones = timezonesResult.timezones || [];
        const currentTimezone = currentTimezoneResult.timezone || '';
        
        
        timezoneSelect.innerHTML = '';
        
        
        timezones.forEach(timezone => {
          const option = document.createElement('option');
          option.value = timezone;
          option.textContent = timezone;
          if (timezone === currentTimezone) {
            option.selected = true;
          }
          timezoneSelect.appendChild(option);
        });
      })
      .catch((error) => {
        console.error('Error loading timezones:', error);
        timezoneSelect.innerHTML = '<option value="">Error loading timezones</option>';
      });
    
    
    timezoneSelect.addEventListener('change', async () => {
      const selectedTimezone = timezoneSelect.value;
      
      if (!selectedTimezone) return;
      
      
      try {
        await window.electronAPI.setTimezone(selectedTimezone);
        console.log('Timezone set to:', selectedTimezone);
      } catch (error) {
        console.error('Error setting timezone:', error);
        alert('Could not set timezone. Please check your system permissions.');
        
        
        try {
          const currentTimezoneResult = await window.electronAPI.getCurrentTimezone();
          timezoneSelect.value = currentTimezoneResult.timezone || '';
        } catch (err) {
          console.error('Error getting current timezone:', err);
        }
      }
    });
  }
  
  
  function openDateTimeModal() {
    const modal = document.getElementById('date-time-modal');
    const datePicker = document.getElementById('date-picker');
    const timePicker = document.getElementById('time-picker');
    const hour24Toggle = document.getElementById('24-hour-time-toggle');
    const dateTimeDisplay = document.getElementById('date-time-display');
    
    if (!modal || !datePicker || !timePicker) return;
    
    
    function formatDateTime() {
      if (!dateTimeDisplay) return;
      
      const now = new Date();
      const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
      const month = months[now.getMonth()];
      const day = now.getDate();
      const year = now.getFullYear();
      
      const hour24Toggle = document.getElementById('24-hour-time-toggle');
      const is24Hour = hour24Toggle ? hour24Toggle.checked : false;
      
      let hours = now.getHours();
      const minutes = String(now.getMinutes()).padStart(2, '0');
      const seconds = String(now.getSeconds()).padStart(2, '0');
      
      if (is24Hour) {
        hours = String(hours).padStart(2, '0');
        dateTimeDisplay.textContent = `${month} ${day}, ${year} at ${hours}:${minutes}:${seconds}`;
      } else {
        const ampm = hours >= 12 ? 'PM' : 'AM';
        hours = hours % 12;
        hours = hours ? hours : 12; 
        hours = String(hours).padStart(2, '0');
        dateTimeDisplay.textContent = `${month} ${day}, ${year} at ${hours}:${minutes}:${seconds} ${ampm}`;
      }
    }
    
    
    const now = new Date();
    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, '0');
    const day = String(now.getDate()).padStart(2, '0');
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    
    
    datePicker.value = `${year}-${month}-${day}`;
    timePicker.value = `${hours}:${minutes}`;
    
    
    const is24Hour = hour24Toggle ? hour24Toggle.checked : false;
    updateTimeLabelInModal(is24Hour);
    
    
    modal.style.display = 'flex';
    
    
    const cancelButton = document.getElementById('date-time-cancel');
    const confirmButton = document.getElementById('date-time-confirm');
    
    if (cancelButton) {
      cancelButton.onclick = () => {
        modal.style.display = 'none';
      };
    }
    
    if (confirmButton) {
      confirmButton.onclick = async () => {
        const selectedDate = datePicker.value;
        const selectedTime = timePicker.value;
        
        
        if (!selectedDate || !selectedTime) {
          alert('Please select both date and time.');
          return;
        }
        
        
        if (window.electronAPI) {
          try {
            await window.electronAPI.setDateTime(selectedDate, selectedTime);
            
            modal.style.display = 'none';
            
            formatDateTime();
          } catch (error) {
            console.error('Error setting date and time:', error);
            alert('Could not set date and time. Please check your system permissions.');
          }
        } else {
          
          console.log('Setting date:', selectedDate, 'and time:', selectedTime);
          modal.style.display = 'none';
        }
      };
    }
    
    
    modal.onclick = (e) => {
      if (e.target === modal) {
        modal.style.display = 'none';
      }
    };
  }
  
  
  function updateTimeLabelInModal(is24Hour) {
    const timeLabel = document.getElementById('time-label');
    if (timeLabel) {
      timeLabel.textContent = is24Hour ? '24-hour time' : 'Time';
    }
  }

  
  let selectedLoginItem = null;
  
  function initLoginItemsPage() {
    const tbody = document.getElementById('login-items-tbody');
    const addButton = document.getElementById('login-item-add');
    const removeButton = document.getElementById('login-item-remove');
    
    if (!tbody || !window.electronAPI) return;
    
    
    const reloadTable = () => {
      window.electronAPI.getAutostartItems()
        .then((result) => {
          const items = result.items || [];
          
          
          tbody.innerHTML = '';
          selectedLoginItem = null;
          if (removeButton) {
            removeButton.disabled = true;
          }
          
          if (items.length === 0) {
            tbody.innerHTML = '<tr><td colspan="2" style="text-align: center; padding: 20px; color: var(--text-secondary);">No login items found.</td></tr>';
            return;
          }
          
          
          items.forEach(item => {
            const row = document.createElement('tr');
            row.style.cursor = 'pointer';
            
            
            row.addEventListener('click', () => {
              
              tbody.querySelectorAll('tr').forEach(r => {
                r.classList.remove('selected');
              });
              
              
              row.classList.add('selected');
              selectedLoginItem = item;
              
              
              if (removeButton) {
                removeButton.disabled = false;
              }
            });
            
            
            const itemCell = document.createElement('td');
            itemCell.style.display = 'flex';
            itemCell.style.alignItems = 'center';
            itemCell.style.gap = '10px';
            
            
            if (item.icon) {
              const iconImg = document.createElement('img');
              iconImg.alt = item.name;
              iconImg.style.width = '20px';
              iconImg.style.height = '20px';
              iconImg.style.objectFit = 'contain';
              
              
              const iconPaths = [
                `file:///usr/share/pixmaps/${item.icon}.png`,
                `file:///usr/share/pixmaps/${item.icon}.xpm`,
                `file:///usr/share/icons/hicolor/48x48/apps/${item.icon}.png`,
                `file:///usr/share/icons/hicolor/48x48/apps/${item.icon}.svg`,
                `file:///usr/share/icons/gnome/48x48/apps/${item.icon}.png`,
                `file:///usr/share/icons/gnome/48x48/apps/${item.icon}.svg`,
                `file://${item.icon}`
              ];
              
              let currentPathIndex = 0;
              
              const tryNextIcon = () => {
                if (currentPathIndex < iconPaths.length) {
                  iconImg.src = iconPaths[currentPathIndex];
                  currentPathIndex++;
                } else {
                  iconImg.style.display = 'none';
                }
              };
              
              iconImg.onerror = tryNextIcon;
              iconImg.onload = () => {
                
              };
              
              
              tryNextIcon();
              
              itemCell.appendChild(iconImg);
            }
            
            
            const nameSpan = document.createElement('span');
            nameSpan.textContent = item.name;
            nameSpan.style.color = 'var(--text-primary)';
            itemCell.appendChild(nameSpan);
            
            
            const kindCell = document.createElement('td');
            kindCell.textContent = item.type;
            kindCell.style.color = 'var(--text-secondary)';
            
            row.appendChild(itemCell);
            row.appendChild(kindCell);
            tbody.appendChild(row);
          });
        })
        .catch((error) => {
          console.error('Error loading autostart items:', error);
          tbody.innerHTML = '<tr><td colspan="2" style="text-align: center; padding: 20px; color: var(--text-secondary);">Error loading login items.</td></tr>';
        });
    };
    
    
    reloadTable();
    
    
    if (addButton) {
      addButton.addEventListener('click', async () => {
        try {
          const result = await window.electronAPI.addAutostartItem();
          if (!result.canceled && result.success) {
            
            reloadTable();
          }
        } catch (error) {
          console.error('Error adding autostart item:', error);
          alert('Could not add item. Please check your system permissions.');
        }
      });
    }
    
    
    if (removeButton) {
      removeButton.addEventListener('click', async () => {
        if (!selectedLoginItem) return;
        
        try {
          await window.electronAPI.removeAutostartItem(selectedLoginItem.file);
          
          reloadTable();
        } catch (error) {
          console.error('Error removing autostart item:', error);
          alert('Could not remove item. Please check your system permissions.');
        }
      });
    }
  }

  
  let selectedBootEntry = null;
  
  function initStartupDiskPage() {
    const bootEntriesGrid = document.getElementById('boot-entries-grid');
    const selectedBootName = document.getElementById('selected-boot-name');
    const selectedBootDisk = document.getElementById('selected-boot-disk');
    const restartButton = document.getElementById('restart-button');
    
    if (!bootEntriesGrid || !window.electronAPI) return;
    
    
    window.electronAPI.getBootEntries()
      .then((result) => {
        const entries = result.entries || [];
        const bootCurrent = result.bootCurrent;
        
        
        bootEntriesGrid.innerHTML = '';
        
        if (entries.length === 0) {
          bootEntriesGrid.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">No boot entries found.</div>';
          return;
        }
        
        
        entries.forEach(entry => {
          const entryCard = document.createElement('div');
          entryCard.className = 'boot-entry-card';
          if (entry.isCurrent) {
            entryCard.classList.add('current');
          }
          
          entryCard.addEventListener('click', () => {
            
            bootEntriesGrid.querySelectorAll('.boot-entry-card').forEach(card => {
              card.classList.remove('selected');
            });
            
            
            entryCard.classList.add('selected');
            selectedBootEntry = entry;
            
            
            if (selectedBootName) {
              selectedBootName.textContent = entry.name;
            }
            if (selectedBootDisk) {
              selectedBootDisk.textContent = entry.disk;
            }
            if (restartButton) {
              restartButton.disabled = false;
            }
          });
          
          const entryName = document.createElement('div');
          entryName.className = 'boot-entry-name';
          entryName.textContent = entry.name;
          
          const entryDisk = document.createElement('div');
          entryDisk.className = 'boot-entry-disk';
          entryDisk.textContent = entry.disk;
          
          if (entry.isCurrent) {
            const currentBadge = document.createElement('span');
            currentBadge.className = 'boot-entry-current';
            currentBadge.textContent = 'Current';
            entryName.appendChild(currentBadge);
          }
          
          entryCard.appendChild(entryName);
          entryCard.appendChild(entryDisk);
          bootEntriesGrid.appendChild(entryCard);
        });
        
        
        const currentEntry = entries.find(e => e.isCurrent);
        if (currentEntry) {
          const currentCard = bootEntriesGrid.querySelector(`.boot-entry-card.current`);
          if (currentCard) {
            currentCard.click();
          }
        }
      })
      .catch((error) => {
        console.error('Error loading boot entries:', error);
        bootEntriesGrid.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">Error loading boot entries. Make sure you have efibootmgr installed and have the necessary permissions.</div>';
      });
    
    
    if (restartButton) {
      restartButton.addEventListener('click', async () => {
        if (!selectedBootEntry) return;
        
        if (confirm(`Are you sure you want to restart your computer with "${selectedBootEntry.name}"?`)) {
          try {
            
            const password = await requestPassword();
            if (!password) {
              return; 
            }
            
            
            await window.electronAPI.setBootEntry(selectedBootEntry.id, password);
            
            
            await window.electronAPI.rebootSystem(password);
          } catch (error) {
            console.error('Error setting boot entry or rebooting:', error);
            alert('Could not set boot entry or restart computer. Please check your system permissions.');
          }
        }
      });
    }
  }

  
  async function initDisplaysPage() {
    const displaysList = document.getElementById('displays-list-page');
    if (!displaysList || !window.electronAPI) return;
    
    try {
      const displays = await window.electronAPI.getDisplays();
      
      
      displaysList.innerHTML = '';
      
      if (!displays || displays.length === 0) {
        displaysList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">No displays found.</div>';
        return;
      }
      
      const connectedDisplays = displays.filter(display => 
        display.isConnected || display.isEnabled || display.isBuiltIn
      );
      
      if (connectedDisplays.length === 0) {
        displaysList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">No displays connected.</div>';
        return;
      }
      
      
      connectedDisplays.forEach((display, index) => {
        const displayItem = document.createElement('div');
        displayItem.className = 'card-section-item';
        displayItem.style.display = 'flex';
        displayItem.style.alignItems = 'center';
        displayItem.style.justifyContent = 'space-between';
        displayItem.style.padding = '12px 0';
        
        
        const leftPart = document.createElement('div');
        leftPart.style.display = 'flex';
        leftPart.style.alignItems = 'center';
        leftPart.style.gap = '12px';
        
        
        const icon = document.createElement('img');
        icon.src = display.isBuiltIn ? './assets/laptop.svg' : './assets/computer.svg';
        icon.alt = display.name;
        icon.style.width = '24px';
        icon.style.height = '24px';
        icon.style.opacity = '0.8';
        
        
        const nameDiv = document.createElement('div');
        nameDiv.style.display = 'flex';
        nameDiv.style.flexDirection = 'column';
        nameDiv.style.gap = '4px';
        
        const displayNameContainer = document.createElement('div');
        displayNameContainer.style.display = 'flex';
        displayNameContainer.style.alignItems = 'center';
        displayNameContainer.style.gap = '8px';
        
        const displayName = document.createElement('div');
        displayName.className = 'display-name';
        displayName.style.fontSize = '13px';
        displayName.style.fontWeight = '500';
        displayName.style.color = 'var(--text-primary)';
        displayName.textContent = display.isBuiltIn ? 'Built-in Display' : display.name;
        
        if (display.isPrimary) {
          const mainBadge = document.createElement('span');
          mainBadge.className = 'display-main-badge';
          mainBadge.textContent = 'main';
          mainBadge.style.fontSize = '10px';
          mainBadge.style.fontWeight = '600';
          mainBadge.style.padding = '2px 6px';
          mainBadge.style.borderRadius = '4px';
          mainBadge.style.backgroundColor = 'var(--bg-card)';
          mainBadge.style.color = 'var(--text-secondary)';
          mainBadge.style.textTransform = 'uppercase';
          displayNameContainer.appendChild(displayName);
          displayNameContainer.appendChild(mainBadge);
        } else {
          displayNameContainer.appendChild(displayName);
        }
        
        
        const displaySpecs = document.createElement('div');
        displaySpecs.className = 'display-specs';
        displaySpecs.style.fontSize = '11px';
        displaySpecs.style.color = 'var(--text-secondary)';
        
        let displaySpecsText = 'Unknown';
        if (display.inches && display.resolution) {
          const [width, height] = display.resolution.split('x');
          displaySpecsText = `${display.inches.replace('"', '')}-inch (${width} x ${height})`;
        } else if (display.resolution) {
          const [width, height] = display.resolution.split('x');
          displaySpecsText = `${width} x ${height}`;
        }
        
        displaySpecs.textContent = displaySpecsText;
        
        nameDiv.appendChild(displayNameContainer);
        nameDiv.appendChild(displaySpecs);
        
        leftPart.appendChild(icon);
        leftPart.appendChild(nameDiv);
        
        
        const rightPart = document.createElement('div');
        rightPart.style.display = 'flex';
        rightPart.style.alignItems = 'center';
        rightPart.style.gap = '10px';
        
        const enableButton = document.createElement('button');
        enableButton.className = 'display-enable-button';
        enableButton.style.padding = '6px 16px';
        enableButton.style.fontSize = '12px';
        enableButton.style.fontWeight = '500';
        enableButton.style.borderRadius = '6px';
        enableButton.style.border = '1px solid var(--border-light)';
        enableButton.style.backgroundColor = display.isEnabled ? 'var(--bg-primary)' : 'var(--bg-card)';
        enableButton.style.color = 'var(--text-primary)';
        enableButton.style.cursor = 'pointer';
        enableButton.style.transition = 'all 0.2s ease';
        enableButton.style.webkitAppRegion = 'no-drag';
        enableButton.textContent = display.isEnabled ? 'Disable' : 'Enable';
        
        enableButton.addEventListener('click', async (e) => {
          e.stopPropagation();
          
          const originalText = enableButton.textContent;
          const originalState = display.isEnabled;
          const newState = !display.isEnabled;
          
          enableButton.disabled = true;
          enableButton.style.opacity = '0.6';
          enableButton.style.cursor = 'wait';
          enableButton.textContent = 'Processing...';
          
          if (!newState) {
            try {
            const allDisplays = await window.electronAPI.getDisplays();
              const enabledDisplays = allDisplays.filter(d => (d.isEnabled || d.isBuiltIn) && d.name !== display.name);
            if (enabledDisplays.length === 0) {
                enableButton.disabled = false;
                enableButton.style.opacity = '1';
                enableButton.style.cursor = 'pointer';
                enableButton.textContent = originalText;
              alert('Cannot disable all displays. At least one display must be enabled.');
                return;
              }
            } catch (checkError) {
              console.error('Error checking displays:', checkError);
              enableButton.disabled = false;
              enableButton.style.opacity = '1';
              enableButton.style.cursor = 'pointer';
              enableButton.textContent = originalText;
              return;
            }
          }
          
          try {
            const previousState = { isEnabled: display.isEnabled };
            await window.electronAPI.setDisplayEnabled(display.name, newState, display.modeId);
            
            startDisplayRevertTimer('enabled', display.name, previousState);
            
            let retryCount = 0;
            const maxRetries = 5;
            const checkAndReload = async () => {
              try {
                const updatedDisplays = await window.electronAPI.getDisplays();
                const updatedDisplay = updatedDisplays.find(d => d.name === display.name);
                
                if (updatedDisplay && updatedDisplay.isEnabled === newState) {
                  await initDisplaysPage();
                } else if (retryCount < maxRetries) {
                  retryCount++;
                  setTimeout(checkAndReload, 300);
                } else {
                  await initDisplaysPage();
                }
              } catch (reloadError) {
                console.error('Error reloading displays:', reloadError);
                await initDisplaysPage();
              }
            };
            
            setTimeout(checkAndReload, 500);
          } catch (error) {
            console.error('Error setting display state:', error);
            enableButton.disabled = false;
            enableButton.style.opacity = '1';
            enableButton.style.cursor = 'pointer';
            enableButton.textContent = originalText;
            
            if (error.message && error.message.includes('Cannot disable all displays')) {
              alert('Cannot disable all displays. At least one display must be enabled.');
            } else {
              alert('Could not change display state. Please check your system permissions.');
            }
          }
        });
        
        enableButton.addEventListener('mouseenter', () => {
          if (!enableButton.disabled) {
            enableButton.style.backgroundColor = 'var(--hover-bg)';
            enableButton.style.borderColor = 'var(--border-color)';
          }
        });
        
        enableButton.addEventListener('mouseleave', () => {
          if (!enableButton.disabled) {
            enableButton.style.backgroundColor = display.isEnabled ? 'var(--bg-primary)' : 'var(--bg-card)';
            enableButton.style.borderColor = 'var(--border-light)';
          }
        });
        
        if (!display.isPrimary) {
          const setMainButton = document.createElement('button');
          setMainButton.className = 'display-set-main-button';
          setMainButton.style.padding = '6px 12px';
          setMainButton.style.fontSize = '12px';
          setMainButton.style.fontWeight = '500';
          setMainButton.style.borderRadius = '6px';
          setMainButton.style.border = '1px solid var(--border-light)';
          setMainButton.style.backgroundColor = 'var(--bg-primary)';
          setMainButton.style.color = 'var(--text-primary)';
          setMainButton.style.cursor = 'pointer';
          setMainButton.style.transition = 'all 0.2s ease';
          setMainButton.style.webkitAppRegion = 'no-drag';
          setMainButton.textContent = 'Set as main';
          
          setMainButton.addEventListener('mouseenter', () => {
            setMainButton.style.backgroundColor = 'var(--hover-bg)';
            setMainButton.style.borderColor = 'var(--border-color)';
          });
          
          setMainButton.addEventListener('mouseleave', () => {
            setMainButton.style.backgroundColor = 'var(--bg-primary)';
            setMainButton.style.borderColor = 'var(--border-light)';
          });
          
          setMainButton.addEventListener('click', async (e) => {
            e.stopPropagation();
            
            console.log(`Setting ${display.name} as primary display`);
            
            try {
              const displays = await window.electronAPI.getDisplays();
              const previousPrimary = displays.find(d => d.isPrimary);
              const previousState = previousPrimary ? { primaryDisplay: previousPrimary.name } : null;
              
              const result = await window.electronAPI.setDisplayPrimary(display.name);
              console.log('Set primary result:', result);
              
              if (previousState) {
                startDisplayRevertTimer('primary', display.name, previousState);
              }
              
              setTimeout(async () => {
                await initDisplaysPage();
              }, 200);
            } catch (error) {
              console.error('Error setting primary display:', error);
              alert('Could not set display as primary. Please check your system permissions.');
            }
          });
          
          rightPart.appendChild(setMainButton);
        }
        
        rightPart.appendChild(enableButton);
        
        displayItem.addEventListener('click', () => {
          const allItems = displaysList.querySelectorAll('.card-section-item');
          allItems.forEach(item => {
            item.style.backgroundColor = '';
            item.style.border = '';
          });
          displayItem.style.backgroundColor = 'var(--hover-bg)';
          displayItem.style.border = '1px solid var(--border-color)';
          
          const resolutionCard = document.getElementById('resolution-settings-card');
          if (resolutionCard) {
            resolutionCard.style.display = 'block';
            initResolutionOptions(display);
            initBrightnessSlider(display);
          }
        });
        
        displayItem.appendChild(leftPart);
        displayItem.appendChild(rightPart);
        displaysList.appendChild(displayItem);
      });
    } catch (error) {
      console.error('Error loading displays:', error);
      displaysList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">Error loading displays.</div>';
    }
  }

  let displayRevertTimer = null;
  let displayRevertState = null;
  
  function startDisplayRevertTimer(operation, displayName, previousState) {
    if (displayRevertTimer) {
      clearInterval(displayRevertTimer);
      displayRevertTimer = null;
    }
    
    displayRevertState = {
      operation: operation,
      displayName: displayName,
      previousState: previousState
    };
    
    const revertCard = document.getElementById('display-revert-card');
    const countdownElement = document.getElementById('revert-countdown');
    const timerText = document.getElementById('revert-timer-text');
    
    if (!revertCard || !countdownElement || !timerText) return;
    
    revertCard.style.display = 'block';
    let secondsLeft = 15;
    countdownElement.textContent = secondsLeft;
    timerText.innerHTML = `Reverting your changes in <span id="revert-countdown">${secondsLeft}</span>s...`;
    
    displayRevertTimer = setInterval(() => {
      secondsLeft--;
      const countdown = document.getElementById('revert-countdown');
      if (countdown) {
        countdown.textContent = secondsLeft;
      }
      
      if (secondsLeft <= 0) {
        clearInterval(displayRevertTimer);
        displayRevertTimer = null;
        
        if (timerText) {
          timerText.textContent = 'Reverting...';
        }
        
        setTimeout(async () => {
          await performDisplayRevert();
        }, 1000);
      }
    }, 1000);
    
    const keepButton = document.getElementById('revert-keep-button');
    const revertButton = document.getElementById('revert-revert-button');
    
    if (keepButton) {
      keepButton.onclick = () => {
        if (displayRevertTimer) {
          clearInterval(displayRevertTimer);
          displayRevertTimer = null;
        }
        if (revertCard) {
          revertCard.style.display = 'none';
        }
        displayRevertState = null;
      };
    }
    
    if (revertButton) {
      revertButton.onclick = async () => {
        if (displayRevertTimer) {
          clearInterval(displayRevertTimer);
          displayRevertTimer = null;
        }
        await performDisplayRevert();
      };
    }
  }
  
  async function performDisplayRevert() {
    if (!displayRevertState) return;
    
    const revertCard = document.getElementById('display-revert-card');
    const timerText = document.getElementById('revert-timer-text');
    
    if (timerText) {
      timerText.textContent = 'Reverting...';
    }
    
    try {
      const { operation, displayName, previousState } = displayRevertState;
      
      if (operation === 'resolution') {
        if (previousState && previousState.resolution) {
          await window.electronAPI.setDisplayResolution(displayName, previousState.resolutionType || 'default');
        }
      } else if (operation === 'enabled') {
        await window.electronAPI.setDisplayEnabled(displayName, previousState.isEnabled);
      } else if (operation === 'primary') {
        if (previousState && previousState.primaryDisplay) {
          await window.electronAPI.setDisplayPrimary(previousState.primaryDisplay);
        }
      }
      
      setTimeout(async () => {
        await initDisplaysPage();
        if (revertCard) {
          revertCard.style.display = 'none';
        }
        displayRevertState = null;
      }, 300);
    } catch (error) {
      console.error('Error reverting display changes:', error);
      if (revertCard) {
        revertCard.style.display = 'none';
      }
      displayRevertState = null;
      alert('Could not revert changes. Please check your system permissions.');
    }
  }
  
  function initResolutionOptions(selectedDisplay) {
    const resolutionOptions = document.querySelectorAll('.resolution-option');
    let currentResolutionType = 'default';
    let largerResolution = null;
    let mediumResolution = null;
    let defaultResolution = null;
    
    if (selectedDisplay && selectedDisplay.availableResolutions && selectedDisplay.availableResolutions.length > 0) {
      const sortedResolutions = [...selectedDisplay.availableResolutions].sort((a, b) => b.totalPixels - a.totalPixels);
      const currentRes = selectedDisplay.resolution;
      
      // Găsește rezoluția curentă în lista de rezoluții disponibile
      const currentResIndex = sortedResolutions.findIndex(r => r.resolution === currentRes);
      
      // Setează cele 3 opțiuni bazate pe rezoluția curentă
      // Default = rezoluția curentă (cea selectată acum)
      // Larger = o rezoluție mai mică (text mai mare)
      // Medium = între default și larger
      
      if (currentResIndex !== -1) {
        defaultResolution = sortedResolutions[currentResIndex];
        currentResolutionType = 'default';
        
        // Larger = cea mai mică rezoluție disponibilă
        largerResolution = sortedResolutions[sortedResolutions.length - 1];
        
        // Medium = la mijloc între current și cea mai mică
        const midIndex = Math.floor((currentResIndex + sortedResolutions.length - 1) / 2);
        mediumResolution = sortedResolutions[midIndex];
        
        // Dacă medium e același cu default sau larger, ajustează
        if (mediumResolution.resolution === defaultResolution.resolution && sortedResolutions.length > 1) {
          mediumResolution = sortedResolutions[Math.min(currentResIndex + 1, sortedResolutions.length - 1)];
        }
      } else {
        // Fallback: folosește prima (cea mai mare) ca default
        defaultResolution = sortedResolutions[0];
        largerResolution = sortedResolutions[sortedResolutions.length - 1];
        mediumResolution = sortedResolutions[Math.floor(sortedResolutions.length / 2)];
        currentResolutionType = 'default';
      }
    }
    
    resolutionOptions.forEach(option => {
      const optionType = option.getAttribute('data-resolution');
      const resolutionValue = option.querySelector('.resolution-value');
      
      const oldClickHandler = option._clickHandler;
      const oldMouseEnterHandler = option._mouseEnterHandler;
      const oldMouseLeaveHandler = option._mouseLeaveHandler;
      
      if (oldClickHandler) {
        option.removeEventListener('click', oldClickHandler);
      }
      if (oldMouseEnterHandler) {
        option.removeEventListener('mouseenter', oldMouseEnterHandler);
      }
      if (oldMouseLeaveHandler) {
        option.removeEventListener('mouseleave', oldMouseLeaveHandler);
      }
      
      option.classList.remove('active');
      
      let resolutionText = '-';
      if (selectedDisplay && selectedDisplay.availableResolutions && selectedDisplay.availableResolutions.length > 0) {
        if (optionType === 'larger' && largerResolution) {
          resolutionText = largerResolution.resolution;
        } else if (optionType === 'medium' && mediumResolution) {
          resolutionText = mediumResolution.resolution;
        } else if (optionType === 'default' && defaultResolution) {
          resolutionText = defaultResolution.resolution;
        }
      }
      
      if (resolutionValue) {
        resolutionValue.textContent = resolutionText;
        resolutionValue.style.opacity = '0';
      }
      
      if (optionType === currentResolutionType) {
        option.classList.add('active');
        option.style.backgroundColor = 'var(--hover-bg)';
        option.style.border = '1px solid var(--border-color)';
      } else {
        option.style.backgroundColor = '';
        option.style.border = '';
      }
      
      const clickHandler = async () => {
        if (!selectedDisplay || !selectedDisplay.isEnabled) {
          alert('Please enable the display first');
          return;
        }
        
        resolutionOptions.forEach(opt => {
          opt.classList.remove('active');
          opt.style.backgroundColor = '';
          opt.style.border = '';
        });
        
        option.classList.add('active');
        option.style.backgroundColor = 'var(--hover-bg)';
        option.style.border = '1px solid var(--border-color)';
        
        try {
          const previousResolutionType = currentResolutionType;
          const previousResolution = selectedDisplay.resolution;
          const previousState = {
            resolution: previousResolution,
            resolutionType: previousResolutionType
          };
          
          await window.electronAPI.setDisplayResolution(selectedDisplay.name, optionType);
          
          startDisplayRevertTimer('resolution', selectedDisplay.name, previousState);
          
          setTimeout(async () => {
            await initDisplaysPage();
            const updatedDisplays = await window.electronAPI.getDisplays();
            const updatedDisplay = updatedDisplays.find(d => d.name === selectedDisplay.name);
            if (updatedDisplay) {
              initResolutionOptions(updatedDisplay);
            }
          }, 300);
        } catch (error) {
          console.error('Error setting resolution:', error);
          alert('Could not set resolution. Please check your system permissions.');
          option.classList.remove('active');
          option.style.backgroundColor = '';
          option.style.border = '';
          const currentOption = Array.from(resolutionOptions).find(opt => opt.getAttribute('data-resolution') === currentResolutionType);
          if (currentOption) {
            currentOption.classList.add('active');
            currentOption.style.backgroundColor = 'var(--hover-bg)';
            currentOption.style.border = '1px solid var(--border-color)';
          }
        }
      };
      
      const mouseEnterHandler = () => {
        if (!option.classList.contains('active')) {
          option.style.backgroundColor = 'var(--hover-bg)';
        }
        if (resolutionValue && resolutionText !== '-') {
          resolutionValue.style.opacity = '1';
          resolutionValue.style.height = 'auto';
        }
      };
      
      const mouseLeaveHandler = () => {
        if (!option.classList.contains('active')) {
          option.style.backgroundColor = '';
        }
        if (resolutionValue) {
          resolutionValue.style.opacity = '0';
          resolutionValue.style.height = '0';
        }
      };
      
      option._clickHandler = clickHandler;
      option._mouseEnterHandler = mouseEnterHandler;
      option._mouseLeaveHandler = mouseLeaveHandler;
      
      option.addEventListener('click', clickHandler);
      option.addEventListener('mouseenter', mouseEnterHandler);
      option.addEventListener('mouseleave', mouseLeaveHandler);
    });
  }

  
  async function initBrightnessSlider(selectedDisplay) {
    const brightnessSlider = document.getElementById('brightness-slider');
    const brightnessSection = document.getElementById('brightness-section');
    const sliderContainer = document.getElementById('brightness-slider-container');
    const progress = document.getElementById('brightness-progress');
    const thumb = document.getElementById('brightness-thumb');
    
    if (!window.electronAPI || !sliderContainer || !progress || !thumb) return;
    
    // Ascunde secțiunea de brightness dacă nu e selectat un display
    if (!selectedDisplay) {
      if (brightnessSection) brightnessSection.style.display = 'none';
      return;
    }
    
    // Afișează secțiunea de brightness
    if (brightnessSection) brightnessSection.style.display = 'flex';
    
    const displayName = selectedDisplay.name;
    
    // Liquid Glass Slider Logic
    let isDragging = false;
    let currentBrightness = 100;
    
    const updateThumbAndProgress = (percent) => {
      percent = Math.max(0, Math.min(100, percent));
      const sliderRect = sliderContainer.getBoundingClientRect();
      const px = (percent / 100) * sliderRect.width;
      progress.style.width = `${percent}%`;
      thumb.style.left = `${px}px`;
      // Nu mai afișăm procentul pentru brightness (avem iconițe soare)
      currentBrightness = percent;
    };
    
    const getPercentFromClientX = (clientX) => {
      const sliderRect = sliderContainer.getBoundingClientRect();
      const offsetX = clientX - sliderRect.left;
      return (offsetX / sliderRect.width) * 100;
    };
    
    const onMove = async (clientX) => {
      const percent = Math.max(0, Math.min(100, getPercentFromClientX(clientX)));
      updateThumbAndProgress(percent);
      
      // Setează brightness-ul
      try {
        await window.electronAPI.setBrightness(Math.round(percent), displayName);
      } catch (error) {
        console.error('Error setting brightness:', error);
      }
    };
    
    // Elimină event listeners vechi
    const oldMouseDown = sliderContainer._mouseDown;
    const oldTouchStart = sliderContainer._touchStart;
    const oldThumbMouseDown = thumb._mouseDown;
    const oldThumbTouchStart = thumb._touchStart;
    
    if (oldMouseDown) sliderContainer.removeEventListener('mousedown', oldMouseDown);
    if (oldTouchStart) sliderContainer.removeEventListener('touchstart', oldTouchStart);
    if (oldThumbMouseDown) thumb.removeEventListener('mousedown', oldThumbMouseDown);
    if (oldThumbTouchStart) thumb.removeEventListener('touchstart', oldThumbTouchStart);
    
    // Event handlers
    const onMouseDown = (e) => {
      isDragging = true;
      onMove(e.clientX);
      thumb.classList.add('active');
    };
    
    const onTouchStart = (e) => {
      isDragging = true;
      onMove(e.touches[0].clientX);
      thumb.classList.add('active');
    };
    
    const onMouseMove = (e) => {
      if (isDragging) onMove(e.clientX);
    };
    
    const onTouchMove = (e) => {
      if (isDragging) {
        e.preventDefault();
        onMove(e.touches[0].clientX);
      }
    };
    
    const stopDrag = () => {
      isDragging = false;
      thumb.classList.remove('active');
    };
    
    // Attach events
    thumb._mouseDown = onMouseDown;
    thumb._touchStart = onTouchStart;
    sliderContainer._mouseDown = onMouseDown;
    sliderContainer._touchStart = onTouchStart;
    
    thumb.addEventListener('mousedown', onMouseDown);
    thumb.addEventListener('touchstart', onTouchStart, { passive: true });
    sliderContainer.addEventListener('mousedown', onMouseDown);
    sliderContainer.addEventListener('touchstart', onTouchStart, { passive: true });
    
    // Global events pentru drag
    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', stopDrag);
    document.addEventListener('touchmove', onTouchMove, { passive: false });
    document.addEventListener('touchend', stopDrag);
    
    // Load initial brightness
    try {
      const result = await window.electronAPI.getBrightness(displayName);
      const initialBrightness = result.brightness || 100;
      updateThumbAndProgress(initialBrightness);
    } catch (error) {
      console.error('Error loading brightness:', error);
      updateThumbAndProgress(100);
    }
  }

  
  async function initWallpaperPage() {
    const currentWallpaperPreview = document.getElementById('current-wallpaper-preview');
    const currentWallpaperName = document.getElementById('current-wallpaper-name');
    const wallpapersList = document.getElementById('wallpapers-list');
    const fillModeSelect = document.getElementById('wallpaper-fill-mode');
    const browseBtn = document.getElementById('browse-wallpaper-btn');
    
    if (!currentWallpaperPreview || !currentWallpaperName || !wallpapersList || !window.electronAPI) return;
    
    // Browse button handler
    if (browseBtn && window.electronAPI.browseWallpaper) {
      browseBtn.onclick = async () => {
        try {
          const selectedPath = await window.electronAPI.browseWallpaper();
          if (selectedPath) {
            await window.electronAPI.setWallpaper(selectedPath);
            initWallpaperPage();
            
            // Actualizează tint-ul automat cu noul wallpaper dacă este activat
            if (globalTintEnabled && window.electronAPI.getWindowPosition && window.electronAPI.getWallpaperColorAtPosition) {
              setTimeout(() => {
                updateGlobalTint();
              }, 300);
            }
          }
        } catch (error) {
          console.error('Error browsing/setting wallpaper:', error);
          alert('Could not set wallpaper. Please check your system permissions.');
        }
      };
    }
    
    try {
      
      const currentWallpaper = await window.electronAPI.getCurrentWallpaper();
      if (currentWallpaper && currentWallpaper.path) {
        
        let wallpaperPath = currentWallpaper.path;
        if (wallpaperPath.startsWith('file://')) {
          wallpaperPath = wallpaperPath.substring(7);
        }
        
        currentWallpaperPreview.src = wallpaperPath;
        currentWallpaperName.textContent = currentWallpaper.name || wallpaperPath.split('/').pop();
      } else {
        currentWallpaperPreview.src = '';
        currentWallpaperName.textContent = 'No wallpaper set';
      }
      
      
      const wallpapers = await window.electronAPI.getWallpapers();
      
      wallpapersList.innerHTML = '';
      
      if (!wallpapers || wallpapers.length === 0) {
        wallpapersList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary); width: 100%;">No wallpapers found.</div>';
        return;
      }
      
      wallpapers.forEach(wallpaper => {
        const wallpaperItem = document.createElement('div');
        wallpaperItem.style.cssText = 'position: relative; cursor: pointer; border-radius: 8px; overflow: hidden; border: 2px solid transparent; transition: all 0.2s ease; aspect-ratio: 16/9;';
        
        const wallpaperImg = document.createElement('img');
        
        wallpaperImg.src = wallpaper.path;
        wallpaperImg.alt = wallpaper.name;
        wallpaperImg.style.cssText = 'width: 100%; height: 100%; object-fit: cover;';
        
        wallpaperItem.appendChild(wallpaperImg);
        
        wallpaperItem.addEventListener('click', async () => {
          try {
            await window.electronAPI.setWallpaper(wallpaper.path);
            
            initWallpaperPage();
            
            // Actualizează tint-ul automat cu noul wallpaper dacă este activat
            if (globalTintEnabled && window.electronAPI && window.electronAPI.getWindowPosition && window.electronAPI.getWallpaperColorAtPosition) {
              setTimeout(() => {
                updateGlobalTint();
              }, 300); // Așteaptă puțin pentru ca wallpaper-ul să se încarce complet
            }
          } catch (error) {
            console.error('Error setting wallpaper:', error);
            alert('Could not set wallpaper. Please check your system permissions.');
          }
        });
        
        wallpaperItem.addEventListener('mouseenter', () => {
          wallpaperItem.style.borderColor = '#007AFF';
          wallpaperItem.style.transform = 'scale(1.05)';
        });
        
        wallpaperItem.addEventListener('mouseleave', () => {
          wallpaperItem.style.borderColor = 'transparent';
          wallpaperItem.style.transform = 'scale(1)';
        });
        
        wallpapersList.appendChild(wallpaperItem);
      });
      
      
      if (fillModeSelect) {
        const existingListener = fillModeSelect._fillModeListener;
        if (existingListener) {
          fillModeSelect.removeEventListener('change', existingListener);
        }
        
        const fillModeListener = async (e) => {
          try {
            await window.electronAPI.setWallpaperFillMode(e.target.value);
            console.log('Fill mode set to:', e.target.value);
            
            // Actualizează tint-ul automat dacă este activat (fill mode-ul afectează poziția culorii)
            if (globalTintEnabled && window.electronAPI && window.electronAPI.getWindowPosition && window.electronAPI.getWallpaperColorAtPosition) {
              setTimeout(() => {
                updateGlobalTint();
              }, 300);
            }
          } catch (error) {
            console.error('Error setting fill mode:', error);
            alert('Could not set fill mode. Please check your system permissions.');
          }
        };
        
        fillModeSelect._fillModeListener = fillModeListener;
        fillModeSelect.addEventListener('change', fillModeListener);
      }
    } catch (error) {
      console.error('Error loading wallpaper page:', error);
      currentWallpaperName.textContent = 'Error loading wallpaper';
      wallpapersList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary); width: 100%;">Error loading wallpapers.</div>';
    }
  }

  
  async function initSoundPage() {
    const outputDeviceSelect = document.getElementById('output-device-select');
    const inputDeviceSelect = document.getElementById('input-device-select');
    const outputVolumeValue = document.getElementById('output-volume-value');
    const inputVolumeValue = document.getElementById('input-volume-value');
    
    // Liquid Glass Slider elements
    const outputSliderContainer = document.getElementById('output-volume-slider-container');
    const outputProgress = document.getElementById('output-volume-progress');
    const outputThumb = document.getElementById('output-volume-thumb');
    const inputSliderContainer = document.getElementById('input-volume-slider-container');
    const inputProgress = document.getElementById('input-volume-progress');
    const inputThumb = document.getElementById('input-volume-thumb');
    
    if (!outputDeviceSelect || !inputDeviceSelect || !window.electronAPI) return;
    
    // Helper function to initialize a liquid glass slider
    function initLiquidSlider(container, progress, thumb, valueElement, onChangeCallback) {
      if (!container || !progress || !thumb) return;
      
      let isDragging = false;
      
      const updateThumbAndProgress = (percent) => {
        percent = Math.max(0, Math.min(100, percent));
        const sliderRect = container.getBoundingClientRect();
        const px = (percent / 100) * sliderRect.width;
        progress.style.width = `${percent}%`;
        thumb.style.left = `${px}px`;
        if (valueElement) valueElement.textContent = `${Math.round(percent)}%`;
        return Math.round(percent);
      };
      
      const getPercentFromClientX = (clientX) => {
        const sliderRect = container.getBoundingClientRect();
        const offsetX = clientX - sliderRect.left;
        return (offsetX / sliderRect.width) * 100;
      };
      
      const onMove = async (clientX) => {
        const percent = Math.max(0, Math.min(100, getPercentFromClientX(clientX)));
        const value = updateThumbAndProgress(percent);
        if (onChangeCallback) {
          try {
            await onChangeCallback(value);
          } catch (error) {
            console.error('Error in slider callback:', error);
          }
        }
      };
      
      const onMouseDown = (e) => {
        isDragging = true;
        onMove(e.clientX);
        thumb.classList.add('active');
      };
      
      const onTouchStart = (e) => {
        isDragging = true;
        onMove(e.touches[0].clientX);
        thumb.classList.add('active');
      };
      
      const onMouseMove = (e) => {
        if (isDragging) onMove(e.clientX);
      };
      
      const onTouchMove = (e) => {
        if (isDragging) {
          e.preventDefault();
          onMove(e.touches[0].clientX);
        }
      };
      
      const stopDrag = () => {
        isDragging = false;
        thumb.classList.remove('active');
      };
      
      thumb.addEventListener('mousedown', onMouseDown);
      thumb.addEventListener('touchstart', onTouchStart, { passive: true });
      container.addEventListener('mousedown', onMouseDown);
      container.addEventListener('touchstart', onTouchStart, { passive: true });
      document.addEventListener('mousemove', onMouseMove);
      document.addEventListener('mouseup', stopDrag);
      document.addEventListener('touchmove', onTouchMove, { passive: false });
      document.addEventListener('touchend', stopDrag);
      
      return { updateThumbAndProgress };
    }
    
    try {
      const [outputs, inputs, currentOutput, currentInput, outputVolume, inputVolume] = await Promise.all([
        window.electronAPI.getAudioOutputs(),
        window.electronAPI.getAudioInputs(),
        window.electronAPI.getCurrentOutputDevice(),
        window.electronAPI.getCurrentInputDevice(),
        window.electronAPI.getOutputVolume(),
        window.electronAPI.getInputVolume()
      ]);
      
      // Populate output devices
      outputDeviceSelect.innerHTML = '';
      if (outputs.length === 0) {
        outputDeviceSelect.innerHTML = '<option value="">No output devices found</option>';
      } else {
        outputs.forEach(output => {
          const option = document.createElement('option');
          option.value = output.index;
          option.textContent = output.description || output.name;
          if (currentOutput.name === output.name) {
            option.selected = true;
          }
          outputDeviceSelect.appendChild(option);
        });
      }
      
      // Populate input devices
      inputDeviceSelect.innerHTML = '';
      if (inputs.length === 0) {
        inputDeviceSelect.innerHTML = '<option value="">No input devices found</option>';
      } else {
        inputs.forEach(input => {
          const option = document.createElement('option');
          option.value = input.index;
          option.textContent = input.description || input.name;
          if (currentInput.name === input.name) {
            option.selected = true;
          }
          inputDeviceSelect.appendChild(option);
        });
      }
      
      // Initialize output volume slider
      const outputSlider = initLiquidSlider(
        outputSliderContainer, outputProgress, outputThumb, outputVolumeValue,
        async (volume) => await window.electronAPI.setOutputVolume(volume)
      );
      
      // Initialize input volume slider
      const inputSlider = initLiquidSlider(
        inputSliderContainer, inputProgress, inputThumb, inputVolumeValue,
        async (volume) => await window.electronAPI.setInputVolume(volume)
      );
      
      // Set initial values
      if (outputVolume.volume !== undefined && outputSlider) {
        outputSlider.updateThumbAndProgress(outputVolume.volume);
      }
      
      if (inputVolume.volume !== undefined && inputSlider) {
        inputSlider.updateThumbAndProgress(inputVolume.volume);
      }
      
      // Device change handlers
      outputDeviceSelect.addEventListener('change', async (e) => {
        try {
          await window.electronAPI.setOutputDevice(e.target.value);
        } catch (error) {
          console.error('Error setting output device:', error);
          alert('Could not set output device. Please check your system permissions.');
        }
      });
      
      inputDeviceSelect.addEventListener('change', async (e) => {
        try {
          await window.electronAPI.setInputDevice(e.target.value);
        } catch (error) {
          console.error('Error setting input device:', error);
          alert('Could not set input device. Please check your system permissions.');
        }
      });
    } catch (error) {
      console.error('Error loading sound page:', error);
      outputDeviceSelect.innerHTML = '<option value="">Error loading devices</option>';
      inputDeviceSelect.innerHTML = '<option value="">Error loading devices</option>';
    }
  }

  
  async function initLockScreenPage() {
    const batteryTimeoutSelect = document.getElementById('battery-timeout-select');
    const acTimeoutSelect = document.getElementById('ac-timeout-select');
    const lockDelaySelect = document.getElementById('lock-delay-select');
    
    if (!batteryTimeoutSelect || !acTimeoutSelect || !lockDelaySelect || !window.electronAPI) return;
    
    try {
      
      const [batteryTimeout, acTimeout, lockDelay] = await Promise.all([
        window.electronAPI.getBatteryTimeout(),
        window.electronAPI.getAcTimeout(),
        window.electronAPI.getLockDelay()
      ]);
      
      
      if (batteryTimeout.timeout !== undefined) {
        const timeoutValue = batteryTimeout.timeout;
        batteryTimeoutSelect.value = timeoutValue.toString();
      }
      
      
      if (acTimeout.timeout !== undefined) {
        const timeoutValue = acTimeout.timeout;
        acTimeoutSelect.value = timeoutValue.toString();
      }
      
      
      if (lockDelay.delay !== undefined) {
        const delayValue = lockDelay.delay;
        
        lockDelaySelect.value = delayValue === -1 ? '-1' : delayValue.toString();
      }
      
      
      batteryTimeoutSelect.addEventListener('change', async (e) => {
        try {
          await window.electronAPI.setBatteryTimeout(parseInt(e.target.value));
        } catch (error) {
          console.error('Error setting battery timeout:', error);
          alert('Could not set battery timeout. Please check your system permissions.');
        }
      });
      
      
      acTimeoutSelect.addEventListener('change', async (e) => {
        try {
          await window.electronAPI.setAcTimeout(parseInt(e.target.value));
        } catch (error) {
          console.error('Error setting AC timeout:', error);
          alert('Could not set AC timeout. Please check your system permissions.');
        }
      });
      
      
      lockDelaySelect.addEventListener('change', async (e) => {
        try {
          await window.electronAPI.setLockDelay(parseInt(e.target.value));
        } catch (error) {
          console.error('Error setting lock delay:', error);
          alert('Could not set lock delay. Please check your system permissions.');
        }
      });
    } catch (error) {
      console.error('Error loading lock screen page:', error);
    }
  }

  
  async function initPrivacySecurityPage() {
    const requireAppsList = document.getElementById('require-apps-list');
    if (!requireAppsList || !window.electronAPI) return;
    
    try {
      const apps = await window.electronAPI.getRequireApps();
      
      requireAppsList.innerHTML = '';
      
      if (!apps || apps.length === 0) {
        requireAppsList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">No applications require your attention.</div>';
        return;
      }
      
      apps.forEach(app => {
        const appItem = document.createElement('div');
        appItem.style.cssText = 'display: flex; flex-direction: column; gap: 12px; padding: 15px; border: 1px solid var(--border-color); border-radius: 8px; background-color: var(--bg-primary); width: 100%; max-width: 600px; box-sizing: border-box;';
        
        
        const header = document.createElement('div');
        header.style.cssText = 'display: flex; align-items: center; gap: 12px;';
        
        const icon = document.createElement('img');
        
        if (app.icon && app.icon.trim()) {
          icon.src = app.icon;
        } else {
          icon.src = './assets/preferences-other.svg';
        }
        icon.alt = app.title;
        icon.style.cssText = 'width: 48px; height: 48px; border-radius: 8px; object-fit: cover;';
        icon.onerror = function() {
          
          if (this.src !== './assets/preferences-other.svg') {
            this.src = './assets/preferences-other.svg';
          }
        };
        
        const titleDiv = document.createElement('div');
        titleDiv.style.cssText = 'flex: 1;';
        
        const title = document.createElement('h4');
        title.textContent = app.title;
        title.style.cssText = 'font-size: 14px; font-weight: 600; color: var(--text-primary); margin: 0 0 4px 0;';
        
        if (app.developer) {
          const developer = document.createElement('div');
          developer.textContent = app.developer;
          developer.style.cssText = 'font-size: 12px; color: var(--text-secondary);';
          titleDiv.appendChild(title);
          titleDiv.appendChild(developer);
        } else {
          titleDiv.appendChild(title);
        }
        
        header.appendChild(icon);
        header.appendChild(titleDiv);
        
        
        const description = document.createElement('p');
        description.textContent = app.description || '';
        description.style.cssText = 'font-size: 12px; color: var(--text-secondary); line-height: 1.5; margin: 0;';
        
        
        let websiteLink = null;
        if (app.website) {
          websiteLink = document.createElement('a');
          websiteLink.href = app.website;
          websiteLink.textContent = app.website;
          websiteLink.target = '_blank';
          websiteLink.style.cssText = 'font-size: 12px; color: #007AFF; text-decoration: none; margin-top: 4px; display: inline-block;';
          websiteLink.addEventListener('click', async (e) => {
            e.preventDefault();
            if (window.electronAPI && window.electronAPI.openExternalLink) {
              await window.electronAPI.openExternalLink(app.website);
            } else {
              window.open(app.website, '_blank');
            }
          });
        }
        
        
        const buttonsDiv = document.createElement('div');
        buttonsDiv.style.cssText = 'display: flex; gap: 10px; justify-content: flex-end; margin-top: 8px;';
        
        const denyButton = document.createElement('button');
        denyButton.textContent = 'Deny';
        denyButton.style.cssText = 'padding: 6px 16px; border: 1px solid var(--border-color); border-radius: 6px; background-color: var(--bg-primary); color: var(--text-primary); font-size: 13px; cursor: pointer; -webkit-app-region: no-drag;';
        denyButton.addEventListener('click', async () => {
          try {
            await window.electronAPI.denyRequireApp(app.file);
            
            initPrivacySecurityPage();
          } catch (error) {
            console.error('Error denying app:', error);
            alert('Could not deny application. Please check your system permissions.');
          }
        });
        
        const allowButton = document.createElement('button');
        allowButton.textContent = 'Allow';
        allowButton.style.cssText = 'padding: 6px 16px; border: none; border-radius: 6px; background-color: #007AFF; color: white; font-size: 13px; cursor: pointer; -webkit-app-region: no-drag;';
        allowButton.addEventListener('click', async () => {
          try {
            await window.electronAPI.allowRequireApp(app.file, app.allowAction);
            
            initPrivacySecurityPage();
          } catch (error) {
            console.error('Error allowing app:', error);
            alert('Could not allow application. Please check your system permissions.');
          }
        });
        
        buttonsDiv.appendChild(denyButton);
        buttonsDiv.appendChild(allowButton);
        
        
        appItem.appendChild(header);
        if (app.description) {
          appItem.appendChild(description);
        }
        if (websiteLink) {
          appItem.appendChild(websiteLink);
        }
        appItem.appendChild(buttonsDiv);
        
        requireAppsList.appendChild(appItem);
      });
    } catch (error) {
      console.error('Error loading require apps:', error);
      requireAppsList.innerHTML = '<div style="text-align: center; padding: 20px; color: var(--text-secondary);">Error loading applications.</div>';
    }
  }

async function initUsersGroupsPage() {
  const usersList = document.getElementById('users-list');
  const autologinDropdown = document.getElementById('autologin-dropdown');
  
  if (!usersList || !window.electronAPI) return;
  
  try {
    const result = await window.electronAPI.getUsers();
    const users = result.users || [];
    
    if (users.length === 0) {
      usersList.innerHTML = '<div style="color: var(--text-secondary);">No users found.</div>';
    } else {
      
      const usersHtml = users.map(user => {
        const faceIcon = user.faceIcon || './assets/cs-user.svg';
        const adminText = user.isAdmin ? '<div style="font-size: 12px; color: var(--text-secondary); margin-top: 4px;">Admin</div>' : '';
        
        return `
          <div class="card-section" style="display: flex; align-items: center; gap: 12px; padding: 12px 0;">
            <img src="${faceIcon}" alt="${user.username}" style="width: 48px; height: 48px; border-radius: 50%; object-fit: cover;" onerror="this.src='./assets/cs-user.svg'">
            <div style="flex: 1;">
              <div style="font-size: 14px; color: var(--text-primary); font-weight: 500;">${user.username}</div>
              ${adminText}
            </div>
          </div>
        `;
      }).join('');
      
      usersList.innerHTML = usersHtml;
    }
    
    
    if (autologinDropdown && window.electronAPI.getAutologinStatus) {
      try {
        const autologinStatus = await window.electronAPI.getAutologinStatus();
        
        autologinDropdown.value = autologinStatus.enabled ? 'on' : 'off';
      } catch (error) {
        console.error('Error loading autologin status:', error);
        
        autologinDropdown.value = 'off';
      }
    } else if (autologinDropdown) {
      
      autologinDropdown.value = 'off';
    }
    
    
    if (autologinDropdown && window.electronAPI.setAutologin) {
      autologinDropdown.addEventListener('change', async (e) => {
        const enabled = e.target.value === 'on';
        
        try {
          await window.electronAPI.setAutologin(enabled);
          
          const autologinStatus = await window.electronAPI.getAutologinStatus();
          autologinDropdown.value = autologinStatus.enabled ? 'on' : 'off';
        } catch (error) {
          console.error('Error setting autologin:', error);
          alert(`Failed to ${enabled ? 'enable' : 'disable'} autologin: ${error.message}`);
          
          const autologinStatus = await window.electronAPI.getAutologinStatus();
          autologinDropdown.value = autologinStatus.enabled ? 'on' : 'off';
        }
      });
    }
  } catch (error) {
    console.error('Error loading users:', error);
    usersList.innerHTML = `<div style="color: var(--text-error);">Error loading users: ${error.message}</div>`;
  }
}

async function initKeyboardPage() {
  const keyRepeatToggle = document.getElementById('key-repeat-toggle');
  const keyRepeatDelaySelect = document.getElementById('key-repeat-delay-select');
  const keyRepeatRateSelect = document.getElementById('key-repeat-rate-select');
  const keyboardLayoutSelect = document.getElementById('keyboard-layout-select');
  
  if (!keyRepeatToggle || !keyRepeatDelaySelect || !keyRepeatRateSelect || !keyboardLayoutSelect || !window.electronAPI) return;
  
  try {
    
    const settings = await window.electronAPI.getKeyboardSettings();
    
    
    keyRepeatToggle.checked = settings.repeatEnabled;
    
    
    const delay = settings.delay;
    if (delay <= 250) {
      keyRepeatDelaySelect.value = '200';
    } else if (delay <= 500) {
      keyRepeatDelaySelect.value = '400';
    } else if (delay <= 700) {
      keyRepeatDelaySelect.value = '600';
    } else {
      keyRepeatDelaySelect.value = '800';
    }
    
    
    const rate = settings.rate;
    if (rate <= 15) {
      keyRepeatRateSelect.value = '10';
    } else if (rate <= 22) {
      keyRepeatRateSelect.value = '20';
    } else if (rate <= 27) {
      keyRepeatRateSelect.value = '25';
    } else {
      keyRepeatRateSelect.value = '30';
    }
    
    
    keyboardLayoutSelect.value = settings.layout || 'us';
    
    
    keyRepeatToggle.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.setKeyRepeat(e.target.checked);
      } catch (error) {
        console.error('Error setting key repeat:', error);
        alert(`Failed to set key repeat: ${error.message}`);
        e.target.checked = !e.target.checked; 
      }
    });
    
    keyRepeatDelaySelect.addEventListener('change', async (e) => {
      try {
        const delay = parseInt(e.target.value);
        const rate = parseInt(keyRepeatRateSelect.value);
        await window.electronAPI.setKeyRepeatRate(delay, rate);
      } catch (error) {
        console.error('Error setting key repeat rate:', error);
        alert(`Failed to set key repeat rate: ${error.message}`);
      }
    });
    
    keyRepeatRateSelect.addEventListener('change', async (e) => {
      try {
        const delay = parseInt(keyRepeatDelaySelect.value);
        const rate = parseInt(e.target.value);
        await window.electronAPI.setKeyRepeatRate(delay, rate);
      } catch (error) {
        console.error('Error setting key repeat rate:', error);
        alert(`Failed to set key repeat rate: ${error.message}`);
      }
    });
    
    keyboardLayoutSelect.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.setKeyboardLayout(e.target.value);
      } catch (error) {
        console.error('Error setting keyboard layout:', error);
        alert(`Failed to set keyboard layout: ${error.message}`);
        
        keyboardLayoutSelect.value = settings.layout || 'us';
      }
    });
  } catch (error) {
    console.error('Error loading keyboard settings:', error);
  }
}

async function initTrackpadPage() {
  const tapToClickToggle = document.getElementById('tap-to-click-toggle');
  const naturalScrollingToggle = document.getElementById('natural-scrolling-toggle');
  const twoFingerScrollingToggle = document.getElementById('two-finger-scrolling-toggle');
  const trackpadSpeedSelect = document.getElementById('trackpad-speed-select');
  
  if (!tapToClickToggle || !naturalScrollingToggle || !twoFingerScrollingToggle || !trackpadSpeedSelect || !window.electronAPI) return;
  
  try {
    
    const settings = await window.electronAPI.getTrackpadSettings();
    
    
    tapToClickToggle.checked = settings.tapToClick;
    naturalScrollingToggle.checked = settings.naturalScrolling;
    twoFingerScrollingToggle.checked = settings.twoFingerScrolling;
    
    
    const speed = settings.speed;
    if (speed <= 0.75) {
      trackpadSpeedSelect.value = '0.5';
    } else if (speed <= 1.25) {
      trackpadSpeedSelect.value = '1.0';
    } else if (speed <= 1.75) {
      trackpadSpeedSelect.value = '1.5';
    } else {
      trackpadSpeedSelect.value = '2.0';
    }
    
    
    tapToClickToggle.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.setTapToClick(e.target.checked);
      } catch (error) {
        console.error('Error setting tap to click:', error);
        alert(`Failed to set tap to click: ${error.message}`);
        e.target.checked = !e.target.checked; 
      }
    });
    
    naturalScrollingToggle.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.setNaturalScrolling(e.target.checked);
      } catch (error) {
        console.error('Error setting natural scrolling:', error);
        alert(`Failed to set natural scrolling: ${error.message}`);
        e.target.checked = !e.target.checked; 
      }
    });
    
    twoFingerScrollingToggle.addEventListener('change', async (e) => {
      try {
        await window.electronAPI.setTwoFingerScrolling(e.target.checked);
      } catch (error) {
        console.error('Error setting two-finger scrolling:', error);
        alert(`Failed to set two-finger scrolling: ${error.message}`);
        e.target.checked = !e.target.checked; 
      }
    });
    
    trackpadSpeedSelect.addEventListener('change', async (e) => {
      try {
        const speed = parseFloat(e.target.value);
        await window.electronAPI.setTrackpadSpeed(speed);
      } catch (error) {
        console.error('Error setting trackpad speed:', error);
        alert(`Failed to set trackpad speed: ${error.message}`);
      }
    });
  } catch (error) {
    console.error('Error loading trackpad settings:', error);
  }
}

  async function initTouchIdPasswordPage() {
    const changePasswordButton = document.getElementById('change-password-button');
    const changePasswordModal = document.getElementById('change-password-modal');
    const changePasswordCancel = document.getElementById('change-password-cancel');
    const changePasswordSubmit = document.getElementById('change-password-submit');
    const oldPasswordInput = document.getElementById('old-password-input');
    const newPasswordInput = document.getElementById('new-password-input');
    const verifyPasswordInput = document.getElementById('verify-password-input');
    const touchIdStatus = document.getElementById('touch-id-status');
    
    if (!changePasswordButton || !changePasswordModal) return;
    
    
    if (touchIdStatus && window.electronAPI) {
      try {
        const [deviceCheck, fingerprints] = await Promise.all([
          window.electronAPI.checkFingerprintDevice(),
          window.electronAPI.listFingerprints()
        ]);
        
        if (!deviceCheck.hasDevice) {
          touchIdStatus.innerHTML = '<div style="color: var(--text-secondary);">No Touch ID device detected</div>';
        } else if (fingerprints.fingerprints && fingerprints.fingerprints.length > 0) {
          
          const currentUsername = fingerprints.fingerprints[0]?.username || 'user';
          
          
          const fingerprintsHtml = fingerprints.fingerprints.map((fp, index) => {
            
            const fingerName = `Finger ${index + 1}`;
            const actualFinger = fp.finger || 'left-index-finger'; 
            const username = fp.username || currentUsername;
            return `
              <div class="fingerprint-item" style="display: flex; flex-direction: column; align-items: center; gap: 8px; position: relative; justify-content: flex-start;">
                <div style="position: relative; display: inline-block; width: 48px; height: 48px;">
                  <img src="./assets/fingerprint.svg" alt="Fingerprint" class="fingerprint-icon" style="width: 48px; height: 48px; opacity: 0.8;">
                  <button class="delete-fingerprint-btn" data-username="${username}" data-finger="${actualFinger}" style="position: absolute; top: -6px; left: -6px; width: 20px; height: 20px; border-radius: 50%; background-color: #6B7280; border: none; color: white; font-size: 12px; display: none; align-items: center; justify-content: center; cursor: pointer; transition: all 0.2s ease; -webkit-app-region: no-drag; z-index: 10; padding: 0; line-height: 1;">×</button>
                </div>
                <div style="font-size: 13px; color: var(--text-primary); text-align: center;">${fingerName}</div>
              </div>
            `;
          }).join('');
          
          
          const addButtonHtml = `
            <div style="display: flex; flex-direction: column; align-items: center; gap: 8px; justify-content: flex-start;">
              <div style="width: 48px; height: 48px; display: flex; align-items: center; justify-content: center;">
                <button id="add-fingerprint-button" style="width: 48px; height: 48px; border-radius: 50%; border: 2px solid var(--border-light); background-color: var(--bg-primary); color: var(--text-primary); font-size: 20px; display: flex; align-items: center; justify-content: center; cursor: pointer; transition: all 0.2s ease; -webkit-app-region: no-drag; padding: 0;" onmouseover="this.style.borderColor='#007AFF'; this.style.backgroundColor='var(--hover-bg)'" onmouseout="this.style.borderColor='var(--border-light)'; this.style.backgroundColor='var(--bg-primary)'">+</button>
              </div>
              <div style="font-size: 13px; color: var(--text-primary); text-align: center;">Add Fingerprint</div>
            </div>
          `;
          
          
          const noticeHtml = `
            <div style="width: 100%; margin-top: 20px; padding: 12px; background-color: var(--bg-secondary); border-radius: 8px; text-align: left;">
              <p style="margin: 0; font-size: 12px; color: var(--text-secondary); line-height: 1.4;">
                Most fingerprint sensors accept only one fingerprint in their memory and that's why you cannot enroll new fingerprints.
              </p>
            </div>
          `;
          
          touchIdStatus.innerHTML = fingerprintsHtml + addButtonHtml + noticeHtml;
          
          
          const deleteButtons = touchIdStatus.querySelectorAll('.delete-fingerprint-btn');
          deleteButtons.forEach(btn => {
            btn.addEventListener('click', async (e) => {
              e.stopPropagation();
              const username = btn.getAttribute('data-username');
              const finger = btn.getAttribute('data-finger');
              
              try {
                if (window.electronAPI && window.electronAPI.deleteFingerprint) {
                  await window.electronAPI.deleteFingerprint(username, finger);
                  
                  setTimeout(() => {
                    initTouchIdPasswordPage();
                  }, 300);
                } else {
                  alert('Delete functionality is not available');
                }
              } catch (error) {
                console.error('Error deleting fingerprint:', error);
                alert(`Failed to delete fingerprint: ${error.message}`);
              }
            });
          });
          
          
          const fingerprintItems = touchIdStatus.querySelectorAll('.fingerprint-item');
          fingerprintItems.forEach(item => {
            const deleteBtn = item.querySelector('.delete-fingerprint-btn');
            const fingerprintIcon = item.querySelector('.fingerprint-icon');
            
            item.addEventListener('mouseenter', () => {
              if (deleteBtn) {
                deleteBtn.style.display = 'flex';
              }
            });
            
            item.addEventListener('mouseleave', () => {
              if (deleteBtn) {
                deleteBtn.style.display = 'none';
              }
            });
          });
          
          
          const addButton = document.getElementById('add-fingerprint-button');
          if (addButton) {
            addButton.addEventListener('click', () => {
              openEnrollModal();
            });
          }
        } else {
          touchIdStatus.innerHTML = `
            <div style="display: flex; flex-direction: column; align-items: center; gap: 15px;">
              <div style="color: var(--text-secondary);">No fingerprints enrolled</div>
              <button class="update-button" id="enroll-fingerprint-button">Enroll</button>
            </div>
          `;
          
          
          const enrollButton = document.getElementById('enroll-fingerprint-button');
          if (enrollButton) {
            enrollButton.addEventListener('click', () => {
              openEnrollModal();
            });
          }
        }
      } catch (error) {
        console.error('Error loading Touch ID status:', error);
        touchIdStatus.innerHTML = '<div style="color: var(--text-secondary);">Error loading Touch ID status</div>';
      }
    }
    
    
    changePasswordButton.addEventListener('click', () => {
      changePasswordModal.style.display = 'flex';
      oldPasswordInput.value = '';
      newPasswordInput.value = '';
      verifyPasswordInput.value = '';
      oldPasswordInput.focus();
    });
    
    
    changePasswordCancel.addEventListener('click', () => {
      changePasswordModal.style.display = 'none';
      oldPasswordInput.value = '';
      newPasswordInput.value = '';
      verifyPasswordInput.value = '';
    });
    
    
    changePasswordModal.addEventListener('click', (e) => {
      if (e.target === changePasswordModal) {
        changePasswordModal.style.display = 'none';
        oldPasswordInput.value = '';
        newPasswordInput.value = '';
        verifyPasswordInput.value = '';
      }
    });
    
    
    changePasswordSubmit.addEventListener('click', async () => {
      const oldPassword = oldPasswordInput.value;
      const newPassword = newPasswordInput.value;
      const verifyPassword = verifyPasswordInput.value;
      
      
      if (!oldPassword || !newPassword || !verifyPassword) {
        alert('Please fill in all fields');
        return;
      }
      
      if (newPassword !== verifyPassword) {
        alert('New password and verify password do not match');
        return;
      }
      
      if (newPassword.length < 1) {
        alert('Password must be at least 1 character long');
        return;
      }
      
      try {
        if (window.electronAPI && window.electronAPI.changePassword) {
          await window.electronAPI.changePassword(oldPassword, newPassword);
          alert('Password changed successfully');
          changePasswordModal.style.display = 'none';
          oldPasswordInput.value = '';
          newPasswordInput.value = '';
          verifyPasswordInput.value = '';
        } else {
          alert('Password change functionality is not available');
        }
      } catch (error) {
        console.error('Error changing password:', error);
        alert(`Failed to change password: ${error.message}`);
      }
    });
    
    
    [oldPasswordInput, newPasswordInput, verifyPasswordInput].forEach(input => {
      input.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
          changePasswordSubmit.click();
        }
      });
    });
  }

  
  async function getNextAvailableFinger() {
    try {
      if (!window.electronAPI || !window.electronAPI.listFingerprints) {
        return 'left-index-finger'; 
      }
      
      const result = await window.electronAPI.listFingerprints();
      const enrolledFingers = result.fingerprints.map(fp => fp.finger);
      
      
      const allFingers = [
        'left-index-finger',
        'right-index-finger',
        'left-middle-finger',
        'right-middle-finger',
        'left-ring-finger',
        'right-ring-finger',
        'left-little-finger',
        'right-little-finger',
        'left-thumb',
        'right-thumb'
      ];
      
      
      for (const finger of allFingers) {
        if (!enrolledFingers.includes(finger)) {
          return finger;
        }
      }
      
      
      return allFingers[0];
    } catch (error) {
      console.error('Error getting next available finger:', error);
      return 'left-index-finger'; 
    }
  }

  
  let enrollModalOpen = false;
  async function openEnrollModal() {
    if (enrollModalOpen) return; 
    
    const enrollModal = document.getElementById('enroll-fingerprint-modal');
    const enrollCancel = document.getElementById('enroll-fingerprint-cancel');
    const enrollStatus = document.getElementById('enroll-status');
    
    if (!enrollModal || !enrollCancel || !enrollStatus) return;
    
    enrollModalOpen = true;
    enrollModal.style.display = 'flex';
    enrollStatus.innerHTML = 'Starting enrollment process...';
    
    
    const closeModal = () => {
      enrollModal.style.display = 'none';
      enrollModalOpen = false;
    };
    
    
    enrollCancel.onclick = closeModal;
    
    
    enrollModal.onclick = (e) => {
      if (e.target === enrollModal) {
        closeModal();
      }
    };
    
    
    try {
      if (window.electronAPI && window.electronAPI.enrollFingerprint) {
        
        const nextFinger = await getNextAvailableFinger();
        
        enrollStatus.innerHTML = `
          <div style="display: flex; flex-direction: column; align-items: center; gap: 15px;">
            <img src="./assets/fingerprint.svg" alt="Fingerprint" style="width: 80px; height: 80px; opacity: 0.8;">
            <div style="text-align: center;">
              <div style="font-size: 14px; color: var(--text-primary); margin-bottom: 8px; font-weight: 500;">Place your finger on the scanner</div>
              <div style="font-size: 12px; color: var(--text-secondary);">Keep your finger on the scanner until enrollment is complete.</div>
            </div>
          </div>
        `;
        
        
        let enrollStage = 0;
        
        
        if (window.electronAPI.onEnrollFingerprintOutput) {
          window.electronAPI.onEnrollFingerprintOutput((data) => {
            const output = data.toLowerCase();
            
            
            if (output.includes('enroll-stage-passed') || output.includes('enroll result: enroll-stage-passed')) {
              enrollStage++;
              enrollStatus.innerHTML = `
                <div style="display: flex; flex-direction: column; align-items: center; gap: 15px;">
                  <img src="./assets/fingerprint.svg" alt="Fingerprint" style="width: 80px; height: 80px; opacity: 0.8;">
                  <div style="text-align: center;">
                    <div style="font-size: 14px; color: var(--text-primary); margin-bottom: 8px; font-weight: 500;">Place your finger on the scanner again</div>
                    <div style="font-size: 12px; color: var(--text-secondary);">Keep your finger on the scanner until enrollment is complete.</div>
                  </div>
                </div>
              `;
            }
            
            else if (output.includes('enroll-completed') || output.includes('enroll result: enroll-completed')) {
              enrollStatus.innerHTML = `
                <div style="display: flex; flex-direction: column; align-items: center; gap: 15px;">
                  <div style="color: #28a745; font-size: 16px; font-weight: 500;">✓ Enrollment completed!</div>
                </div>
              `;
              
              setTimeout(() => {
                initTouchIdPasswordPage();
              }, 500);
            }
            
            else if (output.includes('swipe') || output.includes('scan') || output.includes('place')) {
              enrollStatus.innerHTML = `
                <div style="display: flex; flex-direction: column; align-items: center; gap: 15px;">
                  <img src="./assets/fingerprint.svg" alt="Fingerprint" style="width: 80px; height: 80px; opacity: 0.8;">
                  <div style="text-align: center;">
                    <div style="font-size: 14px; color: var(--text-primary); margin-bottom: 8px; font-weight: 500;">Scanning your fingerprint...</div>
                    <div style="font-size: 12px; color: var(--text-secondary);">Keep your finger steady on the scanner.</div>
                  </div>
                </div>
              `;
            }
          });
        }
        
        const result = await window.electronAPI.enrollFingerprint(nextFinger);
        
        
        if (window.electronAPI.removeEnrollFingerprintOutput) {
          window.electronAPI.removeEnrollFingerprintOutput();
        }
        
        if (result.success) {
          enrollStatus.innerHTML = `
            <div style="display: flex; flex-direction: column; align-items: center; gap: 15px;">
              <div style="color: #28a745; font-size: 16px; font-weight: 500;">✓ Fingerprint enrolled successfully!</div>
            </div>
          `;
          
          setTimeout(() => {
            initTouchIdPasswordPage();
          }, 500);
          setTimeout(() => {
            closeModal();
          }, 2000);
        } else if (result.cancelled) {
          enrollStatus.innerHTML = '<div style="color: var(--text-secondary);">Enrollment cancelled.</div>';
          setTimeout(() => {
            closeModal();
          }, 2000);
        } else {
          enrollStatus.innerHTML = `<div style="color: #dc3545;">Enrollment failed: ${result.message || 'Unknown error'}</div>`;
        }
      } else {
        enrollStatus.innerHTML = '<div style="color: #dc3545;">Enrollment functionality is not available.</div>';
      }
    } catch (error) {
      console.error('Error enrolling fingerprint:', error);
      enrollStatus.innerHTML = `<div style="color: #dc3545;">Error: ${error.message}</div>`;
      if (window.electronAPI.removeEnrollFingerprintOutput) {
        window.electronAPI.removeEnrollFingerprintOutput();
      }
    }
  }

});