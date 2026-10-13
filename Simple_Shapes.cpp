
#include <GL/glut.h>
#include <cmath>

// Draw a red triangle, green square, and blue circle
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Red Triangle
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLES);
        glVertex2f(-0.8f, -0.5f);
        glVertex2f(-0.2f, -0.5f);
        glVertex2f(-0.5f,  0.2f);
    glEnd();

    // 2. Green Square
    glColor3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
        glVertex2f(0.2f, -0.5f);
        glVertex2f(0.8f, -0.5f);
        glVertex2f(0.8f,  0.2f);
        glVertex2f(0.2f,  0.2f);
    glEnd();

    // 3. Blue Circle
    glColor3f(0.0f, 0.0f, 1.0f);

    glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++)
        {
            float theta = i * 3.14159265f / 180.0f;

            float x = 0.2f * cos(theta);
            float y = 0.2f * sin(theta) + 0.5f;

            glVertex2f(x, y);
        }
    glEnd();

    glFlush();
}

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("OpenGL Simple Shapes - CS With Suman");

    init();
    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}

