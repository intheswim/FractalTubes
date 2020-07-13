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

// This is GLUT based version.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#include "infra.h"
#include <unistd.h>

const unsigned int TARGET_FPS = 20;
const double TIME_PER_FRAME = 1000.0 / TARGET_FPS;
int g_start_time_ms;
int g_current_frame_number;

static void Init(void)
{
    GetConfig();
    prepareOpenGL();

    g_start_time_ms = glutGet(GLUT_ELAPSED_TIME);
    g_current_frame_number = 0;
}

static void Reshape(int width, int height)
{
    setFrameSizeForGL (width, height);
}

static void onIdle (void)
{
    double end_frame_time_ms, end_rendering_time_ms, waste_time_ms;

    animateOneFrame();

    glutPostRedisplay(); // using this instead to call Display().

    // wait until it is time to draw the current frame

    end_frame_time_ms = g_start_time_ms + (g_current_frame_number + 1) * TIME_PER_FRAME;

    end_rendering_time_ms = glutGet(GLUT_ELAPSED_TIME);

    waste_time_ms = end_frame_time_ms - end_rendering_time_ms;

    if (waste_time_ms > 0.0)
    {
         usleep (waste_time_ms * 1000.0);    // sleep parameter should be in seconds
    }
 
    // update frame number
    g_current_frame_number = g_current_frame_number + 1;
}

static void onDisplay()
{
    redrawMethod();

    glutSwapBuffers();
}

bool isStandalone = true;

static void parse_arguments(int argc, char *argv[]) 
{
    int idx;

	for (idx = 1; idx < argc; idx++) 
    {
		if (strcasecmp(argv[idx], "-root") == 0) 
        {
			isStandalone = false;
		}
    }
}

int main(int argc, char **argv)
{
    parse_arguments (argc, argv);

    int root_window_id = 0;

    /* open the display (connect to the X server) */
    /* Display * dpy = XOpenDisplay (getenv ("DISPLAY"));

    if (!dpy) printf ("dpy is NULL\n");

    char *xwin = getenv ("XSCREENSAVER_WINDOW");

    int root_window_id = 0;

    if (xwin)
    {
        root_window_id = strtol (xwin, NULL, 0);
    }

    if (isStandalone == false) 
    {
		// Get the root window
        if (root_window_id == 0)
        {
		    // root = DefaultRootWindow(dpy);
            printf ("usage as standalone app: %s -window\n", argv[0]);
            return EXIT_FAILURE;
        }
	} 
    else 
    {
		// Let's create our own window.
		int screen = DefaultScreen(dpy);

		root_window_id = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 24, 48, 860,
				640, 1, BlackPixel(dpy, screen), BlackPixel(dpy, screen));

		XMapWindow(dpy, root_window_id);

        XStoreName( dpy, root_window_id, "Sample xscreensaver" ) ;

        printf ("root window : %d\n", root_window_id);
	}   */

    glutInit (&argc, argv);

    GLenum type = GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE;

    glutInitDisplayMode(type);

    if (root_window_id != 0)
    {
        glutSetWindow (root_window_id);
        /* int t = glutCreateSubWindow (root_window_id, 0, 0, 860, 640);
        glutSetWindow (t);
        glutShowWindow( );  */
    }
    else 
    {
        glutInitWindowSize(800, 600);
        glutCreateWindow("Fractal Tubes - GLUT version");
    }

    Init();

    glutReshapeFunc(Reshape);
    glutIdleFunc(onIdle);
    glutDisplayFunc (onDisplay);

    glutMainLoop();
}
