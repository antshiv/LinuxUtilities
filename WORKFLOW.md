# My Linux Workflow Philosophy

> **10+ years of refined, minimalist Linux setup - Programs, Software, Workspace**

## The Three-Directory Structure

My entire computing workflow revolves around three directories:

```
/home/antshiv/Programs/     → p    (Compiled binaries - production runtime)
/home/antshiv/Software/     → s    (Source code - third-party dependencies)
/home/antshiv/Workspace/    → w    (Active development - my projects)
```

### Shell Aliases

First thing I do on any Linux machine:

```bash
# Add to ~/.bashrc or ~/.zshrc
alias p='cd /home/antshiv/Programs'
alias s='cd /home/antshiv/Software'
alias w='cd /home/antshiv/Workspace'
```

**Result**: One-letter navigation. Muscle memory. No thinking required.

---

## The Philosophy

### Programs/ - Production Runtime

**Purpose**: Where compiled applications live and run from.

**Examples**:
```
Programs/
├── html/
│   └── antsand.com/           # ANTSAND web platform
├── php/                        # Custom PHP build
├── nginx/                      # Custom nginx build
└── bin/                        # Custom compiled tools
```

**Philosophy**:
- This is `/usr/local` or `/opt` for my home directory
- Only production-ready, compiled binaries
- This is what actually runs
- Can be rebuilt from Software/ at any time

### Software/ - Source Code Repository

**Purpose**: Third-party source code for building Programs/.

**Examples**:
```
Software/
├── php-8.3.0/                 # PHP source (tar.gz extracted)
├── nginx-1.24.0/              # nginx source
├── phalcon-5.0.0/             # Phalcon C extension source
└── various libraries...
```

**Philosophy**:
- This is `/usr/src` for my home directory
- Downloaded source tarballs, extracted
- Custom patches applied here
- Full control over build flags
- Can re-download if needed (not critical to backup)

**Why Not apt-get?**
- I control the version (not Ubuntu's schedule)
- I control the build flags (optimization, features)
- I can apply custom patches
- I understand the stack (no black boxes)
- Consistent across machines (Ubuntu vs Mint vs Debian)

### Workspace/ - Active Development

**Purpose**: My projects, active work, experiments.

**Examples**:
```
Workspace/
├── AeroDynControlRig/         # HIL test bench
├── C-Transformer/             # ML in pure C
├── controlSystems/            # Flight control algorithms
├── BLEDroneCode/              # nRF5340 firmware
├── antsand/                   # ANTSAND platform code
└── LinuxUtilities/            # This repository
```

**Philosophy**:
- This is where I create
- This is what I backup religiously
- This is my intellectual property
- This is version-controlled (git)
- This is what matters

---

## Build Workflow Example

**Installing PHP from source** (instead of `apt install php`):

```bash
# 1. Download source
cd ~/Software
wget https://www.php.net/distributions/php-8.3.0.tar.gz
tar xzf php-8.3.0.tar.gz
cd php-8.3.0

# 2. Configure with custom flags
./configure \
    --prefix=/home/antshiv/Programs/php \
    --enable-fpm \
    --with-mysqli \
    --with-pdo-mysql \
    --with-openssl \
    --with-zlib \
    --enable-mbstring

# 3. Build and install
make -j$(nproc)
make install

# 4. Result: custom PHP in Programs/
# /home/antshiv/Programs/php/bin/php --version
```

**Why this matters**:
- I can enable/disable features (no bloat)
- I can optimize for my CPU (march=native)
- I can upgrade on my schedule
- I can apply security patches immediately
- I understand what's running on my system

---

## The Stack: Where Everything Lives

### ANTSAND Platform
```
Programs/html/antsand.com/      ← Production web platform (Phalcon + MongoDB)
Software/phalcon-5.0.0/         ← Phalcon C extension source
Software/php-8.3.0/             ← PHP source
Software/nginx-1.24.0/          ← nginx source
Workspace/antsand/              ← ANTSAND development/config
```

### C Projects
```
Workspace/C-Transformer/        ← Pure C ML training (4,308 lines)
Workspace/controlSystems/       ← Flight control algorithms
Workspace/attitudeMathLibrary/  ← Quaternion math
Workspace/AeroDynControlRig/    ← HIL test bench
```

### Embedded Firmware
```
Workspace/BLEDroneCode/         ← nRF5340 drone firmware (Zephyr RTOS)
Workspace/BLEHandheldController/ ← Remote control firmware
Software/zephyr-sdk/            ← Zephyr toolchain (if needed)
```

---

## Backup Strategy

Because of this structure, backup is obvious:

### Critical (Must Backup)
- **Workspace/** - My code, my IP, irreplaceable
- **Programs/html/** - Websites, content, databases

### Nice to Have
- **Programs/config files** - Can recreate, but tedious

### Don't Need to Backup
- **Software/** - Can re-download source tarballs
- **Programs/binaries** - Can rebuild from Software/

**Backup command**:
```bash
# Rsync Workspace to external drive
rsync -avz --progress ~/Workspace/ /mnt/backup/Workspace/

# Rsync production websites
rsync -avz --progress ~/Programs/html/ /mnt/backup/html/
```

---

## Window Manager Setup

### AwesomeWM

**Why AwesomeWM?**
- Tiling window manager (keyboard-driven)
- Lua configuration (scriptable)
- Lightweight (no bloat)
- Full control (I decide the behavior)

**Configuration**: `rc.lua` in this repository

**Philosophy**: Minimal mouse usage, keyboard shortcuts for everything

### Terminator

**Why Terminator?**
- Split terminals horizontally/vertically
- Keyboard shortcuts for splits
- Multiple tabs
- Copy/paste that actually works

**Typical Layout**:
```
┌─────────────────────────────────────┐
│ Tab 1: Workspace                    │
├──────────────────┬──────────────────┤
│ w                │ git status       │
│ (active coding)  │ (version control)│
│                  │                  │
├──────────────────┴──────────────────┤
│ tail -f logs/error.log              │
│ (monitoring)                        │
└─────────────────────────────────────┘
```

**Common Workflow**:
- `Ctrl+Shift+E` - Split vertically
- `Ctrl+Shift+O` - Split horizontally
- `Alt+Arrow` - Navigate between splits
- `Ctrl+Shift+W` - Close split

---

## Why This Setup Works

### 1. **Consistent Across Machines**
- Same three directories on every Linux machine
- Same aliases (`p`, `s`, `w`)
- Same mental model
- No "where did I put that?" confusion

### 2. **Portable Knowledge**
- I know exactly what version of PHP I'm running (I built it)
- I know exactly what modules are enabled (I configured them)
- I know exactly where things are (Programs/Software/Workspace)
- No distribution-specific quirks

### 3. **Clean Separation of Concerns**
- Development (Workspace) separate from production (Programs)
- Source (Software) separate from binaries (Programs)
- Clear dependencies (Software → Programs)
- Clear ownership (Workspace = mine, Software = third-party)

### 4. **Minimal Cognitive Load**
- One letter to navigate (`p`, `s`, `w`)
- Predictable structure
- No need to remember complex paths
- Muscle memory after 10+ years

### 5. **Control Without Abstraction**
- Build from source = understand the stack
- No package manager surprises
- No "why did apt update break my PHP?"
- Custom optimizations (AVX-512, march=native)

---

## Matches Code Philosophy

This Linux setup **mirrors my code philosophy**:

| Linux Setup | Code Philosophy |
|-------------|-----------------|
| Build from source (Software/) | C-Transformer (pure C, no frameworks) |
| Custom PHP/nginx builds | ANTSAND (Phalcon C extension) |
| Three-directory structure | Clear architecture (layers) |
| 10+ years consistent | Production-tested, stable |
| Minimal abstractions | Zero abstraction cost |
| Full control | No black boxes |

**Both follow the same principle**:
> Understand what's under the hood. Build from first principles. Control the stack.

---

## Getting Started (New Machine Setup)

When I set up a new Linux machine:

```bash
# 1. Create directory structure
mkdir -p ~/Programs ~/Software ~/Workspace

# 2. Add aliases to shell config
echo "alias p='cd ~/Programs'" >> ~/.bashrc
echo "alias s='cd ~/Software'" >> ~/.bashrc
echo "alias w='cd ~/Workspace'" >> ~/.bashrc
source ~/.bashrc

# 3. Install AwesomeWM and Terminator
sudo apt install awesome terminator

# 4. Copy AwesomeWM config
cp ~/Workspace/LinuxUtilities/rc.lua ~/.config/awesome/rc.lua

# 5. Start building the stack
cd ~/Software
# Download and build PHP, nginx, etc.

# 6. Clone Workspace projects
cd ~/Workspace
git clone <repositories>

# Done. Same setup, every machine, every time.
```

---

## Evolution Over 10+ Years

**Early Days (2013-2015)**:
- Used Ubuntu package manager (apt-get)
- Frustrated by version conflicts
- "Why did my PHP update break my website?"

**Learning Phase (2015-2017)**:
- Started building from source
- Created Programs/ directory
- Realized I needed Software/ to keep sources

**Refinement (2017-2020)**:
- Added Workspace/ for clear separation
- Created `p`, `s`, `w` aliases
- Muscle memory developed

**Mature Setup (2020-Present)**:
- Same structure for 10+ years
- No thinking required
- Consistent across all machines
- Can set up new machine in <1 hour

---

## Tools in This Repository

Scripts that support this workflow:

- **rc.lua** - AwesomeWM configuration (window management)
- **import_screenshots.sh** - Manage screenshots/photos for AI workflows
- **redshift.sh** - Screen color temperature (reduce eye strain)
- **remove_duplicate_assets.sh** - Keep system clean
- **rescue-windows.sh** - Dual-boot recovery utilities

---

## Philosophy Summary

**Not Mumbo Jumbo - It's Unix Philosophy:**

1. **Separation of Concerns** (Programs/Software/Workspace)
2. **Build from Source** (understand the stack)
3. **Keyboard-Driven** (AwesomeWM + Terminator)
4. **Minimal Abstraction** (no package manager magic)
5. **Consistent Structure** (10+ years, same setup)

**Result**:
- Fast navigation (one letter: `p`, `s`, `w`)
- Full control (custom builds)
- Predictable behavior (same everywhere)
- Zero cognitive load (muscle memory)

**This is not a "Linux setup" - this is a philosophy applied to computing.**

Just like:
- C-Transformer is not "ML code" - it's first principles AI
- ANTSAND is not "a CMS" - it's conservation infrastructure
- This setup is not "directories" - it's a way of thinking about computing

---

## Related Documentation

- **[WHY_LINUX.md](WHY_LINUX.md)** - Why Linux is the most powerful OS for development
- **[README.md](README.md)** - Linux utilities and scripts for AwesomeWM workflow

---

**Last Updated**: 2025-10-26
**Years of Refinement**: 10+
**Philosophy**: Control the stack. Understand the system. Build from first principles.

*Everything has a place. Everything has a purpose. Everything is one keystroke away.*
