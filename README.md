# ICP – Interakce člověka s počítačem

Semestrální projekt do předmětu ICP.

## Téma projektu: 3D Tetris s efektem okna

> **Stav:** návrh tématu, zatím jen prototyp vizuálního efektu (`prototypes/window_effect`).

Hra typu 3D Tetris (podle klasické hry *Blockout*). Hráč se dívá shora do šachty,
která vede „dovnitř“ monitoru, dílky do ní padají směrem od hráče a cílem je
zaplňovat celé vrstvy.

### Efekt okna (head-coupled perspective)

Webkamera sleduje polohu hlavy hráče a scéna se v každém snímku vykreslí tak, jak
by byla vidět z jeho místa skrz monitor jako skrz okno:

- okraj šachty zůstává přilepený k okrajům obrazovky (rám okna se nehýbe),
- vnitřek šachty se při pohybu hlavy posouvá. Čím hlouběji, tím víc (pohybová paralaxa),
- hráč díky tomu vidí hloubku a může „nakouknout“ do stran šachty.

Efekt řeší hlavní problém 3D Tetrisu: na plochém monitoru se špatně odhaduje,
ve které vrstvě je padající dílek a jestli se trefí do díry.

Technicky jde o mimoosou projekci (`glm::frustum`), při které je obdélník obrazovky
pevný a mění se jen poloha oka.

### Interakce

| Vstup | Zdroj | Akce |
| --- | --- | --- |
| poloha hlavy | kamera | pohled do šachty |
| barevný předmět / ruka | kamera | posun dílku |
| tlesknutí, hlasitost, tón | mikrofon | rotace dílku, rychlý pád |
| odchod od kamery | kamera (detekce obličeje) | pauza |
| klávesnice, myš | události GLFW | záložní ovládání, menu |

### Požadavky předmětu, na které projekt odpovídá

- **Nízká odezva:** latence od pohybu hlavy po změnu obrazu je přímo vidět,
  bude se měřit a zobrazovat.
- **Vícevláknový běh:** snímání kamery, sledování (tracking), zpracování zvuku
  a vykreslování běží v samostatných vláknech.
- **Více vstupů:** kamera, mikrofon a události klávesnice a myši.

Plánovaná architektura vláken:

```text
[vlákno kamery] ──snímek──▶ [vlákno trackeru] ──výsledek──┐
[audio callback] ──vzorky──▶ [analýza zvuku] ─────────────┤
                                                          ▼
                    [hlavní vlákno: události GLFW + OpenGL render + imgui]
```

## Struktura repozitáře

| Složka | Obsah |
| --- | --- |
| `src/` | semestrální projekt (program `icp`), zatím jen kostra |
| `prototypes/` | prototypy nápadů. Každý `.cpp` je samostatný program |
| `labs/` | řešení cvičení (`01lab.cpp`, `02lab.cpp`, …). Každý `.cpp` je samostatný program |
| `resources/` | obrázky, video a nastavení, která programy načítají |

Nový soubor `.cpp` ve složce `labs/` nebo `prototypes/` se automaticky stane
samostatným programem se stejným názvem. Do CMake není potřeba nic dopisovat.

Podklady od vyučujícího (`cviceni/`, `prednasky/`) nejsou v gitu.

## Prototyp: efekt okna

`prototypes/window_effect.cpp`: šachta s mřížkou, několik kostek a padající dílek.
Polohu hlavy zatím simuluje myš.

| Ovládání | |
| --- | --- |
| pohyb myši | poloha hlavy před monitorem |
| kolečko myši | vzdálenost hlavy od monitoru |
| mezerník | efekt zap/vyp (porovnání s obyčejnou kamerou) |
| ESC | konec |

Efekt je nejvýraznější, když se díváte jedním okem.

## Cvičení

| Program | Obsah |
| --- | --- |
| `01lab` | ukázka knihoven (OpenCV, GLFW/GLEW, glm, JSON) |
| `02lab` | hledání objektu podle jasu a podle barvy (HSV), sledování červeného objektu v kameře / videu |

## Sestavení

Projekt používá CMake (≥ 4.0), presety v `CMakePresets.json` a vcpkg pro knihovny
GLFW, GLEW, glm, imgui a nlohmann-json. OpenCV se instaluje zvlášť.

### Požadavky

- CMake 4.0 nebo novější, Ninja
- [vcpkg](https://github.com/microsoft/vcpkg) a proměnná prostředí `VCPKG_ROOT`,
  která ukazuje na jeho složku
- OpenCV:
  - **Windows:** stáhnout z [opencv.org](https://opencv.org/releases)
    a rozbalit do `C:\OpenCV`
    (jinou cestu lze zadat přes `-DOpenCV_DIR=...`). Složku s `opencv_world*.dll`
    (např. `C:\OpenCV\build\x64\vc16\bin`) je potřeba přidat do `PATH`.
  - **Linux:** systémový balíček (`opencv-devel` na Fedoře, `libopencv-dev` na Ubuntu)
    a vývojové balíčky X11/Wayland a OpenGL, které potřebuje GLFW.
- **Windows:** překladač MSVC (Visual Studio nebo Build Tools
  s „Desktop development with C++“).
  MinGW nefunguje, protože OpenCV z opencv.org je zkompilované pro MSVC.

První konfigurace trvá dlouho, protože vcpkg stahuje a kompiluje knihovny.

### Příkazová řádka

Spouštějte z kořene repozitáře. Na Windows v **Developer PowerShell for VS**:

```bash
cmake --preset vcpkg-debug
cmake --build build/debug --target window_effect
```

Programy se spouštějí **z kořene repozitáře**, aby našly složku `resources/`:

```bash
./build/debug/prototypes/window_effect      # Linux
build\debug\prototypes\window_effect.exe    # Windows
```

Bez `--target` se sestaví všechno. Pro měření výkonu použijte preset `vcpkg-release`
(výstup v `build/release`).

### VS Code

Rozšíření CMake Tools → `Ctrl+Shift+P`:

1. **CMake: Select Configure Preset** → `vcpkg-debug`
2. **CMake: Set Build Target** → např. `window_effect`
3. **CMake: Build** (`F7`) a **CMake: Run Without Debugging** (`Shift+F5`)

### CLion

1. *Settings → Build, Execution, Deployment → Toolchains:* na Windows přidat
   **Visual Studio** a přesunout ho nahoru. Zkontrolovat, že verze CMake je
   aspoň 4.0.
2. *Settings → … → CMake:* povolit profil `vcpkg-debug` (načte se z presetů)
   a výchozí profil `Debug` vypnout.
3. Po nastavení `VCPKG_ROOT` CLion restartovat.
4. U programů z `labs/` nastavit v *Run → Edit Configurations* položku
   **Working directory** na `$ProjectFileDir$`.
