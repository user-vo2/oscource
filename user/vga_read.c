#include <inc/lib.h>

void
umain(int argc, char **argv) {
    int fd, n;
    uint8_t buf[600];
    // uint8_t buf_extra[2];
    uint8_t *p = buf;
    binaryname = "vga_read";
    if ((fd = open(argv[1], O_RDONLY)) < 0)
        panic("Failed to open file: %i", fd);
    uint32_t x = 0, color = 0, addr = 0, width = 0, height = 0, /*is_first = 1, */fb_addr = 0/*, cnt =0*/ ;
    n = read(fd, buf, 26);
    addr = *((uint32_t *)&buf[10]);
    width = *((uint32_t *)&buf[18]);
    height = *((uint32_t *)&buf[22]);
    read(fd, buf, addr - 26);
    uint32_t sz = sizeof(buf);
    while ((n = read(fd, p, sz)) == sz) {
        for (uint32_t i = 0; i < sz; i+=3) {
            color = buf[i] + (buf[i + 1] << 8) + (buf[i + 2]<< 16);
            x++;
            fb_addr = (600 - (x / width)) * 800 + (x % width);
            sys_fb_putc(fb_addr, color);
        }
        // addr += sz;
        // cprintf("%u\n", addr);
    }
    sys_fb_putc(0xffffffff, 0xff000000);     
    close(fd);
}
