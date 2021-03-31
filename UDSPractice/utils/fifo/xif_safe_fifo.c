/*
 * xif_safe_fifo.c
 *
 *  Created on: May 16, 2020
 *      Author: yusw
 */
#include "xif_safe.h"
#include <string.h>

#define TRY_COUNT	5
//#define USE_YIELD
#if defined(USE_YIELD)
void check_yield(int* try_count){
	(*try_count)--;
	if ((*try_count) == 0){
		Sleep(1);
		(*try_count) = 3;
	}
}
#else
#define check_yield(try_count) (void)try_count;
#endif
struct xif_datum_s {
	uint8_t data;
};
typedef struct xif_datum_s xif_datum_t;


struct xif_fifo_ops_s;
typedef struct xif_fifo_ops_s xif_fifo_ops_t;

struct xif_fifo_s {
	xif_fifo_ops_t* policy;
	volatile uint32_t head;
	volatile uint32_t tail;
	volatile uint32_t write_cursor;
	volatile uint32_t read_cursor;

	uint32_t capacity;
	uint16_t unit;
	uint16_t access;	/*bit0,1: is access mode; bit2: is free bit*/
	uint8_t* data;
};

struct xif_fifo_ops_s {
	bool (*acquiry_write_cursor)(xif_fifo_t* fifo, uint32_t* write_cursor);
	void (*release_write_cursor)(xif_fifo_t* fifo, uint32_t write_cursor);
	bool (*acquiry_read_cursor)(xif_fifo_t* fifo, uint32_t* write_cursor);
	void (*release_read_cursor)(xif_fifo_t* fifo, uint32_t read_cursor);
};


/*
 * multiply output
 */
static bool xif_mo_acquiry_read_cursor(xif_fifo_t* fifo, uint32_t* cursor){
	bool ret;
	int try_count = TRY_COUNT;
	do{
		check_yield(&try_count);
		(*cursor) = fifo->read_cursor;
		if (fifo->read_cursor == fifo->tail)	/* is it empty? */
			return false;
		ret = CAS(&fifo->read_cursor, (*cursor), (*cursor) + 1);
	}while(!ret);
	return true;
}
static void xif_mo_release_read_cursor(xif_fifo_t* fifo, uint32_t cursor){
	bool ret;
	int try_count = TRY_COUNT;
	do{
		check_yield(&try_count);
		ret = CAS(&fifo->head, cursor, cursor + 1);
	}while (!ret);
}

/*
 * single output
 */
static bool xif_so_acquiry_read_cursor(xif_fifo_t* fifo, uint32_t* cursor){
	/*
	 * do not move the head, because the reading is not finished
	 */
	*cursor = fifo->head;
	if(fifo->head == fifo->tail)	/* is it empty? */
		return false;
	return true;
}
static void xif_so_release_read_cursor(xif_fifo_t* fifo, uint32_t cursor){
	/*
	 * reading finished, the data is free-used
	 */
	fifo->head++;
}

/*
 * multiply input
 */
static bool xif_mi_acquire_write_cursor(xif_fifo_t* fifo, uint32_t* cursor){
	/*
	 * do not move the tail, because the writing is not finished
	 */
	bool ret;
	int try_count = TRY_COUNT;
	do{
		check_yield(&try_count);
		(*cursor) = fifo->write_cursor;
		if ((fifo->write_cursor - fifo->head) == fifo->capacity){	/* is it full ? */
			return false;
		}
		ret = CAS(&fifo->write_cursor, (*cursor), (*cursor) + 1);
	}while(!ret);
	return true;
}

static void xif_mi_release_write_cursor(xif_fifo_t* fifo, uint32_t cursor){
	bool ret;
	int try_count = TRY_COUNT;
	do{
		check_yield(&try_count);
		ret = CAS(&fifo->tail, cursor, cursor + 1);
	} while (!ret);
}

static xif_fifo_ops_t mimo = {
		.acquiry_write_cursor = xif_mi_acquire_write_cursor,
		.release_write_cursor = xif_mi_release_write_cursor,
		.acquiry_read_cursor = xif_mo_acquiry_read_cursor,
		.release_read_cursor = xif_mo_release_read_cursor,
};
static xif_fifo_ops_t miso = {
		.acquiry_write_cursor = xif_mi_acquire_write_cursor,
		.release_write_cursor = xif_mi_release_write_cursor,
		.acquiry_read_cursor = xif_so_acquiry_read_cursor,
		.release_read_cursor = xif_so_release_read_cursor,
};

/*
 * single input
 */
static bool xif_si_acquiry_wrtie_cursor(xif_fifo_t* fifo, uint32_t* cursor){
	/*
	 * do not move the tail, because the writing is not finished
	 */
	*cursor = fifo->tail;
	if(xif_fifo_is_full(fifo) )
		return false;
	return true;
}
static void xif_si_acquiry_write_cursor(xif_fifo_t* fifo, uint32_t cursor){
	/*
	 * writing finished, the data is available
	 */
	fifo->tail = fifo->tail + 1;
}

static xif_fifo_ops_t simo = {
		.acquiry_write_cursor = xif_si_acquiry_wrtie_cursor,
		.release_write_cursor = xif_si_acquiry_write_cursor,
		.acquiry_read_cursor = xif_mo_acquiry_read_cursor,
		.release_read_cursor = xif_mo_release_read_cursor,
};
static xif_fifo_ops_t siso = {
		.acquiry_write_cursor = xif_si_acquiry_wrtie_cursor,
		.release_write_cursor = xif_si_acquiry_write_cursor,
		.acquiry_read_cursor = xif_so_acquiry_read_cursor,
		.release_read_cursor = xif_so_release_read_cursor,
};

/*
 * fifo implementation
 */
static uint8_t* xif_fifo_get_datum(xif_fifo_t* fifo, uint32_t pos){
	/*
	 * if capacity if pow of 2, then we use bit-operation
	 */
#if defined(FIFO_POW_ALIGN)
	pos = pos & (fifo->capacity - 1);
#else
	pos = pos % fifo->capacity;
#endif
	uint8_t* ptr = &(fifo->data[pos*fifo->unit]);
	return ptr;
}
xif_fifo_t* xif_fifo_init(uint8_t* buf, uint32_t len, xif_fifo_attr_t attr){
	//printf("sizeof(xif_fifo_t) = %ld\n", sizeof(xif_fifo_t));
	if (len > attr.capacity*attr.unit + sizeof(xif_fifo_t)){
		xif_fifo_t* fifo = (xif_fifo_t*)buf;
		fifo->capacity = attr.capacity;
		fifo->unit = attr.unit;
		fifo->data = buf + sizeof(xif_fifo_t);
		fifo->policy = &mimo;
		fifo->access = access_mimo;
		xif_fifo_reset(fifo);
		return fifo;
	}
	return NULL;
}
xif_fifo_t* xif_fifo_new(xif_fifo_attr_t attr){
	xif_fifo_t* fifo = 0;
	/*fifo->capacity = attr.capacity;
	fifo->unit = attr.unit;
	fifo->data = 0;
	fifo->policy = &mimo;
	fifo->access = access_mimo;
	fifo->access |= ACCESS_FREE_MASK;
	xif_fifo_reset(fifo);*/
	return fifo;
}

void xif_fifo_free(xif_fifo_t* fifo){
	if(fifo->access & ACCESS_FREE_MASK){
		/* free memory */
		(void)fifo->data;
		(void)fifo;
	}else{
		/* not created from xif_fifo_new */
	}

}
void xif_fifo_reset(xif_fifo_t* fifo){
	fifo->head = 0;
	fifo->tail = 0;
	fifo->write_cursor = 0;
	fifo->read_cursor = 0;
}

void xif_fifo_setmode(xif_fifo_t* fifo, access_mode_t mode){
	switch(mode){
	case access_miso:
		fifo->policy = &miso;
		break;
	case access_mimo:
		fifo->policy = &mimo;
		break;
	case access_siso:
		fifo->policy = &siso;
		break;
	case access_simo:
		fifo->policy = &simo;
		break;
	}
	fifo->access &= ~ACCESS_MODE_MASK;
	fifo->access |= mode;
}

bool xif_fifo_push(xif_fifo_t* fifo, uint8_t* datum){
	uint32_t cursor;

	/* acquire write cursor */
	bool ret = fifo->policy->acquiry_write_cursor(fifo, &cursor);
	if(ret == false)
		return false;

	uint8_t* ptr = xif_fifo_get_datum(fifo, cursor);
	memcpy((void*)ptr, (void*)datum, (size_t) fifo->unit);

	/* finish write operation */
	fifo->policy->release_write_cursor(fifo, cursor);

	return true;
}

bool xif_fifo_pop(xif_fifo_t* fifo, uint8_t* datum){
	uint32_t cursor;

	/* acquire read cursor */
	bool ret = fifo->policy->acquiry_read_cursor(fifo, &cursor);
	if(ret == false)
		return false;

	uint8_t* ptr = xif_fifo_get_datum(fifo, cursor);
	memcpy(datum, ptr, fifo->unit);

	/* finish read operation */
	fifo->policy->release_read_cursor(fifo, cursor);
	return true;
}

bool xif_fifo_is_empty(xif_fifo_t* fifo){
	if(fifo->access & ACCESS_MULTI_OUTPUT_MASK){
		return fifo->read_cursor == fifo->tail;
	}
	return fifo->head == fifo->tail;
}

bool xif_fifo_is_full(xif_fifo_t* fifo){
	if(fifo->access & ACCESS_MULTI_INPUT_MASK){
		return (fifo->write_cursor - fifo->head) == fifo->capacity;
	}
	return (fifo->tail - fifo->head) == fifo->capacity;
}
