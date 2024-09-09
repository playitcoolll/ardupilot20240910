#include "Plane.h"

/*
  calculate speed scaling number for control surfaces. This is applied
  to PIDs to change the scaling of the PID with speed. At high speed
  we move the surfaces less, and at low speeds we move them more.
 */
float Plane::calc_speed_scaler(void)
{
    float aspeed, speed_scaler;
    if (ahrs.airspeed_estimate(aspeed)) {
        if (aspeed > auto_state.highest_airspeed && arming.is_armed_and_safety_off()) {
            auto_state.highest_airspeed = aspeed;
        }
        // ensure we have scaling over the full configured airspeed
        const float airspeed_min = MAX(aparm.airspeed_min, MIN_AIRSPEED_MIN);
        const float scale_min = MIN(0.5, g.scaling_speed / (2.0 * aparm.airspeed_max));
        const float scale_max = MAX(2.0, g.scaling_speed / (0.7 * airspeed_min));
        if (aspeed > 0.0001f) {
            speed_scaler = g.scaling_speed / aspeed;
        } else {
            speed_scaler = scale_max;
        }
        speed_scaler = constrain_float(speed_scaler, scale_min, scale_max);

#if HAL_QUADPLANE_ENABLED
        if (quadplane.in_vtol_mode() && arming.is_armed_and_safety_off()) {
            // when in VTOL modes limit surface movement at low speed to prevent instability
            float threshold = airspeed_min * 0.5;
            if (aspeed < threshold) {
                float new_scaler = linear_interpolate(0.001, g.scaling_speed / threshold, aspeed, 0, threshold);
                speed_scaler = MIN(speed_scaler, new_scaler);

                // we also decay the integrator to prevent an integrator from before
                // we were at low speed persistent at high speed
                rollController.decay_I();
                pitchController.decay_I();
                yawController.decay_I();
            }
        }
#endif
    } else if (arming.is_armed_and_safety_off()) {
        // scale assumed surface movement using throttle output
        float throttle_out = MAX(SRV_Channels::get_output_scaled(SRV_Channel::k_throttle), 1);
        speed_scaler = sqrtf(THROTTLE_CRUISE / throttle_out);
        // This case is constrained tighter as we don't have real speed info
        speed_scaler = constrain_float(speed_scaler, 0.6f, 1.67f);
    } else {
        // no speed estimate and not armed, use a unit scaling
        speed_scaler = 1;
    }
    if (!plane.ahrs.using_airspeed_sensor()  && 
        (plane.flight_option_enabled(FlightOptions::SURPRESS_TKOFF_SCALING)) &&
        (plane.flight_stage == AP_FixedWing::FlightStage::TAKEOFF)) { //scaling is suppressed during climb phase of automatic takeoffs with no airspeed sensor being used due to problems with inaccurate airspeed estimates
        return MIN(speed_scaler, 1.0f) ;
    }
    return speed_scaler;
}

/*
  return true if the current settings and mode should allow for stick mixing
 */
bool Plane::stick_mixing_enabled(void)
{
    if (!rc().has_valid_input()) {
        // never stick mix without valid RC
        return false;
    }
#if AP_FENCE_ENABLED
    const bool stickmixing = fence_stickmixing();
#else
    const bool stickmixing = true;
#endif
#if HAL_QUADPLANE_ENABLED
    if (control_mode == &mode_qrtl &&
        quadplane.poscontrol.get_state() >= QuadPlane::QPOS_POSITION1) {
        // user may be repositioning
        return false;
    }
    if (quadplane.in_vtol_land_poscontrol()) {
        // user may be repositioning
        return false;
    }
#endif
    if (control_mode->does_auto_throttle() && plane.control_mode->does_auto_navigation()) {
        // we're in an auto mode. Check the stick mixing flag
        if (g.stick_mixing != StickMixing::NONE &&
            g.stick_mixing != StickMixing::VTOL_YAW &&
            stickmixing) {
            return true;
        } else {
            return false;
        }
    }

    if (failsafe.rc_failsafe && g.fs_action_short == FS_ACTION_SHORT_FBWA) {
        // don't do stick mixing in FBWA glide mode
        return false;
    }

    // non-auto mode. Always do stick mixing
    return true;
}


/*
  this is the main roll stabilization function. It takes the
  previously set nav_roll calculates roll servo_out to try to
  stabilize the plane at the given roll
 */
void Plane::stabilize_roll()
{
    if (fly_inverted()) {
        // we want to fly upside down. We need to cope with wrap of
        // the roll_sensor interfering with wrap of nav_roll, which
        // would really confuse the PID code. The easiest way to
        // handle this is to ensure both go in the same direction from
        // zero
        nav_roll_cd += 18000;
        if (ahrs.roll_sensor < 0) nav_roll_cd -= 36000;
    }

    const float roll_out = stabilize_roll_get_roll_out();
    SRV_Channels::set_output_scaled(SRV_Channel::k_aileron, roll_out);
}

float Plane::stabilize_roll_get_roll_out()
{
    const float speed_scaler = get_speed_scaler();




    
    // demanded_roll_new = maxroll * channel_roll->norm_input_dz();//期望滚转角验证完毕

    // //防止抖舵 验证完毕
    // if(abs(demanded_roll_new - demanded_roll_old) >= float(25)){
    //     demanded_roll_old = demanded_roll_new;
    // }
    // nav_roll_cd = demanded_roll_old;

    // //检测到输入禁用积分 姿态模式不能用这个东西
    // bool disable_integrator = false;


    // // float k4 = channel_k4->norm_input_dz(); 
    // // if(k4 < 0.0f){
    // //     k4 = 0.9f * k4 + 1.0f;
    // // }else{
    // //     k4 = 1.0f + 5.0f * k4;
    // // }

    // //改成+ ahrs.roll_sensor了，之前是减
    // return rollController.get_servo_out(nav_roll_cd + ahrs.roll_sensor, speed_scaler, disable_integrator,
    //                                     ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)));



    //新
    //检测到输入禁用积分 姿态模式不能用这个东西
    bool disable_integrator = false;

    float k5 =-0.5f + abs(channel_k5->norm_input_dz()); //0.1-1-5
    if(k5 < 0.0f){
        k5 =1.0f + 2.0f * k5;
    }else{
        k5 =1.0f + 9.0f * k5;
    }

    float k7 =-0.5f + abs(channel_k7->norm_input_dz()); //0.1-1-5
    if(k7 < 0.0f){
        k7 =1.0f + 2.0f * k7;
    }else{
        k7 =1.0f + 9.0f * k7;
    }

    int k4 = channel_k4->norm_input_dz();

    
    // const uint32_t now = AP_HAL::millis();
    // if (now - _last_AOA_update_ms_roll > 100) {
    //     // don't update at more than 1Hz
    //     hal.console -> printf("%d,%f,%f\n",k4,k7,k5);
    //      _last_AOA_update_ms_roll = now;
    // }




    float ail = plane.channel_roll->get_control_in();
    
    // if(k4 == 0){
    //     //增稳模式
        disable_integrator = true;
        // hal.console -> printf("%f\n",ail);
        return rollController.get_servo_out(nav_roll_cd - ahrs.roll_sensor, speed_scaler, disable_integrator,
                                            ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
                                            k5,k7,k4,ail);
    // }else{
        //姿态模式
    //     demanded_roll_new = 4500 * channel_roll->norm_input_dz();

    //     // 防止抖舵 验证完毕
    //     if(abs(demanded_roll_new - demanded_roll_old) >= float(25)){
    //         demanded_roll_old = demanded_roll_new;
    //     }
        
    //     nav_roll_cd += demanded_roll_old;

        // //姿态模式
        // return rollController.get_servo_out(nav_roll_cd - ahrs.roll_sensor, speed_scaler, disable_integrator,
        //                                     ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
        //                                     k5,k7);
    // }


    // //改成+ ahrs.roll_sensor了，之前是减
    // return rollController.get_servo_out(nav_roll_cd + ahrs.roll_sensor, speed_scaler, disable_integrator,
    //                                     ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
    //                                     k4);
}

/*
  this is the main pitch stabilization function. It takes the
  previously set nav_pitch and calculates servo_out values to try to
  stabilize the plane at the given attitude.
 */
void Plane::stabilize_pitch()
{
    int8_t force_elevator = takeoff_tail_hold();
    if (force_elevator != 0) {
        // we are holding the tail down during takeoff. Just convert
        // from a percentage to a -4500..4500 centidegree angle
        SRV_Channels::set_output_scaled(SRV_Channel::k_elevator, 45*force_elevator);
        return;
    }

    const float pitch_out = stabilize_pitch_get_pitch_out();
    SRV_Channels::set_output_scaled(SRV_Channel::k_elevator, pitch_out);
}

float Plane::stabilize_pitch_get_pitch_out()
{
    const float speed_scaler = get_speed_scaler();

    // 增稳模式
    // int32_t demanded_pitch = nav_pitch_cd;
    // bool disable_integrator = false;
    // if (control_mode == &mode_stabilize && channel_pitch->get_control_in() != 0) {//channel_pitch->get_control_in()) != 0
    //     disable_integrator = true;
    // }

    //检测到输入禁用积分 姿态模式不能用这个东西
    bool disable_integrator = false;

    // float k1 =1.0f;
    float k1 =-0.5f + abs(channel_k1->norm_input_dz()); //0.1-1-5
    if(k1 < 0.0f){
        k1 =1.0f + 2.0f * k1;//1.8
    }else{
        k1 =1.0f + 9.0f * k1;
    }

    // float k2 =1.0f;
    float k2 =-0.5f + abs(channel_k2->norm_input_dz()); //0.1-1-5
    if(k2 < 0.0f){
        k2 =1.0f + 2.0f * k2;
    }else{
        k2 =1.0f + 9.0f * k2;
    }

    // float k3 =1.0f;
    float k3 =-0.5f + abs(channel_k3->norm_input_dz()); //0.1-1-5
    if(k3 < 0.0f){
        k3 =1.0f + 2.0f * k3;
    }else{
        k3 =1.0f + 9.0f * k3;
    }

    float k5 =-0.5f + abs(channel_k5->norm_input_dz()); //0.1-1-5
    if(k5 < 0.0f){
        k5 =1.0f + 2.0f * k5;
    }else{
        k5 =1.0f + 9.0f * k5;
    }

    float k7 =-0.5f + abs(channel_k7->norm_input_dz()); //0.1-1-5
    if(k7 < 0.0f){
        k7 =1.0f + 2.0f * k7;
    }else{
        k7 =1.0f + 9.0f * k7;
    }

    int k4 = channel_k4->norm_input_dz();

    const uint32_t now = AP_HAL::millis();
    if (now - _last_AOA_update_ms_pitch > 100) {
        // don't update at more than 1Hz
         hal.console -> printf("%d,%f,%f,%f,%f,%f\n",k4,k1,k2,k3,k7,k5);
        _last_AOA_update_ms_pitch = now;
    }



    float elevator = plane.channel_pitch->get_control_in();
    
    if(k4 == 0){
        //增稳模式
        disable_integrator = true;
        // hal.console -> printf("%f\n",elevator);
        return pitchController.get_servo_out(nav_pitch_cd - ahrs.pitch_sensor, speed_scaler, disable_integrator,
                                            ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
                                            k1,k2,k3,k4,elevator);
    }else{
        //姿态模式

        //9.7改 迎角保护
        // //低空速修正
        // const AP_AHRS &_ahrs = AP::ahrs();
        // float speed;
        // float a;
        // float k = 1.0f;
        // if (!_ahrs.airspeed_estimate(speed)) {
        //     speed = 0;
        // }
        // bool underspeed = speed <= float(3);

        // if(underspeed){
        //     a = 0;
        // }else{
        //     a = _ahrs.getAOA();
        // }

        // if(a >= 7.0f){
        //     k = 1 - 1/10*(a - 7.0f); 
        // }

        // k = constrain_float(k , 0.0f,1.0f);
        // // hal.console -> printf("%f\n",a);
        // //9.7改


        demanded_pitch_new =  4500 * channel_pitch->norm_input_dz();
        
        // 防止抖舵 验证完毕
        if(abs(demanded_pitch_new - demanded_pitch_old) >= float(25)){
            demanded_pitch_old = demanded_pitch_new;
        }
        
        nav_pitch_cd += demanded_pitch_old;

    //         const uint32_t now1 = AP_HAL::millis();
    // if (now1 - _last_AOA_update_ms_pitch1 > 100) {
    //     // don't update at more than 1Hz
    //     hal.console -> printf("%ld\n",nav_pitch_cd);
    //     _last_AOA_update_ms_pitch1 = now1;
    // }



        //姿态模式
        return pitchController.get_servo_out(nav_pitch_cd - ahrs.pitch_sensor, speed_scaler, disable_integrator,
                                            ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
                                            k1,k2,k3);
    }



    // if(k4 = 0){
    //     //增稳模式
    //     return pitchController.get_servo_out(nav_pitch_cd - ahrs.pitch_sensor, speed_scaler, disable_integrator,
    //                                         ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
    //                                         elevator);
    // }else{
    //     //姿态模式
    //     return pitchController.get_servo_out(nav_pitch_cd - ahrs.pitch_sensor, speed_scaler, disable_integrator,
    //                                         ground_mode && !(plane.flight_option_enabled(FlightOptions::DISABLE_GROUND_PID_SUPPRESSION)),
    //                                         k1,k2,k3);
    // }
}

/*
  this gives the user control of the aircraft in stabilization modes, only used in Stabilize Mode
  to be moved to mode_stabilize.cpp in future
 */
void ModeStabilize::stabilize_stick_mixing_direct()
{
    if (!plane.stick_mixing_enabled()) {
        return;
    }
#if HAL_QUADPLANE_ENABLED
    if (!plane.quadplane.allow_stick_mixing()) {
        return;
    }
#endif
    // float aileron = SRV_Channels::get_output_scaled(SRV_Channel::k_aileron);
    // aileron = plane.channel_roll->stick_mixing(aileron);
    // SRV_Channels::set_output_scaled(SRV_Channel::k_aileron, aileron);

    float elevator = SRV_Channels::get_output_scaled(SRV_Channel::k_elevator);
    elevator = plane.channel_pitch->stick_mixing(elevator);
    SRV_Channels::set_output_scaled(SRV_Channel::k_elevator, elevator);
}

/*
  this gives the user control of the aircraft in stabilization modes
  using FBW style controls
 */
void Plane::stabilize_stick_mixing_fbw()
{
    if (!stick_mixing_enabled() ||
        control_mode == &mode_acro ||
        control_mode == &mode_fbwa ||
        control_mode == &mode_autotune ||
        control_mode == &mode_fbwb ||
        control_mode == &mode_cruise ||
#if HAL_QUADPLANE_ENABLED
        control_mode == &mode_qstabilize ||
        control_mode == &mode_qhover ||
        control_mode == &mode_qloiter ||
        control_mode == &mode_qland ||
        control_mode == &mode_qacro ||
#if QAUTOTUNE_ENABLED
        control_mode == &mode_qautotune ||
#endif
        !quadplane.allow_stick_mixing() ||
#endif  // HAL_QUADPLANE_ENABLED
        control_mode == &mode_training) {
        return;
    }
    // do FBW style stick mixing. We don't treat it linearly
    // however. For inputs up to half the maximum, we use linear
    // addition to the nav_roll and nav_pitch. Above that it goes
    // non-linear and ends up as 2x the maximum, to ensure that
    // the user can direct the plane in any direction with stick
    // mixing.
    float roll_input = channel_roll->norm_input_dz();
    if (roll_input > 0.5f) {
        roll_input = (3*roll_input - 1);
    } else if (roll_input < -0.5f) {
        roll_input = (3*roll_input + 1);
    }
    nav_roll_cd += roll_input * roll_limit_cd;
    nav_roll_cd = constrain_int32(nav_roll_cd, -roll_limit_cd, roll_limit_cd);

    if ((control_mode == &mode_loiter) && (plane.flight_option_enabled(FlightOptions::ENABLE_LOITER_ALT_CONTROL))) {
        // loiter is using altitude control based on the pitch stick, don't use it again here
        return;
    }

    float pitch_input = channel_pitch->norm_input_dz();
    if (pitch_input > 0.5f) {
        pitch_input = (3*pitch_input - 1);
    } else if (pitch_input < -0.5f) {
        pitch_input = (3*pitch_input + 1);
    }
    if (fly_inverted()) {
        pitch_input = -pitch_input;
    }
    if (pitch_input > 0) {
        nav_pitch_cd += pitch_input * aparm.pitch_limit_max*100;
    } else {
        nav_pitch_cd += -(pitch_input * pitch_limit_min*100);
    }
    nav_pitch_cd = constrain_int32(nav_pitch_cd, pitch_limit_min*100, aparm.pitch_limit_max.get()*100);
}


/*
  stabilize the yaw axis. There are 3 modes of operation:

    - hold a specific heading with ground steering
    - rate controlled with ground steering
    - yaw control for coordinated flight    
 */
void Plane::stabilize_yaw()
{
    bool ground_steering = false;
    if (landing.is_flaring()) {
        // in flaring then enable ground steering
        ground_steering = true;
    } else {
        // otherwise use ground steering when no input control and we
        // are below the GROUND_STEER_ALT
        ground_steering = (channel_roll->get_control_in() == 0 && 
                                            fabsf(relative_altitude) < g.ground_steer_alt);
        if (!landing.is_ground_steering_allowed()) {
            // don't use ground steering on landing approach
            ground_steering = false;
        }
    }


    /*
      first calculate steering for a nose or tail
      wheel. We use "course hold" mode for the rudder when either performing
      a flare (when the wings are held level) or when in course hold in
      FBWA mode (when we are below GROUND_STEER_ALT)
     */
    float steering_output = 0.0;
    if (landing.is_flaring() ||
        (steer_state.hold_course_cd != -1 && ground_steering)) {
        steering_output = calc_nav_yaw_course();
    } else if (ground_steering) {
        steering_output = calc_nav_yaw_ground();
    }

    /*
      now calculate rudder for the rudder
     */
    const float rudder_output = calc_nav_yaw_coordinated();

    if (!ground_steering) {
        // Not doing ground steering, output rudder on steering channel
        SRV_Channels::set_output_scaled(SRV_Channel::k_rudder, rudder_output);
        SRV_Channels::set_output_scaled(SRV_Channel::k_steering, plane.channel_rudder->get_control_in());//plane.channel_yaw->get_control_in()

    } else if (!SRV_Channels::function_assigned(SRV_Channel::k_steering)) {
        // Ground steering active but no steering output configured, output steering on rudder channel
        SRV_Channels::set_output_scaled(SRV_Channel::k_rudder, steering_output);
        SRV_Channels::set_output_scaled(SRV_Channel::k_steering, plane.channel_rudder->get_control_in());//plane.channel_yaw->get_control_in()

    } else {
        // Ground steering with both steering and rudder channels
        SRV_Channels::set_output_scaled(SRV_Channel::k_rudder, rudder_output);
        SRV_Channels::set_output_scaled(SRV_Channel::k_steering, plane.channel_rudder->get_control_in());//plane.channel_yaw->get_control_in()
    }

}

/*
  main stabilization function for all 3 axes
 */
void Plane::stabilize()
{
    uint32_t now = AP_HAL::millis();
#if HAL_QUADPLANE_ENABLED
    if (quadplane.available()) {
        quadplane.transition->set_FW_roll_pitch(nav_pitch_cd, nav_roll_cd);
    }
#endif

    if (now - last_stabilize_ms > 2000) {
        // if we haven't run the rate controllers for 2 seconds then reset
        control_mode->reset_controllers();
    }
    last_stabilize_ms = now;

    if (control_mode == &mode_training ||
            control_mode == &mode_manual) {
        plane.control_mode->run();
#if AP_SCRIPTING_ENABLED
    } else if (nav_scripting_active()) {
        // scripting is in control of roll and pitch rates and throttle
        const float speed_scaler = get_speed_scaler();
        const float aileron = rollController.get_rate_out(nav_scripting.roll_rate_dps, speed_scaler);
        const float elevator = pitchController.get_rate_out(nav_scripting.pitch_rate_dps, speed_scaler);
        SRV_Channels::set_output_scaled(SRV_Channel::k_aileron, aileron);
        SRV_Channels::set_output_scaled(SRV_Channel::k_elevator, elevator);
        float rudder = 0;
        if (yawController.rate_control_enabled()) {
            rudder = nav_scripting.rudder_offset_pct * 45;
            if (nav_scripting.run_yaw_rate_controller) {
                rudder += yawController.get_rate_out(nav_scripting.yaw_rate_dps, speed_scaler, false);
            } else {
                yawController.reset_I();
            }
        }
        SRV_Channels::set_output_scaled(SRV_Channel::k_rudder, rudder);
        SRV_Channels::set_output_scaled(SRV_Channel::k_steering, rudder);
        SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, plane.nav_scripting.throttle_pct);
#endif
    } else {
        plane.control_mode->run();
    }

    /*
      see if we should zero the attitude controller integrators. 
     */
    if (is_zero(get_throttle_input()) &&
        fabsf(relative_altitude) < 5.0f && 
        fabsf(barometer.get_climb_rate()) < 0.5f &&
        ahrs.groundspeed() < 3) {
        // we are low, with no climb rate, and zero throttle, and very
        // low ground speed. Zero the attitude controller
        // integrators. This prevents integrator buildup pre-takeoff.
        rollController.reset_I();
        pitchController.reset_I();
        yawController.reset_I();

        // if moving very slowly also zero the steering integrator
        if (ahrs.groundspeed() < 1) {
            steerController.reset_I();            
        }
    }
}


void Plane::calc_throttle()
{
    if (aparm.throttle_cruise <= 1) {
        // user has asked for zero throttle - this may be done by a
        // mission which wants to turn off the engine for a parachute
        // landing
        SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, 0.0);
        return;
    }

    float commanded_throttle = TECS_controller.get_throttle_demand();
    SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, commanded_throttle);
}

/*****************************************
* Calculate desired roll/pitch/yaw angles (in medium freq loop)
*****************************************/

/*
  calculate yaw control for coordinated flight
 */
int16_t Plane::calc_nav_yaw_coordinated()
{
    const float speed_scaler = get_speed_scaler();
    bool disable_integrator = false;
    int16_t rudder_in = rudder_input();

    int16_t commanded_rudder;
    bool using_rate_controller = false;

    // Received an external msg that guides yaw in the last 3 seconds?
    if (control_mode->is_guided_mode() &&
            plane.guided_state.last_forced_rpy_ms.z > 0 &&
            millis() - plane.guided_state.last_forced_rpy_ms.z < 3000) {
        commanded_rudder = plane.guided_state.forced_rpy_cd.z;
    } else if (autotuning && g.acro_yaw_rate > 0 && yawController.rate_control_enabled()) {
        // user is doing an AUTOTUNE with yaw rate control
        const float rudd_expo = rudder_in_expo(true);
        const float yaw_rate = (rudd_expo/SERVO_MAX) * g.acro_yaw_rate;
        // add in the coordinated turn yaw rate to make it easier to fly while tuning the yaw rate controller
        const float coordination_yaw_rate = degrees(GRAVITY_MSS * tanf(radians(nav_roll_cd*0.01f))/MAX(aparm.airspeed_min,smoothed_airspeed));
        commanded_rudder = yawController.get_rate_out(yaw_rate+coordination_yaw_rate,  speed_scaler, false);
        using_rate_controller = true;
    } else {
        if (control_mode == &mode_stabilize && rudder_in != 0) {
            disable_integrator = true;
        }

        commanded_rudder = yawController.get_servo_out(speed_scaler, disable_integrator);

        // add in rudder mixing from roll
        commanded_rudder += SRV_Channels::get_output_scaled(SRV_Channel::k_aileron) * g.kff_rudder_mix;
        commanded_rudder += rudder_in;
    }

    if (!using_rate_controller) {
        /*
          When not running the yaw rate controller, we need to reset the rate
        */
        yawController.reset_rate_PID();
    }



    const AP_AHRS &_ahrs = AP::ahrs();

    float rate_z = _ahrs.get_gyro().z;
    float aspeed;
    //低空速修正
    if (!_ahrs.airspeed_estimate(aspeed)) {
        aspeed = 0;
    }
    bool underspeed = aspeed <= float(3);

    //侧滑角反馈p
    float ssa;


    // if(underspeed){
    //     // ssa = 0;
    //     ssa =  0.0f;
    // }else{
    //     ssa = _ahrs.getSSA();
    // }

    const aoassa &myssa = *AP::Aoassa();
    if(underspeed){
        // ssa = 0;
        ssa = myssa._ssa * 0.75f;
    }else{
        ssa = myssa._ssa ;
    }

    float my_ssa = ssa;
    my_ssa = constrain_float(my_ssa , -30.0F,30.0F);

    // hal.console -> printf("%f\n",ssa);
    float boost_ssa = 0.3;
    if(aspeed > 0.0f){
        boost_ssa =0.3f - 0.3f/50.0f*((abs(aspeed)));
    }
    boost_ssa = constrain_float(boost_ssa, 0.005f,0.3f);

    float k6 =-0.5f + abs(channel_k6->norm_input_dz()); //0.1-1-5
    if(k6 < 0.0f){
        k6 =1.0f + 1.8f * k6;
    }else{
        k6 =1.0f + 18.0f * k6;
    }

    // hal.console -> printf("%f\n",k6);
    float out_ssa = boost_ssa * (my_ssa*0  - degrees(rate_z) )* k6;

    float commanded_rudder_my = out_ssa + rudder_in * 0.01 *1;//注意rud方向
    // hal.console -> printf("%f\n",ssa);






    return constrain_int16(commanded_rudder * 0 + commanded_rudder_my * 100, -4500, 4500);
}

/*
  calculate yaw control for ground steering with specific course
 */
int16_t Plane::calc_nav_yaw_course(void)
{
    // holding a specific navigation course on the ground. Used in
    // auto-takeoff and landing
    int32_t bearing_error_cd = nav_controller->bearing_error_cd();
    int16_t steering = steerController.get_steering_out_angle_error(bearing_error_cd);
    if (stick_mixing_enabled()) {
        steering = channel_rudder->stick_mixing(steering);
    }
    return constrain_int16(steering, -4500, 4500);
}

/*
  calculate yaw control for ground steering
 */
int16_t Plane::calc_nav_yaw_ground(void)
{
    if (gps.ground_speed() < 1 && 
        is_zero(get_throttle_input()) &&
        flight_stage != AP_FixedWing::FlightStage::TAKEOFF &&
        flight_stage != AP_FixedWing::FlightStage::ABORT_LANDING) {
        // manual rudder control while still
        steer_state.locked_course = false;
        steer_state.locked_course_err = 0;
        return rudder_input();
    }

    // if we haven't been steering for 1s then clear locked course
    const uint32_t now_ms = AP_HAL::millis();
    if (now_ms - steer_state.last_steer_ms > 1000) {
        steer_state.locked_course = false;
    }
    steer_state.last_steer_ms = now_ms;

    float steer_rate = (rudder_input()/4500.0f) * g.ground_steer_dps;
    if (flight_stage == AP_FixedWing::FlightStage::TAKEOFF ||
        flight_stage == AP_FixedWing::FlightStage::ABORT_LANDING) {
        steer_rate = 0;
    }
    if (!is_zero(steer_rate)) {
        // pilot is giving rudder input
        steer_state.locked_course = false;        
    } else if (!steer_state.locked_course) {
        // pilot has released the rudder stick or we are still - lock the course
        steer_state.locked_course = true;
        if (flight_stage != AP_FixedWing::FlightStage::TAKEOFF &&
            flight_stage != AP_FixedWing::FlightStage::ABORT_LANDING) {
            steer_state.locked_course_err = 0;
        }
    }

    int16_t steering;
    if (!steer_state.locked_course) {
        // use a rate controller at the pilot specified rate
        steering = steerController.get_steering_out_rate(steer_rate);
    } else {
        // use a error controller on the summed error
        int32_t yaw_error_cd = -ToDeg(steer_state.locked_course_err)*100;
        steering = steerController.get_steering_out_angle_error(yaw_error_cd);
    }
    return constrain_int16(steering, -4500, 4500);
}


/*
  calculate a new nav_pitch_cd from the speed height controller
 */
void Plane::calc_nav_pitch()
{
    int32_t commanded_pitch = TECS_controller.get_pitch_demand();
    nav_pitch_cd = constrain_int32(commanded_pitch, pitch_limit_min*100, aparm.pitch_limit_max.get()*100);
}


/*
  calculate a new nav_roll_cd from the navigation controller
 */
void Plane::calc_nav_roll()
{
    int32_t commanded_roll = nav_controller->nav_roll_cd();
    nav_roll_cd = constrain_int32(commanded_roll, -roll_limit_cd, roll_limit_cd);
    update_load_factor();
}

/*
  adjust nav_pitch_cd for STAB_PITCH_DOWN_CD. This is used to make
  keeping up good airspeed in FBWA mode easier, as the plane will
  automatically pitch down a little when at low throttle. It makes
  FBWA landings without stalling much easier.
 */
void Plane::adjust_nav_pitch_throttle(void)
{
    int8_t throttle = throttle_percentage();
    if (throttle >= 0 && throttle < aparm.throttle_cruise && flight_stage != AP_FixedWing::FlightStage::VTOL) {
        float p = (aparm.throttle_cruise - throttle) / (float)aparm.throttle_cruise;
        nav_pitch_cd -= g.stab_pitch_down * 100.0f * p;
    }
}


/*
  calculate a new aerodynamic_load_factor and limit nav_roll_cd to
  ensure that the load factor does not take us below the sustainable
  airspeed
 */
void Plane::update_load_factor(void)
{
    float demanded_roll = fabsf(nav_roll_cd*0.01f);
    if (demanded_roll > 85) {
        // limit to 85 degrees to prevent numerical errors
        demanded_roll = 85;
    }
    aerodynamic_load_factor = 1.0f / safe_sqrt(cosf(radians(demanded_roll)));

#if HAL_QUADPLANE_ENABLED
    if (quadplane.available() && quadplane.transition->set_FW_roll_limit(roll_limit_cd)) {
        nav_roll_cd = constrain_int32(nav_roll_cd, -roll_limit_cd, roll_limit_cd);
        return;
    }
#endif

    if (!aparm.stall_prevention) {
        // stall prevention is disabled
        return;
    }
    if (fly_inverted()) {
        // no roll limits when inverted
        return;
    }
#if HAL_QUADPLANE_ENABLED
    if (quadplane.tailsitter.active()) {
        // no limits while hovering
        return;
    }
#endif

    float max_load_factor = smoothed_airspeed / MAX(aparm.airspeed_min, 1);
    if (max_load_factor <= 1) {
        // our airspeed is below the minimum airspeed. Limit roll to
        // 25 degrees
        nav_roll_cd = constrain_int32(nav_roll_cd, -2500, 2500);
        roll_limit_cd = MIN(roll_limit_cd, 2500);
    } else if (max_load_factor < aerodynamic_load_factor) {
        // the demanded nav_roll would take us past the aerodynamic
        // load limit. Limit our roll to a bank angle that will keep
        // the load within what the airframe can handle. We always
        // allow at least 25 degrees of roll however, to ensure the
        // aircraft can be manoeuvred with a bad airspeed estimate. At
        // 25 degrees the load factor is 1.1 (10%)
        int32_t roll_limit = degrees(acosf(sq(1.0f / max_load_factor)))*100;
        if (roll_limit < 2500) {
            roll_limit = 2500;
        }
        nav_roll_cd = constrain_int32(nav_roll_cd, -roll_limit, roll_limit);
        roll_limit_cd = MIN(roll_limit_cd, roll_limit);
    }    
}