/*
 *  draw.cpp
 */
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <stdlib.h>
#include <math.h>
#include <stack>
#include <iostream>
#include "imageio.h"

using namespace std;

const int screenWidth = 600;
const int screenHeight = 600;
const float PI = 3.1415926f;

// texture variables
int texImageWidth;
int texImageHeight;
static GLuint texName;
char maps[] = {"wood1.png"}; // texture file

// default dimension values for 4 disks
int disks = 4;
float disk_height = 1.0;
float length_annulus_mult = 1.0;
float length_annulus_biggest = disks * length_annulus_mult;
float rod_radius = 0.5; // must be less than length_annulus_mult / 2
float rod_height = (double)disk_height * disks * 2;
float top_plat_width = (length_annulus_biggest + 1.0) * 2;
float top_plat_height = 1.0;
float top_plat_length = top_plat_width * 3;
float bot_plat_width = top_plat_width;
float bot_plat_height = 1.0;
float bot_plat_length = top_plat_length * 1.2;

// color variables
GLfloat platform_color1[] = {0.05f, 0.05f, 0.05f};
GLfloat platform_color2[] = {0.0f, 0.0f, 0.0f};
GLfloat rod_color[] = {0.0f, 0.0f, 0.0f};
GLfloat cylinder_color[] = {0.0f, 0.0f, 0.0f};
GLfloat circle_color[] = {0.0f, 0.0f, 0.0f};

// other var
stack<int> s[3]; // three stacks to hold the disks
int moveCount = 0;
bool automate = false;
int auto_move_num = 0;

// necessary function prototypes
void autoPlay(void);

// load texture image
GLubyte *makeTexImage(char *loadfile)
{
    int i, j, c, width, height;
    GLubyte *texImage;

    /*
      Only works for .png or .tif images.  NULL is returned if errors occurred.
      loadImageRGA() is from imageio library downloaded from Internet.
    */
    texImage = loadImageRGBA((char *)loadfile, &width, &height);
    texImageWidth = width;
    texImageHeight = height;

    return texImage;
}

void dimInit(void) // dimension reinitialization for tower of hanoi
{
    try
    {
        cout << "How many discs?" << endl;
        cin >> disks;
        if (cin.fail())
            throw(disks); // Throw an exception when a problem arise
    }
    catch (const exception &e)
    {
        cout << "Invalid input. Defaulting to 4 discs." << endl;
        disks = 4;
    }

    disk_height = 1.0;
    length_annulus_mult = 1.0;
    length_annulus_biggest = disks * length_annulus_mult;
    rod_radius = 0.5; // must be less than length_annulus_mult / 2
    rod_height = (double)disk_height * disks * 2;
    top_plat_width = (length_annulus_biggest + 1.0) * 2;
    top_plat_height = 1.0;
    top_plat_length = top_plat_width * 3;
    bot_plat_width = top_plat_width;
    bot_plat_height = 1.0;
    bot_plat_length = top_plat_length * 1.2;
}

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT);
    glEnable(GL_NORMALIZE);

    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);

    glLineWidth(4);
    glLoadIdentity(); /* clear the matrix */
                      /* viewing transformation  */
    gluLookAt(15.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glScalef(0.5 * (4 / (double)disks), 0.5 * (4 / (double)disks), 0.5 * (4 / (double)disks)); /* modeling transformation */

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT, GL_FILL);

    // initialize the three stacks
    for (int i = disks; i >= 1; i--)
        s[0].push(i);

    // output stack for testing
    for (int i = 0; i < 3; i++)
    {
        cout << "s[" << i << "]: ";
        stack<int> temp = s[i];
        while (!temp.empty())
        {
            cout << temp.top() << " ";
            temp.pop();
        }
        cout << endl;
    }

    // init texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // texName is global
    glGenTextures(1, &texName);
    // enable automatic texture coordinate generation
    GLubyte *texImage = makeTexImage(maps);
    if (!texImage)
    {
        printf("\nError reading %s \n", maps);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, texName);                       // now we work on texName
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // mix with light
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImageWidth,
                 texImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage);

    delete texImage; // free memory holding texture image

    // automatic texture coordinates generation
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    // glEnable(GL_TEXTURE_GEN_S);
    // glEnable(GL_TEXTURE_GEN_T);
    // glEnable(GL_TEXTURE_2D);
}

void drawLight()
{
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {50.0};
    GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_position0[] = {25.0, 25.0, 25.0, 0.0};
    GLfloat light_position1[] = {-1.0, -1.0, -1.0, 0.0};
    GLfloat light_position2[] = {0.0, -1.0, 0.0, 0.0};
    GLfloat lights0[] = {0.7, 0.7, 0.7, 1.0};
    GLfloat lightd0[] = {0.3, 0.3, 0.3, 1.0};
    GLfloat lights1[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightd1[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lights2[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightd2[] = {1.0, 1.0, 1.0, 1.0};

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position2);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightd0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lights0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightd1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lights1);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightd2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lights2);

    GLfloat lmodel_ambient[] = {1.0, 1.0, 1.0, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
}

void drawCircle(void)
{
    float i;
    float triangleAmount = 40; // # of triangles used to draw circle

    GLfloat twicePi = 2.0f * PI;

    glColor3f(circle_color[0], circle_color[1], circle_color[2]);
    glBegin(GL_TRIANGLE_FAN);
    // glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0); // center of circle
    for (i = 0; i <= triangleAmount; i++)
    {
        glNormal3f(sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(i * twicePi / triangleAmount)),
            0.0,
            0.0 + (rod_radius * cos(i * twicePi / triangleAmount)));
    }
    glEnd();
}

void drawCylinderSide(void)
{
    float i;
    float triangleAmount = 40; // # of triangles used to draw circle

    GLfloat twicePi = 2.0f * PI;

    glColor3f(cylinder_color[0], cylinder_color[1], cylinder_color[2]);
    glBegin(GL_TRIANGLE_STRIP);
    for (i = 0; i <= triangleAmount; i++)
    {
        glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(i * twicePi / triangleAmount)),
            0.0,
            0.0 + (rod_radius * cos(i * twicePi / triangleAmount)));
        glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(i * twicePi / triangleAmount)),
            rod_height,
            0.0 + (rod_radius * cos(i * twicePi / triangleAmount)));
    }
    glEnd();
}

void drawPlatform(void)
{
    glColor3f(platform_color1[0], platform_color1[1], platform_color1[2]);
    glBegin(GL_POLYGON);
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(-top_plat_width / 2, -top_plat_height / 2, top_plat_length / 2);
    glVertex3f(top_plat_width / 2, -top_plat_height / 2, top_plat_length / 2);
    glVertex3f(top_plat_width / 2, top_plat_height / 2, top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, top_plat_height / 2, top_plat_length / 2);
    glEnd();

    glColor3f(platform_color2[0], platform_color2[1], platform_color2[2]);
    glBegin(GL_POLYGON);
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-top_plat_width / 2, top_plat_height / 2, top_plat_length / 2);
    glVertex3f(top_plat_width / 2, top_plat_height / 2, top_plat_length / 2);
    glVertex3f(top_plat_width / 2, top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, top_plat_height / 2, -top_plat_length / 2);
    glEnd();

    glColor3f(platform_color1[0], platform_color1[1], platform_color1[2]);
    glBegin(GL_POLYGON);
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-top_plat_width / 2, top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(top_plat_width / 2, top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(top_plat_width / 2, -top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, -top_plat_height / 2, -top_plat_length / 2);
    glEnd();

    glColor3f(platform_color2[0], platform_color2[1], platform_color2[2]);
    glBegin(GL_POLYGON);
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-top_plat_width / 2, -top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(top_plat_width / 2, -top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(top_plat_width / 2, -top_plat_height / 2, top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, -top_plat_height / 2, top_plat_length / 2);
    glEnd();

    glColor3f(platform_color1[0], platform_color1[1], platform_color1[2]);
    glBegin(GL_POLYGON);
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(top_plat_width / 2, -top_plat_height / 2, top_plat_length / 2);
    glVertex3f(top_plat_width / 2, -top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(top_plat_width / 2, top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(top_plat_width / 2, top_plat_height / 2, top_plat_length / 2);
    glEnd();

    glColor3f(platform_color1[0], platform_color1[1], platform_color1[2]);
    glBegin(GL_POLYGON);
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-top_plat_width / 2, -top_plat_height / 2, -top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, -top_plat_height / 2, top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, top_plat_height / 2, top_plat_length / 2);
    glVertex3f(-top_plat_width / 2, top_plat_height / 2, -top_plat_length / 2);
    glEnd();
}

void drawRod(void)
{
    glColor3f(rod_color[0], rod_color[1], rod_color[2]);
    glPushMatrix();
    glTranslatef(0.0, top_plat_height / 2, 0.0);
    drawCircle();
    drawCylinderSide();
    glTranslatef(0.0, rod_height, 0.0);
    drawCircle();
    glPopMatrix();
}

void drawDisk(int size)
{
    float i;
    float triangleAmount = 40; // # of triangles used to draw circle

    GLfloat twicePi = 2.0f * PI;

    glBegin(GL_TRIANGLE_STRIP);
    for (i = 0; i <= triangleAmount; i++)
    {
        glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (size * rod_radius * sin(-i * twicePi / triangleAmount)),
            0.0,
            0.0 + (size * rod_radius * cos(-i * twicePi / triangleAmount)));
        glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (size * rod_radius * sin(-i * twicePi / triangleAmount)),
            disk_height,
            0.0 + (size * rod_radius * cos(-i * twicePi / triangleAmount)));
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for (i = 0; i <= triangleAmount; i++)
    {
        glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(-i * twicePi / triangleAmount)),
            0.0,
            0.0 + (rod_radius * cos(-i * twicePi / triangleAmount)));
        glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(-i * twicePi / triangleAmount)),
            disk_height,
            0.0 + (rod_radius * cos(-i * twicePi / triangleAmount)));
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    // normals not working properly
    glNormal3f(0.0, 1.0, 0.0);
    for (i = 0; i <= triangleAmount; i++)
    {
        // glNormal3f(sin(i * twicePi / triangleAmount), 0.0, cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(i * twicePi / triangleAmount)),
            disk_height,
            0.0 + (rod_radius * cos(i * twicePi / triangleAmount)));
        // glNormal3f(sin(i * twicePi / triangleAmount), 0.0, cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (size * rod_radius * sin(i * twicePi / triangleAmount)),
            disk_height,
            0.0 + (size * rod_radius * cos(i * twicePi / triangleAmount)));
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    // normals not working properly
    glNormal3f(0.0, -1.0, 0.0);
    for (i = triangleAmount; i >= 0; i--)
    {
        // glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (rod_radius * sin(i * twicePi / triangleAmount)),
            0.0,
            0.0 + (rod_radius * cos(i * twicePi / triangleAmount)));
        // glNormal3f(-sin(i * twicePi / triangleAmount), 0.0, -cos(i * twicePi / triangleAmount));
        glVertex3f(
            0.0 + (size * rod_radius * sin(i * twicePi / triangleAmount)),
            0.0,
            0.0 + (size * rod_radius * cos(i * twicePi / triangleAmount)));
    }
    glEnd();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawLight();
    glEnable(GL_LIGHT0);

    // draw platform
    drawPlatform();
    glPushMatrix();
    glTranslatef(0.0, -1.0, 0.0);
    glScalef(1.0, 1.0, 1.1);
    drawPlatform();
    glPopMatrix();

    // draw rods
    drawRod();
    glPushMatrix();
    glTranslatef(0.0, 0.05, -top_plat_width); // offset to avoid z-fighting
    drawRod();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0, 0.05, top_plat_width); // offset to avoid z-fighting
    drawRod();
    glPopMatrix();

    // draw disks
    glPushMatrix();
    for (int i = 0; i < 3; i++)
    {
        stack<int> temp = s[i];

        glPushMatrix();
        cout << s[i].size() << " disks on rod " << i << "\n";
        glTranslatef(0.0, (top_plat_height / 2) + disk_height * (s[i].size() - 1), (1 - i) * top_plat_width);
        for (int j = 0; j < s[i].size(); j++)
        {
            drawDisk((float)temp.top() * length_annulus_mult + rod_radius * 2);
            glTranslatef(0.0, -disk_height, 0.0);
            temp.pop();
        }
        glPopMatrix();
    }
    cout << endl;
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}

void rotateScene()
{
    glutPostRedisplay();
}

void moveDisk(int from, int to)
{
    if (s[from].empty())
    {
        cout << "Error: no disk to move from rod " << from << endl;
        return;
    }
    if (!s[to].empty() && s[from].top() > s[to].top())
    {
        cout << "Error: cannot move disk " << s[from].top() << " from rod " << from << " to rod " << to << endl;
        return;
    }
    s[to].push(s[from].top());
    s[from].pop();

    moveCount++;
    cout << "Moved disk " << s[to].top() << " from rod " << from << " to rod " << to << endl;
    cout << "Move count: " << moveCount << endl;
    glutPostRedisplay();
}

void sceneReset()
{
    for (int i = 0; i < 3; i++)
        while (!s[i].empty())
            s[i].pop();
    for (int i = disks; i >= 1; i--)
        s[0].push(i);
    glutPostRedisplay();

    moveCount = 0;
    cout << "Scene reset\n";
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1': // move disk from rod 0 to rod 1
        moveDisk(0, 1);
        break;
    case '2': // move disk from rod 0 to rod 2
        moveDisk(0, 2);
        break;
    case '3': // move disk from rod 1 to rod 0
        moveDisk(1, 0);
        break;
    case '4': // move disk from rod 1 to rod 2
        moveDisk(1, 2);
        break;
    case '5': // move disk from rod 2 to rod 0
        moveDisk(2, 0);
        break;
    case '6': // move disk from rod 2 to rod 1
        moveDisk(2, 1);
        break;
    case 'x': // rotate 45 degree about x-axis
        glRotatef(45.0, 1.0, 0.0, 0.0);
        rotateScene();
        break;
    case 'y': // rotate 45 degree about y-axis
        glRotatef(45.0, 0.0, 1.0, 0.0);
        rotateScene();
        break;
    case 'z': // rotate 45 degree about z-axis
        glRotatef(45.0, 0.0, 0.0, 1.0);
        rotateScene();
        break;
    case 'X': // rotate -45 degree about x-axis
        glRotatef(-45.0, 1.0, 0.0, 0.0);
        rotateScene();
        break;
    case 'Y': // rotate -45 degree about y-axis
        glRotatef(-45.0, 0.0, 1.0, 0.0);
        rotateScene();
        break;
    case 'Z': // rotate -45 degree about z-axis
        glRotatef(-45.0, 0.0, 0.0, 1.0);
        rotateScene();
        break;
    case 't': // turn texture on
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_2D);
        rod_color[0] = 0.5f;
        rod_color[1] = 0.5f;
        rod_color[2] = 0.5f;
        cylinder_color[0] = 0.5f;
        cylinder_color[1] = 0.5f;
        cylinder_color[2] = 0.5f;
        circle_color[0] = 0.5f;
        circle_color[1] = 0.5f;
        circle_color[2] = 0.5f;
        platform_color1[0] = 0.5f;
        platform_color1[1] = 0.5f;
        platform_color1[2] = 0.5f;
        platform_color2[0] = 0.5f;
        platform_color2[1] = 0.5f;
        platform_color2[2] = 0.5f;
        glutPostRedisplay();
        break;
    case 'T': // turn texture off
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_2D);
        rod_color[0] = 0.0f;
        rod_color[1] = 0.0f;
        rod_color[2] = 0.0f;
        cylinder_color[0] = 0.0f;
        cylinder_color[1] = 0.0f;
        cylinder_color[2] = 0.0f;
        circle_color[0] = 0.0f;
        circle_color[1] = 0.0f;
        circle_color[2] = 0.0f;
        platform_color1[0] = 0.05f;
        platform_color1[1] = 0.05f;
        platform_color1[2] = 0.05f;
        platform_color2[0] = 0.0f;
        platform_color2[1] = 0.0f;
        platform_color2[2] = 0.0f;
        glutPostRedisplay();
        break;
    case 'r': // reset scene
        sceneReset();
        break;
    case 'a': // auto
        sceneReset();
        cout << "Auto play\n";
        automate = true;
        break;
    case 'b': // background color change
        glClearColor(1.0, 1.0, 1.0, 0.0);
        glutPostRedisplay();
        break;
    case 'B': // background color change
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glutPostRedisplay();
        break;
    case '9':
        exit(0);
        break;
    }
}

void idle()
{
    if (automate)
    {
        autoPlay();
        SDL_Delay(1000);
    }
}

void autoPlay()
{
    switch (auto_move_num)
    {
    case 0:
        moveDisk(0, 1);
        auto_move_num++;
        break;
    case 1:
        moveDisk(0, 2);
        auto_move_num++;
        break;
    case 2:
        moveDisk(1, 2);
        auto_move_num++;
        break;
    case 3:
        moveDisk(0, 1);
        auto_move_num++;
        break;
    case 4:
        moveDisk(2, 0);
        auto_move_num++;
        break;
    case 5:
        moveDisk(2, 1);
        auto_move_num++;
        break;
    case 6:
        moveDisk(0, 1);
        auto_move_num++;
        break;
    case 7:
        moveDisk(0, 2);
        auto_move_num++;
        break;
    case 8:
        moveDisk(1, 2);
        auto_move_num++;
        break;
    case 9:
        moveDisk(1, 0);
        auto_move_num++;
        break;
    case 10:
        moveDisk(2, 0);
        auto_move_num++;
        break;
    case 11:
        moveDisk(1, 2);
        auto_move_num++;
        break;
    case 12:
        moveDisk(0, 1);
        auto_move_num++;
        break;
    case 13:
        moveDisk(0, 2);
        auto_move_num++;
        break;
    case 14:
        moveDisk(1, 2);
        auto_move_num++;
        break;
    default:
        automate = false;
        auto_move_num = 0;
        break;
    }
}

int main(int argc, char **argv)
{
    dimInit();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    glutSetWindowTitle("Tower of Nguyen");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
