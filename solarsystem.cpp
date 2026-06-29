#undef UNICODE
#undef _UNICODE
#define _CRT_SECURE_NO_WARNINGS// FOR UNNECESSARY WARNINGS
#define ILUT_USE_OPENGL
#include <GL/glut.h>
#include<cstdlib>
#include<ctime>
#include<cmath>
#include<string>
#include<vector>
#include<algorithm>
#include<iostream>
#include<IL/il.h> //BELONG TO dEVIL imagre library to load image files as textures for the planets
#include<IL/ilu.h>
#include<IL/ilut.h>
#define PI 3.1415926535897932384626433832795
#define NUM_STARS 5000
#define NUM_ASTEROIDS 800

static float timeSpeed=1.0f;
static float currentTime=0.0f;

static int hoveredPlanetIndex=-1;

static float camDistance = 80.0f;
static float camAngleX=25.0f;//rotation of the camera around the scenes
static float camAngleY= -45.0f;

static int focusTarget = -1;
static float targetSmoothX = 0,targetSmoothY = 0, targetSmoothZ=0;
static bool isPlanetSelectedView = false;
static float selectedViewCanDistance = 18.0f;
static float selectedViewCanAngleY = 0.0f;
static float selectedViewCamAngleX = 15.0f;

static int lastMouseX=0, lastMouseY=0;// to change the position of the camera using mouse
static int mouseState = -1;
static int selectedPlanetIndex = -1;

static bool isPaused= false;
static bool showOrbits = true;
static bool showLables= true;
static bool showGrid = false;

GLuint starsList;
GLuint orbitsList;
GLuint asteroidsList;

GLuint planetTextures[9];
GLuint sunTexture;
bool texturesLoaded=false;

struct CelestialBody{
    std::string name;
    float distance;
    float radius;
    float orbitSpeed;
    float rotationSpeed;
    float inclination;
    float r,g,b;
    int moonCount;
    bool hasRings;
    float ringInner, ringOuter;
    std::string info;

    float curX, curY , curZ;

    GLuint textureId;
};

bool loadPlanetTexture(){
    if(texturesLoaded)
    return true;

    const char* planetImageFiles[]={

    "C:\\Users\\Admin\\Desktop\\solar\\media\\sun.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\mercury.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\venus.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\earth.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\mars.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\jupiter.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\saturn.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\uranus.jpg",
    "C:\\Users\\Admin\\Desktop\\solar\\media\\neptune.jpg",
};



    ilInit();
    iluInit();
    ilutInit();
    ilutRenderer(ILUT_OPENGL);

    glGenTextures(9, planetTextures);

    for (int i = 0; i<9; i++){
        ILuint imageID;
        ilGenImages(1,&imageID);
        ilBindImage(imageID);

        std::cout<<"Loading texture:"<<planetImageFiles[i]<<std::endl;

        if(!ilLoadImage(planetImageFiles[i])){
            std:: cerr<<"Failed to load image:"<<planetImageFiles[i]<<std::endl;
            std::cerr<<"Error:" <<iluErrorString(ilGetError())<<std::endl;
            ilDeleteImages(1, &imageID);
            continue;
        }
        if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)){
            std::cerr<<"Failed to convert image:"<<planetImageFiles[i]<<std::endl;
            ilDeleteImages(1,&imageID);
            continue;
        }
        glBindTexture(GL_TEXTURE_2D , planetTextures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        gluBuild2DMipmaps(GL_TEXTURE_2D, 4 , ilGetInteger(IL_IMAGE_WIDTH),
         ilGetInteger(IL_IMAGE_HEIGHT), GL_RGBA,
         GL_UNSIGNED_BYTE, ilGetData());

         ilDeleteImages(1, &imageID);
         std::cout<<"Successfully loaded:"<<planetImageFiles[i]<<std::endl;
    }
    sunTexture=planetTextures[0];

    texturesLoaded=true;
    std::cout<<"All textures loaded successfully!"<<std::endl;
    return true;
}

std::vector<CelestialBody>planets = {
{"Mercury",6.0f,0.38f,4.7f,0.5f,7.0f,0.7f,0.7f,0.7f,0,false,0,0,
"Mercury is the smallest planet in the Solar System and the closest to the Sun.It has no atmosphere to retain heat.",0},
{"Venus", 10.0f,0.95f,3.5f,0.1f,3.4f,0.9f,0.8f,0.4f,0,false, 0,0,
"Venus is the second planet from the Sun.It is the hottest planet in our solar system due to its thick toxic atmosphere.",0},
{"Earth", 14.0f,1.0f,2.98f,20.0f,0.0f,0.2f,0.5f,1.0f,1,false, 0,0,
"Earth is the third planet from the Sun and the only known planet to support life. It has a diverse climate and a protective atmosphere.",0},
{"Mars", 20.0f,0.53f,2.41f,19.0f,1.85f,1.0f,0.5f,0.5f,2,false, 0,0,
"Mars is the fourth planet from the Sun and the second-smallest planet in the Solar System.",0},
{"Jupiter", 35.0f,2.8f,1.31f,45.0f,1.3f,1.0f,0.5f,0.0f,79,false, 0,0,
"Jupiter is the largest planet in the Solar System and is known for its Great Red Spot, a giant storm that has been raging for centuries.",0},
{"Saturn", 48.0f,2.4f,0.97f,42.0f,2.5f,1.0f,1.0f,0.5f,82,true, 3.0f,5.5f,
"Saturn is the sixth planet from the Sun and is known for its prominent ring system.",0},
{"Uranus", 62.0f,1.6f,0.68f,30.0f,0.77f,0.5f,1.0f,1.0f,27,true, 2.0f,3.0f,
"Uranus is the seventh planet from the Sun and is unique for its extreme axial tilt, which causes it to rotate on its side.",0},
{"Neptune", 78.0f,1.5f,0.54f,32.0f,1.77f,0.5f,0.5f,1.0f,14,false, 0,0,
"Neptune is the eighth and farthest known planet from the Sun in the Solar System.",0},
};


void drawString(void* font,float x, float y, const char* str){
    glRasterPos2f(x,y);
    for(const char* c = str; *c !='\0'; ++c){
        glutBitmapCharacter(font,*c);
    }
}

void initStars(){
    starsList=glGenLists(1);
    glNewList(starsList , GL_COMPILE);
    glBegin(GL_POINTS);
    for (int i=0; i<NUM_STARS; ++i){
        float theta = ((float)rand() / RAND_MAX) * 2.0f * PI;
float phi = acosf(1.0f - 2.0f * ((float)rand() / RAND_MAX));
float dist = 500.0f + (rand()%300);

float x = dist * sinf(phi) * cosf(theta);
float y = (dist * cosf(phi)) * 0.3f;  // flatten vertically
float z = dist * sinf(phi) * sinf(theta);

float b = 0.7f + ((rand()%100)/300.0f);  // brighter stars
glColor3f(b, b, b);
        glColor3f(b,b,b);

        glVertex3f(x,y,z);

    }

    glEnd();
    glEnable(GL_LIGHTING );
    glEndList();
}

void initAsteroids(){
    asteroidsList = glGenLists(1);
    glNewList(asteroidsList, GL_COMPILE);
    glDisable(GL_LIGHTING);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for(int i=0; i<NUM_ASTEROIDS; i++){
        float angle = (float)(rand()%360)*PI/180.0f;
        float dist = 26.0f+((rand()%40/10.0f));
        float height = ((rand()%20)/10.0f)-1.0f;
        float r = 0.6f+((rand()%20)/100.0f);
       glColor3f(r,r*0.9f,r*0.8f);
       glVertex3f(dist * cosf(angle),height,dist*sinf(angle));

    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEndList();
}

void initOrbits(){
    orbitsList = glGenLists(1);
    glNewList(orbitsList, GL_COMPILE);
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);
    for (const auto& p :planets){
        glBegin(GL_LINE_LOOP);
        glColor4f(p.r*0.5f,p.g*0.5f,p.b*0.5f,0.4f);
        for(int i = 0;i<360;++i){
        float rad = (float)i*PI/180.0f;
        float x= p.distance *cosf(rad);
        float z=p.distance*sinf(rad);
        float y = z * sinf(p.inclination*PI/180.0f);
    z=z*cosf(p.inclination*PI/180.0f);
        glVertex3f(x,y,z);
        }
        glEnd();
    }


    glEnable(GL_LIGHTING);
    glEndList();}

    void init(){
     glClearColor(0.01f,0.01f,0.03f,1.0f);
     glEnable(GL_DEPTH_TEST);

     glEnable(GL_LIGHTING);
     glEnable(GL_LIGHT0);
     glEnable(GL_COLOR_MATERIAL);
     glEnable(GL_NORMALIZE);
     glShadeModel(GL_SMOOTH);

     glEnable(GL_TEXTURE_2D);

     GLfloat ambient[]={0.05f,0.05f,0.05f,1.0f};
     GLfloat diffuse[]={1.0f,0.95f,0.9f,1.0f};
     GLfloat specular[]={1.0f,1.0f,1.0f,1.0f};
     glLightfv(GL_LIGHT0,GL_AMBIENT, ambient);
     glLightfv(GL_LIGHT0,GL_DIFFUSE, diffuse);
     glLightfv(GL_LIGHT0, GL_SPECULAR , specular);
     glLightfv(GL_LIGHT0,GL_POSITION , new float[4] {0.0f,0.0f,0.0f,1.0f});

     initStars();
     initAsteroids();
     initOrbits();

     if(loadPlanetTexture()){
        for(int i = 0; i<(int)planets.size();i++){
            planets[i].textureId = planetTextures[i+1];
        }
     }
     srand((unsigned int)time(NULL));

    }
void drawSun(){
    glPushMatrix();

    if(texturesLoaded && sunTexture !=0){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, sunTexture);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        GLfloat matColor[]={1.0f , 1.0f ,1.0f,1.0f};
        GLfloat matSpecular[]={1.0f,1.0f,1.0f,1.0f};
        GLfloat matShininess[]={100.0f};
        GLfloat matEmission[]={0.3f,0.2f,0.0f,1.0f};

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,matColor);
        glMaterialfv(GL_FRONT , GL_SPECULAR,matSpecular);
        glMaterialfv(GL_FRONT , GL_SHININESS,matShininess);
        glMaterialfv(GL_FRONT , GL_EMISSION,matEmission);

        glColor3f(1.0f,1.0f,1.0f);

        GLUquadricObj*sunSphere = gluNewQuadric();
        gluQuadricTexture(sunSphere, GL_TRUE);
        gluQuadricNormals(sunSphere, GLU_SMOOTH);

        glRotatef(currentTime*5.0f,0.0f,1.0f,0.0f);
        gluSphere(sunSphere, 3.5f,50,50);
        gluDeleteQuadric(sunSphere);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (int i =1; i<=6; i++){
            glColor4f(1.0f,0.6f,0.1f,0.25f-(i*0.04));
            glutSolidSphere(3.5f+(i*0.4f),40,40);
        }
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        glMaterialfv(GL_FRONT , GL_EMISSION, new float [4] {0,0,0,1});
   }
     else{
        glDisable(GL_LIGHTING);
        glColor3f(1.0f,0.8f,0.2f);
        glutSolidSphere(3.5f,50,50);

        glEnable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
        for(int i=1; i<=6; i++){
            glColor4f(1.0f,0.6f,0.1f,0.25f-(i*0.04f));
            glutSolidSphere(3.5f+(i*0.4f),40,40);
        }
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        GLfloat emission[]={1.0f,0.5f,0.0f,1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION,emission);
    }
    glPopMatrix();
}
void drawRing(float inner,float outer){
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.8f,0.75f,0.6f,0.7f);
    glPushMatrix();
    glRotatef(90.0f,1.0f,0.0f,0.0f);
    GLUquadricObj* disk = gluNewQuadric();
    gluDisk(disk, inner, outer, 60,1);
    gluDeleteQuadric(disk);
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

}

void drawTexturedPlanet(const CelestialBody& p){
    glPushMatrix();
    if(texturesLoaded && p.textureId !=0){
       glEnable(GL_TEXTURE_2D);
       glBindTexture(GL_TEXTURE_2D, p.textureId);

     GLfloat matColor[]={1.0f , 1.0f, 1.0f,1.0f};
     GLfloat matSpecular[]={0.5f,0.5f,0.5f,1.0f};
     GLfloat matShininess[]={50.0f};

     glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, matColor);
     glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
     glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

     glColor3f(1.0f,1.0f,1.0f);

    }
    else{
        glDisable(GL_TEXTURE_2D);
        glColor3f(p.r,p.g,p.b);
        GLfloat matColor[]={p.r,p.g,p.b,1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matColor);


    }
    GLUquadricObj* sphere = gluNewQuadric();
    gluQuadricTexture(sphere , GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);

    gluSphere(sphere, p.radius, 30 , 30);
    gluDeleteQuadric(sphere);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawPlanets(){
    int idx=0;
    for (auto& p : planets){
        glPushMatrix();
        float angle = currentTime * p.orbitSpeed*0.1f;
        float x = p.distance * cosf(angle);
        float z = p.distance * sinf(angle);
        float y = z * sinf(p.inclination*PI/180.0f);
        z = z * cosf(p.inclination*PI/180.0f);
        p.curX = x;
        p.curY = y;
        p.curZ = z;
        glTranslatef(x,y,z);
        if(showLables && selectedPlanetIndex != idx && focusTarget != idx && !isPlanetSelectedView){
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor3f(1,1,1);
        glRasterPos3f(0.0f,p.radius+1.5f,0.0f);
        for (char c : p.name) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        glEnable(GL_LIGHTING);
    }
glRotatef(currentTime * p.rotationSpeed,0.0f,1.0f,0.0f);
drawTexturedPlanet(p);
if(p.hasRings){
    drawRing(p.ringInner, p.ringOuter);
}
if(p.moonCount >0){
    glPushMatrix();
    glRotatef(currentTime *80.0f,0.0f,1.0f,0.0f);
    glTranslatef(p.radius+1.0f,0.0f,0.0f);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.6f,0.6f,0.6f);
    glutSolidSphere(p.radius*0.25f,10,10);
    glPopMatrix();

}
glPopMatrix();
idx++;
}
}

void checkPlanetHover(int x,int y){
    if (isPlanetSelectedView)return;

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble winX, winY, winZ;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    float mouseX = (float)x;
    float mouseY = (float)viewport[3]-(float)y;

    int newHoveredIndex = -1;

    for (int i=0l; i<(int)planets.size();++i){
        gluProject(planets[i].curX, planets[i].curY, planets[i].curZ, modelview, projection, viewport, &winX, &winY, &winZ);
        float dist = (float)sqrt(pow(mouseX-winX,2)+pow(mouseY-winY,2));
        if (dist < 20.0f){
            newHoveredIndex = i;
            break;
        }
    }
    if (newHoveredIndex != hoveredPlanetIndex){
        hoveredPlanetIndex = newHoveredIndex;
        glutPostRedisplay();
    }
}

void drawHoverPopup(){
    if (hoveredPlanetIndex == -1 )return;
    if (isPlanetSelectedView)return;
    if(selectedPlanetIndex != -1 )return;

GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];
GLdouble winX, winY, winZ;

glGetIntegerv(GL_VIEWPORT, viewport);
glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
glGetDoublev(GL_PROJECTION_MATRIX, projection);

CelestialBody& p = planets[hoveredPlanetIndex];
gluProject(p.curX, p.curY, p.curZ, modelview, projection, viewport, &winX, &winY, &winZ);
int w = glutGet(GLUT_WINDOW_WIDTH);
int h = glutGet(GLUT_WINDOW_HEIGHT);
 glDisable(GL_LIGHTING);
 glDisable(GL_DEPTH_TEST);
 glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,w,0,h);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int boxW = 100;
    int boxH = 60;
    int xPos=(int)winX+25;
    int yPos = (int)winY- boxH/2;
    if (xPos + boxW > w)xPos =(int)winX - boxW - 25;
    if (yPos +boxH>h)yPos = h - boxH;
    if(yPos<0)yPos=0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f,0.05f,0.1f,0.7f);

    glBegin(GL_QUADS);
    glVertex2f(xPos,yPos);
    glVertex2f(xPos+boxW,yPos);
    glVertex2f(xPos+boxW,yPos+boxH);
    glVertex2f(xPos,yPos+boxH);
    glEnd();

    void* fonText= GLUT_BITMAP_HELVETICA_10;
int textX= xPos + 10;
int textY= yPos + boxH - 20;
 glColor3f(1,1,1);
 char buffer[100];
    sprintf(buffer,"Size: %.2f Earths", p.radius);
    drawString(fonText, textX, textY, buffer);
    textY -= 15;

    sprintf(buffer,"Distance: %.1f million km", p.distance);
    drawString(fonText, textX, textY, buffer);
    textY -= 15;

    sprintf(buffer,"Moons: %d", p.moonCount);
    drawString(fonText, textX, textY, buffer);


 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();

 glDisable(GL_BLEND);
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_LIGHTING);
}
void drawSelectedPlanetView(){
    if (selectedPlanetIndex == -1)return;

    CelestialBody& p = planets[selectedPlanetIndex];

   if (texturesLoaded && p.textureId !=0){
       glEnable(GL_TEXTURE_2D);
       glBindTexture(GL_TEXTURE_2D, p.textureId);

       glEnable(GL_LIGHTING);
       glEnable(GL_LIGHT0);

     GLfloat matColor[]={1.0f , 1.0f, 1.0f,1.0f};
     GLfloat matSpecular[]={0.5f,0.5f,0.5f,1.0f};
     GLfloat matShininess[]={50.0f};

     glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, matColor);
     glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
     glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT , GL_EMISSION, new float[4] {0,0,0,1});
     glColor3f(1.0f,1.0f,1.0f);

    }
    else{
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glColor3f(p.r,p.g,p.b);
        GLfloat matColor[]={p.r,p.g,p.b,1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matColor);

}
glPushMatrix();
glRotatef(selectedViewCamAngleX,0.0f,1.0f,0.0f);
glRotatef(selectedViewCamAngleX,1.0f,0.0f,0.0f);

glTranslatef(0.0f,0.0f,-selectedViewCanDistance);

glRotatef(currentTime * p.rotationSpeed,0.0f,1.0f,0.0f);

const float FIXED_PLANET_SIZE = 1.5F;

GLUquadricObj* sphere = gluNewQuadric();
gluQuadricTexture(sphere , GL_TRUE);
gluQuadricNormals(sphere, GLU_SMOOTH);

gluSphere(sphere, FIXED_PLANET_SIZE, 50, 50);

gluDeleteQuadric(sphere);

if(p.hasRings){
    glPushMatrix();
    glRotatef(90.0f,1.f,0.0f,0.0f);
    glRotatef(currentTime*15.0f,0.0f,0.0f,1.0f);


    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.8f,0.75f,0.6f,0.7f);

    GLUquadricObj*disk=gluNewQuadric();
    gluDisk(disk,p.ringInner,p.ringOuter,60,1);
    gluDeleteQuadric(disk);

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();

}
glPopMatrix();
glDisable(GL_TEXTURE_2D);

}

void checkPlanetSelection(int x,int y){
    if (isPlanetSelectedView){
        selectedPlanetIndex = -1;
        isPlanetSelectedView = false;
        isPaused = false;

        selectedViewCamAngleX=15.0f;
        selectedViewCanAngleY=0.0f;
        selectedViewCanDistance = 18.0f;

        glutPostRedisplay();
        return;
     }
     GLint viewport[4];
     GLdouble modelview[16];
     GLdouble projection[16];
     GLdouble winX, winY, winZ;

     glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    float mouseX = (float)x;
    float mouseY = (float)viewport[3]-(float)y;

    selectedPlanetIndex=-1;

    for (int i=0l; i<(int)planets.size();++i){
        gluProject(planets[i].curX, planets[i].curY, planets[i].curZ, modelview, projection, viewport, &winX, &winY, &winZ);
        float dist = (float)sqrt(pow(mouseX-winX,2)+pow(mouseY-winY,2));
        if (dist < 25.0f){
            selectedPlanetIndex = i;
            isPlanetSelectedView = true;
             isPaused = true;
            break;
        }
    if (dist < 25.0f){
        selectedPlanetIndex=i;
        isPlanetSelectedView = true;
        isPaused = true;
         break;

    }
    }
    glutPostOverlayRedisplay();
}
void drawHUD(){
    if (isPlanetSelectedView) return;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0,glutGet(GLUT_WINDOW_WIDTH),0,glutGet(GLUT_WINDOW_HEIGHT));
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

    glColor3f(1,1,1);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    drawString(GLUT_BITMAP_HELVETICA_18,20.0f,(float)(h-30),"Solar System Interaction");

    std::string status ="Status:" + std::string(isPaused?"Paused":"Running");
    drawString(GLUT_BITMAP_HELVETICA_12,20.0f,(float)(h-50),status.c_str());
    drawString(GLUT_BITMAP_HELVETICA_12,20.0f,(float)(h-70),"Click on a planet to view details. Click again to exit.");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
 }
 void drawInfoPopupSelectedView(){
    if (selectedPlanetIndex == -1) return;

    CelestialBody& p = planets[selectedPlanetIndex];

    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int boxW = 220;
    int boxH = 120;
    int xPos = 20;
    int yPos = 20;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.05f, 0.1f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + boxW, yPos);
        glVertex2f(xPos + boxW, yPos + boxH);
        glVertex2f(xPos, yPos + boxH);
    glEnd();

    glColor3f(1, 1, 1);
    void* font = GLUT_BITMAP_HELVETICA_12;
    int textX = xPos + 10;
    int textY = yPos + boxH - 20;

    drawString(font, textX, textY, p.name.c_str()); textY -= 18;

    char buffer[200];
    sprintf(buffer, "Radius: %.2f Earths", p.radius);
    drawString(font, textX, textY, buffer); textY -= 15;

    sprintf(buffer, "Moons: %d", p.moonCount);
    drawString(font, textX, textY, buffer); textY -= 15;

    drawString(GLUT_BITMAP_HELVETICA_10, textX, textY, p.info.c_str());

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
 void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(isPlanetSelectedView && selectedPlanetIndex != -1){
    glLoadIdentity();

    float eyeDist=selectedViewCanDistance;
    float eyeX= 0.0f;
    float eyeY=selectedViewCanDistance*0.3f;
    float eyeZ=selectedViewCanDistance;

    float radY =selectedViewCanAngleY*PI/180.0f;
    float radX = selectedViewCamAngleX*PI/180.0f;

    eyeX = eyeDist * sinf(radY)* cosf(radX);
    eyeZ = eyeDist * cosf(radY)* cosf(radX);
    eyeY = eyeDist * sinf(radX);

    float lookY = 5.0f;

    gluLookAt(eyeX, eyeY, eyeZ, 0.0f, lookY, 0.0f, 0.0f, 1.0f, 0.0f);

glDisable(GL_LIGHTING);
glDisable(GL_TEXTURE_2D);
glCallList(starsList);
glEnable(GL_LIGHTING);

drawSelectedPlanetView();
drawInfoPopupSelectedView();
     }
      else{
glLoadIdentity();

float ex,ey,ez,cx,cy,cz;
if(focusTarget==-1){
float radY = camAngleY*PI/180.0f;
float radX = camAngleX*PI/180.0f;
ey=camDistance * sinf(radX);
float proj=camDistance * cosf(radX);
ex = proj * sinf(radY);
ez = proj * cosf(radY);
cx=0.0f;
cy=0.0f;
cz=0.0f;
ex=proj*sinf(radY);
ez=proj*cosf(radY);
 }
 else{
    CelestialBody& p= planets[focusTarget];
    targetSmoothX +=(p.curX-targetSmoothX)*0.1f;
    targetSmoothY +=(p.curY-targetSmoothY)*0.1f;
    targetSmoothZ +=(p.curZ-targetSmoothZ)*0.1f;

    cx = targetSmoothX;
    cy = targetSmoothY;
    cz = targetSmoothZ;
    float offset = p.radius *6.0f + 10.0f;
    ex=cx + offset * sinf(camAngleY*PI/180.0f);
    ez=cz + offset * cosf(camAngleY*PI/180.0f);
    ey=cy + offset * sinf(camAngleX*PI/180.0f);
 }
 gluLookAt(ex,ey,ez,cx,cy,cz,0.0f,1.0f,0.0f);
    glDisable(GL_TEXTURE_2D);
    glCallList(starsList);
    if(showOrbits) glCallList(orbitsList);
    glPushMatrix();
    glRotatef(90.0f,1.0f,0.0f,0.0f);
    glCallList(asteroidsList);
    glPopMatrix();
    if(showGrid && focusTarget == -1){
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.3f,0.3f,0.3f);
        glBegin(GL_LINES);
        for (int i=-100; i<=100; i+=5){
            glVertex3f((float)i,0.0f,-100.0f);
            glVertex3f((float)i,0.0f,100.0f);
            glVertex3f(-100.0f,0.0f,(float)i);
            glVertex3f(100.0f,0.0f,(float)i);
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
    drawSun();
    drawPlanets();
    drawHoverPopup();
}
glutSwapBuffers();
}
void idle(){
    if(!isPaused){
        currentTime += timeSpeed*0.01f;
    } glutPostRedisplay();

}
void mouse(int button, int state, int x, int y){
    if (button == 3){
        if (isPlanetSelectedView){
            selectedViewCanDistance -= 2.0f;
            if (selectedViewCanDistance < 10) selectedViewCanDistance = 10;
        }
        else{
            camDistance-=5.0f; if (camDistance < 10) camDistance = 10;
        }
    }
    else if (button == 4){
        if (isPlanetSelectedView){
            selectedViewCanDistance += 2.0f;
            if(selectedViewCanDistance>50)selectedViewCanDistance=50;
        }
        else{
            camDistance += 5.0f;
                if (camDistance > 300) camDistance = 300;
        }
    }
if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
            mouseState = GLUT_LEFT_BUTTON;
            lastMouseX = x;

            checkPlanetSelection(x,y);

        }
        else if(state == GLUT_UP){
            mouseState = -1;
        } glutPostRedisplay();
    }
    void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / (float)h, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void motion(int x, int y) {
    if (mouseState == GLUT_LEFT_BUTTON) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;
        if (isPlanetSelectedView) {
            selectedViewCanAngleY += dx * 0.5f;
            selectedViewCamAngleX += dy * 0.5f;
        } else {
            camAngleY += dx * 0.5f;
            camAngleX += dy * 0.5f;
            if (camAngleX > 89) camAngleX = 89;
            if (camAngleX < -89) camAngleX = -89;
        }
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}



   int main(int argc,char** argv){
    glutInit(&argc , argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB| GLUT_DEPTH);
    glutInitWindowSize(1280,720);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Solar System Simulator with Realistics Textures");

    init();;
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutReshapeFunc(reshape);
   glutMotionFunc(motion);
   glutMouseFunc(mouse);
   glutPassiveMotionFunc(checkPlanetHover);
   glutMainLoop();
   return 0 ;
   }























