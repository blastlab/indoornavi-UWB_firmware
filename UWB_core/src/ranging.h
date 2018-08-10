/**
 * @brief ranging enginge
 *
 * resposible for initiating new measurements
 *
 * @file ranging.h
 * @author Karol Trzcinski
 * @date 2018-08-07
 */

#ifndef _RANGING_H
#define _RANGING_H

#include "mac/carry.h"
#include "mac/toa.h"
#include "mac/toa_routine.h"
#include "ranging_settings.h"

typedef struct {
  dev_addr_t tempAncList[RANGING_TEMP_ANC_LIST_DEPTH];
  int tempAncListCnt;
  int measureReadInd;
  unsigned int resetTime, lastInitSendTime;
} ranging_instance_t;

/**
 * @brief
 *
 * @param tagDid dst device ID, Tag or Anchor, this device will send Poll
 * @param ancDid list of anchors to measure distance with
 * @param ancCnt number of anchors to measure distance with
 * @return true when success
 * @return false when some error occured
 */
bool RANGING_MeasureAdd(dev_addr_t tagDid, dev_addr_t ancDid[], int ancCnt);

/**
 * @brief return number of saved measure traces
 *
 * @return int number of saved measure traces
 */
int RANGING_MeasureCounter();

/**
 * @brief delete each measure from list with same specified tag
 *
 * @param tagDid tag identifier to delete each trace too
 * @return true when this tag has been found and deleted from measure list
 * @return false otherwise
 */
bool RANGING_MeasureDeleteTag(dev_addr_t tagDid);

/**
 * @brief clear measure list
 *
 * this yields to stop ranging
 *
 */
void RANGING_MeasureDeleteAll();

/**
 * @brief delete a few newest measure traces from list
 *
 * @param number of measures to delete
 */
void RANGING_MeasureDeleteLast(int number);

/**
 * @brief reset temporary anchors addresses
 *
 */
void RANGING_TempAnchorsReset();

/**
 * @brief add anchor to temporary anchors list
 *
 * @param did of new anchor
 * @return true after success
 * @return false otherwise
 */
bool RANGING_TempAnchorsAdd(dev_addr_t did);

/**
 * @brief return number of temporary anchors is on the list
 *
 * @return int number of temporary anchors on the list
 */
int RANGING_TempAnchorsCounter();

/**
 * @brief add measures between a given tag and each anchor from temporary array
 *
 * @param did of device to measure
 * @param maxAncInMeasure number of anchors in single trace
 * @return true after success
 * @return false otherwise
 */
bool RANGING_AddTagWithTempAnchors(dev_addr_t did, int maxAncInMeasure);

/**
 * @brief process ranging engine
 *
 * this function should be called frequently.
 * Measures are initiated from this function.
 * When there is no enough time to send each
 * ranging init message, then ranging period
 * will extend to send each init message.
 *
 */
void RANGING_Control();

#endif
