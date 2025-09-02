#include <stdio.h>
#include <math.h>
#include <glut.h>
#include <SOIL2.h>
#include <ctype.h>

/* ===================== Textures ===================== */
GLuint ice;
GLuint steele;
GLuint gbrick;

void loadTexture() {
    ice = SOIL_load_OGL_texture(
        "C:\\Users\\Dhupika Bandara\\Desktop\\WestrosCG\\Textures\\winterfell\\ice.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
    );
    if (!ice) {
        printf("Texture loading failed: %s\n", SOIL_last_result());
        return;
    }
    glBindTexture(GL_TEXTURE_2D, ice);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void loadTexture1() {
    steele = SOIL_load_OGL_texture(
        "C:\\Users\\Dhupika Bandara\\Desktop\\WestrosCG\\Textures\\winterfell\\steele.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
    );
    if (!steele) {
        printf("Texture loading failed: %s\n", SOIL_last_result());
        return;
    }
    glBindTexture(GL_TEXTURE_2D, steele);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void loadTexture2() {
    gbrick = SOIL_load_OGL_texture(
        "C:\\Users\\Dhupika Bandara\\Desktop\\WestrosCG\\Textures\\winterfell\\brick.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
    );
    if (!gbrick) {
        printf("Texture loading failed: %s\n", SOIL_last_result());
        return;
    }
    glBindTexture(GL_TEXTURE_2D, gbrick);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/* ---------------- Window ---------------- */
int win_posx = 100, win_posy = 100;
int win_width = 720, win_height = 480;
int win_cx = 360, win_cy = 240;

int prev_ms = 0;
int warping = 0;

/* ---------------- Camera ---------------- */
float fovy = 60.0f, z_near = 0.1f, z_far = 1000.0f;

float cam_x = 0.0f, cam_y = 2.0f, cam_z = 6.0f;
float cam_vx = 0.0f, cam_vy = 0.0f, cam_vz = 0.0f;

float cam_yaw = 3.14159f;
float cam_pitch = 0.0f;

const float CAM_SPEED = 3.0f;
const float CAM_ACCEL = 4.0f;
const float CAM_DAMP = 0.0f;
const float SENS = 0.0025f;
const float PITCH_MIN = -1.35f;
const float PITCH_MAX = 1.35f;

/* ---------------- Input ---------------- */
unsigned char keyDown[256] = { 0 };

static void setKey(unsigned char k, int down) {
    k = (unsigned char)tolower(k);
    keyDown[k] = (unsigned char)(down ? 1 : 0);
}
static int isDown(unsigned char k) {
    return keyDown[(unsigned char)tolower(k)] ? 1 : 0;
}

/* ---------------- UI text ---------------- */
void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string);
        string = string + 1;
    }
}

void displayLabel() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, win_width, 0, win_height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 1.0f, 1.0f);

    void* font = GLUT_BITMAP_8_BY_13;
    float x = 10.0f;
    float y = (float)win_height - 20.0f;
    int lh = 18;

    renderBitmapString(x, y, font, "W/S: Forward / Back");
    y = y - lh; renderBitmapString(x, y, font, "A/D: Strafe Left / Right");
    y = y - lh; renderBitmapString(x, y, font, "Q/E: Ascend / Descend");
    y = y - lh; renderBitmapString(x, y, font, "Mouse: Free Look (Yaw/Pitch)");
    y = y - lh; renderBitmapString(x, y, font, "ESC: Exit");

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* ---------------- Debug axes ---------------- */
void axes() {
    glDisable(GL_LIGHTING);

    glLineWidth(2.0f);
    glBegin(GL_LINES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);

    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_LIGHTING);
}

/* ---------------- Camera math & update ---------------- */
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

void updateCamera(float dt) {
    float fwd = 0.0f, strafe = 0.0f, up = 0.0f;
    if (isDown('w')) fwd = fwd + 1.0f;
    if (isDown('s')) fwd = fwd - 1.0f;
    if (isDown('d')) strafe = strafe + 1.0f;
    if (isDown('a')) strafe = strafe - 1.0f;
    if (isDown('q')) up = up + 1.0f;
    if (isDown('e')) up = up - 1.0f;

    float cp = cosf(cam_pitch), sp = sinf(cam_pitch);
    float sy = sinf(cam_yaw), cy = cosf(cam_yaw);

    float fx = cp * sy, fy = sp, fz = cp * cy;
    float rx = cy, ry = 0.0f, rz = -sy;
    float ux = 0.0f, uy = 1.0f, uz = 0.0f;

    float dx = fx * fwd + rx * strafe + ux * up;
    float dy = fy * fwd + ry * strafe + uy * up;
    float dz = fz * fwd + rz * strafe + uz * up;

    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len > 1.0f && len > 0.00001f) {
        dx = dx / len; dy = dy / len; dz = dz / len;
    }

    float tvx = dx * CAM_SPEED;
    float tvy = dy * CAM_SPEED;
    float tvz = dz * CAM_SPEED;

    float a = clampf(CAM_ACCEL * dt, 0.0f, 1.0f);
    cam_vx = lerpf(cam_vx, tvx, a);
    cam_vy = lerpf(cam_vy, tvy, a);
    cam_vz = lerpf(cam_vz, tvz, a);

    if (CAM_DAMP > 0.0f) {
        float damp = clampf(1.0f - CAM_DAMP * dt, 0.0f, 1.0f);
        cam_vx = cam_vx * damp;
        cam_vy = cam_vy * damp;
        cam_vz = cam_vz * damp;
    }

    cam_x = cam_x + cam_vx * dt;
    cam_y = cam_y + cam_vy * dt;
    cam_z = cam_z + cam_vz * dt;
}

void applyCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cp = cosf(cam_pitch), sp = sinf(cam_pitch);
    float sy = sinf(cam_yaw), cy = cosf(cam_yaw);

    float dirx = cp * sy;
    float diry = sp;
    float dirz = cp * cy;

    gluLookAt(
        cam_x, cam_y, cam_z,
        cam_x + dirx, cam_y + diry, cam_z + dirz,
        0.0f, 1.0f, 0.0f
    );
}

/* ---------------- Input callbacks ---------------- */
void keyboardDown(unsigned char key, int, int) {
    if (key == 27) exit(0);
    setKey(key, 1);
}
void keyboardUp(unsigned char key, int, int) {
    setKey(key, 0);
}
void passiveMotion(int x, int y) {
    if (warping) {
        warping = 0;
        return;
    }
    int dx = x - win_cx;
    int dy = y - win_cy;

    cam_yaw = cam_yaw + dx * SENS;
    cam_pitch = cam_pitch - dy * SENS;
    cam_pitch = clampf(cam_pitch, PITCH_MIN, PITCH_MAX);

    warping = 1;
    glutWarpPointer(win_cx, win_cy);
}


void iceCube() {
    glPushMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ice);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    {
        float sPlane[4] = { 2.0f, 0.0f, 0.0f, 0.0f };
        float tPlane[4] = { 0.0f, 2.0f, 0.0f, 0.0f };
        glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
    }

    glTranslatef(3.0f, 0.0f, 0.0f);
    glScalef(6.0f, 1.0f, 6.0f);
    glutSolidCube(1.0f);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

void wallBase() {
    int i;
    for (i = 0; i < 7; i = i + 1) {
        glPushMatrix();
        glTranslatef(i * 6.0f, 0.0f, 0.0f);
        iceCube();
        glPopMatrix();
    }
}

void secondWallBase() {
    int i;
    for (i = 0; i < 7; i = i + 1) {
        glPushMatrix();
        glTranslatef(i * 6.0f, 0.0f, 0.0f);
        if (i % 2 == 0) {
            glRotatef(30.0f, 0.0f, 1.0f, 0.0f);
        }
        iceCube();
        glPopMatrix();
    }
}

void iceWallBase() {
    glPushMatrix();
    wallBase();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.0f, 1.0f, 0.0f);
    secondWallBase();
    glPopMatrix();
}

void iceWall() {
    int i;

    iceWallBase();

    for (i = 1; i < 12; i = i + 1) {
        glPushMatrix();

        if (i % 2 == 0) {
            glTranslatef(1.75f, i * 2.0f, 0.0f);
        }
        else {
            glTranslatef(-1.75f, i * 2.0f, 0.0f);
        }

        {
            float scaleZ = 1.0f - (i * 0.05f);
            if (scaleZ < 0.2f) scaleZ = 0.2f;
            glScalef(1.0f, 1.0f, scaleZ);
        }

        iceWallBase();
        glPopMatrix();
    }
}


void elevatorBase() {
    glPushMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, steele);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    glPushMatrix();
    glTranslatef(0.25f, 2.0f, 0.0f);
    glScalef(1.0f, 8.0f, 1.0f);
    glutSolidCube(0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25f, 4.25f, 0.0f);
    glScalef(2.0f, 0.5f, 0.5f);
    glutSolidCube(0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.25f, 4.25f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(2.0f, 0.5f, 0.5f);
    glutSolidCube(0.5f);
    glPopMatrix();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

static void elevatorLevel() {
    glPushMatrix();
    elevatorBase();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.5f, 0.0f, 0.0f);
    elevatorBase();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.5f, 0.0f, 2.5f);
    elevatorBase();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 2.5f);
    elevatorBase();
    glPopMatrix();
}

void elevator() {
    int i;

    elevatorLevel();

    for (i = 1; i < 10; i = i + 1) {
        glPushMatrix();
        glTranslatef(0.0f, i * 2.0f, 0.0f);
        elevatorLevel();
        glPopMatrix();
    }
}


float elevatorY = 0.5f;
float elevatorDir = 1.0f;
float elevatorMin = 0.5f;
float elevatorMax = 20.0f;
float elevatorSpeed = 0.05f;

float elevatorX = 1.25f;
float elevatorZ = 1.25f;

float elevatorRadius = 0.5f;
float elevatorHeight = 1.2f;

void drawElevatorCar(void) {
    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, steele);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    glTranslatef(elevatorX, elevatorY, elevatorZ);
    glColor3f(0.85f, 0.85f, 0.90f);

    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_FALSE);

    gluCylinder(q, elevatorRadius, elevatorRadius, elevatorHeight, 24, 1);
    gluDisk(q, 0.0f, elevatorRadius, 24, 1);
    glTranslatef(0.0f, 0.0f, elevatorHeight);
    gluDisk(q, 0.0f, elevatorRadius, 24, 1);

    gluDeleteQuadric(q);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void elevatorTimer(int value) {
    elevatorY = elevatorY + elevatorDir * elevatorSpeed;

    if (elevatorY > elevatorMax) {
        elevatorY = elevatorMax;
        elevatorDir = -1.0f;
    }
    if (elevatorY < elevatorMin) {
        elevatorY = elevatorMin;
        elevatorDir = 1.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, elevatorTimer, 0);
}


void castleblack() {


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gbrick);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    {
        float sPlane[4] = { 2.5f, 0.0f, 0.0f, 0.0f };
        float tPlane[4] = { 0.0f, 2.5f, 0.0f, 0.0f };
        glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    
    glPushMatrix();
    glTranslatef(-12.0f, 10.0f * 0.5f, -8.0f);
    glScalef(3.8f, 10.0f, 3.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 10.0f * 0.5f, -8.0f);
    glScalef(3.8f, 10.0f, 3.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-12.0f, 10.0f * 0.5f, 8.0f);
    glScalef(3.8f, 10.0f, 3.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 10.0f * 0.5f, 8.0f);
    glScalef(3.8f, 10.0f, 3.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

  
    glPushMatrix();
    glTranslatef(-12.0f, 10.0f + 0.6f, -8.0f);
    glScalef(4.2f, 0.6f, 4.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 10.0f + 0.6f, -8.0f);
    glScalef(4.2f, 0.6f, 4.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-12.0f, 10.0f + 0.6f, 8.0f);
    glScalef(4.2f, 0.6f, 4.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 10.0f + 0.6f, 8.0f);
    glScalef(4.2f, 0.6f, 4.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

   
    glPushMatrix();
    glTranslatef(-12.0f, 10.0f, -8.0f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCone(1.25f, 3.0f, 24, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 10.0f, -8.0f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCone(1.25f, 3.0f, 24, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-12.0f, 10.0f, 8.0f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCone(1.25f, 3.0f, 24, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 10.0f, 8.0f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCone(1.25f, 3.0f, 24, 2);
    glPopMatrix();

   
    glPushMatrix();
    glTranslatef(-8.0f, 4.0f * 0.5f, -8.0f);
    glScalef(8.0f, 4.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(8.0f, 4.0f * 0.5f, -8.0f);
    glScalef(8.0f, 4.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 4.0f * 0.5f, 8.0f);
    glScalef(24.0f, 4.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-12.0f, 4.0f * 0.5f, 0.0f);
    glScalef(2.0f, 4.0f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12.0f, 4.0f * 0.5f, 0.0f);
    glScalef(2.0f, 4.0f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(-4.0f, 5.0f * 0.5f, -8.0f);
    glScalef(1.2f, 5.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4.0f, 5.0f * 0.5f, -8.0f);
    glScalef(1.2f, 5.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 5.8f, -8.0f);
    glScalef(8.8f, 1.0f, 2.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 7.8f, -8.0f);
    glScalef(9.5f, 2.6f, 3.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(-3.5f, 3.0f * 0.5f, 0.0f);
    glScalef(6.0f, 3.0f, 4.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.5f, 3.0f + 0.45f, 0.0f);
    glRotatef(32.0f, 0, 0, 1);
    glScalef(6.4f, 0.6f, 4.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.5f, 3.0f + 0.45f, 0.0f);
    glRotatef(-32.0f, 0, 0, 1);
    glScalef(6.4f, 0.6f, 4.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
}

/* ---------------- GLUT lifecycle ---------------- */
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyCamera();

    
    {
        GLfloat keyDir[4] = { -0.25f, -0.90f, -0.35f, 0.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, keyDir);
    }

    axes();
    displayLabel();

    glPushMatrix();
    glTranslatef(2.0f, 0.0f, 13.0f);
    glRotatef(180,0,1,0);
    glScalef(0.5f, 0.5f, 0.5f);
    castleblack();
    glPopMatrix();

   
    glPushMatrix();
    glTranslatef(-20.0f, 0.0f, -3.5f);
    iceWall();
    glPopMatrix();

    elevator();
    drawElevatorCar();
    

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    win_width = w; win_height = h;
    win_cx = w / 2; win_cy = h / 2;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, (GLfloat)w / (GLfloat)h, z_near, z_far);

    glMatrixMode(GL_MODELVIEW);

    warping = 1;
    glutWarpPointer(win_cx, win_cy);
}

void idle() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - prev_ms) * 0.001f;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.05f) dt = 0.05f;
    prev_ms = now;

    updateCamera(dt);
    glutPostRedisplay();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);

#ifdef GLUT_CURSOR_NONE
    glutSetCursor(GLUT_CURSOR_NONE);
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    loadTexture();
    loadTexture1();
    loadTexture2();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    
    glEnable(GL_LIGHT1);    

 
    {
        GLfloat globalAmb[4] = { 0.25f, 0.25f, 0.27f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
    }

  
    {
        GLfloat L0amb[4] = { 0.15f, 0.15f, 0.20f, 1.0f };
        GLfloat L0dif[4] = { 1.00f, 1.00f, 1.00f, 1.0f };
        GLfloat L0spec[4] = { 0.70f, 0.70f, 0.80f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, L0amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, L0dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, L0spec);
    }


    {
        GLfloat L1amb[4] = { 0.35f, 0.35f, 0.40f, 1.0f };
        GLfloat L1dif[4] = { 0.30f, 0.30f, 0.35f, 1.0f };
        GLfloat L1pos[4] = { 0.0f,  1.0f,  0.0f, 0.0f }; 
        glLightfv(GL_LIGHT1, GL_AMBIENT, L1amb);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, L1dif);
        glLightfv(GL_LIGHT1, GL_POSITION, L1pos);
    }

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    glutTimerFunc(16, elevatorTimer, 0);
}

/* ---------------- Main ---------------- */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

    glutInitWindowPosition(win_posx, win_posy);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("3D (Smooth Camera + Ice Wall + Elevator + Castle Black)");

    init();
    prev_ms = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    warping = 1;
    glutWarpPointer(win_cx, win_cy);

    glutMainLoop();
    return 0;
}
