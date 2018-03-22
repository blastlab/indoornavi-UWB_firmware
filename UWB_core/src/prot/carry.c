#include "prot/carry.h"

carry_instance_t carry;

void carry_init()
{
}

// return pointer to target or zero
carry_target_t *_carry_find_target(dev_addr_t target)
{
    for (int i = 0; i < CARRY_MAX_TARGETS; ++i)
    {
        if (carry.target[i].addr == target)
        {
            // target found
            return &carry.target[i];
        }
    }

    // when you didn't find
    return 0;
}

// return pointer to trace or zero
carry_trace_t *_carry_find_trace(carry_target_t *ptarget)
{
    if (ptarget == 0)
    {
        return 0;
    }

    for (int i = 0; i < CARRY_MAX_TRACE; ++i)
    {
        if (ptarget->trace[i].fail_cnt < settings.carry.max_fail_counter)
        {
            mac_buff_time_t delta = mac_port_buff_time() - ptarget->trace[i].last_update_time;
            if (delta < settings.carry.max_inactive_time)
            {
                return &ptarget->trace[i];
            }
        }
    }

    // when you didn't found valid trace
    return 0;
}

int carry_write_trace(dev_addr_t *buf, dev_addr_t target)
{
    CARRY_ASSERT(buf != 0);
    carry_target_t *ptarget = _carry_find_target(target);
    carry_trace_t *ptrace = _carry_find_trace(ptarget);

    if (ptrace != 0)
    {
        int len = ptrace->path_len * sizeof(dev_addr_t);
        memcpy(buf, ptrace->path, len);
        return len;
    }

    return 0;
}

FC_CARRY_s *_carry_prot_fill(mac_buf_t *buf)
{
    CARRY_ASSERT(buf != 0);
    FC_CARRY_s *prot = (FC_CARRY_s *)buf;
    prot->FC = FC_CARRY;
    prot->len = CARRY_HEAD_MIN_LEN;
    prot->src_addr = settings.mac.addr;
    prot->flag_hops = CARRY_FLAG_TARGET_DEV;
    return prot;
}

mac_buf_t *carry_prepare_buf_to(dev_addr_t target)
{
    mac_buf_t *buf = mac_buffer_prepare(target, true);

    if (buf->dPtr == &buf->frame.data[0])
    {
        FC_CARRY_s *ppacket = _carry_prot_fill(buf);
        int len = carry_write_trace(&ppacket->hops[0], target);
        // when
        if (len > 0)
        {
            ppacket->len += len;
            buf->dPtr += len;
            return buf;
        }
    }
}