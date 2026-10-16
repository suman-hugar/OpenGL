/*
 * 2D Transformations - a basic 2D OpenGL mini project
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
#define P_NAME     "2D Transformations"
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
static const char *DESC[] = { "A shape you translate, rotate, scale, shear and reflect from the", "keyboard, with the original left as a ghost and the live matrix printed.", NULL };
static const char *ABOUT[] = { "The second classic lab. The transforms are applied with glTranslatef,", "glRotatef, glScalef and a shear built by hand with glMultMatrixf, and", "then the composite is read back out of OpenGL with glGetFloatv and", "printed - so the matrix on screen is the one actually being used.", NULL };
static const char *CONCEPTS[] = { "gluOrtho2D orthographic 2D projection", "glTranslatef, glRotatef, glScalef on the modelview stack", "A shear matrix built by hand and applied with glMultMatrixf", "Reflection as a negative scale", "glGetFloatv(GL_MODELVIEW_MATRIX) read back and printed", "Order of composition shown by the ghost of the original", NULL };
static const char *CONTROLS[] = { "ARROWS      translate            R / E  rotate", "+ / -       scale                X / Y  reflect in x or y", "H / J       shear in x or y      0      reset", "G           ghost and grid on / off", "P           pause", NULL };

/* ======================================================================== */
/*  THE SCENE                                                               */
/* ======================================================================== */

#define HUD_HINT "arrows translate, R/E rotate, +/- scale, X/Y reflect, H/J shear, 0 reset"

static float tx = 0.0f, ty = 0.0f, rot = 0.0f, sx = 1.0f, sy = 1.0f;
static float shx = 0.0f, shy = 0.0f;
static int   refx = 0, refy = 0, ghost = 1;
static const float OX = 420.0f, OY = 330.0f;   /* the origin of the axes */

static void sceneInit(void) { }
static void sceneUpdate(float dt) { (void)dt; }

/* the shape, in its own coordinates: a little house */
static void shape(void)
{
    glBegin(GL_QUADS);
        glVertex2f(-60.0f, 0.0f); glVertex2f(60.0f, 0.0f);
        glVertex2f(60.0f, 90.0f); glVertex2f(-60.0f, 90.0f);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(-74.0f, 90.0f); glVertex2f(74.0f, 90.0f); glVertex2f(0.0f, 150.0f);
    glEnd();
}
static void shapeOutline(void)
{
    glBegin(GL_LINE_LOOP);
        glVertex2f(-60.0f, 0.0f); glVertex2f(60.0f, 0.0f); glVertex2f(60.0f, 90.0f);
        glVertex2f(74.0f, 90.0f); glVertex2f(0.0f, 150.0f); glVertex2f(-74.0f, 90.0f);
        glVertex2f(-60.0f, 90.0f);
    glEnd();
}

static void sceneDraw(void)
{
    int i;
    char b[140];
    float m[16];
    glColor3f(0.07f, 0.09f, 0.14f); fillRect(0.0f, 0.0f, WIN_W, WIN_H);

    if (ghost) {
        glColor3f(0.15f, 0.18f, 0.26f);
        for (i = -12; i <= 12; i++) {
            thickLine(OX + 40.0f * (float)i, 60.0f, OX + 40.0f * (float)i, 640.0f, 1.0f);
            thickLine(20.0f, OY + 40.0f * (float)i, 980.0f, OY + 40.0f * (float)i, 1.0f);
        }
    }
    glColor3f(0.40f, 0.46f, 0.60f);
    thickLine(20.0f, OY, 980.0f, OY, 2.0f);
    thickLine(OX, 60.0f, OX, 640.0f, 2.0f);
    glColor3f(0.56f, 0.62f, 0.76f);
    txt(962.0f, OY + 10.0f, GLUT_BITMAP_HELVETICA_12, "x");
    txt(OX + 10.0f, 624.0f, GLUT_BITMAP_HELVETICA_12, "y");

    /* the original, as a ghost */
    if (ghost) {
        glPushMatrix();
            glTranslatef(OX, OY, 0.0f);
            glColor3f(0.32f, 0.36f, 0.48f);
            glLineWidth(1.6f);
            shapeOutline();
            glLineWidth(1.0f);
        glPopMatrix();
    }

    /* the transformed copy */
    glPushMatrix();
        glTranslatef(OX, OY, 0.0f);
        glPushMatrix();          /* the composite to print, without the axes offset */
            glLoadIdentity();
            glTranslatef(tx, ty, 0.0f);
            glRotatef(rot, 0.0f, 0.0f, 1.0f);
            glScalef(sx * (refx ? -1.0f : 1.0f), sy * (refy ? -1.0f : 1.0f), 1.0f);
            {
                float sm2[16] = { 1.0f, shy, 0.0f, 0.0f,
                                  shx, 1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f };
                glMultMatrixf(sm2);
            }
            glGetFloatv(GL_MODELVIEW_MATRIX, m);
        glPopMatrix();
        glTranslatef(tx, ty, 0.0f);
        glRotatef(rot, 0.0f, 0.0f, 1.0f);
        glScalef(sx * (refx ? -1.0f : 1.0f), sy * (refy ? -1.0f : 1.0f), 1.0f);
        {   /* the shear has no glShear, so build the matrix and multiply */
            float sm[16] = { 1.0f, shy, 0.0f, 0.0f,
                             shx, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f };
            glMultMatrixf(sm);
        }
        glColor3f(0.26f, 0.62f, 0.94f);
        shape();
        glColor3f(0.86f, 0.94f, 1.00f);
        glLineWidth(2.0f);
        shapeOutline();
        glLineWidth(1.0f);
    glPopMatrix();

    /* the matrix OpenGL is actually using, read back */
    glColor3f(0.10f, 0.12f, 0.18f);
    fillRect(700.0f, 430.0f, 280.0f, 210.0f);
    glColor3f(0.34f, 0.40f, 0.54f);
    lineRect(700.0f, 430.0f, 280.0f, 210.0f, 2.0f);
    glColor3f(0.98f, 0.82f, 0.30f);
    txt(716.0f, 612.0f, GLUT_BITMAP_HELVETICA_12, "the composite matrix, read back");
    glColor3f(0.80f, 0.88f, 0.98f);
    for (i = 0; i < 4; i++) {
        sprintf(b, "%7.2f %7.2f %7.2f %7.2f", m[i], m[4 + i], m[8 + i], m[12 + i]);
        txt(714.0f, 580.0f - 22.0f * (float)i, GLUT_BITMAP_HELVETICA_12, b);
    }
    glColor3f(0.58f, 0.66f, 0.82f);
    txt(714.0f, 462.0f, GLUT_BITMAP_HELVETICA_12, "column major, as OpenGL stores it");

    sprintf(b, "translate (%.0f, %.0f)   rotate %.0f   scale (%.2f, %.2f)   shear (%.2f, %.2f)%s%s",
            tx, ty, rot, sx, sy, shx, shy, refx ? "   reflect x" : "", refy ? "   reflect y" : "");
    glColor3f(1.0f, 1.0f, 1.0f);
    txt(30.0f, WIN_H - 42.0f, GLUT_BITMAP_HELVETICA_18, b);
    glColor3f(0.60f, 0.68f, 0.84f);
    txt(30.0f, WIN_H - 64.0f, GLUT_BITMAP_HELVETICA_12,
        "the grey outline is the shape before any of it - order of operations is top to bottom");
}

static void sceneKey(unsigned char k)
{
    if (k == 'r') rot += 6.0f;
    if (k == 'R') rot += 15.0f;
    if (k == 'e') rot -= 6.0f;
    if (k == 'E') rot -= 15.0f;
    if (k == '+' || k == '=') { sx += 0.08f; sy += 0.08f; }
    if (k == '-' || k == '_') { sx -= 0.08f; sy -= 0.08f; }
    if (sx < 0.1f) sx = 0.1f;
    if (sy < 0.1f) sy = 0.1f;
    if (k == 'x' || k == 'X') refx = !refx;
    if (k == 'y' || k == 'Y') refy = !refy;
    if (k == 'h') shx += 0.08f;
    if (k == 'H') shx -= 0.08f;
    if (k == 'j') shy += 0.08f;
    if (k == 'J') shy -= 0.08f;
    if (k == 'g' || k == 'G') ghost = !ghost;
    if (k == '0') { tx = ty = rot = shx = shy = 0.0f; sx = sy = 1.0f; refx = refy = 0; }
}
static void sceneSpecial(int k)
{
    if (k == GLUT_KEY_LEFT)  tx -= 15.0f;
    if (k == GLUT_KEY_RIGHT) tx += 15.0f;
    if (k == GLUT_KEY_UP)    ty += 15.0f;
    if (k == GLUT_KEY_DOWN)  ty -= 15.0f;
}
static void sceneMouse(int btn, float x, float y)
{
    (void)btn;
    tx = x - OX; ty = y - OY;     /* click moves the shape there */
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
