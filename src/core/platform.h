#ifndef PLATFORM_H__
#define PLATFORM_H__

f64 platform_get_time();

typedef struct {
  u8 *data;
  u64 width;
  u64 height;
} Platform_Image_Data;
Platform_Image_Data platform_load_image_bytes_as_rgba(const char *filepath);

#endif
