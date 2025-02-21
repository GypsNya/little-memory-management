#include "mem_manage.h"

void mem_page_init(void) {
	for(int i = 0; i < PAGE_MAP_SIZE/4; i++) {
		page_map->PageMap[i] = 0;
		page_info_map->PageInfoMap[i] = 0;
	}
}

short find_free_page_info(void) {
	uint32_t current_page_sector = 0;
	for(int page_num = 0; page_num < PAGE_NUM; page_num++){
		if (((page_info_map->PageInfoMap[current_page_sector] << (page_num % 32)) & 0x80000000) == 0) {
			page_info_map->PageInfoMap[current_page_sector] |= 0x80000000>>(page_num % 32);
			return page_num;
		}
		if (page_num % 32 == 31) {
			current_page_sector += 1;
		}
	}
	return -1;
}

MemAllocStatusTypeDef mem_page_alloc(uint16_t* handle, uint32_t size) {
	uint32_t current_page_sector = 0;
	uint32_t need_page_num = size%0x1000? size/0x1000+1 : size/0x1000;
	MemPageInfoTypeDef* mem_page_info_struct= 0;
	uint16_t used_page = 0;
	short page_info_find_result;
	
	if(size <= 0){
		return ERR_SIZE;
	}
	
	*handle = find_free_page_info();
	if ((short)*handle == -1) {
		return PAGE_INFO_FULL;
	}
	
	current_page_sector = 0;
	mem_page_info_struct = (MemPageInfoTypeDef*)(*handle*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	for(int page_num = 0; page_num < PAGE_NUM; page_num++){
		if (((page_map->PageMap[current_page_sector] << (page_num % 32)) & 0x80000000) == 0) {
			page_map->PageMap[current_page_sector] |= 0x80000000>>(page_num % 32);
			mem_page_info_struct->page[used_page] = (uint16_t)page_num;
			used_page+=1;
			need_page_num--;
		}
		if (need_page_num == 0) {
			mem_page_info_struct->page_used_num = used_page;
			mem_page_info_struct->behind_page_def_pos = *handle;
			((MemPageInfoTypeDef*)(*handle*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START)) -> front_page_def_pos = (uint16_t)(((uint32_t)mem_page_info_struct - PAGE_INFO_TAB_START)/MEM_PAGE_INFO_SIZE);
			return ALLOC_OK;
		}
		if (page_num % 32 == 31) {
			current_page_sector += 1;
		}
		if (used_page == 13) {
			mem_page_info_struct->page_used_num = used_page;
			
			page_info_find_result = find_free_page_info();
			if (page_info_find_result == -1) {
				return PAGE_INFO_FULL;
			}
			mem_page_info_struct->behind_page_def_pos = page_info_find_result;
			((MemPageInfoTypeDef*)(page_info_find_result * MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START))->front_page_def_pos = (uint16_t)(((uint32_t)mem_page_info_struct - PAGE_INFO_TAB_START)/MEM_PAGE_INFO_SIZE);
			mem_page_info_struct = (MemPageInfoTypeDef*)(page_info_find_result * MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
			used_page = 0;
		}
	}
	return PAGE_FULL;
}

MemAllocStatusTypeDef mem_page_free(uint16_t handle) {
	//uint32_t current_page_info_table = *handle;
	//uint32_t current_page;
	MemPageInfoTypeDef* mem_page_info_struct= (MemPageInfoTypeDef*)(handle*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	uint32_t page_offset;
	uint32_t page_info_tab_offset;
	for (int n = 0; n<PAGE_NUM; n++) {
		for(uint16_t i = 0; i<mem_page_info_struct->page_used_num; i++) {
			page_offset = (uint32_t)(mem_page_info_struct->page[i]);
			page_map->PageMap[page_offset/32] &= (~(0x80000000>>(page_offset%32)));
		}
		page_info_tab_offset = ((uint32_t)mem_page_info_struct - PAGE_INFO_TAB_START)/MEM_PAGE_INFO_SIZE;
		page_info_map->PageInfoMap[page_info_tab_offset/32] &= (~(0x80000000>>((uint32_t)page_info_tab_offset%32)));
		if (mem_page_info_struct->behind_page_def_pos == handle){
			return PAGE_FREE_OK;
		}
		mem_page_info_struct = (MemPageInfoTypeDef*)(mem_page_info_struct->behind_page_def_pos*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	}
	return PAGE_FREE_ERROR;
}

uint32_t size_of(uint16_t handle) {
	uint32_t used_num = 0;
	MemPageInfoTypeDef* mem_page_info_struct= (MemPageInfoTypeDef*)(handle*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	for (int n = 0; n<PAGE_NUM; n++) {
		used_num += mem_page_info_struct->page_used_num;
		if (mem_page_info_struct->behind_page_def_pos == handle){
			return used_num*0x1000;
		}
		mem_page_info_struct = (MemPageInfoTypeDef*)(mem_page_info_struct->behind_page_def_pos*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	}
	return -1;
}

MemOperationStatusTypeDef mov_in(uint16_t handle, char* src_addr, char* des_addr, uint32_t size) {
	MemPageInfoTypeDef* mem_page_info_struct = (MemPageInfoTypeDef*)(handle*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	uint32_t page = (uint32_t)des_addr >> 12;
	uint32_t offset;
	uint32_t end_addr = (uint32_t)(des_addr + size);
	uint32_t page_of_handle = 0;
	
	if(size_of(handle) < end_addr) {
		return WRITE_FLOW;
	}
	
	for(uint32_t current_page_info_struct = 0; current_page_info_struct<page/13; current_page_info_struct++) {
		if(mem_page_info_struct->behind_page_def_pos == handle) {
			return WRITE_FLOW;
		} else {
			mem_page_info_struct = (MemPageInfoTypeDef*)(mem_page_info_struct->behind_page_def_pos*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
		}
	}
	
	for(int data_num = 0; data_num < size; data_num++) {
		page = (uint32_t)des_addr >> 12;
		offset = (uint32_t)des_addr & 0xFFF;
		if((uint32_t)des_addr % 0xD000 == 0 && (uint32_t)des_addr != 0){
			if(mem_page_info_struct->behind_page_def_pos == handle) {
				return WRITE_FLOW;
			} else {
				mem_page_info_struct = (MemPageInfoTypeDef*)(mem_page_info_struct->behind_page_def_pos*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
			}
		}
		((char*)((mem_page_info_struct->page[page%13] << 12) + PAGE_START))[offset] = src_addr[data_num];
		des_addr+=1;
	}
	return WRITE_OK;
}

MemOperationStatusTypeDef mov_out(uint16_t handle, char* src_addr, char* des_addr, uint32_t size) {
	MemPageInfoTypeDef* mem_page_info_struct = (MemPageInfoTypeDef*)(handle*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
	uint32_t page = (uint32_t)src_addr >> 12;
	uint32_t offset;
	uint32_t end_addr = (uint32_t)(src_addr + size);
	uint32_t page_of_handle = 0;
	
	if(size_of(handle) < end_addr) {
		return READ_FLOW;
	}

	for(uint32_t current_page_info_struct = 0; current_page_info_struct<page/13; current_page_info_struct++) {
		if(mem_page_info_struct->behind_page_def_pos == handle) {
			return READ_FLOW;
		} else {
				mem_page_info_struct = (MemPageInfoTypeDef*)(mem_page_info_struct->behind_page_def_pos*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
		}
	}
	
	for(int data_num = 0; data_num < size; data_num++) {
		page = (uint32_t)src_addr >> 12;
		offset = (uint32_t)src_addr & 0xFFF;
		if((uint32_t)src_addr % 0xD000 == 0 && (uint32_t)src_addr != 0){
			if(mem_page_info_struct->behind_page_def_pos == handle) {
				return READ_FLOW;
			} else {
				mem_page_info_struct = (MemPageInfoTypeDef*)(mem_page_info_struct->behind_page_def_pos*MEM_PAGE_INFO_SIZE + PAGE_INFO_TAB_START);
			}
		}
		des_addr[data_num] = ((char*)((mem_page_info_struct->page[page%13] << 12) + PAGE_START))[offset];
		src_addr+=1;
	}
	return READ_OK;
}
