#include"PDshell.h"
void __stack_chk_fail() {



}


unsigned char get_elme(struct Path path, int x, int y) {
	return *(path.path + x * path.w_length + y);
}
unsigned char* get_row(struct Path path, int x) {
	return path.path + x * path.w_length;
}
void set_elme(struct Path* path, unsigned char elme, int x, int y) {
	*(path->path + x * path->w_length + y) = elme;
}
void set_row(struct Path* path, unsigned char* row, int x) {
	for (int i = 0; i < path->w_length; i++) {
		*(path->path + path->w_length * x + i) = *(row + i);
	}
}

int get_depth(struct Path path) {
	int i;
	for (i = 0; i < path.h_length; i++) {
		if (get_elme(path, i, 0) == '\0')
			break;
	}
	return i;
}

void init_Path(struct Path* path) {
	path->h_length = 5;
	path->w_length = 16;
	path->path = (unsigned char*)pd_malloc(path->w_length * path->h_length);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 16; j++) {
			set_elme(path, '\0', i, j);
		}
	}
}
void init_shell() {
	init_Path(&shell.work_path);
	set_elme(&shell.work_path, '/', 0, 0);
	shell.work_Dir_Items = FileSystem.root_DI;
}
void print_cursor_static(unsigned char* addr, unsigned int* color)
{
	int i;
	int j;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 6; j++)
		{
			*((unsigned char*)addr + (j * 4) + i * 1440 * 4 + 0) = *((unsigned char*)color + 0);
			*((unsigned char*)addr + (j * 4) + i * 1440 * 4 + 1) = *((unsigned char*)color + 1);
			*((unsigned char*)addr + (j * 4) + i * 1440 * 4 + 2) = *((unsigned char*)color + 2);
			*((unsigned char*)addr + (j * 4) + i * 1440 * 4 + 3) = *((unsigned char*)color + 3);
		}
	}
}
void print_cursor()
{
	int XPosition;
	int YPosition;
	unsigned char* addr;
	int i, j;
	int count = 1;
	unsigned int color;
	while (1)
	{
		XPosition = Pos.XPosition;
		YPosition = Pos.YPosition;
		addr = (unsigned char*)Pos.FB_addr + YPosition * 1440 * 4 * Pos.YCharSize + XPosition * Pos.XCharSize * 4;
		if (*(PD_Lock.cursor_lock) == (unsigned char)0x0)
		{
			if (count % 2 == 0)
			{
				color = 0x00000000;
				print_cursor_static(addr, &color);
			}
			else
			{
				color = 0x0066ffff;
				// color_printk(RED, BLACK, "%xl",color);
				print_cursor_static(addr, &color);
			}
			sleep(20000);
			if (*(PD_Lock.cursor_lock) == ((unsigned char)0x1))
			{
				//color_printk(RED, BLACK,"x ab");
				color = 0x00000000;
				print_cursor_static(addr, &color);
				// color_printk(RED, BLACK, "  lock  ");
			}
			count++;
		}
		else
			break;
	}
}

void print_path() {
	for (int i = 0; i < get_depth(shell.work_path); i++) {

		color_printk(RED, BLACK, "%s", get_row(shell.work_path, i));
		color_printk(RED, BLACK, "/");
	}
	color_printk(RED, BLACK, ">");
}

int get_dir_items(struct Path path, struct Dir_Items di_start, struct Dir_Items* di_end) {
	if (get_elme(path, 0, 0) == '\0') {
		di_end = &FileSystem.root_DI;
		return -1;
	}
	int i;
	for (int j = 0; j < get_depth(path); j++) {
		for (i = 0; i < di_start.child_DIT->length; i++) {
			if (is_equal((di_start.child_DIT->base_dir_items + i)->name, get_row(path, j))) {
				break;
			}
		}
		if (i == di_start.child_DIT->length)
			return j + 1;
		di_start = *(di_start.child_DIT->base_dir_items + i);
	}
	*di_end = di_start;
	return -1;
}

void make_dir_items(struct Path  path, struct Dir_Items* di_start) {
	struct Dir_Items di_temp = *di_start;
	int i;
	for (int j = 0; j < get_depth(path); j++) {
	A:
		for (i = 0; i < di_temp.child_DIT->length; i++) {
			if (is_equal((di_temp.child_DIT->base_dir_items + i)->name, get_row(path, j))) {
				break;
			}
		}
		if (i == di_temp.child_DIT->length) {
			struct Dir_Items* di;
			di = (struct Dir_Items*)(di_temp.child_DIT->base_dir_items + di_temp.child_DIT->length);
			di_temp.child_DIT->length++;
			init_dir_items(di, get_row(path, j), &di_temp);
			goto A;
		}
		di_temp = *(di_temp.child_DIT->base_dir_items + i);
	}
}

int is_relative_or_absolute(struct Path* path) {
	if (get_elme(*path, 0, 0) == '/') {
		path->path = path->path + path->w_length;
		return 0;
	}
	return 1;
}

void string_to_path(unsigned char* string, struct Path* path) {
	init_Path(path);
	int k = 0;
	int i = 0;
	int j = 0;
	if (*string == '/') {
		set_elme(path, '/', 0, 0);
		j = 1;
		i = 1;
	}
	for (; *(string + i) != '\0'; i++, k++) {
		if (*(string + i) == '/') {
			set_elme(path, '\0', j, k);
			j++;
			k = -1;
			continue;
		}
		set_elme(path, *(string + i), j, k);
	}
	set_elme(path, '\0', j, ++k);
}
void empty_path(struct Path* path) {
	for (int i = 0; i < path->h_length; i++) {
		for (int j = 0; j < path->w_length; j++) {
			set_elme(path, '\0', i, j);
		}
	}
}
void init_root_DI() {
	FileSystem.root_DI.name = (unsigned char*)pd_malloc(16);
	*(FileSystem.root_DI.name) = '/';
	*(FileSystem.root_DI.name + 1) = '\0';
	FileSystem.root_DI.parents_DIT = NULL;
	struct Dir_Items_Table* dit = (struct Dir_Items_Table*)pd_malloc(sizeof(struct Dir_Items_Table));
	init_dir_items_table(dit, &FileSystem.root_DI);
	FileSystem.root_DI.child_DIT = dit;
	FileSystem.root_DI.file_type = DIR;
	FileSystem.root_DI.file_link = NULL;
}
void do_cmd_cd(struct Path path) {
	if (is_relative_or_absolute(&path)) {
		get_dir_items(path, shell.work_Dir_Items, &shell.work_Dir_Items);
		for (int i = get_depth(shell.work_path), j = 0; j < get_depth(path); i++, j++) {
			set_row(&shell.work_path, get_row(path, j), i);
		}
	}
	else {
		if (path.path) {
			shell.work_Dir_Items = FileSystem.root_DI;
			empty_path(&shell.work_path);
			set_elme(&shell.work_path, '/', 0, 0);
		}
		get_dir_items(path, FileSystem.root_DI, &shell.work_Dir_Items);
		empty_path(&shell.work_path);
		for (int i = 0, j = -1; j < get_depth(path); i++, j++) {
			set_row(&shell.work_path, get_row(path, j), i);
		}
	}
}



void do_cmd_ls(struct Path path) {
	struct Dir_Items dir_temp;
	if (is_relative_or_absolute(&path)) {
		if (*path.path == 'l' && *(path.path + 2) == '\0') {
			struct Dir_Items* di = shell.work_Dir_Items.child_DIT->base_dir_items;
			int n = shell.work_Dir_Items.child_DIT->length;
			color_printk(RED, BLACK, "\n");
			for (int i = 0; i < n; i++) {
				color_printk(RED, BLACK, "%s", (di + i)->name);
				Pos.XPosition = (Pos.XPosition + 20 - 1) / 20 * 20;
			}
		}
		else {
			get_dir_items(path, FileSystem.root_DI, &dir_temp);
			struct Dir_Items* di = dir_temp.child_DIT->base_dir_items;
			int n = dir_temp.child_DIT->length;
			color_printk(RED, BLACK, "\n");
			for (int i = 0; i < n; i++) {
				color_printk(RED, BLACK, "%s", (di + i)->name);
				Pos.XPosition = (Pos.XPosition + 20 - 1) / 20 * 20;
			}
		}
	}
	else {
		if (*path.path) {
			get_dir_items(path, shell.work_Dir_Items, &dir_temp);
			struct Dir_Items* di = dir_temp.child_DIT->base_dir_items;
			int n = dir_temp.child_DIT->length;
			color_printk(RED, BLACK, "\n");
			for (int i = 0; i < n; i++) {
				color_printk(RED, BLACK, "%s", (di + i)->name);
				Pos.XPosition = (Pos.XPosition + 20 - 1) / 20 * 20;
			}
		}
		else {
			struct Dir_Items* di = FileSystem.root_DI.child_DIT->base_dir_items;
			int n = FileSystem.root_DI.child_DIT->length;
			color_printk(RED, BLACK, "\n");
			for (int i = 0; i < n; i++) {
				color_printk(RED, BLACK, "%s", (di + i)->name);
				Pos.XPosition = (Pos.XPosition + 20 - 1) / 20 * 20;
			}
		}
	}
}
void do_cmd_mkdir(struct Path path) {
	if (is_relative_or_absolute(&path)) {
		make_dir_items(path, &shell.work_Dir_Items);
	}
	else {
		make_dir_items(path, &FileSystem.root_DI);
	}
}
void empty_io_buffer() {
	for (int i = 0; i < PD_buffer.size; i++) {
		PD_buffer.io_Buffer[i] = '\0';
	}
	PD_buffer.length = 0;
}
void do_cmd_clear() {
	unsigned char * addr = Pos.FB_addr;
	int i;
	int j;
	for (i = 0; i < 3600; i++) {
		for( j =0;j<1440;j++){
			*((unsigned char*)addr +i*1440+j+ 0) = (unsigned char)0x00;
			*((unsigned char*)addr +i*1440+j+ 0) = (unsigned char)0x00;
			*((unsigned char*)addr +i*1440+j+ 0) = (unsigned char)0x00;
			*((unsigned char*)addr +i*1440+j+ 0) = (unsigned char)0x00;
		}
		
	}
	

	Pos.XPosition = 0;
	Pos.YPosition = 0;
}
void do_cmd() {
	color_printk(RED, BLACK, "\n");
	int i = 0;
	struct Path cmd_path;
	if (*PD_buffer.io_Buffer == 'l' && *(PD_buffer.io_Buffer + 2) == '\0');
	else {
		int mark = 0;
		while (*(PD_buffer.io_Buffer + i) == ' ' || mark == 0) {
			if (*(PD_buffer.io_Buffer + i) == ' ')
				mark = 1;
			i++;
		}
	}
	string_to_path(PD_buffer.io_Buffer + i, &cmd_path);
	switch (*(PD_buffer.io_Buffer + 1)) {
	case 'd':
		do_cmd_cd(cmd_path);
		color_printk(RED, BLACK, "\n\n");
		break;
	case's':
		do_cmd_ls(cmd_path);
		color_printk(RED, BLACK, "\n\n");
		break;
	case'k':
		do_cmd_mkdir(cmd_path);
		color_printk(RED, BLACK, "\n\n");
		break;
	case'l':
		do_cmd_clear();
		break;
	default:
		color_printk(RED, BLACK, "unknown command \n");
	}
	empty_io_buffer(PD_buffer);
	*PD_Lock.cursor_lock = (unsigned char)0;
}
void do_shell() {
	init_PD_Memory();
	//color_printk(RED,BLACK,"do shell");
	init_buffer();
	// color_printk(RED,BLACK,"do buffer");
	init_lock();
	// color_printk(RED,BLACK,"do lock");
	init_root_DI();
	// color_printk(RED,BLACK,"do root_DI");
	init_shell();
	// color_printk(RED,BLACK,"do init_shell");
	while (1) {
		print_path();
		print_cursor();
		do_cmd();
	}
}