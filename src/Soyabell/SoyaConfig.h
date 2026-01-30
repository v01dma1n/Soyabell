#ifndef SOYABELL_SOYACONFIG_H
#define SOYABELL_SOYACONFIG_H
#include "EncTypes.h" // Use the library types

struct SoyaConfig : BaseConfig {
    char api_user[MAX_PREF_STRING_LEN];
    char api_pass[MAX_PREF_STRING_LEN];
    char did[MAX_PREF_STRING_LEN];
    char dst[MAX_PREF_STRING_LEN];
};
#endif