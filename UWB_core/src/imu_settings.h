/**
 * @brief measure traces settings
 *
 * @file imu_settings.h
 * @author Dawid Peplinski
 * @date 2018-09-12
 */

#ifndef _IMU_SETTINGS_H
#define _IMU_SETTINGS_H

typedef struct {
  uint8_t is_enabled;
  uint32_t no_motion_period;
} imu_settings_t;

#define IMU_SETTINGS_DEF \
  { .is_enabled = 0, .no_motion_period = 30 }

#endif
