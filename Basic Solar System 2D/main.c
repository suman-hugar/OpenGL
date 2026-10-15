/*
 * Solar System 2D - a basic 2D OpenGL mini project
 * Copyright (c) 2026 Suman Hugar. All rights reserved.
 *
 * Build:  make
 * Run:    make run
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------
 * Screenshot hook (added by the project toolkit).
 *
 * Set CG_SHOT=<milliseconds> in the environment and the program grabs
 * the frame it is about to present, writes output_preview.ppm and
 * exits. Nothing changes when CG_SHOT is unset.
 *
 * The grab happens on the buffer being presented rather than on the
 * front buffer, because reading the front buffer is unreliable under
 * a compositor (Xwayland in particular hands back a black frame).
 * ------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int cg_shot_armed = 0;

static void cg_grab(unsigned int buf)
{
    int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT), y;
    unsigned char *px;
    FILE *fp;
    if (w <= 0 || h <= 0) _exit(2);
    px = (unsigned char *)malloc((size_t)w * h * 3);
    if (!px) _exit(2);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(buf);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px);
    fp = fopen("output_preview.ppm", "wb");
    if (fp) {
        fprintf(fp, "P6\n%d %d\n255\n", w, h);
        for (y = h - 1; y >= 0; y--)
            fwrite(px + (size_t)y * w * 3, 1, (size_t)w * 3, fp);
        fclose(fp);
        printf("saved output_preview.ppm (%d x %d)\n", w, h);
    }
    free(px);
    fflush(NULL);
    _exit(0);
}

/* These wrappers must be defined before the macros below rename the
 * calls, so that the real GL entry points are still reachable here. */
static void cg_swap(void)   { if (cg_shot_armed) cg_grab(GL_BACK);  glutSwapBuffers(); }
static void cg_flush(void)  { glFlush(); if (cg_shot_armed) cg_grab(GL_FRONT); }

#define glutSwapBuffers() cg_swap()
#define glFlush()         cg_flush()

static void cg_shot_fire(int v) { (void)v; cg_shot_armed = 1; glutPostRedisplay(); }
static void cg_shot_arm(void)
{
    const char *e = getenv("CG_SHOT");
    int ms;
    if (!e || !*e) return;
    ms = atoi(e);
    glutTimerFunc(ms > 100 ? ms : 2500, cg_shot_fire, 0);
}
/* ---------------------------------------------------------------- */

/* ---- submission detail (edit these eleven lines only) ------------------ */
#define P_TITLE    "COMPUTER GRAPHICS PROJECT"
#define P_NAME     "Solar System 2D"
#define P_SUBTITLE "A 2D OpenGL Computer Graphics Mini Project"
#define P_STUDENT  "Suman Hugar"
#define P_USN      "1XX00XX000"
#define P_BRANCH   "Computer Science and Engineering"
#define P_COLLEGE  "Visvesvaraya Technological University"
#define P_SUBJECT  "Computer Graphics"
#define P_TECH     "OpenGL / freeglut, C"
#define P_YEAR     "2025 - 2026"
#define P_GUIDE    "Department of Computer Science and Engineering"

#define WIN_W 1000
#define WIN_H 700

/* ---- screens ----------------------------------------------------------- */
enum { SCR_TITLE = 0, SCR_RUN = 1, SCR_INFO = 2 };
static int   gScreen = SCR_TITLE;
static int   gPrev   = SCR_TITLE;      /* screen to return to from info     */
static float gT      = 0.0f;           /* scene clock, seconds              */
static float gMX = 500.0f, gMY = 350.0f;
static int   gPaused = 0;
static int   gDown   = 0;      /* a mouse button is held */

/* ---- small drawing helpers -------------------------------------------- */
static void blendOn(void)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
static void blendOff(void) { glDisable(GL_BLEND); }

static void fillRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

static void lineRect(float x, float y, float w, float h, float lw)
{
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f);
}

static void fillCircle(float cx, float cy, float r)
{
    int i, n = 48;
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (i = 0; i <= n; i++) {
            float a = 2.0f * 3.14159265f * (float)i / (float)n;
            glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
        }
    glEnd();
}

static void ringCircle(float cx, float cy, float r, float lw)
{
    int i, n = 48;
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
        for (i = 0; i < n; i++) {
            float a = 2.0f * 3.14159265f * (float)i / (float)n;
            glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
        }
    glEnd();
    glLineWidth(1.0f);
}

/* filled pie slice, angles in degrees */
static void pie(float cx, float cy, float r, float a0, float a1)
{
    int i, n = 32;
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (i = 0; i <= n; i++) {
            float a = (a0 + (a1 - a0) * (float)i / (float)n) * 3.14159265f / 180.0f;
            glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
        }
    glEnd();
}

static void thickLine(float x1, float y1, float x2, float y2, float lw)
{
    glLineWidth(lw);
    glBegin(GL_LINES); glVertex2f(x1, y1); glVertex2f(x2, y2); glEnd();
    glLineWidth(1.0f);
}

static void txt(float x, float y, void *font, const char *s)
{
    glRasterPos2f(x, y);
    while (*s) glutBitmapCharacter(font, *s++);
}

static float txtW(void *font, const char *s)
{
    float w = 0.0f;
    while (*s) w += (float)glutBitmapWidth(font, *s++);
    return w;
}

static void txtMid(float cx, float y, void *font, const char *s)
{
    txt(cx - txtW(font, s) * 0.5f, y, font, s);
}

static void strokeMid(float cx, float y, float h, float lw, const char *s)
{
    float sc = h / 119.05f, w = 0.0f;
    const char *p = s;
    while (*p) w += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *p++);
    w *= sc;
    glLineWidth(lw);
    glPushMatrix();
        glTranslatef(cx - w * 0.5f, y, 0.0f);
        glScalef(sc, sc, 1.0f);
        for (p = s; *p; p++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glPopMatrix();
    glLineWidth(1.0f);
}

/* ---- per project text -------------------------------------------------- */
static const char *DESC[] = { "The sun with six planets going round it, each on its own orbit at its", "own speed. Click a planet to read its name, O hides the orbit rings.", NULL };
static const char *ABOUT[] = { "Each planet is one line in a table - orbit radius, body radius, angular", "speed and colour - and the draw loop walks the table. The position is", "polar: the angle advances with the clock and cosine and sine turn it", "into x and y, which is the whole of the animation.", NULL };
static const char *CONCEPTS[] = { "gluOrtho2D orthographic 2D projection", "Polar to Cartesian placement of each body", "GL_LINE_LOOP orbit rings and GL_TRIANGLE_FAN bodies", "glPushMatrix / glRotatef for the moon going round the Earth", "Alpha blending for the solar corona and the trails", "Mouse picking - click a planet to select it", NULL };
static const char *CONTROLS[] = { "RIGHT / LEFT   speed up or slow down time", "O              orbit rings on / off", "L              labels on / off", "Click a planet   select it and read its detail", "P              pause", NULL };

/* ======================================================================== */
/*  THE SCENE                                                               */
/* ======================================================================== */

#define HUD_HINT "arrows = time speed, O = orbits, L = labels, click a planet"

#define NPL 6
typedef struct { const char *nm; float orb, rad, spd, r, g, b; } Planet;
static Planet pl[NPL] = {
    { "Mercury",  92.0f,  7.0f, 96.0f, 0.72f, 0.66f, 0.58f },
    { "Venus",   138.0f, 12.0f, 62.0f, 0.90f, 0.72f, 0.36f },
    { "Earth",   192.0f, 14.0f, 44.0f, 0.24f, 0.52f, 0.88f },
    { "Mars",    248.0f, 10.0f, 30.0f, 0.82f, 0.36f, 0.22f },
    { "Jupiter", 322.0f, 26.0f, 16.0f, 0.82f, 0.64f, 0.44f },
    { "Saturn",  392.0f, 22.0f, 10.0f, 0.88f, 0.80f, 0.54f }
};
static float ang[NPL];
static float tSpeed = 1.0f;
static int   showOrb = 1, showLab = 1, sel = 2;
static const float SUNX = 500.0f, SUNY = 350.0f;

static void sceneInit(void)
{
    int i; for (i = 0; i < NPL; i++) ang[i] = (float)(i * 47);
}

static void sceneUpdate(float dt)
{
    int i;
    for (i = 0; i < NPL; i++) {
        ang[i] += pl[i].spd * tSpeed * dt;
        if (ang[i] > 360.0f) ang[i] -= 360.0f;
    }
}

static void planetPos(int i, float *x, float *y)
{
    float a = ang[i] * 3.14159265f / 180.0f;
    *x = SUNX + pl[i].orb * cosf(a);
    *y = SUNY + pl[i].orb * sinf(a) * 0.62f;   /* a little tilt */
}

static void sceneDraw(void)
{
    int i;
    glColor3f(0.01f, 0.01f, 0.04f); fillRect(0.0f, 0.0f, WIN_W, WIN_H);
    glColor3f(0.85f, 0.86f, 0.95f);
    for (i = 0; i < 150; i++) {
        float sx = fmodf((float)(i * 173 + 11), 1000.0f);
        float sy = fmodf((float)(i * 311 + 7), 700.0f);
        fillCircle(sx, sy, (i % 11 == 0) ? 1.9f : 1.0f);
    }

    if (showOrb) {
        blendOn();
        glColor4f(0.40f, 0.52f, 0.74f, 0.42f);
        for (i = 0; i < NPL; i++) {
            glBegin(GL_LINE_LOOP);
                int s; for (s = 0; s < 72; s++) {
                    float a = 2.0f * 3.14159265f * (float)s / 72.0f;
                    glVertex2f(SUNX + pl[i].orb * cosf(a), SUNY + pl[i].orb * 0.62f * sinf(a));
                }
            glEnd();
        }
        blendOff();
    }

    /* the sun */
    blendOn();
    glColor4f(1.00f, 0.66f, 0.12f, 0.20f); fillCircle(SUNX, SUNY, 96.0f + sinf(gT * 2.0f) * 4.0f);
    glColor4f(1.00f, 0.76f, 0.20f, 0.34f); fillCircle(SUNX, SUNY, 70.0f);
    blendOff();
    glColor3f(1.00f, 0.86f, 0.28f); fillCircle(SUNX, SUNY, 48.0f);
    glColor3f(1.00f, 0.96f, 0.72f); fillCircle(SUNX - 12.0f, SUNY + 12.0f, 20.0f);

    for (i = 0; i < NPL; i++) {
        float x, y;
        planetPos(i, &x, &y);
        if (i == sel) {
            blendOn();
            glColor4f(0.60f, 0.92f, 1.00f, 0.30f);
            fillCircle(x, y, pl[i].rad + 12.0f + sinf(gT * 4.0f) * 2.0f);
            blendOff();
        }
        glColor3f(pl[i].r, pl[i].g, pl[i].b);
        fillCircle(x, y, pl[i].rad);
        glColor3f(pl[i].r * 0.45f, pl[i].g * 0.45f, pl[i].b * 0.45f);
        pie(x, y, pl[i].rad, 100.0f, 260.0f);
        if (i == 5) {   /* Saturn's ring */
            glColor3f(0.92f, 0.86f, 0.62f);
            glPushMatrix();
                glTranslatef(x, y, 0.0f);
                glRotatef(-18.0f, 0.0f, 0.0f, 1.0f);
                glBegin(GL_LINE_LOOP);
                    int s; for (s = 0; s < 40; s++) {
                        float a = 2.0f * 3.14159265f * (float)s / 40.0f;
                        glVertex2f(38.0f * cosf(a), 12.0f * sinf(a));
                    }
                glEnd();
                glBegin(GL_LINE_LOOP);
                    int s2; for (s2 = 0; s2 < 40; s2++) {
                        float a = 2.0f * 3.14159265f * (float)s2 / 40.0f;
                        glVertex2f(45.0f * cosf(a), 15.0f * sinf(a));
                    }
                glEnd();
            glPopMatrix();
        }
        if (i == 2) {   /* the Moon, on its own matrix */
            glPushMatrix();
                glTranslatef(x, y, 0.0f);
                glRotatef(ang[i] * 7.0f, 0.0f, 0.0f, 1.0f);
                glColor3f(0.80f, 0.80f, 0.84f);
                fillCircle(30.0f, 0.0f, 5.0f);
            glPopMatrix();
        }
        if (showLab) {
            glColor3f(0.72f, 0.80f, 0.94f);
            txt(x + pl[i].rad + 6.0f, y + 6.0f, GLUT_BITMAP_HELVETICA_10, pl[i].nm);
        }
    }

    char b[140];
    sprintf(b, "time x%.1f      selected: %s      orbit %.0f px, period %.1f s",
            tSpeed, pl[sel].nm, pl[sel].orb, 360.0f / (pl[sel].spd * (tSpeed == 0.0f ? 1.0f : tSpeed)));
    glColor3f(1.0f, 1.0f, 1.0f);
    txt(24.0f, WIN_H - 40.0f, GLUT_BITMAP_HELVETICA_18, b);
}

static void sceneKey(unsigned char k)
{
    if (k == 'o' || k == 'O') showOrb = !showOrb;
    if (k == 'l' || k == 'L') showLab = !showLab;
    if (k == '+' || k == '=') tSpeed += 0.25f;
    if (k == '-' || k == '_') { tSpeed -= 0.25f; if (tSpeed < 0.0f) tSpeed = 0.0f; }
}
static void sceneSpecial(int k)
{
    if (k == GLUT_KEY_RIGHT) { tSpeed += 0.25f; if (tSpeed > 8.0f) tSpeed = 8.0f; }
    if (k == GLUT_KEY_LEFT)  { tSpeed -= 0.25f; if (tSpeed < 0.0f) tSpeed = 0.0f; }
}
static void sceneMouse(int btn, float x, float y)
{
    (void)btn;
    int i;
    for (i = 0; i < NPL; i++) {
        float px, py; planetPos(i, &px, &py);
        float dx = x - px, dy = y - py;
        if (dx * dx + dy * dy < (pl[i].rad + 12.0f) * (pl[i].rad + 12.0f)) { sel = i; return; }
    }
}

static void sceneDrag(float x, float y) { (void)x; (void)y; }


/* ======================================================================== */
/*  FRONT PAGE                                                              */
/* ======================================================================== */
#define BTN_X 400.0f
#define BTN_Y 96.0f
#define BTN_W 200.0f
#define BTN_H 52.0f

static int overStart(void)
{
    return gMX > BTN_X && gMX < BTN_X + BTN_W && gMY > BTN_Y && gMY < BTN_Y + BTN_H;
}

static void detailRow(float y, const char *k, const char *v)
{
    glColor3f(0.62f, 0.70f, 0.82f);
    txt(300.0f, y, GLUT_BITMAP_HELVETICA_12, k);
    glColor3f(0.94f, 0.96f, 1.00f);
    txt(470.0f, y, GLUT_BITMAP_HELVETICA_12, v);
}

static void drawTitlePage(void)
{
    int i;
    /* background wash */
    glBegin(GL_QUADS);
        glColor3f(0.04f, 0.07f, 0.16f); glVertex2f(0, 0); glVertex2f(WIN_W, 0);
        glColor3f(0.10f, 0.17f, 0.34f); glVertex2f(WIN_W, WIN_H); glVertex2f(0, WIN_H);
    glEnd();

    glColor3f(0.20f, 0.32f, 0.55f);
    lineRect(28.0f, 28.0f, WIN_W - 56.0f, WIN_H - 56.0f, 2.0f);

    glColor3f(0.98f, 0.82f, 0.30f);
    strokeMid(500.0f, 596.0f, 40.0f, 2.6f, P_TITLE);
    glColor3f(0.86f, 0.92f, 1.00f);
    strokeMid(500.0f, 542.0f, 30.0f, 2.0f, P_NAME);
    glColor3f(0.58f, 0.70f, 0.88f);
    txtMid(500.0f, 514.0f, GLUT_BITMAP_HELVETICA_12, P_SUBTITLE);

    glColor3f(0.20f, 0.32f, 0.55f);
    thickLine(300.0f, 498.0f, 700.0f, 498.0f, 1.5f);

    for (i = 0; DESC[i]; i++) {
        glColor3f(0.76f, 0.84f, 0.96f);
        txtMid(500.0f, 470.0f - 18.0f * (float)i, GLUT_BITMAP_HELVETICA_12, DESC[i]);
    }

    detailRow(392.0f, "Project Title",  P_NAME);
    detailRow(368.0f, "Submitted by",   P_STUDENT);
    detailRow(344.0f, "USN",            P_USN);
    detailRow(320.0f, "Branch",         P_BRANCH);
    detailRow(296.0f, "College",        P_COLLEGE);
    detailRow(272.0f, "Subject",        P_SUBJECT);
    detailRow(248.0f, "Technology",     P_TECH);
    detailRow(224.0f, "Academic Year",  P_YEAR);
    detailRow(200.0f, "Guided by",      P_GUIDE);

    /* START button */
    if (overStart()) glColor3f(0.24f, 0.62f, 0.42f);
    else             glColor3f(0.16f, 0.46f, 0.32f);
    fillRect(BTN_X, BTN_Y, BTN_W, BTN_H);
    glColor3f(0.52f, 0.92f, 0.70f);
    lineRect(BTN_X, BTN_Y, BTN_W, BTN_H, 2.0f);
    glColor3f(0.96f, 1.00f, 0.98f);
    txtMid(BTN_X + BTN_W * 0.5f, BTN_Y + 20.0f, GLUT_BITMAP_HELVETICA_18, "START PROJECT");

    glColor3f(0.54f, 0.64f, 0.82f);
    txtMid(500.0f, 62.0f, GLUT_BITMAP_HELVETICA_12,
           "Press ENTER or click START   |   I = information page   |   ESC = quit");
}

/* ======================================================================== */
/*  INFORMATION PAGE                                                        */
/* ======================================================================== */
static void drawInfoPage(void)
{
    int i;
    float y;
    glColor3f(0.05f, 0.08f, 0.14f);
    fillRect(0, 0, WIN_W, WIN_H);
    glColor3f(0.20f, 0.32f, 0.55f);
    lineRect(24.0f, 24.0f, WIN_W - 48.0f, WIN_H - 48.0f, 2.0f);

    glColor3f(0.98f, 0.82f, 0.30f);
    strokeMid(500.0f, 626.0f, 28.0f, 2.0f, "INFORMATION");
    glColor3f(0.60f, 0.72f, 0.90f);
    txtMid(500.0f, 604.0f, GLUT_BITMAP_HELVETICA_12, P_NAME "   -   " P_SUBJECT);

    y = 566.0f;
    glColor3f(0.98f, 0.82f, 0.30f);
    txt(70.0f, y, GLUT_BITMAP_HELVETICA_18, "About this project");
    y -= 26.0f;
    for (i = 0; ABOUT[i]; i++) {
        glColor3f(0.84f, 0.89f, 0.98f);
        txt(70.0f, y, GLUT_BITMAP_HELVETICA_12, ABOUT[i]);
        y -= 18.0f;
    }

    y -= 16.0f;
    glColor3f(0.98f, 0.82f, 0.30f);
    txt(70.0f, y, GLUT_BITMAP_HELVETICA_18, "Graphics concepts used");
    y -= 26.0f;
    for (i = 0; CONCEPTS[i]; i++) {
        glColor3f(0.60f, 0.90f, 0.72f);
        txt(70.0f, y, GLUT_BITMAP_HELVETICA_12, "*");
        glColor3f(0.84f, 0.89f, 0.98f);
        txt(90.0f, y, GLUT_BITMAP_HELVETICA_12, CONCEPTS[i]);
        y -= 18.0f;
    }

    y = 566.0f;
    glColor3f(0.98f, 0.82f, 0.30f);
    txt(540.0f, y, GLUT_BITMAP_HELVETICA_18, "Controls");
    y -= 26.0f;
    glColor3f(0.84f, 0.89f, 0.98f);
    txt(540.0f, y, GLUT_BITMAP_HELVETICA_12, "ENTER   start the project from the front page");
    y -= 18.0f;
    txt(540.0f, y, GLUT_BITMAP_HELVETICA_12, "I / F1  show or hide this information page");
    y -= 18.0f;
    txt(540.0f, y, GLUT_BITMAP_HELVETICA_12, "T       back to the front page");
    y -= 18.0f;
    txt(540.0f, y, GLUT_BITMAP_HELVETICA_12, "ESC     quit");
    y -= 24.0f;
    for (i = 0; CONTROLS[i]; i++) {
        glColor3f(0.84f, 0.89f, 0.98f);
        txt(540.0f, y, GLUT_BITMAP_HELVETICA_12, CONTROLS[i]);
        y -= 18.0f;
    }

    glColor3f(0.54f, 0.64f, 0.82f);
    txtMid(500.0f, 52.0f, GLUT_BITMAP_HELVETICA_12,
           "Press I or F1 to go back   |   ESC = quit");
}

/* ======================================================================== */
/*  GLUT PLUMBING                                                           */
/* ======================================================================== */
static void drawHud(void)
{
    /* long enough for the longest P_NAME and HUD_HINT in the collection -
       at 160 the four longest overflowed it and the program aborted */
    char buf[256];
    blendOn();
    glColor4f(0.00f, 0.00f, 0.00f, 0.42f);
    fillRect(0.0f, 0.0f, (float)WIN_W, 26.0f);
    blendOff();
    sprintf(buf, "%s   |   %s   |   I = information   |   T = front page   |   ESC = quit",
            P_NAME, HUD_HINT);
    glColor3f(0.90f, 0.94f, 1.00f);
    txt(12.0f, 8.0f, GLUT_BITMAP_HELVETICA_12, buf);
}

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    if      (gScreen == SCR_TITLE) drawTitlePage();
    else if (gScreen == SCR_INFO)  drawInfoPage();
    else { sceneDraw(); drawHud(); }
    glutSwapBuffers();
}

static void timer(int v)
{
    if (gScreen == SCR_RUN && !gPaused) {
        float dt = 0.016f;
        gT += dt;
        sceneUpdate(dt);
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

static void keyboard(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    if (key == 27) exit(0);
    if (key == 'i' || key == 'I') {
        if (gScreen == SCR_INFO) gScreen = gPrev;
        else { gPrev = gScreen; gScreen = SCR_INFO; }
        return;
    }
    if (gScreen == SCR_TITLE) {
        if (key == 13 || key == ' ') gScreen = SCR_RUN;
        return;
    }
    if (gScreen == SCR_INFO) return;
    if (key == 't' || key == 'T') { gScreen = SCR_TITLE; return; }
    if (key == 'p' || key == 'P') { gPaused = !gPaused; return; }
    sceneKey(key);
}

static void special(int key, int x, int y)
{
    (void)x; (void)y;
    if (key == GLUT_KEY_F1) {
        if (gScreen == SCR_INFO) gScreen = gPrev;
        else { gPrev = gScreen; gScreen = SCR_INFO; }
        return;
    }
    if (gScreen != SCR_RUN) return;
    sceneSpecial(key);
}

static void mouse(int btn, int state, int x, int y)
{
    float mx = (float)x, my = (float)(WIN_H - y);
    gMX = mx; gMY = my;
    gDown = (state == GLUT_DOWN);
    if (state != GLUT_DOWN) return;
    if (gScreen == SCR_TITLE) {
        if (overStart()) gScreen = SCR_RUN;
        return;
    }
    if (gScreen == SCR_INFO) { gScreen = gPrev; return; }
    sceneMouse(btn, mx, my);
}

static void motion(int x, int y)
{
    gMX = (float)x; gMY = (float)(WIN_H - y);
    if (gScreen == SCR_RUN && gDown) sceneDrag(gMX, gMY);
}

static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (double)WIN_W, 0.0, (double)WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    int i, forceTitle = 0;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--notitle")) gScreen = SCR_RUN;
        if (!strcmp(argv[i], "--title"))   forceTitle = 1;
        if (!strcmp(argv[i], "--info"))    gScreen = SCR_INFO;
    }
    /* a screenshot run opens straight into the scene unless --title says not to */
    if (getenv("CG_SHOT") && !forceTitle && gScreen == SCR_TITLE) gScreen = SCR_RUN;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(80, 40);
    glutCreateWindow(P_NAME " - " P_SUBJECT " Mini Project");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    reshape(WIN_W, WIN_H);
    sceneInit();

    /* A screenshot run can be handed its first frame before the timer has
       fired many times, which photographs the scene at time zero. Run the
       simulation forward first so the still shows the project working. */
    if (getenv("CG_SHOT")) {
        int w;
        for (w = 0; w < 240; w++) { gT += 0.016f; sceneUpdate(0.016f); }
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);
    glutTimerFunc(16, timer, 0);
    cg_shot_arm();

    glutMainLoop();
    return 0;
}
