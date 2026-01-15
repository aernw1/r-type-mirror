# Breakout

Breakout game demonstrating the reusability of the R-Type game engine.

## Objective

Destroy all bricks by bouncing the ball with the paddle.

## Controls

- **Left Arrow** : Move the paddle to the left
- **Right Arrow** : Move the paddle to the right
- **Escape** : Quit the game

## Game Mechanics

- **Ball** : Bounces off walls (left, right, top) and the paddle
- **Paddle** : The bounce angle depends on where the ball hits the paddle
- **Bricks** : A brick is destroyed in one hit
- **Acceleration** : The ball gradually accelerates over time
- **Ball Loss** : If the ball goes out at the bottom, it reappears at the center with a random downward direction
- **Score** : Increases with each brick destroyed
- **Victory** : When all bricks are destroyed, a "WIN" message appears

## Compilation

```bash
cd build
make breakout
./breakout
```

