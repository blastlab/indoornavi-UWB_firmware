#include "mac.h"
#include "settings.h"

// global mac instance
mac_instance_t mac;

mac_buf_t *mac_buf_get_oldest_to_tx();
void _mac_buffer_reset(mac_buf_t *buf);
int _mac_get_frame_len(mac_buf_t *buf);
int _mac_get_slot_time();

// get time from start of super frame in mac_get_port_sync_time units
uint64_t mac_to_slots_time(uint64_t sync_time_raw)
{
    int super_time = (sync_time_raw - mac.sync_offset) % settings.mac.slots_sum_time;
    return super_time;
}

// return frame len in bytes
int _mac_get_frame_len(mac_buf_t *buf)
{
    MAC_ASSERT(buf != 0);
    MAC_ASSERT((unsigned int)buf < (unsigned int)buf->dPtr);
    int len = (int)(buf->dPtr - (unsigned char *)buf);
    MAC_ASSERT(len > 0);
    return len;
}

void mac_your_slot_isr()
{
    uint64_t time = transceiver_get_sync_time();
    mac_transmitted_isr(time);
}

void mac_transmitted_frame_isr(uint64_t tx_timestamp)
{
    mac_try_transmited_frame_in_slot(tx_timestamp);
}

void mac_try_transmited_frame_in_slot(uint64_t time)
{
    // calc time from begining of yours slot
    uint64_t slot_time = mac_to_slots_time(time);
    slot_time -= settings.mac.slot_number * settings.mac.slot_time;
    slot_time -= settings.mac.slot_guard_time;
    if (slot_time > settings.mac.slot_time)
    {
        return;
    }

    mac_buf_t *buf = mac_buf_get_oldest_to_tx();
    if (buf == 0)
    {
        return;
    }
    int len = _mac_get_frame_len(buf);
    unsigned int tx_est_time = transceiver_estimate_tx_time_us(len);
    if (slot_time + tx_est_time > settings.mac.slot_time)
    {
        return;
    }

    // when you have enouth time to send next message, then do it
    int ret = transceiver_send(buf->buf, len);
    if (ret == 0)
    {
        buf->state = buf->state == WAIT_FOR_TX_ACK ? WAIT_FOR_ACK : FREE;
    }
    else
    {
        ++buf->retransmit_fail_cnt;
        if (buf->retransmit_fail_cnt > settings.mac.max_frame_fail_cnt)
        {
            buf->state = FREE;
        }
    }
}

void mac_ack_frame_isr(mac_buf_t *buffer)
{
    char seq_num = buffer->frame.seq_num;
    for (int i = 0; i < MAC_BUF_CNT; ++i)
    {
        if (mac.buf[i].state == WAIT_FOR_ACK)
        {
            mac.buf[i].state = FREE;
        }
    }
}

void _mac_buffer_reset(mac_buf_t *buf)
{
    buf->dPtr = buf->buf;
    buf->state == BUSY;
    buf->retransmit_fail_cnt = 0;
    buf->last_update_time = mac_port_get_time();
}

mac_buf_t *mac_buf_get_oldest_to_tx()
{
    int oldest_index = MAC_BUF_CNT;
    int current_time = mac_port_get_sync_time();
    int oldest_time = current_time - mac.buf[0].last_update_time;

    for (int i = 0; i < MAC_BUF_CNT; ++i)
    {
        mac_buf_state state = mac.buf[i].state;
        if (state == WAIT_FOR_TX || state == WAIT_FOR_TX_ACK)
        {
            int buff_age = current_time - mac.buf[i].last_update_time;
            if (buff_age > oldest_time)
            {
                oldest_index = i;
                oldest_time = buff_age;
            }
        }
    }

    return oldest_index < MAC_BUF_CNT ? &mac.buf[oldest_index] : 0;
}

// reserve buffer
mac_buf_t *mac_buffer()
{
    int oldest_index = 0;
    int current_time = mac_port_get_sync_time();
    int oldest_time = current_time - mac.buf[0].last_update_time;
    mac_buf_t *buf = &mac.buf[mac.buf_get_ind];

    // next buffer buffer is FREE so return them
    if (buf->state == FREE)
    {
        mac.buf_get_ind = (mac.buf_get_ind + 1) % MAC_BUF_CNT;
        _mac_buffer_reset(buf);
        return buf;
    }

    // else search for empty buffer
    for (int i = 0; i < MAC_BUF_CNT; ++i)
    {
        int buff_age = current_time - mac.buf[i].last_update_time;
        if (mac.buf[i].state == FREE)
        {
            buf = &mac.buf[i];
            _mac_buffer_reset(buf);
            return buf;
        }
        else if (buff_age > oldest_time)
        {
            oldest_index = i;
            oldest_time = buff_age;
        }
    }

    // // else get the oldest one
    // MAC_LOG_ERR("mac_buffer() overload");
    // buf = mac.buf[oldest_index];
    // _mac_buffer_reset(buf);
    // return buf;

    // else return null
    return 0;
}

void mac_fill_frame_to(mac_buf_t *buf, dev_addr_t target)
{
    MAC_ASSERT(buf != 0);
    buf->frame.src = mac.addr;
    buf->frame.dst = targer;
    buf->dPtr = &buf->frame->data[0];
}

mac_buf_t *mac_buffer_prepare(dev_addr_t target, bool can_append)
{
    mac_buf_t *buf;
    // search buffer to this target
    if (can_append)
    {
        for (int i = 0; i < MAC_BUF_CNT; ++i)
        {
            buf = &mac.buf[i];
            if (buf->state == WAIT_FOR_TX || buf->state == WAIT_FOR_TX_ACK)
            {
                // do not use mac_fill_frame_to, becouse the buff is already partially filled
                buf->state = BUSY;
                return buf;
            }
        }
    }

    // else get free buffer
    buf = mac_buffer();
    if (buf != 0)
    {
        mac_fill_frame_to(buf, target);
    }
    return buf;
}

// free buffer
void mac_free(mac_buf_t *buff);
{
    MAC_ASSERT((unsigned int)buf > (unsigned int)mac.buf);
    int i = ((char *)buf - (char *)mac.buf) / sizeof(*buf);
    MAC_ASSERT(i < MAC_BUF_CNT);
    mac.buf[i].state = FREE;
}

// add frame to the transmit queue
void mac_send(mac_buf_t *buf, bool ack_require)
{
    MAC_ASSERT(buf != 0);
    if (ack_require)
    {
        buf.state = WAIT_FOR_TX_ACK;
    }
    else
    {
        buf.state = WAIT_FOR_TX;
    }
}

unsigned char mac_read8(mac_buf_t *frame)
{
    MAC_ASSERT(frame != 0);
    MAC_ASSERT((int)frame->dPtr > (int)&frame);
    return *frame->dPtr++;
}

void mac_write8(mac_buf_t *frame, unsigned char value)
{
    MAC_ASSERT(frame != 0);
    MAC_ASSERT((int)frame->dPtr > (int)&frame);
    *frame->dPtr = value;
    ++frame->dPtr;
}

void mac_read(mac_buf_t *frame, void *destination, unsigned int len)
{
    MAC_ASSERT(frame != 0);
    MAC_ASSERT((int)frame->dPtr > (int)&frame);
    MAC_ASSERT(destination != 0);
    MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
    while (len > 0)
    {
        *(unsigned char *)destination = *frame->dPtr;
        ++(unsigned char *)destination;
        ++frame->dPtr;
    }
}

void mac_write(mac_buf_t *frame, const void *src, unsigned int len)
{
    MAC_ASSERT(frame != 0);
    MAC_ASSERT((int)frame->dPtr > (int)&frame);
    MAC_ASSERT(src != 0);
    MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
    while (len > 0)
    {
        *frame->dPtr = *(unsigned char *)frame->dPtr;
        ++frame->dPtr;
        ++(unsigned char *)src;
    }
}