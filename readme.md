# Fyro

[![windows](https://github.com/madeso/fyro/actions/workflows/windows.yml/badge.svg)](https://github.com/madeso/fyro/actions/workflows/windows.yml)
[![linux](https://github.com/madeso/fyro/actions/workflows/linux.yml/badge.svg)](https://github.com/madeso/fyro/actions/workflows/linux.yml)

Fyrö or fyro is a simple 2d engine.

## The Name

Quad or quadliteral translated to swedish is fyrhörning. There isn't any accepted short form fyrhörning so I picked the first four letters that made sense and sounded good.

Procounce it as either fyro: "FIR (as in fire) -Oh" or as fyrö: "Phyr(phy as in physical) -Uh (as in Uhm)"

Fyrö also means lightouse island but that's not related but it could be the base of a logo or something :)

## The Design and Goal

The overarching goal of the engine is to be able to quiclky create a game from some assets that was just released. This means that there should be some some template system helping you create code, hot reloading code and assets while the game is running and editor comfort to make tcreating code quickly. We are currently not here, there is a long way to go.

The design goal is to have a "runner" similar to love2d but perhaps behaving more like znes and have the actual gamnes be a single file like love2d or pico8. The api design would be closer to how flixel/game maker works/worked with a state class that contain a level that contain objects.

The znes inspiration means that the input from the engine to the game file would be seen as some sort of gamepad similar to how a snes game would see only a snes controller. In future development there is a desire to add support for mouse or touch based games but I'm unsure how this would fit into the gamepad model.

Given the znes inspiration the current design is also geared towards "pixel" games but besides the controller and only supporting tile based levels there is no restriction on this. Adding a phsyics system like box2d, spline based levels and spine import is within the scope of the current design and exist on some roadmap.

## The Todo list (no specific order)

(if the project gain contributors this needs to be turned into github issues but for now a list in markdown works just fine)

-   clean up code and remove not used things
-   expand current collision handling to support moving platforms
-   add collision handling between actors

```lox
on_collision((_: Player, coin: Coin) => {
  coin.destroy();
});
on_collision(player: Player, enemy: Enemy) => {
  if(player.is_falling && over_enemy) {
    player.bounce();
    enemy.die();
  } else {
    player.push_away();
    player.hurt();
    enemy.attack_anim();
  }
});
```

-   replace OpenGL rendering with the SDL 2d renderer. This would probably make the engine easier to port but might remove shader support. We might want 2d normal maps, basic water effects or some 2d lightning later...
-   add sound and music
-   name input so we can assure player1 is the same in meny as in game
-   change input? to support mouse only games (cpp samples) and dual-stick shooters with "mouselook"
-   add hot reload
-   add data "screenshot" dump of the last x seconds, with debugging, logging, log diff and replayability
-   get debugging ideas from
    -   [Tomorrow Corporation Tech Demo](https://www.youtube.com/watch?v=72y2EC5fkcE)
    -   [Bret Victor - Inventing on Principle](https://www.youtube.com/watch?v=PUv66718DII)
-   add serialization/savefiles
-   add a quick hiscore
-   add a quick menu system
-   add support for both pixel and rotate phsycis games
-   add gif/mp4 recording of the last X seconds

-   port flixel samples? like mode https://github.com/HaxeFlixel/flixel-demos/tree/dd0afb866ed57b0507e8679d191ce68d4a8e2407/Platformers/Mode https://github.com/AdamAtomic/Mode
-   port to webgl, android, raspberry pi
-   port gamejam games for more samples
-   port "16 games in c++ sfml" for more samples?
