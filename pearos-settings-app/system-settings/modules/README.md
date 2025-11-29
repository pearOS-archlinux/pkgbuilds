# Sistemul de Module pentru PearOS Settings

## Structura unui Modul

Fiecare modul trebuie să fie un director în `modules/` cu următoarea structură:

```
modules/
└── nume-modul/
    ├── manifest.module    # Manifestul modulului (OBLIGATORIU)
    ├── script.js          # Script-ul JavaScript al modulului (OPȚIONAL)
    └── assets/            # Resurse (icoane, imagini, etc.) (OPȚIONAL)
        └── ...
```

## Exemplu: Developer Settings Module

```
modules/
└── developer-settings/
    ├── manifest.module
    ├── script.js
    └── assets/
        └── developer-icon.svg
```

## Manifest.module

Manifestul definește:
- **__main**: Informații despre modul (name, description, developer, website)
- **__item**: Item-ul din sidebar (label, icon, page-id)
- **__page**: Pagina de setări cu toate elementele UI

### Exemple de elemente UI suportate:
- `section-title` - Titluri de secțiune
- `settings-card` - Card-uri de setări
- `card-section` - Secțiuni orizontale
- `toggle` / `switch` - Comutatoare (cu `on-change:`)
- `select` - Meniuri dropdown (cu `on-change:`)
- `button` - Butoane (cu `on-click:`)
- `input` - Câmpuri de text
- `radio-group` - Grupuri de radio buttons
- Și multe altele...

## Script.js

Fișierul `script.js` este **complet opțional**. Dacă există, este încărcat automat și poate conține funcții care vor fi apelate de elementele UI.

### API disponibil în script.js:

```javascript
// Obiectul 'module' este furnizat automat
module.moduleId          // ID-ul modulului
module.electronAPI       // API-ul Electron (dacă e disponibil)
module.getElement(id)    // Obține un element DOM
module.registerFunction(name, func)  // Înregistrează o funcție
```

### Exemplu de script.js:

```javascript
// Înregistrează o funcție pentru toggle
module.registerFunction('handleToggle', (checked, event, element) => {
  console.log('Toggle changed:', checked);
  // Logica ta aici
});

// Înregistrează o funcție pentru buton
module.registerFunction('handleButtonClick', (event, element) => {
  console.log('Button clicked!');
  // Logica ta aici
});

// Înregistrează o funcție pentru select
module.registerFunction('handleSelectChange', (value, event, element) => {
  console.log('Selected:', value);
  // Logica ta aici
});
```

### Cum se conectează funcțiile cu elementele UI:

În `manifest.module`:

```
toggle my-toggle checked
    on-change: "handleToggle"

button my-button
    "Click Me"
    on-click: "handleButtonClick"

select my-select
    option: "option1|Option 1" selected
    option: "option2|Option 2"
    on-change: "handleSelectChange"
```

## Crearea unui Modul Nou

1. **Creează directorul modulului** în `modules/`:
   ```bash
   mkdir modules/nume-modul
   ```

2. **Creează manifest.module**:
   ```
   __main
   name: "Nume Modul"
   description: "Descriere modul"
   developer: "Nume Developer"
   website: "https://website.com"
   
   __item
   label: "Nume în Sidebar"
   icon: "./assets/icon.svg"
   page-id: "nume-page"
   
   __page
   id: "nume-page"
   
   settings-card {
       card-section {
           label: "Setare"
           toggle my-toggle
               on-change: "handleToggle"
       }
   }
   ```

3. **Creează script.js** (opțional):
   ```javascript
   module.registerFunction('handleToggle', (checked) => {
       // Logica ta
   });
   ```

4. **Adaugă resurse** în `assets/` dacă e nevoie

5. **Gata!** Modulul va fi încărcat automat la următoarea pornire a aplicației.

## Note Importante

- ✅ Fiecare modul este **complet autonom** - nu trebuie să modifice codul aplicației principale
- ✅ Modulele pot fi **adăugate/șterse** fără să afecteze aplicația
- ✅ Fiecare modul are **propriul script.js** - nu există cod hardcodat
- ✅ Toate funcțiile trebuie să fie **înregistrate** cu `module.registerFunction()`
- ✅ Modulele **se validează automat** - dacă manifestul e invalid, nu se încarcă

## Securitate

- Modulele rulează în contextul aplicației principale
- Event handlers sunt verificați și apelați în mod sigur
- Recomandăm ca modulele să fie verificate înainte de instalare

