#include <stdarg.h>
#include "printk.h"
//#include "lib.h"
#include "linkage.h"
#ifndef _LIB_H_PD
#define _LIB_H_PD

#define NULL 0

#define container_of(ptr,type,member)							\
({											\
	typeof(((type *)0)->member) * p = (ptr);					\
	(type *)((unsigned long)p - (unsigned long)&(((type *)0)->member));		\
})


#define sti() 		__asm__ __volatile__ ("sti	\n\t":::"memory")
#define cli()	 	__asm__ __volatile__ ("cli	\n\t":::"memory")
#define nop() 		__asm__ __volatile__ ("nop	\n\t")
#define io_mfence() __asm__ __volatile__ ("mfence	\n\t":::"memory")


struct List
{
	struct List* prev;
	struct List* next;
};

 void list_init(struct List* list)
{
	list->prev = list;
	list->next = list;
}

 void list_add_to_behind(struct List* entry, struct List* new)	////add to entry behind
{
	new->next = entry->next;
	new->prev = entry;
	new->next->prev = new;
	entry->next = new;
}

 void list_add_to_before(struct List* entry, struct List* new)	////add to entry behind
{
	new->next = entry;
	entry->prev->next = new;
	new->prev = entry->prev;
	entry->prev = new;
}

 void list_del(struct List* entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

 long list_is_empty(struct List* entry)
{
	if (entry == entry->next && entry->prev == entry)
		return 1;
	else
		return 0;
}

 struct List* list_prev(struct List* entry)
{
	if (entry->prev != NULL)
		return entry->prev;
	else
		return NULL;
}

 struct List* list_next(struct List* entry)
{
	if (entry->next != NULL)
		return entry->next;
	else
		return NULL;
}

 void* memcpy(void* From, void* To, long Num)
{
	int d0, d1, d2;
	__asm__ __volatile__(\
		"cld				\n\t"
		"rep				\n\t"
		"movsq				\n\t"
		"testb	$4,%b4		\n\t"
		"je	1f				\n\t"
		"movsl				\n\t"
		"1:\ttestb	$2,%b4	\n\t"
		"je	2f				\n\t"
		"movsw				\n\t"
		"2:\ttestb	$1,%b4	\n\t"
		"je	3f				\n\t"
		"movsb				\n\t"
		"3:					\n\t"
		:"=&c"(d0), "=&D"(d1), "=&S"(d2)
		: "0"(Num / 8), "q"(Num), "1"(To), "2"(From)
		: "memory"
	);
	return To;
}



 int memcmp(void* FirstPart, void* SecondPart, long Count)
{
	register int __res;

	__asm__	__volatile__(\
		"cld	\n\t"
		"repe	\n\t"
		"cmpsb	\n\t"
		"je	1f	\n\t"
		"movl	$1,	%%eax	\n\t"
		"jl	1f	\n\t"
		"negl	%%eax		\n\t"
		"1:		\n\t"
		:"=a"(__res)
		: "0"(0), "D"(FirstPart), "S"(SecondPart), "c"(Count)
		:
	);
	return __res;
}


 void* memset(void* Address, unsigned char C, long Count)
{
	int d0, d1;
	unsigned long tmp = C * 0x0101010101010101UL;
	__asm__	__volatile__("cld	\n\t"
		"rep				\n\t"
		"stosq				\n\t"
		"testb	$4, %b3		\n\t"
		"je	1f				\n\t"
		"stosl				\n\t"
		"1:\ttestb	$2, %b3	\n\t"
		"je	2f				\n\t"
		"stosw				\n\t"
		"2:\ttestb	$1, %b3	\n\t"
		"je	3f				\n\t"
		"stosb				\n\t"
		"3:					\n\t"
		:"=&c"(d0), "=&D"(d1)
		: "a"(tmp), "q"(Count), "0"(Count / 8), "1"(Address)
		: "memory"
	);
	return Address;
}


 char* strcpy(char* Dest, char* Src)
{
	__asm__	__volatile__("cld	\n\t"
		"1:					\n\t"
		"lodsb				\n\t"
		"stosb				\n\t"
		"testb	%%al,	%%al\n\t"
		"jne	1b			\n\t"
		:
	: "S"(Src), "D"(Dest)
		:

		);
	return 	Dest;
}

 char* strncpy(char* Dest, char* Src, long Count)
{
	__asm__	__volatile__("cld	\n\t"
		"1:					\n\t"
		"decq	%2			\n\t"
		"js	2f	\n\t"
		"lodsb	\n\t"
		"stosb	\n\t"
		"testb	%%al,	%%al\n\t"
		"jne	1b			\n\t"
		"rep	\n\t"
		"stosb	\n\t"
		"2:		\n\t"
		:
	: "S"(Src), "D"(Dest), "c"(Count)
		:
		);
	return Dest;
}

 char* strcat(char* Dest, char* Src)
{
	__asm__	__volatile__("cld	\n\t"
		"repne				\n\t"
		"scasb				\n\t"
		"decq	%1			\n\t"
		"1:					\n\t"
		"lodsb				\n\t"
		"stosb				\n\r"
		"testb	%%al,	%%al\n\t"
		"jne	1b			\n\t"
		:
	: "S"(Src), "D"(Dest), "a"(0), "c"(0xffffffff)
		:
		);
	return Dest;
}

 int strcmp(char* FirstPart, char* SecondPart)
{
	register int __res;
	__asm__	__volatile__("cld	\n\t"
		"1:					\n\t"
		"lodsb				\n\t"
		"scasb				\n\t"
		"jne	2f			\n\t"
		"testb	%%al,	%%al\n\t"
		"jne	1b			\n\t"
		"xorl	%%eax,	%%eax\n\t"
		"jmp	3f			\n\t"
		"2:					\n\t"
		"movl	$1,	%%eax	\n\t"
		"jl	3f				\n\t"
		"negl	%%eax		\n\t"
		"3:					\n\t"
		:"=a"(__res)
		: "D"(FirstPart), "S"(SecondPart)
		:
	);
	return __res;
}

 int strncmp(char* FirstPart, char* SecondPart, long Count)
{
	register int __res;
	__asm__	__volatile__("cld	\n\t"
		"1:					\n\t"
		"decq	%3			\n\t"
		"js	2f				\n\t"
		"lodsb				\n\t"
		"scasb				\n\t"
		"jne	3f			\n\t"
		"testb	%%al,	%%al\n\t"
		"jne	1b			\n\t"
		"2:					\n\t"
		"xorl	%%eax,	%%eax\n\t"
		"jmp	4f			\n\t"
		"3:					\n\t"
		"movl	$1,	%%eax	\n\t"
		"jl	4f				\n\t"
		"negl	%%eax		\n\t"
		"4:					\n\t"
		:"=a"(__res)
		: "D"(FirstPart), "S"(SecondPart), "c"(Count)
		:
	);
	return __res;
}


 int strlen(char* String)
{
	register int __res;
	__asm__	__volatile__("cld	\n\t"
		"repne	\n\t"
		"scasb	\n\t"
		"notl	%0	\n\t"
		"decl	%0	\n\t"
		:"=c"(__res)
		: "D"(String), "a"(0), "0"(0xffffffff)
		:
	);
	return __res;
}


 unsigned long bit_set(unsigned long* addr, unsigned long nr)
{
	return *addr | (1UL << nr);
}

 unsigned long bit_get(unsigned long* addr, unsigned long nr)
{
	return	*addr & (1UL << nr);
}

 unsigned long bit_clean(unsigned long* addr, unsigned long nr)
{
	return	*addr & (~(1UL << nr));
}

 unsigned char io_in8(unsigned short port)
{
	unsigned char ret = 0;
	__asm__ __volatile__("inb	%%dx,	%0	\n\t"
		"mfence			\n\t"
		:"=a"(ret)
		: "d"(port)
		: "memory");
	return ret;
}

void sleep(int times) {
	int t = times * 1000;
	__asm__ __volatile__(
		"pushq %%rcx          \n\t"
		"1:                   \n\t"
		"pause                \n\t"
		"loop 1b              \n\t"
		"popq %%rcx           \n\t"
		:
	: "c" (t)
		: "memory"
		);
}


 void io_out8(unsigned short port, unsigned char value)
{
	__asm__ __volatile__("outb	%0,	%%dx	\n\t"
		"mfence			\n\t"
		:
	: "a"(value), "d"(port)
		: "memory");
}

#define port_insw(port,buffer,nr)	\
__asm__ __volatile__("cld;rep;insw;mfence;"::"d"(port),"D"(buffer),"c"(nr):"memory")

#define port_outsw(port,buffer,nr)	\
__asm__ __volatile__("cld;rep;outsw;mfence;"::"d"(port),"S"(buffer),"c"(nr):"memory")

#endif



void putchar(unsigned int * fb,int Xsize,int x,int y,unsigned int FRcolor,unsigned int BKcolor,unsigned char font)
{
	int i = 0,j = 0;
	unsigned int * addr = NULL;
	unsigned char * fontp = NULL;
	int testval = 0;
	fontp = font_ascii[font];

	for(i = 0; i< 16;i++)
	{
		addr = fb + Xsize * ( y + i ) + x;
		testval = 0x100;
		for(j = 0;j < 8;j ++)		
		{
			testval = testval >> 1;
			if(*fontp & testval)
				*addr = FRcolor;
			else
				*addr = BKcolor;
			addr++;
		}
		fontp++;		
	}
}



int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

/*

*/

static char* number(char* str, long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[50];
	const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type & SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type & LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN && num < 0) {
		sign = '-';
		num = -num;
	}
	else
		sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type & SPECIAL)
		if (base == 16) size -= 2;
		else if (base == 8) size--;
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else while (num != 0)
		tmp[i++] = digits[do_div(num, base)];
	if (i > precision) precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL)
		if (base == 8)
			*str++ = '0';
		else if (base == 16)
		{
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;

	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}



int vsprintf(char * buf,const char *fmt, va_list args)
{
	char * str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;

	int qualifier;		/* 'h', 'l', 'L' or 'Z' for integer fields */

	for(str = buf; *fmt; fmt++)
	{

		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
		repeat:
			fmt++;
			switch(*fmt)
			{
				case '-':flags |= LEFT;	
				goto repeat;
				case '+':flags |= PLUS;	
				goto repeat;
				case ' ':flags |= SPACE;	
				goto repeat;
				case '#':flags |= SPECIAL;	
				goto repeat;
				case '0':flags |= ZEROPAD;	
				goto repeat;
			}


			field_width = -1;
			if(is_digit(*fmt))
				field_width = skip_atoi(&fmt);
			else if(*fmt == '*')
			{
				fmt++;
				field_width = va_arg(args, int);
				if(field_width < 0)
				{
					field_width = -field_width;
					flags |= LEFT;
				}
			}
			

			precision = -1;
			if(*fmt == '.')
			{
				fmt++;
				if(is_digit(*fmt))
					precision = skip_atoi(&fmt);
				else if(*fmt == '*')
				{	
					fmt++;
					precision = va_arg(args, int);
				}
				if(precision < 0)
					precision = 0;
			}
			
			qualifier = -1;
			if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
			{	
				qualifier = *fmt;
				fmt++;
			}
							
			switch(*fmt)
			{
				case 'c':

					if(!(flags & LEFT))
						while(--field_width > 0)
							*str++ = ' ';
					*str++ = (unsigned char)va_arg(args, int);
					while(--field_width > 0)
						*str++ = ' ';
					break;

				case 's':
				
					s = va_arg(args,char *);
					if(!s)
						s = '\0';
					len = strlen(s);
					if(precision < 0)
						precision = len;
					else if(len > precision)
						len = precision;
					
					if(!(flags & LEFT))
						while(len < field_width--)
							*str++ = ' ';
					for(i = 0;i < len ;i++)
						*str++ = *s++;
					while(len < field_width--)
						*str++ = ' ';
					break;

				case 'o':
					
					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
					break;

				case 'p':

					if(field_width == -1)
					{
						field_width = 2 * sizeof(void *);
						flags |= ZEROPAD;
					}

					str = number(str,(unsigned long)va_arg(args,void *),16,field_width,precision,flags);
					break;

				case 'x':

					flags |= SMALL;

				case 'X':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
					break;

				case 'd':
				case 'i':

					flags |= SIGN;
				case 'u':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),10,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),10,field_width,precision,flags);
					break;

				case 'n':
					
					if(qualifier == 'l')
					{
						long *ip = va_arg(args,long *);
						*ip = (str - buf);
					}
					else
					{
						int *ip = va_arg(args,int *);
						*ip = (str - buf);
					}
					break;

				case '%':
					
					*str++ = '%';
					break;

				default:

					*str++ = '%';	
					if(*fmt)
						*str++ = *fmt;
					else
						fmt--;
					break;
			}

	}
	*str = '\0';
	return str - buf;
}

int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char * fmt,...)
{
	int i = 0;
	int count = 0;
	int line = 0;
	va_list args;
	va_start(args, fmt);

	i = vsprintf(buf,fmt, args);

	va_end(args);

	for(count = 0;count < i || line;count++)
	{
		if(line > 0)
		{
			count--;
			goto Label_tab;
		}
		if((unsigned char)*(buf + count) == '\n')
		{
			Pos.YPosition++;
			Pos.XPosition = 0;
		}
		else if((unsigned char)*(buf + count) == '\b')
		{
			Pos.XPosition--;
			if(Pos.XPosition < 0)
			{
				Pos.XPosition = (Pos.XResolution / Pos.XCharSize - 1) * Pos.XCharSize;
				Pos.YPosition--;
				if(Pos.YPosition < 0)
					Pos.YPosition = (Pos.YResolution / Pos.YCharSize - 1) * Pos.YCharSize;
			}	
			putchar(Pos.FB_addr , Pos.XResolution , Pos.XPosition * Pos.XCharSize , Pos.YPosition * Pos.YCharSize , FRcolor , BKcolor , ' ');	
		}
		else if((unsigned char)*(buf + count) == '\t')
		{
			line = ((Pos.XPosition + 8) & ~(8 - 1)) - Pos.XPosition;

Label_tab:
			line--;
			putchar(Pos.FB_addr , Pos.XResolution , Pos.XPosition * Pos.XCharSize , Pos.YPosition * Pos.YCharSize , FRcolor , BKcolor , ' ');	
			Pos.XPosition++;
		}
		else
		{
			putchar(Pos.FB_addr , Pos.XResolution , Pos.XPosition * Pos.XCharSize , Pos.YPosition * Pos.YCharSize , FRcolor , BKcolor , (unsigned char)*(buf + count));
			Pos.XPosition++;
		}


		if(Pos.XPosition >= (Pos.XResolution / Pos.XCharSize))
		{
			Pos.YPosition++;
			Pos.XPosition = 0;
		}
		if(Pos.YPosition >= (Pos.YResolution / Pos.YCharSize))
		{
			Pos.YPosition = 0;
		}

	}
	return i;
}
