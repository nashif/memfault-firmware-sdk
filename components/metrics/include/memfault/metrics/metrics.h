#pragma once

//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
//!
//! @brief
//! The Memfault metric events API
//!
//! This APIs allows one to collect periodic events known as heartbeats for visualization in the
//! Memfault web UI. Heartbeats are a great way to inspect the overall health of devices in your
//! fleet.
//!
//! Typically, two types of information are collected:
//! 1) values taken at the end of the interval (i.e battery life, heap high water mark, stack high water mark)
//! 2) changes over the hour (i.e the percent battery drop, the number of bytes sent out over
//!    bluetooth, the time the mcu was running or in stop mode)
//!
//! From the Memfault web UI, you can view all of these metrics plotted for an individual device &
//! configure alerts to fire when values are not within an expected range.
//!
//! For a step-by-step walk-through about how to integrate the Metrics component into your system,
//! check out https://mflt.io/2D8TRLX

#include <inttypes.h>

#include "memfault/core/event_storage.h"
#include "memfault/metrics/ids_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

//! The frequency in seconds to collect heartbeat metrics. The suggested interval is once per hour
//! but the value can be overriden to be as low as once every 15 minutes.
#ifndef MEMFAULT_METRICS_HEARTBEAT_INTERVAL_SECS
#define MEMFAULT_METRICS_HEARTBEAT_INTERVAL_SECS 3600
#endif

#include "memfault/core/compiler.h"

//! Type of a metric value
typedef enum MemfaultMetricValueType {
  //! unsigned integer (max. 32-bits)
  kMemfaultMetricType_Unsigned = 0,
  //! signed integer (max. 32-bits)
  kMemfaultMetricType_Signed,
  //! Tracks durations (i.e the time a certain task is running, or the time a MCU is in sleep mode)
  kMemfaultMetricType_Timer,

  //! Number of valid types. Must _always_ be last
  kMemfaultMetricType_NumTypes,
} eMemfaultMetricType;

//! Defines a key/value pair used for generating Memfault events.
//!
//! This define should _only_ be used for defining events in 'memfault_metric_heartbeat_config.def'
//! i.e, the *.def file for a heartbeat which tracks battery level & temperature would look
//! something like:
//!
//! // memfault_metrics_heartbeat_config.def
//! MEMFAULT_METRICS_KEY_DEFINE(battery_level, kMemfaultMetricType_Unsigned)
//! MEMFAULT_METRICS_KEY_DEFINE(ambient_temperature_celcius, kMemfaultMetricType_Signed)
//!
//! @param key_name The name of the key, without quotes. This gets surfaced in the Memfault UI, so
//! it's useful to make these names human readable. C variable naming rules apply.
//! @param value_type The type for this key
//!
//! @note key_names must be unique
#define MEMFAULT_METRICS_KEY_DEFINE(key_name, value_type) \
  _MEMFAULT_METRICS_KEY_DEFINE(key_name, value_type)

//! Uses a metric key. Before you can use a key, it should defined using MEMFAULT_METRICS_KEY_DEFINE
//! in memfault_metrics_heartbeat_config.def.
//! @param key_name The name of the key, without quotes, as defined using MEMFAULT_METRICS_KEY_DEFINE.
#define MEMFAULT_METRICS_KEY(key_name) \
  _MEMFAULT_METRICS_ID(key_name)

//! Initializes the metric events API.
//! All heartbeat values will be initialized to 0.
//! @note Memfault will start collecting metrics once this function returns.
//! @return 0 on success, else error code
int memfault_metrics_boot(const sMemfaultEventStorageImpl *storage_impl);

//! Set the value of a signed integer metric.
//! @param key The key of the metric. @see MEMFAULT_METRICS_KEY
//! @param value The new value to set for the metric
//! @return 0 on success, else error code
//! @note The metric must be of type kMemfaultMetricType_Signed
int memfault_metrics_heartbeat_set_signed(MemfaultMetricId key, int32_t signed_value);

//! Same as @memfault_metrics_heartbeat_set_signed except for a unsigned integer metric
int memfault_metrics_heartbeat_set_unsigned(MemfaultMetricId key, uint32_t unsigned_value);

//! Used to start a "timer" metric
//!
//! Timer metrics can be useful for tracking durations of events which take place while the
//! system is running. Some examples:
//!  - time a task was running
//!  - time spent in different power modes (i.e run, sleep, idle)
//!  - amount of time certain peripherals were running (i.e accel, bluetooth, wifi)
//!
//! @return 0 if starting the metric was successful, else error code.
int memfault_metrics_heartbeat_timer_start(MemfaultMetricId key);

//! Same as @memfault_metrics_heartbeat_start but *stops* the timer metric
//!
//! @return 0 if stopping the timer was successful, else error code
int memfault_metrics_heartbeat_timer_stop(MemfaultMetricId key);

//! Add the value to the current value of a metric.
//! @param key The key of the metric. @see MEMFAULT_METRICS_KEY
//! @param inc The amount to increment the metric by
//! @return 0 on success, else error code
//! @note The metric must be of type kMemfaultMetricType_Counter
int memfault_metrics_heartbeat_add(MemfaultMetricId key, int32_t amount);

//! For debugging purposes: prints the current heartbeat values using MEMFAULT_LOG_DEBUG().
void memfault_metrics_heartbeat_debug_print(void);

//! For debugging purposes: triggers the heartbeat data collection handler, as if the heartbeat timer
//! had fired. We recommend also testing that the heartbeat timer fires by itself. To get the periodic data
//! collection triggering rapidly for testing and debugging, consider using a small value for
//! MEMFAULT_METRICS_HEARTBEAT_INTERVAL_SECS.
void memfault_metrics_heartbeat_debug_trigger(void);

//! For debugging and unit test purposes, allows for the extraction of different values
int memfault_metrics_heartbeat_read_unsigned(MemfaultMetricId key, uint32_t *read_val);
int memfault_metrics_heartbeat_read_signed(MemfaultMetricId key, int32_t *read_val);
int memfault_metrics_heartbeat_timer_read(MemfaultMetricId key, uint32_t *read_val);

#ifdef __cplusplus
}
#endif
