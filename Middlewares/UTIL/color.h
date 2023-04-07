#ifndef __COLOR_H__
#define __COLOR_H__

#define R5_SIZE 32
#define G6_SIZE 64
#define B5_SIZE 32

#define R8_SIZE 256
#define G8_SIZE 256
#define B8_SIZE 256

#define RGB888toRGB565(RGB888)                                                 \
    ((((RGB888) >> 16 & 0XFF) * R5_SIZE / R8_SIZE) << 11) +                    \
        ((((RGB888) >> 8 & 0XFF) * G6_SIZE / G8_SIZE) << 5) +                  \
        (((((RGB888)&0XFF) * B5_SIZE / B8_SIZE)))

#define RGB888_BLACK 0X000000
#define RGB888_WHITE 0XFFFFFF
#define RGB888_RED 0XFF0000

#endif
