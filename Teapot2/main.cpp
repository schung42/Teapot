
#include <GLUT/glut.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <vector>
#include "SOIL.h"
using namespace std;

struct Vec3d{
    double x, y, z, s, t;
    
};

//struct for face indices.
struct triples{
    int id1;
    int id2;
    int id3;
};

//array of vertices.
vector<Vec3d> vertices;

//array of faces, face normals, and unit face normals.
vector<triples> faceIndexes;
vector<Vec3d> faceNormals;
vector<Vec3d> unitNormals;
vector<Vec3d> unitVectors;

//keyboard variables
float vertical = 0.0, horizontal = 0.0;
float up =0.0;
float right1= 0.0;
bool lightOn = false;
bool textureOn = true;
bool reflectionOn = false;

//texture variables
GLuint reflection_texture;
GLuint texture_teapot;
GLuint teapot_background;


void parse_object_data()
{
    FILE * stream;
    char buffer[200];
    
    stream = fopen("/Users/ShaunChung/Desktop/Teapot2/Teapot2/teapot_0.obj", "r");
   
        while ( !feof(stream) )
        {
            if ( fgets (buffer , 200 , stream) == NULL )
                break;
            
            char* element = strtok(buffer, " ");
            char* xarg = strtok(NULL, " ");
            char* yarg = strtok(NULL, " ");
            char* zarg = strtok(NULL, " ");
            
            if ( strcmp(element, "v") ==0 ) {
                Vec3d vec;
                vec.x =atof(xarg);
                vec.y =atof(yarg);
                vec.z =atof(zarg);
                
                double theta = atan2(vec.z, vec.x);
                double pi = 3.14159265;
                vec.s = (theta+pi)/(2*pi);
                vec.t = vec.y/3.15;
                
                vertices.push_back(vec);
                
            }
            else if ( strcmp(element, "f") == 0 ) {
                triples face;
                face.id1 =atoi(xarg);
                face.id2 =atoi(yarg);
                face.id3 =atoi(zarg);
                faceIndexes.push_back(face);
                
            }
        }
    fclose(stream);
}

void calculate_face_normals()
{
    double index1, index2, index3;
    
    Vec3d vec1, vec2, vec3;
    
    for (int findex= 0; findex<faceIndexes.size(); findex++) {
        triples face = faceIndexes[findex];
        
        index1= face.id1;
        index2 = face.id2;
        index3 = face.id3;
        
        vec1 = vertices[index1-1];
        vec2 = vertices[index2-1];
        vec3 = vertices[index3-1];
        
        Vec3d V,W,N;
        V.x = vec2.x - vec1.x;
        V.y = vec2.y - vec1.y;
        V.z = vec2.z - vec1.z;
        
        W.x = vec3.x - vec1.x;
        W.y = vec3.y - vec1.y;
        W.z = vec3.z - vec1.z;
        
        //normals
        N.x = (V.y * W.z)-(V.z * W.y);
        N.y = (V.z * W.x)-(V.x * W.z);
        N.z = (V.x * W.y)-(V.y * W.x);
        
        faceNormals.push_back(N);
    }
}

void calculate_unit_vectors()
{
    double xSquared, ySquared, zSquared, dist;
    for(int index = 0; index< faceNormals.size(); index++)
    {
        Vec3d face, unitVec;
        
        face = faceNormals[index];
        
        xSquared = pow(face.x, 2);
        ySquared = pow(face.y, 2);
        zSquared = pow(face.z, 2);
        
        dist = pow( xSquared+ySquared+zSquared, .5);
        
        unitVec.x = face.x /dist;
        unitVec.y = face.y /dist;
        unitVec.z = face.z /dist;
        
        unitNormals.push_back(unitVec);
    }

    for(int index = 0; index< vertices.size(); index++)
    {
        Vec3d vec, normalized;
        vec = vertices[index];
        
        xSquared = pow(vec.x, 2);
        ySquared = pow(vec.y, 2);
        zSquared = pow(vec.z, 2);
        dist = pow( xSquared+ySquared+zSquared, .5);
        
        normalized.x = vec.x/dist;
        normalized.y = vec.y/dist;
        normalized.z = vec.z/dist;
        normalized.s = vec.s;
        normalized.t = vec.t;
        
        unitVectors.push_back(normalized);
        
    }
}

void init()
{
    glClearColor(0.5,0.5,1.0,1.0);
    glEnable(GL_DEPTH_TEST);
    parse_object_data();
    calculate_face_normals();
    calculate_unit_vectors();
    
}


void lighting_init()
{
    GLfloat position[] ={1.0, 1.0, 1.0, 1.0};
    GLfloat light_ambient[] = { -1.0, -1.0, 0.0, 1.0 };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
}

void texture_init()
{
    
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture_teapot);
    glBindTexture(GL_TEXTURE_2D, texture_teapot);
    
    texture_teapot = SOIL_load_OGL_texture("/Users/ShaunChung/Desktop/Teapot2/Teapot2/steel_texture.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    static int length = 10;
    static int zValue = 10;
    
    glBegin(GL_QUADS);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,-length);
    glTexCoord2f(0,0);
    glVertex3f(-length,-length,-zValue);
    
    glNormal3f(1,0,-1);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,length);
    glTexCoord2f(0,1);
    glVertex3f(-length,length,-zValue);
    
    glNormal3f(1,0,-1);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,length);
    glTexCoord2f(1,1);
    glVertex3f(length,length,-zValue);
    
    glNormal3f(1,0,-1);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,-length);
    glTexCoord2f(1,0);
    glVertex3f(length,-length,-zValue);
    glEnd();
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &teapot_background);
    glBindTexture(GL_TEXTURE_2D, teapot_background);
    
    teapot_background = SOIL_load_OGL_texture("/Users/ShaunChung/Desktop/Teapot2/Teapot2/steel_background1.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
}

void reflection_init()
{
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture_teapot);
    glBindTexture(GL_TEXTURE_2D, texture_teapot);
    
    texture_teapot = SOIL_load_OGL_texture("/Users/ShaunChung/Desktop/Teapot2/Teapot2/probe_background.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    static int length = 10;
    static int zValue = 10;
    
    
    
    glBegin(GL_QUADS);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,-length);
    glTexCoord2f(0,0);
    glVertex3f(-length,-length,-zValue);
    
    glNormal3f(1,0,-1);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,-length,length);
    glTexCoord2f(0,1);
    glVertex3f(-length,length,-zValue);
    
    glNormal3f(1,0,-1);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,length);
    glTexCoord2f(1,1);
    glVertex3f(length,length,-zValue);
    
    glNormal3f(1,0,-1);
    glMultiTexCoord2fARB(GL_TEXTURE2_ARB,length,-length);
    glTexCoord2f(1,0);
    glVertex3f(length,-length,-zValue);
    glEnd();
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &reflection_texture);
    glBindTexture(GL_TEXTURE_2D, reflection_texture);
    
    reflection_texture = SOIL_load_OGL_texture("/Users/ShaunChung/Desktop/Teapot2/Teapot2/probe1.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
    
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


void display(void)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    
    glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    
    glMatrixMode( GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
    
    if(lightOn)
    {
        lighting_init();
    }
    else if(textureOn) {
        texture_init();
    }
    else if(reflectionOn){
        reflection_init();
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
    glPushMatrix();
    
    //scroll translate and scale
    GLfloat transform[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, transform);
    glLoadIdentity();
    glScalef(.5, .5, .5);
    glTranslatef(0.0, -2.0, 3.0);
    glMultMatrixf(transform);
    
    //up rotations
    glGetFloatv(GL_MODELVIEW_MATRIX, transform);
    glLoadIdentity();
    glRotatef(up,1.0, 0.0, 0.0);
    glMultMatrixf(transform);
    
    //right rotations
    glGetFloatv(GL_MODELVIEW_MATRIX, transform);
    glLoadIdentity();
    glRotatef(right1, 0.0, 1.0, 0.0);
    glMultMatrixf(transform);
    
    //scroll down or up, and left or right
    glGetFloatv(GL_MODELVIEW_MATRIX, transform);
    glLoadIdentity();
    glTranslatef(horizontal, -vertical, -3.0);
    glMultMatrixf(transform);
    
    glBegin(GL_TRIANGLES);
    for(int i = 0; i < faceIndexes.size(); i++)
    {
        triples face = faceIndexes[i];
        
        Vec3d v1 = vertices[face.id1-1];
        Vec3d v2 = vertices[face.id2-1];
        Vec3d v3 = vertices[face.id3-1];
        
        glNormal3f(v1.x, v1.y, v1.z);
        glTexCoord2f(v1.s, v1.t);
        glVertex3f(v1.x, v1.y, v1.z);
        
        glNormal3f(v2.x, v2.y, v2.z);
        glTexCoord2f(v2.s, v2.t);
        glVertex3f(v2.x, v2.y, v2.z);
        
        glNormal3f(v3.x, v3.y, v3.z);
        glTexCoord2f(v3.s, v3.t);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    
    glEnd();
    
    glTranslatef(0.0, 0.1, 0.0);
    glMultMatrixf(transform);
    
    glPopMatrix();
    glutSwapBuffers();
    glFlush ();
    
    glutPostRedisplay();
}

void keyboard(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
        up -= 2.9;
    else if(key == GLUT_KEY_DOWN)
        up+= 2.9;
    else if(key == GLUT_KEY_LEFT)
        right1 -= 2.9;
    else if(key == GLUT_KEY_RIGHT)
        right1 +=2.9;
    else if(key == 'l'){
        lightOn = !lightOn;
        if(!lightOn)
            glDisable(GL_LIGHTING);
    }
    else if(key == 'r'){
        reflectionOn = !reflectionOn;
        textureOn = false;
    }
   
    else if(key == 't'){
        textureOn = !textureOn;
        reflectionOn = false;
    }
    else if(key == 'q'){
        exit(0);
    }
}


void timerFunc(int value) {
        glutPostRedisplay();
        glutTimerFunc (30, timerFunc, 0);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (550, 550);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("MP3 Teapot");
    init();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutTimerFunc(30, timerFunc, 0);
    
    glutMainLoop();
    return 0;
}
