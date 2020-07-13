#pragma once

#include <unordered_map>
#include <string>

class UserDefaults
{
private:
    static UserDefaults * m_pInstance;
    UserDefaults();

    std::unordered_map <std::string, int> values;

public:
    bool setInteger (const int value, const char *key);
    int getInteger (const char *key, const int defaultValue);
    void setBool (const bool value, const char *key);
    bool getBool (const char *key, const bool defaultValue);
    bool deleteKey (const char *key);

    static UserDefaults* sharedInstance();

};