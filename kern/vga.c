#include <inc/lib.h>
#include <inc/uefi.h>

#include <kern/picirq.h>
#include <kern/vgareg.h>
// #include <kern/on-my-puter.h>
#include <kern/paper.h>
// #include <kern/mona.h>
#include <kern/timer.h>

static bool graphics_exists = false;
static uint32_t uefi_vres;
static uint32_t uefi_hres;
static uint32_t uefi_stride;
static uint32_t crt_rows;
static uint32_t crt_cols;
static uint32_t crt_size;
static uint16_t crt_pos;
static uint32_t *crt_buf = (uint32_t *)FRAMEBUFFER;
static uint32_t invisible_buf[800 * 600];


static void print_registers(void) {
	cprintf("Miscellaneous outbut Register: 0x%x\n", inb(0x3cc));
	cprintf("Feature Control Register: 0x%x\n", inb(0x3CA));
	cprintf("Input Status #0 Register: 0x%x\n", inb(0x3C2));
	cprintf("Input Status #1 Register(mono): 0x%x\n", inb(0x3BA));
	cprintf("Input Status #1 Register(color): 0x%x\n", inb(0x3DA));
	cprintf("Graphics Registers:\n");
	for (uint8_t i = 0; i <= 0x8; i++) {
		uint8_t val = inb(0x3ce);
		outb(0x3ce, i);
		cprintf("Index %02xh: 0x%x\n", i, inb(0x3cf));
		outb(0x3ce, val);
	}
	cprintf("Sequencer Registers:\n");
	for (uint8_t i = 0; i <= 0x4; i++) {
		uint8_t val = inb(0x3c4);
		outb(0x3c4, i);
		cprintf("Index %02xh: 0x%x\n", i, inb(0x3c5));
		outb(0x3c4, val);
	}
	cprintf("CRT Controller Registers:\n");
	for (uint8_t i = 0; i <= 0x18; i++) {
		uint8_t val = inb(0x3d4);
		outb(0x3d4, i);
		cprintf("Index %02xh: 0x%x\n", i, inb(0x3d5));
		outb(0x3d4, val);
	}
}

static void set_clock28(void) {
	outb(0x3c2, (inb(0x3cc) & 0xf3) | 0xf7);
}

static void set_clock25(void) {
	outb(0x3c2, (inb(0x3cc) & 0xf3));
}

static void dump_buf(void) {
	memcpy(crt_buf, invisible_buf, sizeof(invisible_buf));
}

static void set_pixel(uint32_t x, uint32_t y, uint32_t color) {
	invisible_buf[x * uefi_stride + y] = color;
}

static void set_pixel_x2(uint32_t x, uint32_t y, uint32_t color) {
	set_pixel(x << 1, y << 1, color);
	set_pixel((x << 1) + 1, y << 1, color);
	set_pixel(x << 1, (y << 1) + 1, color);
	set_pixel((x << 1) + 1, (y << 1) + 1, color);
}

static void fill(uint32_t fill_color) {
	for (uint32_t i = 0; i < uefi_vres; i++) {
		for (uint32_t j = 0; j < uefi_hres; j++) {
			set_pixel(i, j, fill_color);
		}
	}
	dump_buf();
}

static void set_res(void) {
	uint8_t val = inb(0x3d4);
	outb(0x3d4, 0x11);
	outb(0x3d5, 0xc);
	outb(0x3d4, val);
	val = inb(0x3d4);
	outb(0x3d4, 0x0);
	outb(0x3d5, 0x11);
	outb(0x3d4, val);
}


static void get_res(void) {
	uint8_t val = inb(0x3d4);
	outb(0x3d4, 0x0);
	cprintf("Horizontal Total: 0x%x\n", inb(0x3d5));
	outb(0x3d4, val);
	val = inb(0x3d4);
	outb(0x3d4, 0x11);
	cprintf("Vertical Retrace End: 0x%x\n", inb(0x3d5));
	outb(0x3d4, val);
}

static void print_lp(void) {
	LOADER_PARAMS *lp = (LOADER_PARAMS *)uefi_lp;
    uefi_vres = lp->VerticalResolution;
    uefi_hres = lp->HorizontalResolution;
    uefi_stride = lp->PixelsPerScanLine;
    crt_rows = uefi_vres /*/ SYMBOL_SIZE*/;
    crt_cols = uefi_hres /*/ SYMBOL_SIZE*/;
    crt_size = crt_rows * crt_cols;
    crt_pos = crt_cols;

    

    /* Clear screen */
    memset(crt_buf, 0, lp->FrameBufferSize);
    cprintf("Vertical Resolution: %u\n", uefi_vres);
    cprintf("Horizontal Resolution: %u\n", uefi_hres);
    cprintf("Stride: %u\n", uefi_stride);
    cprintf("Framebuffer Size: %u\n", crt_cols);
    graphics_exists = true;
}

void display_picture_bmp (uint8_t const *buf) {
	uint32_t x = 0, color = 0;
	uint32_t addr = *((uint32_t *)&buf[10]);
	uint32_t width = *((uint32_t *)&buf[18]);
	uint32_t height = *((uint32_t *)&buf[22]);
	
	for (uint32_t i = addr; i < width * height * 3 + addr; i += 3) {
		color = buf[i] + (buf[i + 1] << 8) + (buf[i + 2]<< 16);
		set_pixel(crt_rows - x / width, x % width, color);
		x++;
	}
	dump_buf();
}

void display_ready_picture_bmp (uint32_t const *buf) {
	cprintf("%lu\n", sizeof(invisible_buf));
	memset(crt_buf, 0, crt_size);
	cprintf("%lu\n", sizeof(invisible_buf));
	memcpy(crt_buf, buf, sizeof(invisible_buf));
}


void vga_init(void) {
	uint32_t fill_color = 0xf0f;
	print_lp();
	print_registers();
	set_clock25();
	memset(crt_buf, 0, crt_size);
	fill(fill_color);
	display_picture_bmp(paper);
	
	// cprintf("%d\n", start);
#if 1 //set to zero to continue normally
	uint32_t start = pmtimer_get_timeval();
	while(pmtimer_get_timeval() - start < 10000000) {
		// cprintf("%d\n", pmtimer_get_timeval());
		;
	}
	start = pmtimer_get_timeval();
	
	memset(crt_buf, 0, crt_size);
	// display_picture_bmp(paper);
	// dump_buf();
	// while(pmtimer_get_timeval() - start < 10000000) {
	// 	// cprintf("%d\n", pmtimer_get_timeval());
	// 	;
	// }
#endif
}

