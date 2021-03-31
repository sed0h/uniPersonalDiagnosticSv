/*
 * xif_safe.h
 *
 *  Created on: May 16, 2020
 *      Author: yusw
 */

#ifndef ASYNC_IO_XIF_SAFE_H_
#define ASYNC_IO_XIF_SAFE_H_

#include <stdint.h>
#include <stdbool.h>
#if defined(WIN32)
#include <Windows.h>
#endif

#define ACCESS_MULTI_INPUT_MASK		0x01
#define ACCESS_MULTI_OUTPUT_MASK	0x02
#define ACCESS_MODE_MASK	0x03

#define ACCESS_FREE_MASK	0x04

/*
 * bit 0 : input mode (0:single, 1: multiply)
 * bit 1 : output mode (0:single, 1: multiply)
 */
enum access_mode_e {
	access_siso,	/*single-in single-out*/
	access_miso,	/*multi-in single-out*/
	access_simo,	/*single-in multi-out*/
	access_mimo,	/*multi-in multi-out*/
};
typedef enum access_mode_e access_mode_t;

struct xif_rwbuf_s;
typedef struct xif_rwbuf_s xif_rwbuf_t;

struct xif_fifo_s;
typedef struct xif_fifo_s xif_fifo_t;

struct xif_fifo_attr_s;
typedef struct xif_fifo_attr_s xif_fifo_attr_t;

/*
 * fifo attribute
 */
struct xif_fifo_attr_s {
	uint16_t capacity;
	uint16_t unit;
};


#ifdef __cplusplus
extern "C" {
#endif

/*
 * read & write buffer (ring)
 */
xif_rwbuf_t* xif_rwbuf_init(uint8_t* buf, uint32_t len);
xif_rwbuf_t* xif_rwbuf_new(uint32_t size);
void xif_rwbuf_free(xif_rwbuf_t* rdbuf);
void xif_rwbuf_reset(xif_rwbuf_t* rdbuf);
void xif_rwbuf_setmode(xif_rwbuf_t* rdbuf, access_mode_t mode);
uint32_t xif_rwbuf_write(xif_rwbuf_t* rdbuf, uint8_t* ub, uint32_t len);
uint32_t xif_rwbuf_read(xif_rwbuf_t* rdbuf, uint8_t* ub, uint32_t len);
uint32_t xif_rwbuf_peek(xif_rwbuf_t* rdbuf, uint8_t* ub, uint32_t len);

/*
 * FIFO (ring)
 */
xif_fifo_t* xif_fifo_init(uint8_t* buf, uint32_t len, xif_fifo_attr_t attr);
xif_fifo_t* xif_fifo_new(xif_fifo_attr_t attr);
void xif_fifo_free(xif_fifo_t* fifo);
void xif_fifo_reset(xif_fifo_t* fifo);
void xif_fifo_setmode(xif_fifo_t* fifo, access_mode_t mode);
bool xif_fifo_push(xif_fifo_t* fifo, uint8_t* datum);
bool xif_fifo_pop(xif_fifo_t* fifo, uint8_t* datum);
bool xif_fifo_is_empty(xif_fifo_t* fifo);
bool xif_fifo_is_full(xif_fifo_t* fifo);


#if defined(WIN32)
bool CAS(volatile uint32_t* val, uint32_t ov, uint32_t nv);
#else
#define CAS(val, ov, nv)	__sync_bool_compare_and_swap(val, ov, nv)
#endif


#ifdef __cplusplus
}
#endif

#endif /* ASYNC_IO_XIF_SAFE_H_ */
