#pragma once

#include "AP_RCMapper_config.h"

#if AP_RCMAPPER_ENABLED

#include <inttypes.h>
#include <AP_Common/AP_Common.h>
#include <AP_Param/AP_Param.h>

class RCMapper {
public:
    RCMapper();

    /* Do not allow copies */
    CLASS_NO_COPY(RCMapper);

    // get singleton instance
    static RCMapper *get_singleton()
    {
        return _singleton;
    }

    /// roll - return input channel number for roll / aileron input
    uint8_t roll() const { return _ch_roll; }

    /// pitch - return input channel number for pitch / elevator input
    uint8_t pitch() const { return _ch_pitch; }

    /// throttle - return input channel number for throttle input
    uint8_t throttle() const { return _ch_throttle; }

    /// yaw - return input channel number for yaw / rudder input
    uint8_t yaw() const { return _ch_yaw; }

    /// forward - return input channel number for forward input
    uint8_t forward() const { return _ch_forward; }

    /// lateral - return input channel number for lateral input
    uint8_t lateral() const { return _ch_lateral; }

    /// 
    uint8_t k1() const { return _ch_k1; }

    /// 
    uint8_t k2() const { return _ch_k2; }

    /// 
    uint8_t k3() const { return _ch_k3; }

    /// 
    uint8_t k4() const { return _ch_k4; }

    /// 
    uint8_t k5() const { return _ch_k5; }

    /// 
    uint8_t k6() const { return _ch_k6; }

    /// 
    uint8_t k7() const { return _ch_k7; }
    static const struct AP_Param::GroupInfo var_info[];

private:
    // channel mappings
    AP_Int8 _ch_roll;
    AP_Int8 _ch_pitch;
    AP_Int8 _ch_yaw;
    AP_Int8 _ch_throttle;
    AP_Int8 _ch_forward;
    AP_Int8 _ch_lateral;

    AP_Int8 _ch_k1;
    AP_Int8 _ch_k2;
    AP_Int8 _ch_k3;
    AP_Int8 _ch_k4;
    AP_Int8 _ch_k5;
    AP_Int8 _ch_k6;
    AP_Int8 _ch_k7; 
    
    static RCMapper *_singleton;
};

namespace AP
{
RCMapper *rcmap();
};

#endif  // AP_RCMAPPER_ENABLED
