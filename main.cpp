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

// Written with this sample as initial guide:
// https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_GLX_and_Xlib

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <unistd.h>

#include "infra.h"
#include "AppConsts.h"
#include "AppParams.h"

GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

// --- Find a MSAA FBConfig ---
static const int Visual_attribs[] =
{
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      GLX_SAMPLE_BUFFERS  , 1,            // <-- MSAA
      GLX_SAMPLES         , 4,            // <-- MSAA
      None
};

struct argvOptions // comand line options
{
    bool standalone;
    bool verbose;
    bool useLog;
    int delay_ms;
    int detail;
    int interval;
    int palette;
    int light_source;
    int rotation_speed;
    int transparency;
    int speed;
    int dist2light;
    int gloss_level;
    bool two_sided_light;
    bool shuffle;
    int smooth;

    argvOptions ()
    {
        standalone = true;
        verbose = false;
        useLog = false;
        smooth = true; 

        detail = DEF_DETAIL_LEVEL;
        interval = DEF_INTERVAL;
        palette = DEF_PALETTE;
        light_source = DEF_LIGHT_SOURCE;
        transparency = DEF_TRANSPARENCY;
        rotation_speed = DEF_ROTATION_SPEED;
        dist2light = DEF_LIGHT_DISTANCE;
        gloss_level = DEF_GLOSS_LEVEL;
        two_sided_light = DEF_2_SIDED_LIGHT;
        shuffle = DEF_SHUFFLE;
        delay_ms = 10;
    }
};

static void parse_arguments (int argc, char *argv[], 
            struct argvOptions * params) 
{
	int idx;

    int delay_idx = -1;
    int detail_idx = -1;
	int palette_idx = -1;
	int interval_idx = -1;
    int light_idx = -1;
    int transp_idx = -1;
    int speed_idx = -1;
    int dist2light_idx = -1;
    int gloss_idx = -1;

	for (idx = 1; idx < argc; idx++) 
    {
		if (strcasecmp(argv[idx], "-root") == 0) {
			params->standalone = false;
		}

        else if (strcasecmp(argv[idx], "-verbose") == 0) {
			params->verbose = true;
		}

        else if (strcasecmp(argv[idx], "-log") == 0) {
			params->useLog = true;
		}

        else if (strcmp(argv[idx], "-delay") == 0) {
			delay_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-detail") == 0) {
			detail_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-interval") == 0) {
			interval_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-palette") == 0) {
			palette_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-light") == 0) {
			light_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-transparency") == 0) {
			transp_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-speed") == 0) {
			speed_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-dist_to_light") == 0) {
			dist2light_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-gloss") == 0) {
			gloss_idx = idx + 1;
		}

        else if (strcmp(argv[idx], "-onesided") == 0) {
			params->two_sided_light = false;
		}

        else if (strcmp(argv[idx], "-shuffle") == 0) {
			params->shuffle = false;
		}

        else if (strcmp(argv[idx], "-nosmooth") == 0) {
			params->smooth = false;
		}

        else if (idx == delay_idx)
        {
            params->delay_ms = atoi (argv[idx]); 
        }

        else if (idx == detail_idx)
        {
            params->detail = atoi (argv[idx]); 
        }
        else if (idx == interval_idx)
        {
            params->interval = atoi (argv[idx]); 
        }
        else if (idx == transp_idx)
        {
            params->transparency = atoi (argv[idx]); 
        }
        else if (idx == speed_idx)
        {
            params->speed = atoi (argv[idx]); 
        }

        else if (idx == dist2light_idx)
        {
            params->dist2light = atoi (argv[idx]); 
        }

        else if (idx == gloss_idx)
        {
            if (strcmp(argv[idx], "random"))
                params->gloss_level = -1;
            else     
                params->gloss_level = atoi (argv[idx]); 
        }

        else if (idx == palette_idx)
        {
            if (strcmp(argv[idx], "alt") == 0)
                params->palette = PALETTE_ALT; 
            if (strcmp(argv[idx], "std") == 0)
                params->palette = PALETTE_STD;
        }
        else if (idx == light_idx)
        {
            if (strcmp(argv[idx], "fixed") == 0)
                params->light_source = LIGHT_FIXED; 
            if (strcmp(argv[idx], "moving") == 0)
                params->light_source = LIGHT_MOVING;
        }

	}

	return;
}

XVisualInfo * getMultispampleXVisual (Display *dpy, int screen)
{
    XVisualInfo *vi = NULL;

    int attribs [ 100 ] ;
    memcpy( attribs, Visual_attribs, sizeof( Visual_attribs ) );

    GLXFBConfig fbconfig = 0;
    int         fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig( dpy, screen, attribs, &fbcount );
    if ( fbc )
    {
        if ( fbcount >= 1 )
            fbconfig = fbc[0];
        XFree( fbc );
    }

    if ( !fbconfig )
    {
        fprintf(stderr, "Failed to get MSAA GLXFBConfig\n" );
        return NULL;
    }  
    
    vi = glXGetVisualFromFBConfig( dpy, fbconfig );

    return vi;
}
 
int main (int argc, char *argv[]) {

    Display *dpy;
    Window  win;

    struct argvOptions params;

    parse_arguments (argc, argv, &params); 

    dpy = XOpenDisplay (getenv ("DISPLAY"));

    if (!dpy) 
    {
        fprintf(stderr, "Cannot connect to X server.\n");
        return EXIT_FAILURE;
    }

    char *xwin = getenv ("XSCREENSAVER_WINDOW");

    if (params.useLog)
    {
        printf ("Display : %s\n", getenv("DISPLAY"));

        printf ("xwin : %s\n", xwin);
    }

    int root_window_id = 0;

    if (xwin)
    {
        root_window_id = strtol (xwin, NULL, 0);
    }

    XVisualInfo *vi = NULL;

    if (params.standalone == false) 
    {
		// Get the root window
        if (root_window_id == 0)
        {
		    // root = DefaultRootWindow(dpy);
            printf ("usage as standalone app: %s -window\n", argv[0]);
            return EXIT_FAILURE;
        }
        else
    	{
      		win = root_window_id;

            int screen = DefaultScreen(dpy);  

            if (params.smooth)
            {
                vi = getMultispampleXVisual (dpy, screen);
            }

            if (vi == NULL)
            {
                vi = glXChooseVisual(dpy, screen, att);
            }

            if (vi == NULL) 
            {
	            fprintf(stderr, "\nNo appropriate visual found\n\n");
                return EXIT_FAILURE;
            }

            XSelectInput (dpy, win, ExposureMask | StructureNotifyMask);
    	}
	} 
    else 
	{
		/* Let's create our own window. */
		int screen = DefaultScreen(dpy);
        Window root = DefaultRootWindow(dpy);

        if (params.smooth)
        {
            vi = getMultispampleXVisual (dpy, screen);
        }

        if (vi == NULL)
        {
            vi = glXChooseVisual(dpy, screen, att);
        }

        if (vi == NULL) 
        {
	        fprintf(stderr, "\nNo appropriate visual found\n\n");
            return EXIT_FAILURE;
        } 

        unsigned int mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;

        int border_width = 0;

        Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

        XSetWindowAttributes swa;

        swa.background_pixmap = None ;
        swa.background_pixel  = 0    ;
        swa.border_pixel      = 0    ;

        swa.colormap = cmap;
        swa.event_mask = ExposureMask | StructureNotifyMask;

		win = XCreateWindow(dpy, root, 0, 0, 860, 640, border_width,
            vi->depth, InputOutput, vi->visual, mask, &swa);

		XMapWindow(dpy, win);

		XStoreName(dpy, win, "Fractal Tubes");
	}

    // this is to terminate nicely:
	Atom wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

    if (params.useLog) { createLog(); }

    GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);

    if (!glc)
    {
        fprintf(stderr, "\nglXCreateContext failed.\n\n");
        return EXIT_FAILURE;
    }

    if (!glXMakeCurrent(dpy, win, glc))
    {
        fprintf(stderr, "\nglXMakeCurren failed.\n\n");
        return EXIT_FAILURE;
    }

    if (params.verbose)
    {
        fprintf (stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    }
 
    glEnable (GL_DEPTH_TEST); 

    XWindowAttributes gwa;

    XGetWindowAttributes(dpy, win, &gwa);

    AppParams::setDetailLevel (params.detail);
    AppParams::setIntervalOption (params.interval);
    AppParams::setPalette (params.palette);
    AppParams::setLightSource (params.light_source);
    AppParams::setTransparencyLevel (params.transparency);
    AppParams::setRotationSpeed (params.rotation_speed);
    AppParams::setLightDistance (params.dist2light);
    AppParams::setLight2Sides (params.two_sided_light);
    AppParams::setShuffle (params.shuffle);

    if (params.gloss_level < 0) // random 
    {
        AppParams::setGlossRandom(true);
    }
    else 
    {
        AppParams::setGlossRandom(false);
        AppParams::setGlossLevel (params.gloss_level);
    }

    GetConfig();

    setFrameSizeForGL (gwa.width, gwa.height);
    
    prepareOpenGL();
 
    while (true) 
    {
        XEvent  event;

        long mask = StructureNotifyMask;

        if (XCheckWindowEvent(dpy, win, mask, &event) ||
		   XCheckTypedWindowEvent (dpy, win, ClientMessage, &event)) // needed to catch ClientMessage
        {
            if (event.type == ConfigureNotify) 
        	{
          		XConfigureEvent xce = event.xconfigure;

		        // This event type is generated for a variety of
          		// happenings, so check whether the window has been
          		// resized.

          		if (xce.width != gwa.width || xce.height != gwa.height) 
          		{
            		gwa.width = xce.width;
            		gwa.height = xce.height;

                    setFrameSizeForGL (gwa.width, gwa.height);
          
            		continue;
          		}
        	}

            else if (event.type == ClientMessage)
        	{
            	if ((Atom)event.xclient.data.l[0] == wmDeleteMessage)
            	{
                	break;
            	}
			}                
        }

        animateOneFrame();

        redrawMethod();
        glXSwapBuffers(dpy, win);

        // sleep for 10 ms
        usleep(1000 * params.delay_ms);
    } 

    CleanupAnimation();

    if (params.useLog) { closeLog (); }

    glXMakeCurrent(dpy, None, NULL);
 	glXDestroyContext(dpy, glc);
 	XDestroyWindow(dpy, win);
 	XCloseDisplay(dpy);

    return EXIT_SUCCESS;
} 