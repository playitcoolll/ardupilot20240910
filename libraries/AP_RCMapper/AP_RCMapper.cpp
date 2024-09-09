#include "AP_RCMapper_config.h"

#if AP_RCMAPPER_ENABLED

#include <AP_HAL/AP_HAL.h>
#include "AP_RCMapper.h"

const AP_Param::GroupInfo RCMapper::var_info[] = {
    // @Param: ROLL
    // @DisplayName: Roll channel
    // @Description: Roll channel number. This is useful when you have a RC transmitter that can't change the channel order easily. Roll is normally on channel 1, but you can move it to any channel with this parameter.  Reboot is required for changes to take effect.
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("ROLL",        0, RCMapper, _ch_roll, 1),

    // @Param: PITCH
    // @DisplayName: Pitch channel
    // @Description: Pitch channel number. This is useful when you have a RC transmitter that can't change the channel order easily. Pitch is normally on channel 2, but you can move it to any channel with this parameter.  Reboot is required for changes to take effect.
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("PITCH",       1, RCMapper, _ch_pitch, 2),

    // @Param: THROTTLE
    // @DisplayName: Throttle channel
    // @Description: Throttle channel number. This is useful when you have a RC transmitter that can't change the channel order easily. Throttle is normally on channel 3, but you can move it to any channel with this parameter. Reboot is required for changes to take effect.
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("THROTTLE",    2, RCMapper, _ch_throttle, 3),

    // @Param: YAW
    // @DisplayName: Yaw channel
    // @Description: Yaw channel number. This is useful when you have a RC transmitter that can't change the channel order easily. Yaw (also known as rudder) is normally on channel 4, but you can move it to any channel with this parameter.  Reboot is required for changes to take effect.
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("YAW",         3, RCMapper, _ch_yaw, 4),

    // @Param{Sub}: FORWARD
    // @DisplayName: Forward channel
    // @Description: Forward channel number. This is useful when you have a RC transmitter that can't change the channel order easily. Forward is normally on channel 5, but you can move it to any channel with this parameter. Reboot is required for changes to take effect.
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO_FRAME("FORWARD",    15, RCMapper, _ch_forward, 15, AP_PARAM_FRAME_SUB),

    // @Param{Sub}: LATERAL
    // @DisplayName: Lateral channel
    // @Description: Lateral channel number. This is useful when you have a RC transmitter that can't change the channel order easily. Lateral is normally on channel 6, but you can move it to any channel with this parameter. Reboot is required for changes to take effect.
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO_FRAME("LATERAL",    16, RCMapper, _ch_lateral, 16, AP_PARAM_FRAME_SUB),

    // @Param: K1
    // @DisplayName: k1 channel
    // @Description: k1 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K1",       5, RCMapper, _ch_k1, 5 ),

    // @Param: K2
    // @DisplayName: k2 channel
    // @Description: k2 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K2",       6, RCMapper, _ch_k2, 6),

    // @Param: K3
    // @DisplayName: k3 channel
    // @Description: k3 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K3",       7, RCMapper, _ch_k3, 7),

    // @Param: K4
    // @DisplayName: k4 channel
    // @Description: k4 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K4",       8, RCMapper, _ch_k4, 8),

    // @Param: K5
    // @DisplayName: k5 channel
    // @Description: k5 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K5",       9, RCMapper, _ch_k5, 9),

    // @Param: K6
    // @DisplayName: k6 channel
    // @Description: k6 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K6",       10, RCMapper, _ch_k6, 10),

    // @Param: K7
    // @DisplayName: k7 channel
    // @Description: k7 channel number. This is useful when you have a RC transmitter that can't change the channel order easily. 
    // @Range: 1 16
    // @Increment: 1
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("K7",       11, RCMapper, _ch_k7, 11),
    AP_GROUPEND
};

// singleton instance
RCMapper *RCMapper::_singleton;

// object constructor.
RCMapper::RCMapper(void)
{
    if (_singleton != nullptr) {
        AP_HAL::panic("RCMapper must be singleton");
    }
    AP_Param::setup_object_defaults(this, var_info);
    _singleton = this;
}

RCMapper *AP::rcmap() {
    return RCMapper::get_singleton();
}

#endif  // AP_RCMAPPER_ENABLED
