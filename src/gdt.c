#include "gdt.h"

GDT DEFAULT_GDT = {
    {0, 0, 0, 0x00, 0x00, 0}, /* 0x00 null  */
    {0, 0, 0, 0x9a, 0xa0, 0}, /* 0x08 kernel code (kernel base selector) */
    {0, 0, 0, 0x92, 0xa0, 0}, /* 0x10 kernel data */
    {0, 0, 0, 0x00, 0x00, 0}, /* 0x18 null (user base selector) */
    {0, 0, 0, 0x9a, 0xa0, 0}, /* 0x20 user code */
    {0, 0, 0, 0x92, 0xa0, 0}, /* 0x28 user data */
};
