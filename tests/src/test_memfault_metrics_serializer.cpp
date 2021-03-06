//! @file
//!
//! @brief

#include "CppUTest/MemoryLeakDetectorMallocMacros.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "fakes/fake_memfault_event_storage.h"
#include "memfault/core/event_storage.h"
#include "memfault/metrics/serializer.h"
#include "memfault/metrics/utils.h"

static const sMemfaultEventStorageImpl *s_fake_event_storage_impl;
#define FAKE_EVENT_STORAGE_SIZE 50

TEST_GROUP(MemfaultMetricsSerializer){
  void setup() {
     static uint8_t s_storage[FAKE_EVENT_STORAGE_SIZE];
     s_fake_event_storage_impl = memfault_events_storage_boot(
         &s_storage, sizeof(s_storage));
  }
  void teardown() {
    mock().checkExpectations();
    mock().clear();
  }
};

//
// For the purposes of our serialization test, we will
// just serialize 1 of each supported type
//

void memfault_metrics_heartbeat_iterate(MemfaultMetricIteratorCallback cb, void *ctx) {
  sMemfaultMetricInfo info = { 0 };
  info.key._impl = "unsigned_int";
  info.type = kMemfaultMetricType_Unsigned;
  info.val.u32 = 1000;
  cb(ctx, &info);

  info.key._impl = "signed_int";
  info.type = kMemfaultMetricType_Signed;
  info.val.i32 = -1000;
  cb(ctx, &info);

  info.key._impl = "timer_key";
  info.type = kMemfaultMetricType_Timer;
  info.val.u32 = 1234;
  cb(ctx, &info);
}

size_t memfault_metrics_heartbeat_get_num_metrics(void) {
  // if this fails, it means we need to add add a report for the new type
  // to the fake "memfault_metrics_heartbeat_iterate"
  LONGS_EQUAL(kMemfaultMetricType_NumTypes, 3);
  return kMemfaultMetricType_NumTypes;
}

TEST(MemfaultMetricsSerializer, Test_MemfaultMetricSerialize) {
  mock().expectOneCall("prv_begin_write");
  mock().expectOneCall("prv_finish_write").withParameter("rollback", false);

  memfault_metrics_heartbeat_serialize(s_fake_event_storage_impl);

  // {
  // "2": 1,
  // "3": 1,
  // "7": "DAABBCCDD",
  // "10": "main",
  // "9": "1.2.3",
  // "6": "evt_24",
  // "4": {
  //  "1": [ 1000, -1000, 1234 ]
  //  }
  // }
  const uint8_t expected_serialization[] = {
      0xa7, 0x02, 0x01, 0x03, 0x01, 0x07, 0x69, 0x44, 0x41, 0x41,
      0x42, 0x42, 0x43, 0x43, 0x44, 0x44, 0x0a, 0x64, 0x6d, 0x61,
      0x69, 0x6e, 0x09, 0x65, 0x31, 0x2e, 0x32, 0x2e, 0x33, 0x06,
      0x66, 0x65, 0x76, 0x74, 0x5f, 0x32, 0x34, 0x04, 0xa1, 0x01,
      0x83, 0x19, 0x03, 0xe8, 0x39, 0x03, 0xe7, 0x19, 0x04, 0xd2,
  };

  fake_event_storage_assert_contents_match(expected_serialization, sizeof(expected_serialization));
}

TEST(MemfaultMetricsSerializer, Test_MemfaultMetricSerializeWorstCaseSize) {
  const size_t worst_case_storage = memfault_metrics_heartbeat_compute_worst_case_storage_size();
  LONGS_EQUAL(56, worst_case_storage);
}

TEST(MemfaultMetricsSerializer, Test_MemfaultMetricSerializeOutOfSpace) {
  // iterate over all buffer sizes less than the encoding we need
  // this should exercise all early exit paths
  for (size_t i = 0; i < FAKE_EVENT_STORAGE_SIZE - 1; i++) {
    fake_memfault_event_storage_clear();
    fake_memfault_event_storage_set_available_space(i);

    mock().expectOneCall("prv_begin_write");
    mock().expectOneCall("prv_finish_write").withParameter("rollback", true);

    memfault_metrics_heartbeat_serialize(s_fake_event_storage_impl);

    mock().checkExpectations();
  }
}

TEST(MemfaultMetricsSerializer, Test_MemfaultMetricTypes) {
  //! These should never change so that the same value can
  //! always be used to recover the type on the server
  CHECK_EQUAL(0, kMemfaultMetricType_Unsigned);
  CHECK_EQUAL(1, kMemfaultMetricType_Signed);
  CHECK_EQUAL(2, kMemfaultMetricType_Timer);
  //! This can change if new types are appended to the enum
  //! but we assert here to remind us to add the new type
  //! to the check here
  CHECK_EQUAL(3, kMemfaultMetricType_NumTypes);
}
