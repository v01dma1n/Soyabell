#ifndef SOYA_CONFIG_H
#define SOYA_CONFIG_H
#include "enc_types.h" // Use the library types

struct SoyaConfig : BaseConfig {
    char api_user[MAX_PREF_STRING_LEN];
    char api_pass[MAX_PREF_STRING_LEN];
    char did[MAX_PREF_STRING_LEN];
    char dst[MAX_PREF_STRING_LEN];
};
#endif