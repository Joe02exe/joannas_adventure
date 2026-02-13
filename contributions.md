# Contributions

**Martin Berktold**:

- Menu
  - src/systems/ui/menu.cpp
  - include/joanna/systems/menu.h
- Savegame Manager
  - src/core/savegamemanager.cpp 
  - include/joanna/core/savegamemanager.h
- Inventory System
  - src/game/inventory/inventory.cpp
  - include/joanna/entities/inventory.h
- Window Manager
- Logger
- Resource Manager
- ImGUI
- CI pipeline

**Philipp Dablander**:
- NPCs - src/entities/npcs/npc.cpp, include/joanna/entities/npc.h, assets/dialog/dialog.json, assets/player/npc
- Dialogue box - src/systems/ui/dialogue_box.cpp, include/joanna/utils/dialogue_box.h
- Stats - src/entities/components/stats.cpp, include/joanna/entities/stats.h, src/entities/player/player.cpp:178-206, Stats influencing combat
- Section 2 (Desert)
- Damage indicator
- Random spawning skeletons
- Items influencing stats (Sword, Shield)
- Some other sprites (Shield, Key, half heart, BIG CACTUS, Bone, Final Treasure)

**Leonhard Kohl Lörting**:
- Sound Effects & Music, Audio Manager:
    - Self composed music and sound effects using 8-bit synthesizer.
    - src/systems/audio/audiomanager.cpp & .h (all lines)
- Rendering & Post-Processing:
    - Post-processing CRT shader for pixelated retro look.
    - assets/shader/crt_shader.frag (all lines)
    - src/core/renderer/postprocessing.cpp & .h (all lines)
    - Basic font rendering system for UI text.
    - src/systems/ui/font_renderer.cpp &.h (all lines)
- UI:
    - Stats visual rendering.
    - src/entities/components/stats.cpp (draw function)
    - Small main menu overhaul, improving visual appeal.
    - src/systems/ui/menu.cpp
- Direction in finding usable asset pack, general planning.
- Testing in terms of analyzing UI clarity and player communication.
  
**Johannes Hans Karl Schneider**:
- Combat System: 
    - src/game/combat/combat_system.cpp (0-200, 230-470)
    - include/joanna/game/combat/combat_system.h (0-108)
- Enemy and enemy AI: 
    - src/entities/enemies/enemy.cpp (all lines)
    - include/joanna/entities/enemy.h (all lines)
- Gameover Screen: 
    - src/systems/ui/gameover.cpp (all lines)
    - include/joanna/systems/gameover.h (all lines)
- Tilemanager: Collision system and Y-Sorting
- NPCs & Interaction Button
- Interactables (Chest, Stone, NPCs), Entity class
- Game loop
- Map (Overworld + Cave)
- Sprites (Enemies, Player)
