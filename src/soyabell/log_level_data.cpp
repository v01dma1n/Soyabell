#include "log_level_data.h"

// This array provides the display names and integer values for the log levels.
const PrefSelectOption logLevels[] = {
    {"Error", "0"}, // Corresponds to APP_LOG_ERROR
    {"Info",  "1"}, // Corresponds to APP_LOG_INFO
    {"Debug", "2"}  // Corresponds to APP_LOG_DEBUG
};
const int numLogLevels = sizeof(logLevels) / sizeof(PrefSelectOption);
