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

#include "UserDefaults.h"
#include <limits.h>

//////////////////////////////////////////////////////

UserDefaults * UserDefaults::m_pInstance = NULL;

UserDefaults::UserDefaults()
{
}

UserDefaults* UserDefaults::sharedInstance()
{
   if (!m_pInstance)   // Only allow one instance of class to be generated.
      m_pInstance = new UserDefaults();

   return m_pInstance;
}

bool UserDefaults::deleteKey (const char *key)
{
    values.erase(key);

    return true;
}

bool UserDefaults::setInteger (const int value, const char *key)
{
   values[key] = value;

   return true;
}

int UserDefaults::getInteger (const char *key, const int defaultValue)
{
     if (values.find (key) == values.end())
        return defaultValue;

    return values[key];
}

void UserDefaults::setBool (const bool value, const char *key)
{
    UserDefaults::setInteger (value ? 1 : 0, key);
}

bool UserDefaults::getBool (const char *key, const bool defaultValue)
{
   int ret = UserDefaults::getInteger (key, INT_MAX);

   if (ret == INT_MAX) return defaultValue;

   return (ret != 0);

}
