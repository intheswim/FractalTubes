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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AppParams.h"
#include "AppConsts.h"
#include "m3d.h"
#include "UserDefaults.h"

#define kIntervalOption     "interval_option"
#define kRotationSpeed      "rotation_speed"
#define kDetailLevel        "detail_level"
#define kShuffleImages      "shuffle_images"
#define kTransparencyLevel  "transp_level"
#define kPaletteType        "palette_type"
#define kLightSource        "light_source"
#define kLightDistance      "light_distance"
#define kLight2Sides        "light_2_sides"
#define kGlossyOption       "gloss_level"
#define kGlossRandom        "gloss_random"

char * AppParams::getImageKey (const int index)
{
    static char key[256];
    memset (key, 0, 256);
    sprintf (key, "image_%d", index);
    return key;
}


void AppParams::resetToDefaults()
{
   UserDefaults::sharedInstance()->deleteKey (kIntervalOption);
   UserDefaults::sharedInstance()->deleteKey (kRotationSpeed);
   UserDefaults::sharedInstance()->deleteKey (kDetailLevel);
   UserDefaults::sharedInstance()->deleteKey (kShuffleImages);
   UserDefaults::sharedInstance()->deleteKey (kTransparencyLevel);
   UserDefaults::sharedInstance()->deleteKey (kPaletteType);
   UserDefaults::sharedInstance()->deleteKey (kLightSource);
   UserDefaults::sharedInstance()->deleteKey (kGlossyOption);
   UserDefaults::sharedInstance()->deleteKey (kGlossRandom);

   UserDefaults::sharedInstance()->deleteKey (kLightDistance);
   UserDefaults::sharedInstance()->deleteKey (kLight2Sides);

   for (int i=0; i < PRESET_COUNT; i++)
   {
      char *key = getImageKey (i);
      UserDefaults::sharedInstance()->deleteKey (key);
   }
}

void AppParams::setTransparencyLevel (const int value)
{
    UserDefaults::sharedInstance()->setInteger (value, kTransparencyLevel);
}

int AppParams::getTransparencyLevel ()
{
    int ret = UserDefaults::sharedInstance()->getInteger (kTransparencyLevel, DEF_TRANSPARENCY);
    if (ret < MIN_TRANSP) ret = MIN_TRANSP;
    if (ret > MAX_TRANSP) ret = MAX_TRANSP;

    return ret;
}

float AppParams::getTransparency ()
{
    int level = getTransparencyLevel();
    return (1.0f - 0.05f * level);
}

void AppParams::setRotationSpeed (int value)
{
    UserDefaults::sharedInstance()->setInteger (value, kRotationSpeed);
}

float AppParams::getRotationSpeed ()
{
    int ret = UserDefaults::sharedInstance()->getInteger (kRotationSpeed, DEF_ROTATION_SPEED);
    if (ret < MIN_SPEED) ret = MIN_SPEED;
    if (ret > MAX_SPEED) ret = MAX_SPEED;
    return (float)ret;
}

void AppParams::setDetailLevel (int value)
{
    UserDefaults::sharedInstance()->setInteger (value, kDetailLevel);
}

int AppParams::getDetailLevel()
{
   int ret = UserDefaults::sharedInstance()->getInteger (kDetailLevel, DEF_DETAIL_LEVEL);
   if (ret < 0) ret = 0;
   if (ret > 4) ret = 4;

   return ret;
}

float AppParams::getDetailValue()
{
    float v = (float)getDetailLevel();

    return 1.2f + (v-3) * 0.2f; // 0.6, 0.8, 1.0, 1.2.
}

void AppParams::setLightSource (int index)
{
   UserDefaults::sharedInstance()->setInteger (index, kLightSource);
}

int AppParams::getLightSource()
{
    return UserDefaults::sharedInstance()->getInteger (kLightSource, DEF_LIGHT_SOURCE);
}

void AppParams::setLight2Sides (bool value)
{
    UserDefaults::sharedInstance()->setBool (value, kLight2Sides);
}

bool AppParams::getLight2Sides()
{
    return UserDefaults::sharedInstance()->getBool (kLight2Sides, DEF_2_SIDED_LIGHT);
}

void AppParams::setLightDistance(float value)
{
    UserDefaults::sharedInstance()->setInteger (value * 10, kLightDistance);
}

float AppParams::getLightDistance()
{
    // default return 10. Range 2..30
    float value = 0.1 * UserDefaults::sharedInstance()->getInteger (kLightDistance, 10 * DEF_LIGHT_DISTANCE);

    if (value < MIN_DISTANCE) value = MIN_DISTANCE;
    if (value > MAX_DISTANCE) value = MAX_DISTANCE;

    return value;
}

void AppParams::setPalette (int index)
{
    UserDefaults::sharedInstance()->setInteger (index, kPaletteType);
}

int AppParams::getPalette()
{
    return UserDefaults::sharedInstance()->getInteger (kPaletteType, DEF_PALETTE);
}

bool AppParams::isPaletteAlternative()
{
    int source = getPalette();

    if (source == PALETTE_STD) { return NO; }
    if (source == PALETTE_ALT) { return YES; }

    int v = rand() & 0x01;

    return (v == 0);
}

bool AppParams::getLightMoving()
{
    int source = getLightSource();

    if (source == LIGHT_MOVING) return true;
    if (source == LIGHT_FIXED) return false;

    int v = rand() & 0x01;

    return (v == 0);
}

void AppParams::setIntervalOption (int v)
{
   UserDefaults::sharedInstance()->setInteger (v, kIntervalOption);
}

int AppParams::getIntervalOption()
{
    return UserDefaults::sharedInstance()->getInteger (kIntervalOption, DEF_INTERVAL);
}

float AppParams::getIntervalInSecs()
{
    int option = getIntervalOption();

    if (option == INTERVAL_45) return 45.0;
    if (option == INTERVAL_60) return 60.0;

    return 90.0;
}

void AppParams::setGlossRandom (const bool value)
{
    UserDefaults::sharedInstance()->setBool (value, kGlossRandom);
}

bool AppParams::getGlossRandom()
{
   return UserDefaults::sharedInstance()->getBool (kGlossRandom, false);
}

void AppParams::setGlossLevel (const int value)
{
    UserDefaults::sharedInstance()->setInteger (value, kGlossyOption);
}

float AppParams::getGlossLevel (void)
{
    // from 0 to 10

    int value = UserDefaults::sharedInstance()->getInteger (kGlossyOption, DEF_GLOSS_LEVEL);

    if (value < MIN_GLOSS) value = MIN_GLOSS;
    if (value > MAX_GLOSS) value = MAX_GLOSS;

    return (float)value;
}

bool AppParams::getShuffle()
{
    return UserDefaults::sharedInstance()->getBool (kShuffleImages, DEF_SHUFFLE);
}

void AppParams::setShuffle (bool value)
{
    UserDefaults::sharedInstance()->setBool (value, kShuffleImages);
}

void AppParams::setImageSelected (const int index, bool selected)
{
    char *key = getImageKey (index);

    UserDefaults::sharedInstance()->setBool (selected, key);
}

bool AppParams::getImageSelected (const int index)
{
    char *key = getImageKey (index);

    return UserDefaults::sharedInstance()->getBool (key, true);
}