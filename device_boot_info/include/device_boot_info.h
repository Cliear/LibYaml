#ifndef DEVICE_BOOT_INFO_H
#define DEVICE_BOOT_INFO_H

#include <type.h>
#include <memory_info.h>

typedef enum {DEVICE_PIXEL_RGBA, DEVICE_PIXEL_BGRA, DEVICE_PIXEL_TYPE_COUNT} DisplayDevicePixelType;

typedef struct
{
    RegisterSize frame_buff;
    RegisterSize buff_size;
    RegisterSize width;
    RegisterSize height;
    RegisterSize pixs_per_line;
    RegisterSize format;        // value is from DisplayDevicePixelType
} DeviceVideoInfo;

typedef struct
{
    RegisterSize image_number;
    DeviceVideoInfo video;
    DeviceMemoryInfo memory;
} DeviceBootInfo;

#endif