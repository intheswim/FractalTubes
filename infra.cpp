/* FractalTubes (formerly M3DSaver) screensaver. 
 * Copyright (c) 2003-2020 Yuriy Yakimenko
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <time.h>

#include "m3d.h"

#include <math.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "infra.h"
#include "AppParams.h"
#include "UserDefaults.h"

static FILE *log1 = NULL;
void createLog ()
{
#ifndef _DEBUG
   log1 = fopen ("log.txt", "w+t");
#endif

}
void closeLog ()
{
#ifndef _DEBUG
   if (log1)
   {
      fputs ("Closing log\n", log1);
      fclose (log1);
      log1 = 0;
   }
#endif
}
void writeLog (const char *buffer)
{
#ifndef _DEBUG
   if (log1)
   {
      fputs (buffer, log1);
      fflush (log1);
   }
#endif
}

#define SHADE_MODE  GL_SMOOTH

    static float rotation;
    static float rotationSpeed;
    
    static float detailLevel;
    
    static float screen_width;
    static float screen_height;
    
    struct pointData *pointData;
    
    struct corePointData *corePoints;
    
    static long m3d_pt_count;
    static int m3d_pt_cycle;
    static double m3d_y_range;
        
    static int grid_step;
    
    static time_t rollover_time; // time of last rollover
    
    static long rolloverPeriod;  // time between rollovers in seconds
    
    static int current_set_index;
    static double image_alpha;    // fading coeficient during rollover
    static bool after_rollover ;  // helps to fade-in image after rollover
    static float x_shift;
    
    bool m_Preview = false;
            
        
    static float stdAlpha; // corresp. to transparency
    
    static bool USE_LIGHT;
    
    static bool light2Sides;
    static float distance2Light;
    
    static float shineFactor;
    
    static int lightOption;
    static int paletteOption;
    
#ifdef USE_RUNTIME
    
    bool stopRotation;
    float baseZ;
    float baseY;
    float baseX;
    bool freeze;
#endif

void setFrameSizeForGL (int width, int height)
{    
    screen_height = height;
    screen_width = width;
	
	// Reshape
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
    gluPerspective( 45.0f, (float)width / (float)height, 0.1f, 100.0f );
    
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();	
}

static void setUpOpenGL (void)
{    
    glEnable(GL_POLYGON_SMOOTH);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_LINE_SMOOTH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    if (m_Preview)
    {
        glClearColor( 0.0f, 0.0f, 0.125f, 0.0f );
    }
    else
    {
	    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    }

	glClearDepth( 1.0f ); // default is 1; range from 0 to 1
	glEnable( GL_DEPTH_TEST );
    
	glDepthFunc( GL_LEQUAL );
    
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    
    glDisable (GL_NORMALIZE);
    
    if (USE_LIGHT)
    {        
        glShadeModel( SHADE_MODE );

        // both needed to enable light:
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    
        // without this call everything is gray
        glEnable ( GL_COLOR_MATERIAL ) ;
    }
        
	rotation = 0.0f;
            
    rollover();
}

void prepareOpenGL ()
{
    setUpOpenGL();
}

static void getTransformedCoords (float orig_x, float orig_y, float orig_z,
                           float tX, float tY, float tZ,
                           float angle, float rX, float rY, float rZ,
                           double & ret_x, double &ret_y, double & ret_z)
{
    orig_x += tX;
    orig_y += tY;
    orig_z += tZ;
    
    tX = tY = tZ = 0;
    
    double radians = M_PI * angle / 180.0;
    
    double c = cos (radians);
    double s = sin (radians);
    
    // normalize rX, rY, rZ
    
    double r = sqrt (rX * rX + rY * rY + rZ * rZ);
    
    rX /= r;
    rY /= r;
    rZ /= r;
    
    double A = rX * rX * (1-c) + c;
    double B = rX * rY * (1-c) - rZ * s;
    double C = rX * rZ * (1-c) + rY * s;
    double D = rY * rX * (1-c) + rZ * s;
    double E = rY * rY * (1-c) + c;
    double F = rY * rZ * (1-c) - rX * s;
    double G = rX * rZ * (1-c) - rY * s;
    double H = rY * rZ * (1-c) + rX * s;
    double I = rZ * rZ * (1-c) + c;
    
    ret_x = A * orig_x + B * orig_y + C * orig_z + tX;
    ret_y = D * orig_x + E * orig_y + F * orig_z + tY;
    ret_z = G * orig_x + H * orig_y + I * orig_z + tZ;
    
}

int compareByDist (struct corePointData const & a, struct corePointData const & b)
{
    return (a.dist > b.dist);
}

static void sortByDistanceToEye (float eye_x, float eye_y, float eye_z,
                          struct corePointData * pointData, long cnt)
{    
    for (int i=0; i < cnt; i++)
    {
        struct corePointData * pd = pointData + i;
        
        pd->dist = (pd->x - eye_x) * (pd->x - eye_x) +
            (pd->y - eye_y) * (pd->y - eye_y) + (pd->z - eye_z) * (pd->z - eye_z);
    }
      
    // prsemuably faster than qsort:
    std::sort (pointData, pointData + cnt, &compareByDist);
}

static void adjustNormals()
{
    long core_cnt = m3d_pt_count / m3d_pt_cycle;
    
    for (int core = 0; core < core_cnt; core++)
    {
        struct corePointData *cpd = &(corePoints[core]);
        
        for (int mode = 0; mode < 2; mode++)
        {
            for (int j = 0; j < m3d_pt_cycle; j++)
            {

                int i = cpd->index + j;
                int next = i + grid_step;
                
                if (next >= m3d_pt_count - 1)
                {
                    continue;
                }
                
                struct pointData *pd = pointData + i;
                struct pointData *pdNext1 = pointData + (next + 1);                
                
                if (j + 1 == m3d_pt_cycle) // last point connects to first on this "ring"
                {
                    pdNext1 = pointData + (next + 1 - grid_step);                    
                }
                
                pd->normx = pd->normx + pdNext1->normx;
                pd->normy = pd->normy + pdNext1->normy;
                pd->normz = pd->normz + pdNext1->normz;
                
                double r = sqrt (pd->normx * pd->normx + pd->normy * pd->normy + pd->normz * pd->normz);
                
                pd->normx = pd->normx / r;
                pd->normy = pd->normy / r;
                pd->normz = pd->normz / r;
            }
        }
    }
}

void redrawMethod()
{    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
	glLoadIdentity();
    
    float tX = x_shift;
    
    float tY = 0;
    
    float tZ = -3.0f + 0.5 * cos(rotation * 0.02f);
    
    float angle = 2.0 * rotation;
    float rX = 1.0 ;
    float rY = 0.5;
    float rZ = 0.5 ;
    
#ifdef USE_RUNTIME
    tZ = tX = 0;
    
    tZ = -3.0f + baseZ;
    
    angle = baseY + 2 * rotation;
    rX = 1;
    rY = 0;
    rZ = 0;
    
    float rX2 = 0;
    float rY2 = 1;
    float rZ2 = 0;
    float angle2 = baseX;
#endif
    
    double eye_x = 0, eye_y = 0, eye_z = 0;
    
  	glTranslated ( tX, tY, tZ );
    glRotated ( angle, rX, rY, rZ ); // (angle, x, y, z)
    
#ifdef USE_RUNTIME
    glRotated ( angle2, rX2, rY2, rZ2 );
#endif
    
    getTransformedCoords (0, 0, 0, -tX, -tY, -tZ,
                          -angle, rX, rY, rZ,
                          eye_x, eye_y, eye_z);
    
#ifdef USE_RUNTIME
    getTransformedCoords (eye_x, eye_y, eye_z, 0, 0, 0,
                          -angle2, rX2, rY2, rZ2,
                          eye_x, eye_y, eye_z);
#endif
        
    double shine = shineFactor;
    
    if (USE_LIGHT)
    {
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, light2Sides);
        
        GLfloat ambGlobal[] = { 0.2, 0.2, 0.2, 1.0 };
        
        for (int i=0; i < 3; i++)
        {
            ambGlobal[i] += (float)(shine * 0.06f);
        }
        
        glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambGlobal);
        
        GLfloat light_position[4] = { 0.0, 5.0, 0.0, 1.0 };
            
        for (int i=0; i < 3; i++)
        {
            light_position[i] = -distance2Light;
        }
    
        glLightfv( GL_LIGHT0, GL_POSITION, light_position);
        
        float ambK = (1 - shine * 0.05) ;
        
        GLfloat ambientColor[] = { ambK, ambK, ambK, 1 };
        glLightfv (GL_LIGHT0, GL_AMBIENT, ambientColor);

        float diffK = (1 - shine * 0.09) ;
        
        GLfloat diffuseColor[] = { diffK, diffK, diffK, 1 };
        glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuseColor);

        float specK = (0.1 + shine * 0.09) ;
        
        GLfloat specularColor[] = { specK, specK, specK, 0.0 };
        glLightfv (GL_LIGHT0, GL_SPECULAR, specularColor);
        
        GLfloat mat_ambient_front[] = { 0.1f, 0.1f, 0.9f, (float)image_alpha };
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient_front);
        
        GLfloat mat_diffuse_back[] = { 0, 0, 0, (float)image_alpha };
        
        glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse_back);
                
        float specMat = 0.1 + shine * 0.08;
        
        GLfloat mat_specular[] = { (float)(image_alpha * specMat * 0.5f), 
                                    (float)(image_alpha * specMat), 
                                    (float)(image_alpha * specMat), 
                                    (float)(image_alpha) };
    
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    
        GLfloat mat_shininess[] = { 7.0 };
        
        mat_shininess[0] += (float)(12.0f * shine);
    
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    
        glShadeModel ( SHADE_MODE );
        
    } // USE_LIGHT
    
    
    long core_cnt = 0;
    
    // we sort "cores" (centers of the "rings") by distance to the observer, starting from far to near.
    // Then for each core we draw the correspomding "ring", in 2 phases:
    // forst "back/rear" points, then "front".
    // This way we make sure that all points are distplayed from "back" to "front" and
    // correspondingly we can show as many leves of transparency as we want.
    // (See polygon transparency order OpenGL issue for more detail).
    
    // P.S.
    // Original code had no "separate" rings. They were all part of one big spiral.
    // When drawing as points this had no difference; however when drawing as connected
    // "rectangles" it was not possible to guarantee 100% that all points appear from
    // back to front, even with additional hacks I tried there was still part of the
    // "ring" which was showing the split line.
    
    if (corePoints != NULL)
    {
        core_cnt = m3d_pt_count / m3d_pt_cycle;
        
        sortByDistanceToEye (eye_x, eye_y, eye_z, corePoints, core_cnt);
        
    }
    
    //  glEnable (GL_POLYGON_OFFSET_LINE);
    //  glPolygonOffset (-1.f,-1.f);
       
    glBegin ( GL_QUADS ); 
    {
        for (int core = 0; core < core_cnt; core++)
        {
            struct corePointData *cpd = &(corePoints[core]);
            
            for (int mode = 0; mode < 2; mode++)
            {
                for (int j = 0; j < m3d_pt_cycle; j++)
                    // for (int i=0, next = grid_step; i < m3d_pt_count - grid_step - 1; i++, next++)
                    // for (int i = m3d_pt_count - grid_step - 2, next = i + grid_step; i >= 0; i--, next--)
                {
                    int i = cpd->index + j;
                    int next = i + grid_step;
                    
                    if (next >= m3d_pt_count - 1)
                    {
                        continue;
                    }
                    
                    struct pointData *pd = pointData + i;
                    
                    bool show = (pd->ucGreen > 0 || pd->ucBlue > 0 || pd->ucRed > 0);
                    
                    if (show == false)
                    {
                        continue;
                    }
                    
                    double vx = eye_x - pd->x;
                    double vy = eye_y - pd->y;
                    double vz = eye_z - pd->z;
                    
                    if (mode==0) { pd->cMult = 0; }
                    
                    if (mode == 0)
                    {
                       // if (mult > 0) continue; // passes back only
                        double mult = vx * pd->normx + vy * pd->normy + vz * pd->normz;
                        pd->cMult = (mult < 0) ? -1 : 1;
                        
                        if (pd->cMult > 0) continue;
                    }
                    else
                    {
                        if (pd->cMult <= 0) continue; // passes front only
                    }
                    
                    float b = pd->ucBlue;
                    float g = pd->ucGreen;
                    float r = pd->ucRed;
                    
                    if (g > 0 || b > 0)
                    {
                        if (image_alpha == 1.0)
                        {
                            glColor4ub( r, g, b, (unsigned char)(stdAlpha * 255.0));                            
                        }
                        else
                        {
                            glColor4ub( r * image_alpha, g * image_alpha , b * image_alpha, (unsigned char)(stdAlpha * 255.0));
                        }
                    }
                    else
                    {
                        if (pd->ucRed > 0)
                        {
                            float alpha = r;
                            
                            float alpha2 = r * (2.0 - r /255.0f);
                                                        
                            alpha = 0.5 * (alpha + alpha2);
                            
                            glColor4ub ( (unsigned char)(r * image_alpha), 0, 0, (unsigned char)(alpha * image_alpha));
                        }
                    }
                    
                    
                    if ( USE_LIGHT )
                    {
                        glNormal3d ( pd->normx, pd->normy, pd->normz );
                    }

                    for (int mode2=0; mode2 < 2; mode2++)
                    {
                        if (mode2==1)
                        {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        }
                        else
                        {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        }
                    
                        struct pointData *pdNext = pointData + next;
                    
                        if (j + 1 == m3d_pt_cycle) // last point connects to first on this "ring"
                        {
                            struct pointData *pd1 = pointData + (i + 1 - grid_step);
                        
                            struct pointData *pdNext1 = pointData + (next + 1 - grid_step);
                        
                            glVertex3d ( pd->x , pd->y , pd->z );
                            glVertex3d ( pd1->x , pd1->y , pd1->z );
                            glVertex3d ( pdNext1->x , pdNext1->y , pdNext1->z );
                            glVertex3d ( pdNext->x , pdNext->y , pdNext->z );
                        }
                        else
                        {
                            struct pointData *pd1 = pointData + (i + 1);
                        
                            struct pointData *pdNext1 = pointData + (next + 1);
                                                
                            glVertex3d ( pd->x , pd->y , pd->z );
                            glVertex3d ( pd1->x , pd1->y , pd1->z );
                            glVertex3d ( pdNext1->x , pdNext1->y , pdNext1->z );
                            glVertex3d ( pdNext->x , pdNext->y , pdNext->z );
                        }

                   }  // mode1
                    
                }
            }
        } // 2nd for

        glEnd();
    }
    
    if (!m_Preview)
    {
#ifdef USE_RUNTIME
       /* int lx, ly, lz;
        GetNextLightDir (lx, ly, lz, 0);
        NSString *s = [NSString stringWithFormat:@"%d %d %d", lx, ly, lz];
        [self outputText:8 y:4 r:.8f g:.8f b:.8f font:descrFont str:s fontSize:12];  */
#else
#endif
        
    }
    
    glFlush();
}

static void freePoints()
{
    if (m3d_pt_count > 0)
    {
        delete[] pointData;
        delete[] corePoints;
        
        corePoints = NULL;
    }
    
    m3d_pt_count = 0;
}

void rollover()
{
    freePoints();
    
    rotation = 10.0 * (float)rand() / RAND_MAX;
    
    if (rand() & 1)
    {
        rotationSpeed = fabs (rotationSpeed);
    }
    else
    {
        rotationSpeed = -fabs (rotationSpeed);
    }
    
    x_shift = ((float)rand() / RAND_MAX - 0.5f) * 1.6f;
    
    image_alpha = 0.0;
    
    
    float scale = detailLevel;
    
#ifdef USE_RUNTIME
    scale = 2 * scale;
#endif
    
    if (m_Preview) scale = 0.4;
    
    M3DDrawer *m3d = new M3DDrawer (300 * scale, 200 * scale);
    
  //  std::unique_ptr <int *> p;
    
  //  std::unique_ptr<M3DDrawer> m3d (m3d_raw);
    
    m3d->SetDefaultPrams ();
    
    bool altPalette = AppParams::isPaletteAlternative();
    
    int palIndex = 0;
    
#ifndef USE_RUNTIME
    if (altPalette)
    {
        palIndex = rand();
    }
#endif
    
    m3d->SetAltPalette (altPalette, palIndex);
    
    bool lightChanged = NO;
    
    lightOption = AppParams::getLightSource();
    
    paletteOption = AppParams::getPalette();
    
    light2Sides = AppParams::getLight2Sides() ? GL_TRUE : GL_FALSE;
    
    distance2Light = AppParams::getLightDistance();
    
    bool newUseLight = AppParams::getLightMoving();
        
    if (newUseLight != USE_LIGHT)
    {
        USE_LIGHT = newUseLight;
        lightChanged = YES;
    }
    
    if ( USE_LIGHT )
    {
        m3d->DisableBrightness();
    }
    
    double light_x = 1; 
    double light_y = 1; 
    double light_z = 1; 
    
    m3d->SetLightDirection(light_x, light_y, light_z);
    
    if (!m3d->Paint (current_set_index, true, &pointData, &corePoints ))
    {
          if ((!m_Preview) && (AppParams::getDetailLevel() > 0))
          {
            writeLog ("Out of memory. Try smaller detail level.");
          }
          else if (!m_Preview)
          {
            writeLog ("Out of Memory");
          }

          return;
    }
    
    m3d_pt_count = m3d->getPointCount(m3d_pt_cycle);
    
    m3d_y_range = m3d->getYRange();
    
    delete m3d;
    
    shineFactor = AppParams::getGlossLevel();
        
    if (AppParams::getGlossRandom())
    {
        shineFactor = 0.1f * (rand() % 101);
    }
    
    if (lightChanged)
    {
        if (USE_LIGHT)
        {
            glShadeModel( SHADE_MODE );
            
            // both needed to enable light:
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            
            glEnable ( GL_COLOR_MATERIAL ) ;
        }
        else
        {
            glShadeModel( SHADE_MODE );
        
            // both needed to enable light:
            glDisable (GL_LIGHTING);
            glDisable (GL_LIGHT0);
        
            glDisable ( GL_COLOR_MATERIAL ) ;
        }
    }
    
    rolloverPeriod = AppParams::getIntervalInSecs();
    
    rollover_time = time (NULL);
        
    grid_step = m3d_pt_cycle;

    //char buffer[256];
    //sprintf (buffer, "%d in cycle\n", m3d_pt_cycle);
    //writeLog (buffer);
    
    if (USE_LIGHT)
    {
        adjustNormals();
    }
    
    after_rollover = true;

}

void animateOneFrame( )
{
    // Adjust our state
#ifdef USE_RUNTIME
    if (NO == stopRotation)
#endif
    {
        rotation += rotationSpeed;
    }
	
	// Redraw
    // InvalidateRect (window, NULL, false);
    // UpdateWindow (window);
    
#ifdef USE_RUNTIME
    if (stopRotation)
    {
        return;
    }
#endif
    
    if (after_rollover == false)
    {
        if ((time(NULL) - rollover_time > rolloverPeriod) && (image_alpha > 0)
#ifdef USE_RUNTIME
            && (stopRotation==NO)
#endif
            )
        {
            image_alpha -= 0.01;
            
            if (rollover_time == 0)
            {
                image_alpha -= 0.03;
            }
            
            if (image_alpha < 0)
            {
                image_alpha = 0;
            }
        }
        
        else if (image_alpha == 0)
        {

            selectNextImage();
            
            rollover();
        }
    }
    else
    {
        image_alpha += 0.01;
        
        if (image_alpha >= 1.0)
        {
            image_alpha = 1.0;
            after_rollover = false;
        }
    }
    
}

void CleanupAnimation()
{
     freePoints();
}

void selectNextImage()
{
#ifndef FREE_VERSION

    std::vector<int> indexes;
    
    for (int i=0; i < PRESET_COUNT; i++)
    {
        bool selected = AppParams::getImageSelected (i);
        
        if (selected)
        {
            indexes.push_back (i);
        }
    }

    if (indexes.size() == 0)
    {
        current_set_index = 0;
        return;
    }
    
    if (AppParams::getShuffle() )
    {
        if (indexes.size() > 1)
        {
            size_t temp = std::find(indexes.begin(), indexes.end(), current_set_index) - indexes.begin();

            if (temp < indexes.size() ) // current index is among selected
            {
                int increase = rand() % (indexes.size() - 1);
                
                temp = (temp + increase + 1) % indexes.size();
                
                current_set_index = indexes [ temp] ;
            }
            else // current index is NOT among selected. Select any of the selected randomly.
            {
                temp = ( rand() % indexes.size() );
                
                current_set_index = indexes [temp] ;
            }
        }
        else
        {
            current_set_index = indexes [0] ;
        }
    }
    else
    {
        current_set_index++;
        
        if (current_set_index >= PRESET_COUNT)
        {
            current_set_index = 0;
        }
        
        while (false == AppParams::getImageSelected (current_set_index))
        {
            current_set_index++;
            if (current_set_index >= PRESET_COUNT)
            {
                current_set_index = 0;
            }
        }
    }
    
#else
    current_set_index = 0;
#endif

}

void GetConfig (void)
{
    m3d_pt_count = 0;
    current_set_index = 0;
        
    if (m_Preview == NO)
    {
        current_set_index = -1;
        selectNextImage();
    }
        
    corePoints = NULL;
        
#ifdef USE_RUNTIME
        freeze = NO;
      //  timerInterval = 1/10.0f;
#else
      //  timerInterval = 1/30.0f;
#endif
                        
    srand ((unsigned int)time(NULL));
        
    stdAlpha = AppParams::getTransparency();
        
    rotationSpeed = AppParams::getRotationSpeed() * 0.01f;
        
    detailLevel = AppParams::getDetailValue();
        
    lightOption = AppParams::getLightSource();
        
    paletteOption = AppParams::getPalette();
        
    USE_LIGHT = AppParams::getLightMoving();
        
    rolloverPeriod = AppParams::getIntervalInSecs();
}

