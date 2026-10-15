# Basic Solar System 2D: How to Run

This project is a C program that draws and animates a two-dimensional solar system with OpenGL and freeglut. These instructions apply only to this folder.

## What you need

- A C compiler (`gcc`)
- OpenGL and GLU development libraries
- freeglut development files
- A desktop session capable of opening a graphics window

The easiest Windows setup for this source is MSYS2 with its UCRT64 MinGW-w64 toolchain. The source includes `<unistd.h>` for its optional screenshot helper, so use the MSYS2 MinGW toolchain rather than Microsoft Visual C++.

## Windows: MSYS2 UCRT64

### 1. Install MSYS2 and the project tools

Install MSYS2 from [msys2.org](https://www.msys2.org/). Open the **MSYS2 UCRT64** terminal from the Start menu and install the compiler and freeglut:

```bash
pacman -Syu
```

If the update tells you to close the terminal, close it, reopen **MSYS2 UCRT64**, and run `pacman -Syu` again. Then install the packages:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-freeglut
```

### 2. Build and start the program

In the **MSYS2 UCRT64** terminal, change to this project's directory, compile `main.c`, and start the program:

```bash
cd "/c/Users/QRST/Downloads/GIT/OpenGL/Basic Solar System 2D"
gcc -O2 -o solar_2d.exe main.c -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32 -lm
./solar_2d.exe
```

The first command changes directory; the second creates `solar_2d.exe`; the third opens the graphics window. Keep the MSYS2 UCRT64 terminal open while the program runs.

## Linux: Ubuntu or Debian

Install the compiler and graphics development packages once:

```bash
sudo apt update
sudo apt install -y build-essential freeglut3-dev libglu1-mesa-dev mesa-common-dev
```

From a terminal, enter the project directory, compile, and run:

```bash
cd "/path/to/Basic Solar System 2D"
gcc -O2 -o solar_2d main.c -lglut -lGLU -lGL -lm
./solar_2d
```

Replace `/path/to/Basic Solar System 2D` with the actual location of this folder. A graphical desktop session is required; a text-only SSH session cannot display the window.

## Controls

| Input | Action |
|---|---|
| `ENTER` or `SPACE` | Start from the title screen |
| Click **START PROJECT** | Start from the title screen |
| `RIGHT` / `LEFT` arrow | Increase / decrease simulation speed |
| `+` or `=` / `-` or `_` | Increase / decrease speed |
| `P` | Pause or resume the animation |
| `O` | Show or hide orbit outlines |
| `L` | Show or hide planet labels |
| Click a planet | Select it and update the information at the top |
| `I` or `F1` | Open or close the information screen |
| `T` | Return to the title screen from the running scene |
| `ESC` | Quit |

The left-arrow and minus controls stop at speed zero. The right-arrow key is capped at 8x; the plus key is not capped in the current source.

## Optional startup screens

The executable accepts a few arguments useful when checking individual screens:

```bash
./solar_2d.exe --notitle
./solar_2d.exe --info
./solar_2d.exe --title
```

Use `./solar_2d` instead of `./solar_2d.exe` on Linux. `--notitle` opens the scene, `--info` opens the information screen, and `--title` explicitly keeps the title screen. Press `I` or `F1` to leave the information screen.


## Troubleshooting

| Problem | What to check |
|---|---|
| `GL/glut.h: No such file or directory` | Install the freeglut development package; on Windows, run the install command in the MSYS2 UCRT64 terminal. |
| `gcc` is not found | Use the MSYS2 UCRT64 terminal on Windows, or install `build-essential` on Ubuntu/Debian. |
| Undefined references to `glut...` | Keep the library flags after `main.c` in the compile command. |
| `libfreeglut.dll` is missing | Run from the MSYS2 UCRT64 terminal so its `ucrt64/bin` libraries are on the environment path. |
| The window does not open | Run from a local desktop session with a working graphics driver; a headless terminal has no display. |
| The picture looks stretched after resizing | The program uses a fixed 1000 by 700 coordinate system; the current reshape handler does not preserve the original aspect ratio. |
