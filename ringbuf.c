#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define RINGBUF_CAPACITY 8u

#define RINGBUF_OK          0
#define RINGBUF_ERR_FULL   (-1)
#define RINGBUF_ERR_EMPTY  (-2)

typedef struct {
    uint8_t  data[RINGBUF_CAPACITY];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} ring_buffer_t;

static uint16_t ringbuf_advance_index(uint16_t index)
{
    return (uint16_t)((index + 1) & (RINGBUF_CAPACITY - 1));
}

void ringbuf_init(ring_buffer_t *rb)
{
    memset(rb->data, 0, sizeof(rb->data));
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

int ringbuf_is_full(const ring_buffer_t *rb)
{
    return rb->count == RINGBUF_CAPACITY;
}

int ringbuf_is_empty(const ring_buffer_t *rb)
{
    return rb->count == 0;
}

uint16_t ringbuf_count(const ring_buffer_t *rb)
{
    return rb->count;
}

int ringbuf_write(ring_buffer_t *rb, uint8_t byte)
{
    if (ringbuf_is_full(rb)) {
        return RINGBUF_ERR_FULL;
    }

    rb->data[rb->head] = byte;
    rb->head = ringbuf_advance_index(rb->head);
    rb->count++;

    return RINGBUF_OK;
}

int ringbuf_read(ring_buffer_t *rb, uint8_t *out_byte)
{
    if (ringbuf_is_empty(rb)) {
        return RINGBUF_ERR_EMPTY;
    }

    *out_byte = rb->data[rb->tail];
    rb->tail = ringbuf_advance_index(rb->tail);
    rb->count--;

    return RINGBUF_OK;
}

int main(void)
{
    ring_buffer_t rb;
    ringbuf_init(&rb);

    const uint8_t write_bytes[RINGBUF_CAPACITY] = {
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48
    };

    for (int i = 0; i < RINGBUF_CAPACITY; i++) {
        int result = ringbuf_write(&rb, write_bytes[i]);
        if (result == RINGBUF_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   write_bytes[i],
                   ringbuf_count(&rb),
                   ringbuf_is_full(&rb) ? " FULL" : "");
        } else {
            printf("[WRITE] 0x%02X -> FAIL (buffer full)\n", write_bytes[i]);
        }
    }

    {
        uint8_t byte = 0x99;
        int result = ringbuf_write(&rb, byte);
        if (result == RINGBUF_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%u)\n", byte, ringbuf_count(&rb));
        } else {
            printf("[WRITE] 0x%02X -> FAIL (buffer full)\n", byte);
        }
    }

    for (int i = 0; i < 3; i++) {
        uint8_t byte;
        int result = ringbuf_read(&rb, &byte);
        if (result == RINGBUF_OK) {
            printf("[READ] -> 0x%02X (count=%u)\n", byte, ringbuf_count(&rb));
        } else {
            printf("[READ] (empty) -> FAIL (buffer empty)\n");
        }
    }

    const uint8_t new_bytes[3] = { 0x49, 0x4A, 0x4B };
    for (int i = 0; i < 3; i++) {
        int result = ringbuf_write(&rb, new_bytes[i]);
        if (result == RINGBUF_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   new_bytes[i],
                   ringbuf_count(&rb),
                   ringbuf_is_full(&rb) ? " FULL" : "");
        } else {
            printf("[WRITE] 0x%02X -> FAIL (buffer full)\n", new_bytes[i]);
        }
    }

    for (int i = 0; i < RINGBUF_CAPACITY; i++) {
        uint8_t byte;
        int result = ringbuf_read(&rb, &byte);
        if (result == RINGBUF_OK) {
            printf("[READ] -> 0x%02X (count=%u)\n", byte, ringbuf_count(&rb));
        } else {
            printf("[READ] (empty) -> FAIL (buffer empty)\n");
        }
    }

    {
        uint8_t byte;
        int result = ringbuf_read(&rb, &byte);
        if (result == RINGBUF_OK) {
            printf("[READ] -> 0x%02X (count=%u)\n", byte, ringbuf_count(&rb));
        } else {
            printf("[READ] (empty) -> FAIL (buffer empty)\n");
        }
    }

    return EXIT_SUCCESS;
}
