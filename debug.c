#include <debug.h>
#include <cpu/cpu.h>

static void putc_e9(char c){
    outb(0xE9,(uint8_t)c);
}

void qemu_debug_putc(char c){
    if(c=='\n') putc_e9('\r');
    putc_e9(c);
}

static void put_uint(unsigned int val,int base){
    char buf[32];
    const char*dig="0123456789abcdef";
    int i=0;
    if(val==0){qemu_debug_putc('0');return;}
    while(val){buf[i++]=dig[val%base];val/=base;}
    while(i) qemu_debug_putc(buf[--i]);
}

void qemu_debug_printf(const char *fmt,...){
    va_list args;va_start(args,fmt);
    for(const char*p=fmt;*p;p++){
        if(*p!='%'){qemu_debug_putc(*p);continue;}
        p++;if(!*p)break;
        switch(*p){
            case 'c':qemu_debug_putc((char)va_arg(args,int));break;
            case 's':{char* s=va_arg(args,char*);while(*s)qemu_debug_putc(*s++);}break;
            case 'd':case 'i':{int v=va_arg(args,int);if(v<0){qemu_debug_putc('-');v=-v;}put_uint(v,10);}break;
            case 'u': put_uint(va_arg(args,unsigned int),10);break;
            case 'x':case 'X': put_uint(va_arg(args,unsigned int),16);break;
            case '%': qemu_debug_putc('%');break;
            default: qemu_debug_putc('%');qemu_debug_putc(*p);break;
        }
    }
    va_end(args);
} 