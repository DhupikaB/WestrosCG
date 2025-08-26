// Smooth free-fly camera + mouse look (keeps your structure)
// Uses math.h, GLUT; draws axes + help label only.

#include <stdio.h>
#include <math.h>
#include <glut.h>
#include <SOIL2.h> // not used yet, kept for your project
#include <ctype.h> 

// ---------------- Window ----------------
int win_posx = 100, win_posy = 100;
int win_width = 720, win_height = 480;
int win_cx = 360, win_cy = 240;

int prev_ms = 0;
int warping = 0; // guard when we warp mouse to center

// ---------------- Legacy vars (kept) ----------------
float fovy = 60.0f, z_near = 0.1f, z_far = 1000.0f;

// ---------------- Camera state (smooth) ----------------
float cam_x = 0.0f, cam_y = 2.0f, cam_z = 6.0f;  // position
float cam_vx = 0.0f, cam_vy = 0.0f, cam_vz = 0.0f; // velocity

float cam_yaw = 3.14159f;   // radians, facing -Z
float cam_pitch = 0.0f;     // radians

// tuning
const float CAM_SPEED = 3.0f;    // base m/s
const float CAM_ACCEL = 4.0f;    // how quickly we approach target vel
const float CAM_DAMP = 0.0f;     // extra damping (0..6), 0 is fine
const float SENS = 0.0025f;  // mouse sensitivity (rad per pixel)
const float PITCH_MIN = -1.35f;   // ~ -77°
const float PITCH_MAX = 1.35f;   // ~ +77°

// ---------------- Input state ----------------
unsigned char keyDown[256] = { 0 };
static void setKey(unsigned char k, int down) {
    k = (unsigned char)tolower(k);
    keyDown[k] = (unsigned char)(down ? 1 : 0);
}
static int isDown(unsigned char k) {
    return keyDown[(unsigned char)tolower(k)] ? 1 : 0;
}

// ---------------- UI text ----------------
void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string);
        ++string;
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
    glColor3f(1.0f, 1.0f, 1.0f);

    void* font = GLUT_BITMAP_8_BY_13;
    float x = 10.0f;
    float y = win_height - 20.0f;
    int lh = 18;

    renderBitmapString(x, y, font, "W/S: Forward / Back");
    renderBitmapString(x, (y -= lh), font, "A/D: Strafe Left / Right");
    renderBitmapString(x, (y -= lh), font, "Q/E: Ascend / Descend");
    renderBitmapString(x, (y -= lh), font, "Mouse: Free Look (Yaw/Pitch)");
    renderBitmapString(x, (y -= lh), font, "ESC: Exit");

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

// ---------------- Debug axes ----------------
void axes() {
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f); // X
    glVertex3f(-100.0f, 0.0f, 0.0f); glVertex3f(100.0f, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f); // Y
    glVertex3f(0.0f, -100.0f, 0.0f); glVertex3f(0.0f, 100.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f); // Z
    glVertex3f(0.0f, 0.0f, -100.0f); glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();
    glLineWidth(1.0f);
}

// ---------------- Camera math & update ----------------
static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

void updateCamera(float dt) {
    // Input intent
    float fwd = 0.0f, strafe = 0.0f, up = 0.0f;
    if (isDown('w')) fwd += 1.0f;
    if (isDown('s')) fwd -= 1.0f;
    if (isDown('d')) strafe += 1.0f;
    if (isDown('a')) strafe -= 1.0f;
    if (isDown('q')) up += 1.0f;
    if (isDown('e')) up -= 1.0f;

    // Build basis
    float cp = cosf(cam_pitch), sp = sinf(cam_pitch);
    float sy = sinf(cam_yaw), cy = cosf(cam_yaw);

    // forward (camera look), right (horizontal), world up
    float fx = cp * sy, fy = sp, fz = cp * cy;
    float rx = cy, ry = 0, rz = -sy;
    float ux = 0, uy = 1, uz = 0;

    // desired direction (normalize if >1 so diagonal speed is consistent)
    float dx = fx * fwd + rx * strafe + ux * up;
    float dy = fy * fwd + ry * strafe + uy * up;
    float dz = fz * fwd + rz * strafe + uz * up;
    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len > 1.0f && len > 0.00001f) { dx /= len; dy /= len; dz /= len; }

    // target velocity
    float tvx = dx * CAM_SPEED;
    float tvy = dy * CAM_SPEED;
    float tvz = dz * CAM_SPEED;

    // smooth towards target velocity
    float a = clampf(CAM_ACCEL * dt, 0.0f, 1.0f);
    cam_vx = lerpf(cam_vx, tvx, a);
    cam_vy = lerpf(cam_vy, tvy, a);
    cam_vz = lerpf(cam_vz, tvz, a);

    // optional damping
    if (CAM_DAMP > 0.0f) {
        float damp = clampf(1.0f - CAM_DAMP * dt, 0.0f, 1.0f);
        cam_vx *= damp; cam_vy *= damp; cam_vz *= damp;
    }

    // integrate
    cam_x += cam_vx * dt;
    cam_y += cam_vy * dt;
    cam_z += cam_vz * dt;
}

void applyCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cp = cosf(cam_pitch), sp = sinf(cam_pitch);
    float sy = sinf(cam_yaw), cy = cosf(cam_yaw);

    float dirx = cp * sy;
    float diry = sp;
    float dirz = cp * cy;

    gluLookAt(cam_x, cam_y, cam_z,
        cam_x + dirx, cam_y + diry, cam_z + dirz,
        0.0f, 1.0f, 0.0f);
}

// ---------------- Input callbacks ----------------
void keyboardDown(unsigned char key, int, int) {
    if (key == 27) exit(0); // ESC
    setKey(key, 1);
}
void keyboardUp(unsigned char key, int, int) {
    setKey(key, 0);
}

void passiveMotion(int x, int y) {
    if (warping) { warping = 0; return; }
    int dx = x - win_cx;
    int dy = y - win_cy;

    cam_yaw += dx * SENS;
    cam_pitch -= dy * SENS;
    cam_pitch = clampf(cam_pitch, PITCH_MIN, PITCH_MAX);

    // recenter for infinite mouse look
    warping = 1;
    glutWarpPointer(win_cx, win_cy);
}



// ---------------- GLUT lifecycle ----------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    applyCamera();
    axes();
    displayLabel();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (w <= 0) w = 1; if (h <= 0) h = 1;
    win_width = w; win_height = h;
    win_cx = w / 2; win_cy = h / 2;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, (GLfloat)w / (GLfloat)h, z_near, z_far);
    glMatrixMode(GL_MODELVIEW);

    // keep mouse centered after resize
    warping = 1;
    glutWarpPointer(win_cx, win_cy);
}

void idle() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - prev_ms) * 0.001f;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.05f) dt = 0.05f; // clamp big steps
    prev_ms = now;

    updateCamera(dt);
    glutPostRedisplay();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // ash sky
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
#ifdef GLUT_CURSOR_NONE
    glutSetCursor(GLUT_CURSOR_NONE); // hide cursor for free look
#endif
}

// ---------------- Main ----------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

    glutInitWindowPosition(win_posx, win_posy);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("3D (Smooth Camera + Mouse Look)");

    init();
    prev_ms = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    // center mouse at start
    warping = 1;
    glutWarpPointer(win_cx, win_cy);

    glutMainLoop();
    return 0;
}
