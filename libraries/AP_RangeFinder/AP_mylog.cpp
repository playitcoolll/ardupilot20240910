// #include <AP_Logger/AP_Logger_config.h>

// #if HAL_LOGGING_ENABLED

// #include "my.h"
// #include <AP_Logger/AP_Logger.h>




// // Write AOA and SSA
// void aoassa::Write_AOA_SSA(void) const
// {
//     const struct log_my_AOA_SSA my_aoa_ssa{
//         LOG_PACKET_HEADER_INIT(LOG_AOA_SSA_MSG),
//         time_us         : AP_HAL::micros64(),
//         AOA_my             : get_aoa(),
//         SSA_my            : get_ssa()
//     };

//     AP::logger().WriteBlock(&my_aoa_ssa, sizeof(my_aoa_ssa));
// }