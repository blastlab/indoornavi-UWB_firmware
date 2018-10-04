/**
 * @file logs_crit.h
 * @author Karol Trzcinski
 * @brief critical logs definitions
 * @date 2018-10-02
 *
 * @copyright Copyright (c) 2018
 *
 */
ADD_ITEM_M(1000, CRIT_OTHER, "Critical error")

ADD_ITEM_M(1001, CRIT_LOG_CODES_ARE_NOT_UNIQ, "logger codes aren't uniq, code:%d")
ARG("code", "message code")
COMMENT("it is logger self test error")

ADD_ITEM_M(1002, CRIT_LOG_CODES_ARE_NOT_MONOTONOUS, "logger codes aren't monotonous, code:%d")
ARG("code", "message code")
COMMENT("it is logger self test error")
COMMENT("when codes aren't monotonous then probability of error is bigger")
