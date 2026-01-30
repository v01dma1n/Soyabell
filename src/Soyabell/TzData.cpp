#include "TzData.h"

const PrefSelectOption timezones[] = {
  {"(GMT-12:00) International Date Line West", "Etc/GMT+12"},
  {"(GMT-11:00) Midway Island, Samoa", "Pacific/Samoa"},
  {"(GMT-10:00) Hawaii", "Pacific/Honolulu"},
  {"(GMT-09:00) Alaska", "America/Anchorage"},
  {"(GMT-08:00) Pacific Time (US & Canada)", "PST8PDT,M3.2.0,M11.1.0"},
  {"(GMT-07:00) Mountain Time (US & Canada)", "MST7MDT,M3.2.0,M11.1.0"},
  {"(GMT-06:00) Central Time (US & Canada)", "CST6CDT,M3.2.0,M11.1.0"},
  {"(GMT-05:00) Eastern Time (US & Canada)", "EST5EDT,M3.2.0,M11.1.0"},
  {"(GMT-04:00) Atlantic Time (Canada)", "America/Halifax"},
  {"(GMT-03:00) Buenos Aires, Georgetown", "America/Argentina/Buenos_Aires"},
  {"(GMT-02:00) Mid-Atlantic", "Atlantic/South_Georgia"},
  {"(GMT-01:00) Azores, Cape Verde Is.", "Atlantic/Azores"},
  {"(GMT+00:00) Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London", "GMT0BST,M3.5.0/1,M10.5.0"},
  {"(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna", "CET-1CEST,M3.5.0,M10.5.0"},
  {"(GMT+02:00) Athens, Bucharest, Istanbul", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
  {"(GMT+03:00) Moscow, St. Petersburg, Volgograd", "Europe/Moscow"},
  {"(GMT+08:00) Beijing, Perth, Singapore, Hong Kong", "Asia/Hong_Kong"},
  {"(GMT+09:00) Tokyo, Seoul, Osaka, Sapporo, Yakutsk", "Asia/Tokyo"},
  {"(GMT+10:00) Brisbane, Canberra, Melbourne, Sydney", "Australia/Sydney"},
  {"(GMT+12:00) Auckland, Wellington", "Pacific/Auckland"}
};

const int num_timezones = sizeof(timezones) / sizeof(PrefSelectOption);