#pragma once

#ifndef BME_VERSION
#define BME_VERSION "1b10"
#endif

#define BME_VERSION_LONG "Black Market Edition v" BME_VERSION

#ifdef _DEBUG
#define BME_CHANNEL "dev"
#else
#ifdef STAGING
#define BME_CHANNEL "staging"
#else
#define BME_CHANNEL "release"
#endif
#endif
