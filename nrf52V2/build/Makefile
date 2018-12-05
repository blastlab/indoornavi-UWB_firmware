PROJECT_NAME     := uwb_nrf_15_2
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

SDK_ROOT := ../nrf_libs
PROJ_DIR := ../
UWB_DIR := ../../UWB_core

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := uwb_nrf_gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(PROJ_DIR)/main.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spim.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_saadc.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_wdt.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_timer.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rtc.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/crc16/crc16.c \
  $(SDK_ROOT)/components/libraries/fifo/app_fifo.c \
  $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(PROJ_DIR)/platform/port_ble.c \
  $(PROJ_DIR)/platform/logs.c \
  $(PROJ_DIR)/platform/port.c \
  $(PROJ_DIR)/platform/port_adc.c \
  $(PROJ_DIR)/platform/port_crc.c \
  $(PROJ_DIR)/platform/port_flash.c \
  $(PROJ_DIR)/platform/port_iassert.c \
  $(PROJ_DIR)/platform/port_mutex.c \
  $(PROJ_DIR)/platform/port_spi.c \
  $(PROJ_DIR)/platform/port_time.c \
  $(PROJ_DIR)/platform/port_usb.c \
  $(PROJ_DIR)/platform/port_imu.c \
  $(UWB_DIR)/src/decadriver/deca_device.c \
  $(UWB_DIR)/src/decadriver/deca_params_init.c \
  $(UWB_DIR)/src/logger/logs_common.c \
  $(UWB_DIR)/src/logger/logs_buffer.c \
  $(UWB_DIR)/src/mac/carry.c \
  $(UWB_DIR)/src/mac/mac.c \
  $(UWB_DIR)/src/mac/sync.c \
  $(UWB_DIR)/src/mac/toa.c \
  $(UWB_DIR)/src/mac/toa_calib.c \
  $(UWB_DIR)/src/mac/toa_routine.c \
  $(UWB_DIR)/src/parsers/base64.c \
  $(UWB_DIR)/src/parsers/bin_parser.c \
  $(UWB_DIR)/src/parsers/bin_parser_cb.c \
  $(UWB_DIR)/src/parsers/listener_parser.c \
  $(UWB_DIR)/src/parsers/printer.c \
  $(UWB_DIR)/src/parsers/txt_parser.c \
  $(UWB_DIR)/src/parsers/txt_parser_cb.c \
  $(UWB_DIR)/src/prot/FU.c \
  $(UWB_DIR)/src/settings.c \
  $(UWB_DIR)/src/tools.c \
  $(UWB_DIR)/src/transceiver.c \
  $(UWB_DIR)/src/transceiver_translator.c \
  $(UWB_DIR)/src/ranging.c \
  $(UWB_DIR)/src/uwb_main.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/softdevice/s132/headers \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/crc16 \
  $(SDK_ROOT)/components/libraries/uart \
  $(SDK_ROOT)/components/libraries/fifo \
  $(PROJ_DIR) \
  ../config \
  ../platform \
  $(UWB_DIR)/src/decadriver \
  $(UWB_DIR)/src/mac \
  $(UWB_DIR)/src/parsers \
  $(UWB_DIR)/src/prot \
  $(UWB_DIR)/src/platform \
  $(UWB_DIR)/src \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74
CFLAGS += -DNRF_SD_BLE_API_VERSION=6
CFLAGS += -DS132
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -DSWI_DISABLE0
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=6
ASMFLAGS += -DS132
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DSWI_DISABLE0

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52832_xxaa: CFLAGS += -D__HEAP_SIZE=8192
nrf52832_xxaa: CFLAGS += -D__STACK_SIZE=8192
nrf52832_xxaa: ASMFLAGS += -D__HEAP_SIZE=8192
nrf52832_xxaa: ASMFLAGS += -D__STACK_SIZE=8192

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm


.PHONY: default help

# Default target - first one defined
default: nrf52832_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52832_xxaa
	@echo		sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash flash_softdevice erase

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $< --sectorerase --reset
	
flash_no_reset: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $< --sectorerase

erase:
	nrfjprog -f nrf52 --eraseall
	
# Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_6.1.0_softdevice.hex
	nrfjprog -f nrf52 --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_6.1.0_softdevice.hex --sectorerase --reset

flash_all: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	mergehex -m $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_6.1.0_softdevice.hex ../bootloader/build/_build/nrf52832_xxaa.hex $< -o $(OUTPUT_DIRECTORY)/SD_BL_APP.hex
	nrfjprog --program $(OUTPUT_DIRECTORY)/SD_BL_APP.hex --chiperase --reset -f NRF52
	
generate_bin:
	make clean
	python ../updateHash.py
	make -j8
	
reset:
	nrfjprog --reset -f NRF52
	