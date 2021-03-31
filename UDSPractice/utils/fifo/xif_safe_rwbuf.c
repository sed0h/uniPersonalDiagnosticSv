/*
 * xif_safe_rwbuf.c
 *
 *  Created on: May 17, 2020
 *      Author: yusw
 */
#include "xif_safe.h"
#include <string.h>

/*
 * order descriptor
 * 1. reader.head : 	readable, before this can be write
 * 2. reader.cursor :	in reading
 * 3. write.tail : 		write finished, before this can be read
 * 4. write.cursor : 	in writing
 */
struct xif_datum_s {
	uint8_t data;
};
typedef struct xif_datum_s xif_datum_t;


struct xif_rwbuf_ops_s;
typedef struct xif_rwbuf_ops_s xif_rwbuf_ops_t;

struct xif_rwbuf_s {
	xif_rwbuf_ops_t* policy;
	struct _writer {
		uint32_t watermark;
		volatile uint32_t write_cursor;
		volatile uint32_t tail;
	} writer;

	struct _reader {
		volatile uint32_t read_cursor;
		volatile uint32_t head;
	} reader;

	uint32_t capacity;
	uint16_t access;	/*bit0,1: is access mode; bit2: is free bit*/
	uint8_t* data;
};

struct xif_rwbuf_ops_s {
	bool (*acquiry_write_cursor)(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t* write_cursor);
	void (*release_write_cursor)(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t write_cursor);
	bool (*acquiry_read_cursor)(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t* read_cursor);
	void (*release_read_cursor)(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t read_cursor);
};



/*
 * multiply output
 */
static bool xif_mo_acquiry_read_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t* cursor){
	uint32_t len;
	do{
		(*cursor) = rwbuf->reader.read_cursor;
		len = rwbuf->writer.tail - (*cursor);
		if( size > len ){
			/* not enough readable data */
			return false;
		}
	}while(!CAS(&rwbuf->reader.read_cursor, (*cursor), (*cursor) + size));
	return true;
}
static void xif_mo_release_read_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t cursor){
	while(!CAS(&rwbuf->reader.head, cursor, cursor + size));
}

/*
 * single output
 */
static bool xif_so_acquiry_read_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t* cursor){
	/*
	 * do not move the head, because the reading is not finished
	 */
	(*cursor) = rwbuf->reader.head;
	uint32_t len = rwbuf->writer.tail - (*cursor);
	if( size > len ){
		/* not enough readable data */
		return false;
	}
	return true;
}
static void xif_so_release_read_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t cursor){
	/*
	 * reading finished, the data is free-used
	 */
	rwbuf->reader.head += size;
}

/*
 * multiply input
 */
static bool xif_mi_acquire_write_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t* cursor){
	/*
	 * do not move the tail, because the writing is not finished
	 */
	uint32_t remains;
	do{
		(*cursor) = rwbuf->writer.write_cursor;
		remains = rwbuf->capacity - ((*cursor) - rwbuf->reader.head);
		if( size > remains ){
			/* not enough buffer */
			return false;
		}
	}while(!CAS(&rwbuf->writer.write_cursor, (*cursor), (*cursor) + size));
	return true;
}

static void xif_mi_release_write_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t cursor){
	while(!CAS(&rwbuf->writer.tail, cursor, cursor + size));
}

static xif_rwbuf_ops_t mimo = {
		.acquiry_write_cursor = xif_mi_acquire_write_cursor,
		.release_write_cursor = xif_mi_release_write_cursor,
		.acquiry_read_cursor = xif_mo_acquiry_read_cursor,
		.release_read_cursor = xif_mo_release_read_cursor,
};
static xif_rwbuf_ops_t miso = {
		.acquiry_write_cursor = xif_mi_acquire_write_cursor,
		.release_write_cursor = xif_mi_release_write_cursor,
		.acquiry_read_cursor = xif_so_acquiry_read_cursor,
		.release_read_cursor = xif_so_release_read_cursor,
};

/*
 * single input
 */
static bool xif_si_acquiry_wrtie_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t* cursor){
	/*
	 * do not move the tail, because the writing is not finished
	 */
	uint32_t remains;
	(*cursor) = rwbuf->writer.tail;
	remains = rwbuf->capacity - ( (*cursor) - rwbuf->reader.head );
	if( size > remains ){
		/* not enough buffer */
		return false;
	}
	return true;
}
static void xif_si_acquiry_write_cursor(xif_rwbuf_t* rwbuf, uint32_t size, uint32_t cursor){
	/*
	 * writing finished, the data is available
	 */
	rwbuf->writer.tail += size;
}

static xif_rwbuf_ops_t simo = {
		.acquiry_write_cursor = xif_si_acquiry_wrtie_cursor,
		.release_write_cursor = xif_si_acquiry_write_cursor,
		.acquiry_read_cursor = xif_mo_acquiry_read_cursor,
		.release_read_cursor = xif_mo_release_read_cursor,
};
static xif_rwbuf_ops_t siso = {
		.acquiry_write_cursor = xif_si_acquiry_wrtie_cursor,
		.release_write_cursor = xif_si_acquiry_write_cursor,
		.acquiry_read_cursor = xif_so_acquiry_read_cursor,
		.release_read_cursor = xif_so_release_read_cursor,
};

/*
 *
 */
static uint8_t* xif_rwbuf_get_datum(xif_rwbuf_t* rwbuf, uint32_t pos){
	/*
	 * if capacity if pow of 2, then we use bit-operation
	 */
#if defined(FIFO_POW_ALIGN)
	pos = pos & (rwbuf->capacity - 1);
#else
	pos = pos % rwbuf->capacity;
#endif
	uint8_t* ptr = &(rwbuf->data[pos]);
	return ptr;
}

xif_rwbuf_t* xif_rwbuf_init(uint8_t* buf, uint32_t len){
	xif_rwbuf_t* rwbuf = (xif_rwbuf_t*)buf;
	rwbuf->capacity = len - sizeof(xif_rwbuf_t);
	rwbuf->data = buf + sizeof(xif_rwbuf_t);
	rwbuf->policy = 0;
	rwbuf->access = access_mimo;
	xif_rwbuf_reset(rwbuf);
	return rwbuf;
}
xif_rwbuf_t* xif_rwbuf_new(uint32_t size){
	xif_rwbuf_t* rwbuf = 0;
	/*rwbuf->capacity = size;
	rwbuf->data = 0;
	rwbuf->policy = 0;
	rwbuf->access = access_mimo;
	rwbuf->access |= ACCESS_FREE_MASK;
	xif_rwbuf_reset(rwbuf);*/
	return rwbuf;
}
void xif_rwbuf_free(xif_rwbuf_t* rwbuf){

}
void xif_rwbuf_reset(xif_rwbuf_t* rwbuf){
	rwbuf->reader.head = 0;
	rwbuf->reader.read_cursor = 0;
	rwbuf->writer.tail = 0;
	rwbuf->writer.write_cursor = 0;
	rwbuf->writer.watermark = 0;
}
void xif_rwbuf_setmode(xif_rwbuf_t* rwbuf, access_mode_t mode){
	switch(mode){
	case access_miso:
		rwbuf->policy = &miso;
		break;
	case access_mimo:
		rwbuf->policy = &mimo;
		break;
	case access_siso:
		rwbuf->policy = &siso;
		break;
	case access_simo:
		rwbuf->policy = &simo;
		break;
	}
	rwbuf->access &= ~ACCESS_MODE_MASK;
	rwbuf->access |= mode;
}
uint32_t xif_rwbuf_write(xif_rwbuf_t* rwbuf, uint8_t* ub, uint32_t len){
	uint32_t cursor;

	/* acquire write cursor */
	bool ret = rwbuf->policy->acquiry_write_cursor(rwbuf, len, &cursor);
	if(ret == false)
		return 0;

	uint8_t* ptr = xif_rwbuf_get_datum(rwbuf, cursor);
	memcpy((void*)ptr, (void*)ub, (size_t) len);

	/* finish write operation */
	rwbuf->policy->release_write_cursor(rwbuf, len, cursor);

	return len;
}

uint32_t xif_rwbuf_read(xif_rwbuf_t* rwbuf, uint8_t* ub, uint32_t len){
	uint32_t cursor;

	/* acquire read cursor */
	bool ret = rwbuf->policy->acquiry_read_cursor(rwbuf, len, &cursor);
	if(ret == false)
		return 0;

	uint8_t* ptr = xif_rwbuf_get_datum(rwbuf, cursor);
	memcpy(ub, ptr, len);

	/* finish read operation */
	rwbuf->policy->release_read_cursor(rwbuf, len, cursor);
	return len;

}
uint32_t xif_rwbuf_peek(xif_rwbuf_t* rwbuf, uint8_t* ub, uint32_t len){
	return 0;
}

