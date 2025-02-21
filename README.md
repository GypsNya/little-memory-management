This is a memory management component used on stm32F series chips to management SDRAM space.
It will split memory space to n page with size 0x1000, similer to Intel 80386 arch's memory management.

Use page management make function can find 255MB for each handle, but physics size is can't reused, so it just can use size your ask for.

This funtion can only management max to 255MB!!!

#  CONSTANT NEED CONFIG
## MEMORY_BASE
you need config this constant point to address you want management.

## MEMORY_SIZE
config this constant memory size you want management.

#  FUNCTION
## mem_page_init(void)
Initialize memory. It must running before other operation, otherwise it can't find page to allcocation.

## mem_page_alloc(uint16_t* handle, uint32_t size)
Allocate page. first it will find which page info tab is empty and which page is empty. When it got enough page as your size, it will allocate page to handle, so you need takecare handle and keep it, if you drop handle before free it, it will make the page you used dead in memory.
Allocate time is about O(n).

## mem_page_free(uint16_t handle)
Free page. You need free page before drop handle. Otherwise the page handle point to will be dead in memory never gone find again.
Free time is about O(n).

## size_of(uint16_t handle)
Return size of handle.
Need about O(n) time.

## mov_in(uint16_t handle, char* src_addr, char* des_addr, uint32_t size)
`handle`  : handle allocate to.
`src_addr`: point to data you want put into page.
`des_addr`: point to page address you want put in.
`size`    : size of data.
Need about O(n) time.

## mov_out(uint16_t handle, char* src_addr, char* des_addr, uint32_t size)
`handle`  : handle allocate to.
`src_addr`: point to page address you want get out page.
`des_addr`: point to address you want put in.
`size`    : size of data.
Need about O(n) time.
