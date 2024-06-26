#include "memory.h"
#include "lib.h"


unsigned long page_init(struct Page* page, unsigned long flags)
{
	if (!page->attribute)
	{
		*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
		page->attribute = flags;
		page->reference_count++;
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		page->zone_struct->total_pages_link++;
	}
	else if ((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U) || (flags & PG_Referenced) || (flags & PG_K_Share_To_U))
	{
		page->attribute |= flags;
		page->reference_count++;
		page->zone_struct->total_pages_link++;
	}
	else
	{
		*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
		page->attribute |= flags;
	}
	return 0;
}


unsigned long page_clean(struct Page* page)
{
	if (!page->attribute)
	{
		page->attribute = 0;
	}
	else if ((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U))
	{
		page->reference_count--;
		page->zone_struct->total_pages_link--;
		if (!page->reference_count)
		{
			page->attribute = 0;
			page->zone_struct->page_using_count--;
			page->zone_struct->page_free_count++;
		}
	}
	else
	{
		*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) &= ~(1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64);

		page->attribute = 0;
		page->reference_count = 0;
		page->zone_struct->page_using_count--;
		page->zone_struct->page_free_count++;
		page->zone_struct->total_pages_link--;
	}
	return 0;
}


void init_memory()
{
	int i, j;
	unsigned long TotalMem = 0;
	struct E820* p = NULL;

	color_printk(BLUE, BLACK, "Display Physics Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:Undefine)\n");
	p = (struct E820*)0xffff800000007e00;

	for (i = 0; i < 32; i++)
	{
		color_printk(INDIGO, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#010x\n", p->address, p->length, p->type);
		unsigned long tmp = 0;
		if (p->type == 1)
			TotalMem += p->length;

		memory_management_struct.e820[i].address += p->address;

		memory_management_struct.e820[i].length += p->length;

		memory_management_struct.e820[i].type = p->type;

		memory_management_struct.e820_length = i;

		p++;
		if (p->type > 4 || p->length == 0 || p->type < 1)
			break;
	}

	color_printk(INDIGO, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);

	TotalMem = 0;

	for (i = 0; i <= memory_management_struct.e820_length; i++)
	{
		unsigned long start, end;
		if (memory_management_struct.e820[i].type != 1)
			continue;
		start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
		if (end <= start)
			continue;
		TotalMem += (end - start) >> PAGE_2M_SHIFT;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#010x=%010d\n", TotalMem, TotalMem);

	TotalMem = memory_management_struct.e820[memory_management_struct.e820_length].address + memory_management_struct.e820[memory_management_struct.e820_length].length;

	//bits map construction init

	memory_management_struct.bits_map = (unsigned long*)((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);

	memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;

	memory_management_struct.bits_length = (((unsigned long)(TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));

	memset(memory_management_struct.bits_map, 0xff, memory_management_struct.bits_length);		//init bits map memory

	//pages construction init

	memory_management_struct.pages_struct = (struct Page*)(((unsigned long)memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);

	memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;

	memory_management_struct.pages_length = ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));

	memset(memory_management_struct.pages_struct, 0x00, memory_management_struct.pages_length);	//init pages memory

	//zones construction init

	memory_management_struct.zones_struct = (struct Zone*)(((unsigned long)memory_management_struct.pages_struct + memory_management_struct.pages_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);

	memory_management_struct.zones_size = 0;

	memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));

	memset(memory_management_struct.zones_struct, 0x00, memory_management_struct.zones_length);	//init zones memory



	for (i = 0; i <= memory_management_struct.e820_length; i++)
	{
		unsigned long start, end;
		struct Zone* z;
		struct Page* p;
		unsigned long* b;

		if (memory_management_struct.e820[i].type != 1)
			continue;
		start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
		if (end <= start)
			continue;

		//zone init

		z = memory_management_struct.zones_struct + memory_management_struct.zones_size;
		memory_management_struct.zones_size++;

		z->zone_start_address = start;
		z->zone_end_address = end;
		z->zone_length = end - start;

		z->page_using_count = 0;
		z->page_free_count = (end - start) >> PAGE_2M_SHIFT;

		z->total_pages_link = 0;

		z->attribute = 0;
		z->GMD_struct = &memory_management_struct;

		z->pages_length = (end - start) >> PAGE_2M_SHIFT;
		z->pages_group = (struct Page*)(memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));

		//page init
		p = z->pages_group;
		for (j = 0; j < z->pages_length; j++, p++)
		{
			p->zone_struct = z;
			p->PHY_address = start + PAGE_2M_SIZE * j;
			p->attribute = 0;

			p->reference_count = 0;

			p->age = 0;

			*(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->PHY_address >> PAGE_2M_SHIFT) % 64;

		}

	}


	memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;

	memory_management_struct.pages_struct->PHY_address = 0UL;
	memory_management_struct.pages_struct->attribute = 0;
	memory_management_struct.pages_struct->reference_count = 0;
	memory_management_struct.pages_struct->age = 0;


	memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));

	color_printk(ORANGE, BLACK, "bits_map:\t%#018lx\tbits_size:%#018lx\tbits_length:%#018lx\n", memory_management_struct.bits_map, memory_management_struct.bits_size, memory_management_struct.bits_length);

	color_printk(ORANGE, BLACK, "pages_struct:\t%#018lx\tpages_size:%#018lx\tpages_length:%#018lx\n", memory_management_struct.pages_struct, memory_management_struct.pages_size, memory_management_struct.pages_length);

	color_printk(ORANGE, BLACK, "zones_struct:\t%#018lx\tzones_size:%#018lx\tzones_length:%#018lx\n", memory_management_struct.zones_struct, memory_management_struct.zones_size, memory_management_struct.zones_length);

	ZONE_DMA_INDEX = 0;	//need rewrite in the future
	ZONE_NORMAL_INDEX = 0;	//need rewrite in the future

	for (i = 0; i < memory_management_struct.zones_size; i++)
	{
		struct Zone* z = memory_management_struct.zones_struct + i;
		color_printk(ORANGE, BLACK, "zone_start_address:%#018lx\tzone_end_address:%#018lx\tzone_length:%#018lx\tpages_group:%#018lx\tpages_length:%#018lx\n", z->zone_start_address, z->zone_end_address, z->zone_length, z->pages_group, z->pages_length);

		if (z->zone_start_address == 0x100000000)
			ZONE_UNMAPED_INDEX = i;
	}

	memory_management_struct.end_of_struct = (unsigned long)((unsigned long)memory_management_struct.zones_struct + memory_management_struct.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));	////need a blank to separate memory_management_struct

	color_printk(ORANGE, BLACK, "start_code:%#018lx\tend_code:%#018lx\tend_data:%#018lx\tend_brk:%#018lx\tend_of_struct:%#018lx\n", memory_management_struct.start_code, memory_management_struct.end_code, memory_management_struct.end_data, memory_management_struct.end_brk, memory_management_struct.end_of_struct);


	i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;

	for (j = 0; j <= i; j++)
	{
		page_init(memory_management_struct.pages_struct + j, PG_PTable_Maped | PG_Kernel_Init | PG_Active | PG_Kernel);
	}
}

struct Page* alloc_pages(int zone_select, int number, unsigned long page_flags)
{
	int i;
	unsigned long page = 0;

	int zone_start = 0;
	int zone_end = 0;

	switch (zone_select)
	{
	case ZONE_DMA:
		zone_start = 0;
		zone_end = ZONE_DMA_INDEX;

		break;

	case ZONE_NORMAL:
		zone_start = ZONE_DMA_INDEX;
		zone_end = ZONE_NORMAL_INDEX;

		break;

	case ZONE_UNMAPED:
		zone_start = ZONE_UNMAPED_INDEX;
		zone_end = memory_management_struct.zones_size - 1;

		break;

	default:
		color_printk(RED, BLACK, "alloc_pages error zone_select index\n");
		return NULL;
		break;
	}

	for (i = zone_start; i <= zone_end; i++)
	{
		struct Zone* z;
		unsigned long j;
		unsigned long start, end, length;
		unsigned long tmp;

		if ((memory_management_struct.zones_struct + i)->page_free_count < number)
			continue;

		z = memory_management_struct.zones_struct + i;
		start = z->zone_start_address >> PAGE_2M_SHIFT;
		end = z->zone_end_address >> PAGE_2M_SHIFT;
		length = z->zone_length >> PAGE_2M_SHIFT;

		tmp = 64 - start % 64;

		for (j = start; j <= end; j += j % 64 ? tmp : 64)
		{
			unsigned long* p = memory_management_struct.bits_map + (j >> 6);
			unsigned long shift = j % 64;
			unsigned long k;
			for (k = shift; k < 64 - shift; k++)
			{
				if (!(((*p >> k) | (*(p + 1) << (64 - k))) & (number == 64 ? 0xffffffffffffffffUL : ((1UL << number) - 1))))
				{
					unsigned long	l;
					page = j + k - 1;
					for (l = 0; l < number; l++)
					{
						struct Page* x = memory_management_struct.pages_struct + page + l;
						page_init(x, page_flags);
					}
					goto find_free_pages;
				}
			}

		}
	}

	return NULL;

find_free_pages:

	return (struct Page*)(memory_management_struct.pages_struct + page);
}

void init_PD_Memory() {
	struct Page* page;
	page = alloc_pages(ZONE_NORMAL, 64, PG_PTable_Maped | PG_Active | PG_Kernel);
	PD_M.PD_system_memory = (unsigned char*)page->PHY_address;
	PD_M.size = 128 * 1024 * 1024;
	PD_M.length = 0;
}

unsigned char* pd_malloc(long size) {
	unsigned char* re = (unsigned char*)(PD_M.PD_system_memory + PD_M.length);
	PD_M.length += size;
	if (PD_M.length > PD_M.size)
		while (1) {

		}
	return re;
}
void init_buffer() {
	PD_buffer.size = 100;
	PD_buffer.io_Buffer = (unsigned char*)pd_malloc(PD_buffer.size);
	PD_buffer.length = 0;
}

void init_lock() {
	PD_Lock.cursor_lock = (unsigned char*)pd_malloc(1);
	*PD_Lock.cursor_lock = 0;
}