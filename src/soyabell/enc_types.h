#ifndef ENC_TYPES_H
#define ENC_TYPES_H

#include <Arduino.h>

#define UNSET_VALUE -999.0f
#define PASSWORD_MASKED "********" 
#define PREF_NAMESPACE "config"
#define MAX_PREF_STRING_LEN 64

enum AppLogLevel { APP_LOG_NONE, APP_LOG_ERROR, APP_LOG_INFO, APP_LOG_DEBUG };

enum PrefType { 
  PREF_NONE, PREF_STRING, PREF_BOOL, PREF_INT, PREF_ENUM, PREF_SELECT
};

struct PrefSelectOption {
  const char* name;
  const char* value;
};

struct FormField {
    const char* id;
    const char* name;
    bool isMasked;
    PrefType prefType;
    union {
        char* str_pref;
        bool* bool_pref;
        int32_t* int_pref;
    } pref;
    const PrefSelectOption* select_options;
    int num_select_options;
};

// Generic Config Struct
struct BaseConfig {
    char ssid[MAX_PREF_STRING_LEN];
    char password[MAX_PREF_STRING_LEN];
    char hostname[MAX_PREF_STRING_LEN];
    char time_zone[MAX_PREF_STRING_LEN];
    AppLogLevel logLevel;
};

#endif