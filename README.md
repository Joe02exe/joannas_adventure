# Advanced CPP Project

## Team

## Setup

### Install vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git
```

```bash
cd vcpkg && ./bootstrap-vcpkg.sh
```

Create a file 'CMakeUserPresets.json'

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "default",
      "inherits": "vcpkg",
      "environment": {
        "VCPKG_ROOT": "<path to vcpkg>"
      }
    }
  ]
}
```

### Ubuntu

```bash
sudo apt install \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libfreetype-dev
```

## Build

```bash
cmake --preset=default
cmake --build build
```

## Roadmap

## Specifications

- (2) Overworld with **3** different sections:
  - Each with its own tone (architecture, sprites, music, enemies, …)
  - The second is only accessible after some story progress
  - Player can roam the world and interact with other entities
- (1) Characters
  - Player can talk to other characters
- (1) Resources
  - Player can manage acquired resources through dedicated menus
- (1) Stats
  - Player's combatants have stats that influence the combat
  - Player's combatants get experience from combat, increasing their stats
  - Stats are influenced by equipment
- (3) Combat
  - Turn-based
  - Player selects which attacks, spells, items, etc. to use on which target
  - Enemies use attacks, spells, items, etc. to combat the player
  - *Game Over* when all of player's combatants are dead
- (1) Save points
  - Player can save her progress at specific points in the game
  - Saved progress is persistent across play sessions (application termination)
- (1) Audio
  - Background music
  - Sound effects
- (1) Main menu
  - New game
  - Load game
  - Exit
- (2) Map
- (1) Minimap
