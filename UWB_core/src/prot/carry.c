#include "carry.h"

carry_instance_t carry;

void CARRY_Init() {}

// return pointer to target or zero
carry_target_t *_CARRY_FindTarget(dev_addr_t target) {
  for (int i = 0; i < CARRY_MAX_TARGETS; ++i) {
    if (carry.target[i].addr == target) {
      // target found
      return &carry.target[i];
    }
  }

  // when you didn't find
  return 0;
}

// return pointer to trace or zero
carry_trace_t *_CARRY_FindTrace(carry_target_t *ptarget) {
  if (ptarget == 0) {
    return 0;
  }

  for (int i = 0; i < CARRY_MAX_TRACE; ++i) {
    if (ptarget->trace[i].fail_cnt < settings.carry.max_fail_counter) {
      mac_buff_time_t delta = mac_port_buff_time();
      delta -= ptarget->trace[i].last_update_time;
      if (delta < settings.carry.max_inactive_time) {
        return &ptarget->trace[i];
      }
    }
  }

  // when you didn't found valid trace
  return 0;
}

int CARRY_WriteTrace(dev_addr_t *buf, dev_addr_t target) {
  CARRY_ASSERT(buf != 0);
  carry_target_t *ptarget = _CARRY_FindTarget(target);
  carry_trace_t *ptrace = _CARRY_FindTrace(ptarget);

  if (ptrace != 0) {
    int len = ptrace->path_len * sizeof(dev_addr_t);
    memcpy(buf, ptrace->path, len);
    return len;
  }

  return 0;
}

FC_CARRY_s *_CARRY_ProtFill(mac_buf_t *buf) {
  CARRY_ASSERT(buf != 0);
  FC_CARRY_s *prot = (FC_CARRY_s *)buf->dPtr;
  prot->FC = FC_CARRY;
  prot->len = CARRY_HEAD_MIN_LEN;
  prot->src_addr = settings.mac.addr;
  prot->flag_hops = CARRY_FLAG_TARGET_DEV;
  buf->dPtr += prot->len;
  return prot;
}

mac_buf_t *CARRY_PrepareBufTo(dev_addr_t target) {
  mac_buf_t *buf = MAC_BufferPrepare(target, true);

  if (buf != 0) {
    FC_CARRY_s *ppacket = _CARRY_ProtFill(buf);
    int len = CARRY_WriteTrace(&ppacket->hops[0], target);
    // do not overwrite trace to this target
    if (len > 0) {
      ppacket->len += len;
      buf->dPtr += len;
    }
  }
  return buf;
}
