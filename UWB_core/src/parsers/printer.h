/**
 * @brief Common messages printers
 * 
 * Printers has been developed to help keep printed comunicates consistent.
 * Moreover printing some struct data from any part of code will look this same.
 * 
 * @file printer.h
 * @author Karol Trzcinski
 * @date 2018-06-28
 */
#ifndef _PRINTER_H
#define _PRINTER_H
#include "bin_struct.h"
#include "../mac/toa.h"
#include "../logs.h"

/**
 * @brief print version message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did);

/**
 * @brief print status message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_Stat(const FC_STAT_s *data, dev_addr_t did);

/**
 * @brief print turn on message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_TurnOn(const FC_TURN_ON_s *data, dev_addr_t did);

/**
 * @brief print turn off message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_TurnOff(const FC_TURN_OFF_s *data, dev_addr_t did);

/**
 * @brief print beacon message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_Beacon(const FC_BEACON_s *data, dev_addr_t did);


/**
 * @brief print device accepted info
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_DeviceAccepted(const FC_DEV_ACCEPTED_s *data, dev_addr_t did);

/**
 * @brief print device accepted info
 *
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_SettingsSaveResult(const FC_SETTINGS_SAVE_RESULT_s *data, dev_addr_t did);

/**
 * @brief print rf settings
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_RFSet(const FC_RF_SET_s *data, dev_addr_t did);

/** 
 * @brief print ble settings
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_BleSet(const FC_BLE_SET_s *data, dev_addr_t did);

/**
 * @brief print imu settings
 *
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_ImuSet(const FC_IMU_SET_s *data, dev_addr_t did);
/**
 * @brief print measure info
 * 
 * @param data pointer to structure with data to print
 */
void PRINT_Measure(const measure_t *data);

/**
 * @brief print measure initialization informations
 *
 * @param data pointer to structure with data to print
 */
void PRINT_MeasureInitInfo(const measure_init_info_t *data);


/**
 * @brief print sink ranging time
 */
void PRINT_RangingTime();

/**
 * @brief print toa settings info
 *
 * @param prefix string printed before data
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_ToaSettings(const char* prefix, const toa_settings_t *data, dev_addr_t did);

/**
 * @brief print parent for device
 * 
 * @param parent did
 * @param child did
 * @param level number of hops between sink and device
 */
void PRINT_Parent(dev_addr_t parent, dev_addr_t child, int level);

#endif
