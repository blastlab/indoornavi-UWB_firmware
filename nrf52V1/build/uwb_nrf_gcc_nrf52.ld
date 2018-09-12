/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

PROG_PAGE_SIZE = 4K;
PROG_FLASH_SIZE = 0x5C000;
PROG_FLASH_START = 0x23000;
PROG_BOOTLOADER_MAX_SIZE = 32K;
PROG_MAX_PROGRAM_SIZE = (PROG_FLASH_SIZE - PROG_BOOTLOADER_MAX_SIZE) / 2;
PROG_DESTINATION1 = PROG_FLASH_START + PROG_BOOTLOADER_MAX_SIZE;
PROG_DESTINATION2 = PROG_DESTINATION1 + PROG_MAX_PROGRAM_SIZE;

/* 1 - Only App1 */
/* 2 - Only App2 */
COMPILE = 1;
FINAL_PROG_DEST = COMPILE==1 ? PROG_DESTINATION1 : PROG_DESTINATION2;
PROG_START = FINAL_PROG_DEST;
PROG_SETTINGS = PROG_START + PROG_PAGE_SIZE;
PROG_TEXT = PROG_SETTINGS + 2*PROG_PAGE_SIZE;

MEMORY
{
  FLASH (rx) : ORIGIN = FINAL_PROG_DEST, LENGTH = PROG_MAX_PROGRAM_SIZE
  RAM (rwx) :  ORIGIN = 0x20003478, LENGTH = 0xCB88
}

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .cli_sorted_cmd_ptrs :
  {
    PROVIDE(__start_cli_sorted_cmd_ptrs = .);
    KEEP(*(.cli_sorted_cmd_ptrs))
    PROVIDE(__stop_cli_sorted_cmd_ptrs = .);
  } > RAM
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > FLASH
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
  .sdh_req_observers :
  {
    PROVIDE(__start_sdh_req_observers = .);
    KEEP(*(SORT(.sdh_req_observers*)))
    PROVIDE(__stop_sdh_req_observers = .);
  } > FLASH
  .sdh_state_observers :
  {
    PROVIDE(__start_sdh_state_observers = .);
    KEEP(*(SORT(.sdh_state_observers*)))
    PROVIDE(__stop_sdh_state_observers = .);
  } > FLASH
  .sdh_stack_observers :
  {
    PROVIDE(__start_sdh_stack_observers = .);
    KEEP(*(SORT(.sdh_stack_observers*)))
    PROVIDE(__stop_sdh_stack_observers = .);
  } > FLASH
    .cli_command :
  {
    PROVIDE(__start_cli_command = .);
    KEEP(*(.cli_command))
    PROVIDE(__stop_cli_command = .);
  } > FLASH
  

} INSERT AFTER .text

INCLUDE "nrf52_common_uwb.ld"
