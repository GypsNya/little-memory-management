#ifndef __MEM_MANAGE_H
#define __MEM_MANAGE_H

#include "stm32f4xx_hal.h"
#include "page_conf.h"

#define MEM_PAGE_DIV_BASE		MEMORY_BASE
#define MEM_PAGE_DIV_SIZE		MEMORY_SIZE

//page
#define page_map		((PageMapTypeDef*)PAGE_MAP_START)
#define page_info_map	((PageInfoMapTypeDef*)PAGE_INFO_MAP_START)
//
#define PAGE_SIZE				0x1000
#define PAGE_NUM				(THEORY_PAGE_NUM - PAGE_HEAD_SIZE / PAGE_SIZE)
#define THEORY_PAGE_NUM			(MEM_PAGE_DIV_SIZE / PAGE_SIZE)

#define MEM_PAGE_INFO_SIZE		32

//PAGE_HEAD
#define PAGE_MAP_SIZE			(((32 - (THEORY_PAGE_NUM % 32)) + THEORY_PAGE_NUM/32)*4)
#define PAGE_INFO_MAP_SIZE		PAGE_MAP_SIZE
#define PAGE_INFO_TAB_SIZE		(THEORY_PAGE_NUM * MEM_PAGE_INFO_SIZE)

#define PAGE_HEAD_SIZE			((PAGE_SIZE - (PAGE_MAP_SIZE + PAGE_INFO_MAP_SIZE + PAGE_INFO_TAB_SIZE)%PAGE_SIZE) + (PAGE_MAP_SIZE + PAGE_INFO_MAP_SIZE + PAGE_INFO_TAB_SIZE))

//PAGE_BASE
#define PAGE_MAP_START			MEM_PAGE_DIV_BASE
#define PAGE_INFO_MAP_START		(MEM_PAGE_DIV_BASE + PAGE_MAP_SIZE)
#define PAGE_INFO_TAB_START		(MEM_PAGE_DIV_BASE + PAGE_MAP_SIZE + PAGE_MAP_SIZE)
#define PAGE_START				((PAGE_SIZE - (PAGE_MAP_SIZE + PAGE_INFO_MAP_SIZE + PAGE_INFO_TAB_SIZE)%PAGE_SIZE) + (PAGE_MAP_SIZE + PAGE_INFO_MAP_SIZE + PAGE_INFO_TAB_SIZE) + MEM_PAGE_DIV_BASE)
//#define PAGE_START				
//#define page_info_size	((uint32_t)0x20)
//#define extend		((MemPageExtendTypeDef*)(MEM_PAGE_DIV_BASE + MEM_PAGE_DIV_SIZE))

typedef unsigned int HANDLE;

typedef struct {
	uint32_t	PageMap[PAGE_MAP_SIZE/4];
} PageMapTypeDef;

typedef struct {
	uint32_t	PageInfoMap[PAGE_INFO_MAP_SIZE/4];
} PageInfoMapTypeDef;

typedef struct {
	uint16_t	handle[512];
} HandleMapTypeDef;

typedef struct {
	uint16_t	front_page_def_pos;
	uint16_t	behind_page_def_pos;
	uint16_t	page_used_num;
	uint16_t	page[13];
}MemPageInfoTypeDef;

typedef enum {
	ALLOC_OK = 0,
	ALLOC_FAILED,
	PAGE_FULL,
	PAGE_INFO_FULL,
	PAGE_FREE_OK,
	PAGE_FREE_FAILED,
	PAGE_FREE_ERROR,
	ERR_SIZE,
} MemAllocStatusTypeDef;

typedef enum {
	READ_OK = 0,
	WRITE_OK,
	OPERATION_OK,
	READ_ERR,
	WRITE_ERR,
	OPERATION_ERR,
	READ_FLOW,
	WRITE_FLOW,
}MemOperationStatusTypeDef;

void mem_page_init(void);

MemAllocStatusTypeDef mem_page_alloc(uint16_t* handle, uint32_t size);
MemAllocStatusTypeDef mem_page_free(uint16_t handle);

uint32_t size_of(uint16_t handle);
MemOperationStatusTypeDef mov_out(uint16_t handle, char* src_addr, char* des_addr, uint32_t size);
MemOperationStatusTypeDef mov_in(uint16_t handle, char* src_addr, char* des_addr, uint32_t size);

#endif
