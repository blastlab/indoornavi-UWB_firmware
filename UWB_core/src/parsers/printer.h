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
#include "../logs.h"


/**
 * @brief print version message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device
 */
void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did);


/**
 * @brief print status message
 * 
 * @param data pointer to structure with data to print
 * @param did identifier of device
 */
void PRINT_Stat(const FC_STAT_s *data, dev_addr_t did);

#endif
