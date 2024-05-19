# Tower-of-Nguyen

A recreation of the Tower of Hanoi game for research purposes.

## Table of Contents

- [Screenshots](#screenshots)
  - [Four Disc Setup](#four-disc-setup)
  - [Seven Disc Setup](#seven-disc-setup)
  - [Background Color Change and Texture Toggled](#background-color-change-and-texture-toggled)
- [Demo](#demo)
- [Setup](#setup)
- [Interface](#interface)
- [License](#license)

## Screenshots

### Four Disc Setup

![four][four]

### Seven Disc Setup

![seven][seven]

### Background Color Change and Texture Toggled

![texture][texture]

[four]: public/assets/imgs/fourdisc.png
[seven]: public/assets/imgs/sevendisc.png
[texture]: public/assets/imgs/texturebackground.png

## Demo

## Setup

To get this application working locally, make sure to use a Linux distro. Simply follow these steps.

Navigate to the src folder

```
src\
```

Run make command

```sh
make
```

Run application

```sh
./draw
```

## Interface

To begin playing the game, you can press any of the following keys:

- Key "1" moves a disc from the first rod to the second.
- Key "2" moves a disc from the first rod to the third.
- Key "3" moves a disc from the second rod to the first.
- Key "4" moves a disc from the second rod to the third.
- Key "5" moves a disc from the third rod to the first.
- Key "6" moves a disc from the third rod to the first.
- Key "r" will allow the user to reset the positions of the discs and the score.
- Key "x" will rotate the scene 45 degrees about the x-axis.
- Key "y" will rotate the scene 45 degrees about the y-axis.
- Key "z" will rotate the scene 45 degrees about the z-axis.
- Key "x" will rotate the scene -45 degrees about the x-axis.
- Key "y" will rotate the scene -45 degrees about the y-axis.
- Key "z" will rotate the scene -45 degrees about the z-axis.
- Key "t" will turn on textures and change material color.
- Key "T" will turn off textures and revert material color.
- Key "a" will automate the game for the best moves, assuming 4 discs.
- Key "b" will change the background of the scene to white.
- Key "B" will change the background of the scene to black.
- Key "9" will cause the program to exit.

## License

MIT License
