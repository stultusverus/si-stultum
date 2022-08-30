#ifndef __KERNEL_H__
#define __KERNEL_H__

typedef struct {
  unsigned long fbbase;
  unsigned long fbsize;
  unsigned int width;
  unsigned int height;
  unsigned int ppsl;
} FrameBuffer;

typedef struct {
  FrameBuffer *fb;
  void *mmap;
  unsigned long mmap_size;
  unsigned long mmap_desc_size;
} BootInfo;

#endif // __KERNEL_H__