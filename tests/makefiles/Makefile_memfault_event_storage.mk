COMPONENT_NAME=memfault_metrics_storage

SRC_FILES = \
  $(MFLT_COMPONENTS_DIR)/core/src/memfault_event_storage.c

MOCK_AND_FAKE_SRC_FILES += \
  $(MFLT_COMPONENTS_DIR)/util/src/memfault_circular_buffer.c \
  $(MFLT_TEST_FAKE_DIR)/fake_memfault_platform_debug_log.c \
  $(MFLT_TEST_FAKE_DIR)/fake_memfault_platform_locking.c \

TEST_SRC_FILES = \
  $(MFLT_TEST_SRC_DIR)/test_memfault_event_storage.cpp \
  $(MOCK_AND_FAKE_SRC_FILES)

include $(CPPUTEST_MAKFILE_INFRA)
