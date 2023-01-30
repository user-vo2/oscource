#ifndef JOS_KERN_VGA_H
#define JOS_KERN_VGA_H



void vga_init(void);
void display_picture_bmp (uint8_t const *buf);
void display_ready_picture_bmp (uint32_t const *buf);
#endif