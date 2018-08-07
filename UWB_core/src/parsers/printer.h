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
 * @brief print rf settings
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device connected with this data
 */
void PRINT_RFSet(const FC_RF_SET_s *data, dev_addr_t did);

/**
 * @brief print measure info
 * 
 * @param data pointer to structure with data to print
 */
void PRINT_Measure(const measure_t *data);

#endif
