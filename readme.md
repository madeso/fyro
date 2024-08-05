# Fyro

[![windows](https://github.com/madeso/fyro/actions/workflows/windows.yml/badge.svg)](https://github.com/madeso/fyro/actions/workflows/windows.yml)
[![linux](https://github.com/madeso/fyro/actions/workflows/linux.yml/badge.svg)](https://github.com/madeso/fyro/actions/workflows/linux.yml)

Fyrö or fyro is a simple 2d rendering opengl playground/engine.

Quad or quadliteral translated to swedish is fyrhörning. There isn't any accepted short form fyrhörning so I picked the first four letters that made sense and sounded good.

Procounce it as either fyro: "FIR (as in fire) -Oh" or as fyrö: "Phyr(phy as in physical) -Uh (as in Uhm)"

Fyrö also means lightouse island but that's not related but it could be the base of a logo or something :)

## todo (no specific order)

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

-   add sound and music
-   change examples to data driven with scripting similar to love2d
-   extend api to something similar to flixel
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

-   port flixel samples?
-   port to webgl, android, raspberry pi
-   port gamejam games for more samples
-   port "16 games in c++ sfml" for more samples?
