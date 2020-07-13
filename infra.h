#pragma once

// these are three logging calls for debugging.
void createLog ();
void closeLog ();
void writeLog (const char *buffer);

// call this first to initialize and load options.
void GetConfig (void);

// call second when window size is ready and then any time when window is resized.
void setFrameSizeForGL (int width, int height);

// call third only once.
void prepareOpenGL ();

// rollover occurs automatically overy 45/60/90 seconds, causing
// change of the tube displayed. There's no need to call it directly.
void rollover();

// call last before exiting to free memory.
void CleanupAnimation();

// no need to call this directly.
// this is called automatically right before rollover() to change image based
// on user preferences. 
void selectNextImage();

// this should be called on timer (every 0.1 secs or so) right before redramMethod().
// it updates (increments) rotation parameter and "fades iamge" when needed.
// it also calls rollover() and selectNextImage() when time is right.
void animateOneFrame();

// call after animateOneFrame() to update backround GL buffer.
// after this call, invoke GL "swap buffers" call.
void redrawMethod();