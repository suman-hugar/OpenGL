# Basic Solar System 2D: Project Explained

This guide starts with the basics and follows the program from its first line to the animation loop. The project is implemented in `main.c` using C, OpenGL, GLU, and freeglut.

## 1. What the program does

When started, the program creates a window with a title screen. Press `ENTER`, `SPACE`, or the start button to see a starfield, the Sun, and six orbiting planets: Mercury, Venus, Earth, Mars, Jupiter, and Saturn. You can change the animation speed, hide the orbit lines or labels, pause the animation, and click a planet to select it. An information screen describes the project and its controls.

This is an educational illustration, not a scale model or a physical simulation. The distances, planet sizes, colors, orbit speeds, and flattened orbit shapes are chosen for a readable picture. The scene is two-dimensional: it does not calculate gravity, use 3D perspective, or draw persistent trails behind the planets.

## 2. A few ideas before reading the code

### Pixels and coordinates

The program draws using a coordinate area from `(0, 0)` to `(1000, 700)`. The bottom-left is `(0, 0)` and the top-right is `(1000, 700)`. The Sun is centered at `(500, 350)`. A position is an `(x, y)` pair; a radius is measured in these same screen units.

### OpenGL drawing commands

OpenGL receives instructions such as “draw a group of connected lines” or “fill this triangle fan.” `glBegin(...)` starts a shape, `glVertex2f(x, y)` adds a 2D point, and `glEnd()` finishes the shape. The color set with `glColor3f` or `glColor4f` applies to the vertices that follow. Color channels range from `0.0` to `1.0`.

This file uses the older, immediate-mode OpenGL interface. GLUT, supplied here by freeglut, creates the window and calls the program when it needs to draw or when the user presses a key or clicks the mouse.

### Angles and circular motion

The planet's current angle is stored in degrees. The C math functions `cosf` and `sinf` expect radians, so the program converts an angle with:

```text
radians = degrees * pi / 180
```

For an orbit radius `r` around a center `(cx, cy)`, the circular position is:

```text
x = cx + r * cos(angle)
y = cy + r * sin(angle)
```

The program multiplies the vertical offset by `0.62`, making the displayed circular path look like a flattened ellipse. This is a visual choice, not a 3D camera projection.

## 3. The file's structure

The program is organized as one C file. Its major parts are:

1. Header files and an optional screenshot-capture helper.
2. Project title and window-size constants.
3. Application state and small drawing/text helper functions.
4. Planet data, position calculations, scene drawing, and scene controls.
5. Title and information screens.
6. GLUT callbacks for drawing, time updates, keyboard, mouse, and resizing.
7. `main`, which initializes OpenGL/freeglut and starts the event loop.

Reading in this order is helpful: first understand the shared state and the planet table, then read `planetPos`, `sceneUpdate`, and `sceneDraw`, and finally follow the callbacks and `main`.

## 4. Header files and screenshot helper

The `#include` lines make declarations from other libraries available:

- `<GL/glut.h>` provides OpenGL/GLU/freeglut declarations used by the window, drawing, fonts, and input callbacks.
- `<stdio.h>` provides formatted text output and file operations such as `printf`, `sprintf`, `fopen`, and `fwrite`.
- `<stdlib.h>` provides memory allocation, environment access, and process-exit functions.
- `<string.h>` provides `strcmp` for comparing command-line arguments.
- `<math.h>` provides `sinf`, `cosf`, and `fmodf` for the drawing calculations.
- `<unistd.h>` is used by the optional screenshot hook for `_exit`; it is why the Windows instructions use the MSYS2 MinGW toolchain rather than MSVC.

The block beginning with `cg_shot_armed` is auxiliary screenshot tooling, not the solar-system animation itself. If the environment variable `CG_SHOT` is set, `cg_shot_arm` schedules a timer. The timer arms a capture; the wrappers around `glutSwapBuffers` and `glFlush` then call `cg_grab` after a frame is ready.

`cg_grab` asks GLUT for the window dimensions, allocates three bytes per pixel (red, green, blue), reads the OpenGL buffer with `glReadPixels`, and writes a binary PPM file named `output_preview.ppm`. It writes rows from bottom to top because OpenGL's first pixel row is at the bottom while image viewers normally display the first row at the top. It then exits. When `CG_SHOT` is not set, this helper does nothing and ordinary drawing continues.

The `#define` lines temporarily redirect calls to `glutSwapBuffers()` and `glFlush()` through the helper wrappers. The wrappers are defined before those macros so their own calls can reach the original library functions.

## 5. Project constants and shared state

The `P_...` constants hold the title-page details, such as the project title, student, subject, and academic year. `WIN_W` and `WIN_H` set the logical window dimensions to 1000 by 700.

The screen constants `SCR_TITLE`, `SCR_RUN`, and `SCR_INFO` identify the three screens. `gScreen` says which screen is currently active. `gPrev` remembers which screen to return to after opening the information screen.

Other shared variables include:

- `gT`: a clock in simulated seconds, used for animation and pulsing effects.
- `gMX`, `gMY`: the latest mouse position in the program's coordinate system.
- `gPaused`: whether time-based scene updates are temporarily stopped.
- `gDown`: whether a mouse button is currently held.
- `ang[]`: the current angle of each planet in degrees.
- `tSpeed`: the simulation speed multiplier; `1.0` is the default and `0.0` stops motion.
- `showOrb`, `showLab`: whether orbit outlines and labels are visible.
- `sel`: the index of the selected planet. It starts at `2`, which is Earth in the table.

These variables are `static`, so they are private to this source file. Functions in the file can share them without exposing them as a public interface to other C files.

## 6. Small drawing and text helpers

These functions give descriptive names to repeated low-level OpenGL operations:

- `blendOn` enables alpha blending; `blendOff` disables it. The blend formula lets partially transparent colors show the already-drawn scene underneath.
- `fillRect` draws a filled four-corner rectangle. It is used for backgrounds and panels.
- `lineRect` draws a rectangular outline and restores the line width afterward.
- `fillCircle` approximates a filled circle with a triangle fan containing 48 edge segments. More segments make the polygon look rounder.
- `ringCircle` draws a 48-segment circular outline.
- `pie` draws a filled wedge between two angles. The scene uses it to add a darker half to each planet.
- `thickLine` draws a line at a requested width and restores the default width.
- `txt` draws a string one character at a time using a GLUT bitmap font.
- `txtW` measures bitmap text, and `txtMid` uses that width to center a string.
- `strokeMid` centers larger outline-font text. It temporarily translates and scales the coordinate system, draws each glyph, then restores the previous transform.

Functions such as `fillCircle` build geometry from repeated points. The loop calculates each point on the circle using cosine and sine, just as the planet-position calculation does.

## 7. Planet data and animation mathematics

`NPL` is `6`, the number of planets. A `Planet` structure groups the values needed to draw one planet:

- `nm`: its label.
- `orb`: its orbit radius, in screen units.
- `rad`: its drawn body radius, in screen units.
- `spd`: its angular speed in degrees per simulated second.
- `r`, `g`, `b`: its red, green, and blue color components from 0 to 1.

The `pl` array contains one structure for each planet. For example, Mercury has an orbit radius of 92, a body radius of 7, and a speed of 96 degrees per simulated second. These are illustrative values; they are not astronomical measurements.

`sceneInit` assigns starting angles: planet index 0 starts at 0 degrees, index 1 at 47 degrees, index 2 at 94 degrees, and so on. Different starting angles spread the planets around their orbits when the scene first appears.

`sceneUpdate(dt)` advances each angle. For planet `i`, the calculation is:

```text
angle change = planet angular speed * time multiplier * elapsed time
```

The callback currently supplies `dt = 0.016` (about 16 milliseconds). When an angle passes 360 degrees, the function subtracts 360 to keep it in one revolution. Since the supported arrow-key speed is at most 8x, one update advances by less than 360 degrees.

`planetPos(i, &x, &y)` converts planet `i`'s angle into a screen position. It changes the angle to radians, then adds the horizontal and flattened vertical offsets to the Sun's center. The same function is used both to draw planets and to decide which planet was clicked, so selection matches the picture.

## 8. How `sceneDraw` builds the scene

`sceneDraw` paints the running scene in layers. Later shapes are drawn on top of earlier shapes, so the order matters.

1. **Background:** a dark rectangle covers the whole logical viewport.
2. **Stars:** a loop places 150 small points. Their coordinates are calculated deterministically from the loop index, so the background does not flicker or change randomly each frame.
3. **Orbit outlines:** when `showOrb` is true, a `GL_LINE_LOOP` draws 72 points for each flattened orbit. Alpha blending makes the lines translucent.
4. **Sun:** three filled circles create the corona, warm outer glow, and bright center. The outer glow radius varies with `sinf(gT * 2.0f)`, producing a small pulse.
5. **Planets:** a loop gets each position with `planetPos`, draws a translucent selection glow for the selected body, fills the planet circle, then draws a darker pie-shaped half.
6. **Saturn's rings:** for planet index 5, two elliptical line loops are drawn. `glPushMatrix`, `glTranslatef`, and `glRotatef` move and rotate the drawing coordinates around Saturn; `glPopMatrix` restores the previous coordinates afterward.
7. **Earth's Moon:** for planet index 2, the code translates to Earth's position, rotates the local coordinates using Earth's angle multiplied by 7, and draws the Moon 30 units from that local center. The matrix push/pop keeps this transformation from moving later objects.
8. **Labels and status:** if `showLab` is true, each planet's name is drawn beside it. A status line shows the speed, selected planet, orbit radius, and an estimated period.

The information-screen `CONCEPTS` text currently mentions “trails,” but the scene code does not retain or draw trails. The visible rings are orbit outlines, not trails.

### Why the matrix push and pop matter

OpenGL transformations affect subsequent drawing. The Saturn code translates to Saturn and rotates for its rings; the Moon code translates to Earth and rotates for the Moon. `glPushMatrix` saves the current transformation and `glPopMatrix` restores it. Without this pair, a local transformation could accidentally affect the next planet or label.

## 9. Scene controls and mouse picking

`sceneKey` processes ordinary character keys while the running scene is active:

- `O` toggles orbit outlines.
- `L` toggles labels.
- `+` or `=` increases `tSpeed` by `0.25`.
- `-` or `_` decreases `tSpeed` by `0.25`, but clamps it at zero.

`sceneSpecial` processes special keys such as the arrows. The right arrow increases speed by `0.25` up to 8. The left arrow decreases it by `0.25` down to zero.

`sceneMouse` checks each planet's current position. It calculates the horizontal and vertical distance from the click, then compares `dx * dx + dy * dy` with the squared hit radius. Squaring both sides avoids a square-root calculation. The hit radius is the planet radius plus 12 units, making small planets easier to select. The mouse-button value is currently ignored, so any button press can select a planet.

`sceneDrag` is registered for drag motion but currently does nothing. Moving the mouse does not drag planets or change the view.

## 10. Title and information screens

`drawTitlePage` paints the title screen: a two-color background, border, large project heading, short description, submission details, a start button, and keyboard hints. `overStart` checks whether the current mouse position falls inside the start button; it also controls the button's hover color.

`drawInfoPage` paints a separate information screen. It prints the `ABOUT`, `CONCEPTS`, and `CONTROLS` string arrays along with basic navigation hints. The `detailRow` helper aligns each project-detail label and value into two columns.

The arrays near the top of the scene section hold this explanatory screen text. They are ordinary C arrays of string pointers terminated by `NULL`; drawing loops continue until they reach that null pointer.

## 11. GLUT callbacks: how the program reacts

freeglut runs an event loop. Instead of drawing continuously inside `main`, the program registers callback functions. freeglut calls the appropriate function when the window needs a frame, the timer expires, the user presses a key, or the mouse moves.

- `display` clears the old frame, draws the currently selected screen, and swaps the two buffers. Double buffering draws a complete frame off-screen before displaying it, which reduces flicker.
- `timer` runs about every 16 milliseconds. If the program is on the running screen and not paused, it advances `gT` and calls `sceneUpdate`. It then requests a redraw and schedules itself again. The redraw continues while paused, but the simulation clock and planet angles stay still.
- `keyboard` handles `ESC`, `I`, and screen-specific keys. On the title screen, `ENTER` or `SPACE` starts the scene. During the scene, `T` returns to the title and `P` toggles pause; remaining keys go to `sceneKey`.
- `special` handles `F1` on any screen and sends running-scene arrow keys to `sceneSpecial`.
- `mouse` converts GLUT's top-down mouse y-coordinate into the program's bottom-up coordinate system. It starts the project when the title button is clicked, returns from the information screen on a click, or asks `sceneMouse` to select a planet.
- `motion` updates the mouse coordinates and calls `sceneDrag` while a button is held. Since `sceneDrag` is empty, this currently has no visible effect.
- `reshape` updates the OpenGL viewport and projection when the window is resized. The projection remains fixed at 1000 by 700, so a differently shaped window can stretch the scene.

The general keyboard callback receives an ASCII character. Arrow keys and `F1` are not ordinary characters, so freeglut delivers them to the separate special-key callback.

## 12. `main`: from program start to the event loop

`main` is the program's entry point. It performs these steps:

1. Reads optional arguments. `--notitle` starts on the scene; `--info` starts on the information screen; `--title` forces the title screen.
2. If screenshot mode (`CG_SHOT`) is active and `--title` was not requested, starts directly in the scene. It advances the scene briefly so an automatic screenshot shows moving bodies at useful positions.
3. Calls `glutInit`, requests a double-buffered RGB window, sets the window size and position, and creates the window.
4. Sets the clear color, establishes the initial 2D projection, and calls `sceneInit` to set the planets' starting angles.
5. Registers the display, resize, keyboard, special-key, mouse, motion, and timer callbacks.
6. Calls `cg_shot_arm` to activate the optional screenshot timer if requested.
7. Calls `glutMainLoop`. This hands control to freeglut, which repeatedly dispatches events and callbacks until the program exits.

`reshape` calls `gluOrtho2D(0, 1000, 0, 700)`. This creates a flat orthographic coordinate system: screen position depends on x and y, not on perspective or depth.

## 13. Follow one animation update

At the default speed, an update for Earth proceeds like this:

1. `timer` decides that the scene is active and not paused.
2. It sets `dt` to `0.016` and calls `sceneUpdate(dt)`.
3. Earth is item 2 in `pl`, with a speed of 44 degrees per simulated second. Its angle advances by `44 * 1.0 * 0.016 = 0.704` degrees.
4. On the next `display` callback, `sceneDraw` calls `planetPos(2, ...)`.
5. `planetPos` converts Earth's angle to radians and calculates its x/y position around `(500, 350)` at orbit radius 192.
6. `sceneDraw` draws the Earth at that position, then draws the Moon relative to Earth.

The repeated timer and redraw callbacks make the motion look continuous. The program is a teaching animation; its fixed time step and approximate timer interval are not intended for precise scientific timing.

## 14. Helpful C and graphics vocabulary

- **Array:** a group of values stored under one variable name, such as `ang` or `pl`.
- **Structure (`struct`):** a custom type that groups related fields, such as a planet's name, size, speed, and color.
- **Function:** a named block of instructions that can be called from other parts of the program.
- **Callback:** a function registered with freeglut for it to call when a particular event occurs.
- **Loop:** repeated instructions; `for` loops draw stars, vertices, planets, and text characters.
- **Pointer:** a value that refers to another value's memory location. `planetPos` receives pointers to the x/y variables it should fill in.
- **Boolean-style flag:** an integer used as on/off state, such as `showOrb` or `gPaused`.
- **Alpha:** a fourth color component that controls transparency. `glColor4f` takes red, green, blue, and alpha.
- **Projection:** the rule that maps scene coordinates onto the window. `gluOrtho2D` creates a flat 2D mapping.
- **Matrix stack:** saved transformation state. Push before local movement/rotation and pop afterward to restore it.
- **Frame:** one complete image drawn into the window. The timer updates the state; the display callback paints a frame from that state.

## 15. Where to make common changes

- Change the displayed project/student details in the `P_...` definitions near the beginning of `main.c`.
- Change planet names, positions, sizes, speeds, or colors in the `pl` table.
- Change the Sun's screen position in `SUNX` and `SUNY`.
- Change the initial planet placement in `sceneInit`.
- Change keyboard behavior in `sceneKey`, `sceneSpecial`, or `keyboard`.
- Change the on-screen help text in `DESC`, `ABOUT`, `CONCEPTS`, `CONTROLS`, and `HUD_HINT`.
- Change the window's logical dimensions using `WIN_W` and `WIN_H`; the drawing coordinates and resize projection also depend on these values.

For exact installation and build commands, see [HOW-TO-RUN.md](HOW-TO-RUN.md).
