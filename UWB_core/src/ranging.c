#include "ranging.h"
#include "parsers/printer.h"

ranging_instance_t ranging;

measure_init_info_t* RANGING_MeasureNext() {
	int ind = ranging.measureReadInd;
	if (ind >= settings.ranging.measureCnt) {
		return 0;
	}

	measure_init_info_t* ptr = &settings.ranging.measure[ind];
	++ranging.measureReadInd;
	return ptr;
}

bool RANGING_MeasureAdd(dev_addr_t tagDid, dev_addr_t ancDid[], int ancCnt) {
	if (settings.ranging.measureCnt >= MEASURE_TRACE_MEMORY_DEPTH) {
		return false;
	}
	if (ancCnt <= 0 || TOA_MAX_DEV_IN_POLL <= ancCnt) {
		return false;
	}
	if (ancDid[0] == ADDR_BROADCAST) {
		return false;
	}

	int cnt = settings.ranging.measureCnt;
	measure_init_info_t* ptr = &settings.ranging.measure[cnt];
	++settings.ranging.measureCnt;

	memset(ptr, 0, sizeof(*ptr));
	memcpy(ptr->ancDid, ancDid, ancCnt * sizeof(*ptr->ancDid));
	ptr->numberOfAnchors = ancCnt;
	ptr->tagDid = tagDid;
	return true;
}

int RANGING_MeasureCounter() {
	return settings.ranging.measureCnt;
}

measure_init_info_t* RANGING_MeasureFind(dev_addr_t did1, dev_addr_t did2) {
	measure_init_info_t* meas = &settings.ranging.measure[0];
	for (int i = 0; i < settings.ranging.measureCnt; ++i, ++meas) {
		// gdy did1 to target a did2 to anchor
		if (meas->tagDid == did1) {
			for (int j = 0; j < meas->numberOfAnchors; ++j) {
				if (meas->ancDid[j] == did2) {
					return meas;
				}
			}
		}
		// oraz sytuacja odwrotna
		if (meas->tagDid == did2) {
			for (int j = 0; j < meas->numberOfAnchors; ++j) {
				if (meas->ancDid[j] == did1) {
					return meas;
				}
			}
		}
	}
	return 0;
}

bool RANGING_MeasureDeleteTag(dev_addr_t tagDid) {
	bool result = false;
	// for each measure, begginning from the end
	for (int i = settings.ranging.measureCnt - 1; i >= 0; --i) {
		// when it is measure to delete
		if (settings.ranging.measure[i].tagDid == tagDid) {
			--settings.ranging.measureCnt;  // then remove the last one
			if (i != settings.ranging.measureCnt) {
				// replace measure to delete with the previously deleted one
				memcpy(&settings.ranging.measure[i], &settings.ranging.measure[settings.ranging.measureCnt],
				       sizeof(*settings.ranging.measure));
			}
			result = true;
		}
	}
	return result;
}

void RANGING_MeasureDeleteLast(int number) {
	if (number > 0) {
		settings.ranging.measureCnt -= MIN(number, settings.ranging.measureCnt);
	}
}

void RANGING_MeasureDeleteAll() {
	settings.ranging.measureCnt = 0;
	ranging.measureReadInd = 0;
}

void RANGING_TempAnchorsReset() {
	ranging.tempAncListCnt = 0;
}

bool RANGING_TempAnchorsAdd(dev_addr_t did) {
	if (did >= ADDR_BROADCAST) {
		return false;
	}
	if (ranging.tempAncListCnt < RANGING_TEMP_ANC_LIST_DEPTH) {
		ranging.tempAncList[ranging.tempAncListCnt++] = did;
		return true;
	} else {
		return false;
	}
}

int RANGING_TempAnchorsCounter() {
	return ranging.tempAncListCnt;
}

bool RANGING_AddTagWithTempAnchors(dev_addr_t did, int maxAncInMeasure) {
	// wpisz do bufra adresy urzadzen z ktorymi jeszcze nie wykonujesz pomiarow
	// oraz nie zawieraj tam swojego adresu
	dev_addr_t* buf = (dev_addr_t*)MAC_Buffer();
	int adr_cnt = 0;
	if (buf == 0) {
		return false;
	}
	for (int i = 0; i < ranging.tempAncListCnt; ++i) {
		if (did != ranging.tempAncList[i] && RANGING_MeasureFind(did, ranging.tempAncList[i]) == 0) {
			buf[adr_cnt] = ranging.tempAncList[i];
			++adr_cnt;
		}
	}
	// dodaj pomiary pomiedzy anchorami z bufora i zadanym urzadzeniam
	int cnt = MIN(maxAncInMeasure, adr_cnt);
	int meas_cnt = 0;
	for (int i = 0; cnt != 0;) {
		if (!RANGING_MeasureAdd(did, &buf[i], cnt)) {
			// gdy cos poszlo zle, to usun wszystkie dodane pomiary do tego tag'a
			if (i > 0) {
				RANGING_MeasureDeleteLast(meas_cnt);
				return false;
			}
		}
		++meas_cnt;
		i += cnt;
		cnt = MIN(maxAncInMeasure, adr_cnt - i);
	}
	// sprawdz czy wystarczy czasu na wykonanie wszystkich pomiarow
	// gdy nie wystarczy, to zwieksz okres pomiarow i wyswietl ostrzezenie
	int meas_count = RANGING_MeasureCounter();
	if (meas_count > settings.ranging.rangingPeriodMs / settings.ranging.rangingDelayMs) {
		settings.ranging.rangingPeriodMs = meas_count * settings.ranging.rangingDelayMs;
		LOG_WRN(WRN_RANGING_TOO_SMALL_PERIOD, meas_count, settings.ranging.rangingPeriodMs);
		PRINT_RangingTime();
	}
	return true;
}

bool RANGING_SendNextInit() {
	const measure_init_info_t* meas = RANGING_MeasureNext();
	// you can't send next init when meas point to null
	if (meas == 0) {
		return 0;
	}
	if (meas->tagDid == settings.mac.addr) {
		TOA_SendPoll(meas->ancDid, meas->numberOfAnchors);
	} else {
		TOA_SendInit(meas->tagDid, meas->ancDid, meas->numberOfAnchors);
	}
	return 1;
}

void RANGING_MeasureNewLoop() {
	ranging.measureReadInd = 0;
}

void RANGING_Control() {
	unsigned int currTime = PORT_TickMs();
	// this function is only for sink
	if (settings.mac.role != RTLS_SINK) {
		return;
	}
	// where it's a time to send new measure init
	if (currTime - ranging.lastInitSendTime > settings.ranging.rangingDelayMs) {
		ranging.lastInitSendTime = currTime;
		bool sent = RANGING_SendNextInit();
		unsigned int cycleTime = currTime - ranging.resetTime;
		// when there is no more measure info to send
		// and ranging period elapsed then start new ranging cycle
		if (!sent && cycleTime > settings.ranging.rangingPeriodMs) {
			ranging.resetTime = currTime;
			RANGING_MeasureNewLoop();
			RANGING_SendNextInit();
		}
	}
}
