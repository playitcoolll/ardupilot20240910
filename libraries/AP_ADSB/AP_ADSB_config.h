#pragma once

#include <AP_HAL/AP_HAL_Boards.h>

#include <GCS_MAVLink/GCS_config.h>

#ifndef HAL_ADSB_ENABLED
#define HAL_ADSB_ENABLED BOARD_FLASH_SIZE > 1024
#endif

#ifndef HAL_ADSB_BACKEND_DEFAULT_ENABLED
#define HAL_ADSB_BACKEND_DEFAULT_ENABLED HAL_ADSB_ENABLED
#endif

#ifndef HAL_ADSB_UAVIONIX_MAVLINK_ENABLED
#define HAL_ADSB_UAVIONIX_MAVLINK_ENABLED HAL_ADSB_BACKEND_DEFAULT_ENABLED && HAL_GCS_ENABLED
#endif

#ifndef HAL_ADSB_SAGETECH_ENABLED
#define HAL_ADSB_SAGETECH_ENABLED HAL_ADSB_BACKEND_DEFAULT_ENABLED
#endif

#ifndef HAL_ADSB_UCP_ENABLED
#define HAL_ADSB_UCP_ENABLED HAL_ADSB_BACKEND_DEFAULT_ENABLED
#endif

#ifndef HAL_ADSB_SAGETECH_MXS_ENABLED
    // this feature is only enabled by default by select hardware
    #define HAL_ADSB_SAGETECH_MXS_ENABLED HAL_ADSB_BACKEND_DEFAULT_ENABLED && CONFIG_HAL_BOARD == HAL_BOARD_SITL
#endif
