// #pragma once

// #include <AP_Logger/LogStructure.h>

// #define LOG_IDS_FROM_AHRS \
//     LOG_IDS_FROM_AOASSA_MY



// // @LoggerMessage: AOA
// // @Description: Angle of attack and Side Slip Angle values
// // @Field: TimeUS: Time since system startup
// // @Field: AOA: Angle of Attack calculated from airspeed, wind vector,velocity vector 
// // @Field: SSA: Side Slip Angle calculated from airspeed, wind vector,velocity vector
// struct PACKED my_log_AOA_SSA {
//     LOG_PACKET_HEADER;
//     uint64_t time_us;
//     float AOA;
//     float SSA;
// };



// #define LOG_STRUCTURE_FROM_AHRS \
//     { LOG_IDS_FROM_AOASSA_MY, sizeof(log_AOA_SSA), \
//         "AOA", "Qff", "TimeUS,AOA,SSA", "sdd", "F00" , true },

