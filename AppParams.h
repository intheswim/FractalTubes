#pragma once

#define MIN_TRANSP  0
#define MAX_TRANSP  10

#define MIN_SPEED  10
#define MAX_SPEED  40

#define MIN_GLOSS  0
#define MAX_GLOSS  10

#define MIN_DISTANCE 2
#define MAX_DISTANCE 30

#define DEF_TRANSPARENCY    1
#define DEF_DETAIL_LEVEL    1  /* valid values are 0 to 4. 4 is most detail & slowest */ 
#define DEF_ROTATION_SPEED  25
#define DEF_LIGHT_SOURCE    LIGHT_RANDOM
#define DEF_LIGHT_DISTANCE  10
#define DEF_2_SIDED_LIGHT   true
#define DEF_GLOSS_LEVEL     MIN_GLOSS
#define DEF_SHUFFLE         NO
#define DEF_PALETTE         PALETTE_RANDOM
#define DEF_INTERVAL        INTERVAL_45 // how often images change, in seconds 


class AppParams
{
  private:

  static char * getImageKey (const int index);

  public:
  static void resetToDefaults();

  static int getDetailLevel();
  static void setDetailLevel (int value);
  static int getIntervalOption();
  static void setIntervalOption (int v);
  static void setTransparencyLevel (const int value);
  static int getTransparencyLevel ();
  static float getTransparency ();
  static void setRotationSpeed (int value);
  static float getRotationSpeed ();
  static float getDetailValue();
  static void setLight2Sides (bool value);
  static bool getLight2Sides();
  static void setLightDistance(float value);
  static float getLightDistance();
  static void setLightSource (int);
  static int getLightSource();
  static void setPalette (int index);
  static int getPalette();
  static bool isPaletteAlternative();

  static bool getLightMoving();

  static float getIntervalInSecs();

  static void setGlossLevel (const int value);
  static float getGlossLevel (void);
  static void setGlossRandom (const bool value);
  static bool getGlossRandom();
  static bool getShuffle();
  static void setShuffle (bool value);
  static void setImageSelected (const int index, bool selected);
  static bool getImageSelected (const int i);
};