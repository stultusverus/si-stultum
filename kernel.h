#ifndef __KERNEL_H__
#define __KERNEL_H__

typedef struct {
  unsigned long fbbase;
  unsigned long fbsize;
  unsigned int width;
  unsigned int height;
  unsigned int ppsl;
} FrameBuffer;

#endif