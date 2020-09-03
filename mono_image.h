#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t map[];
} Image;

Image *ImageInit(uint32_t width, uint32_t height) {
  Image *image = (Image*)malloc(sizeof(Image) + sizeof(uint8_t) * width * height);
  image->width = width;
  image->height = height;
  return image;
}

void ImageFree(Image *image) { free(image); }

#define ImageGet(image, x, y) (image->map[y * image->width + x])
bool ImageWrite(FILE *fp, Image *image) {
  uint8_t header[8] = {0};
  for (size_t i = 0; i < 4; i++) {
    header[0 + i] = (image->width >> ((3 - i) * 8)) & 0xFF;
  }
  for (size_t i = 0; i < 4; i++) {
    header[4 + i] = (image->height >> ((3 - i) * 8)) & 0xFF;
  }
  if (fwrite(header, sizeof(uint8_t), 8, fp) != 8) {
    perror("ImageWrite write header failed.");
    return false;
  }
  size_t size = image->width * image->height;
  if (fwrite(image->map, sizeof(uint8_t), size, fp) != size) {
    perror("ImageWrite write map failed.");
    return false;
  }
  return true;
}
Image *ImageRead(FILE *fp) {
  uint8_t header[8] = {0};
  if (fread(header, sizeof(uint8_t), 8, fp) != 8) {
    perror("ImageRead read header failed.");
    return NULL;
  }
  uint32_t width = 0;
  uint32_t height = 0;
  for (size_t i = 0; i < 4; i++) {
    width += header[i] << i * 8;
    width <<= 8;
  }
  for (size_t i = 0; i < 4; i++) {
    height += header[4 + i] << i * 8;
    height <<= 8;
  }

  Image *image = ImageInit(width, height);
  size_t size = image->width * image->height;
  if (fread(image->map, sizeof(uint8_t), size, fp) != size) {
    perror("ImageRead read map failed.");
    ImageFree(image);
    return NULL;
  }
  return image;
}

bool ImageToBMP(FILE *fp, Image *image) {

  uint8_t header[54] = {0};

  /* file header */
  /* file type */
  header[0] = 'B';
  header[1] = 'M';
  /* file size */
  uint32_t file_size = 54 + image->width * image->height * 3;
  for (size_t i = 0; i < 4; i++) {
    header[2 + i] = (file_size >> i * 8) & 0xFF;
  }
  /* reserved1 */
  header[6] = 0;
  header[7] = 0;
  /* reserved2 */
  header[8] = 0;
  header[9] = 0;
  /* offset */
  uint32_t offset = 54;
  for (size_t i = 0; i < 4; i++) {
    header[10 + i] = (offset >> i * 8) & 0xFF;
  }

  /* information header support Windows */
  uint32_t header_size = 40;
  for (int i = 0; i < 4; i++) {
    header[14 + i] = (header_size >> i * 8) & 0xFF;
  }

  /* width */
  int32_t width = image->width;
  for (size_t i = 0; i < 4; i++) {
    header[18 + i] = (width >> i * 8) & 0xFF;
  }
  /* height */
  int32_t height = image->height;
  for (size_t i = 0; i < 4; i++) {
    header[22 + i] = (height >> i * 8) & 0xFF;
  }
  /* planes */
  header[26] = 1;
  header[27] = 0;

  /* bit count */
  uint16_t bitcount = 24;
  for (size_t i = 0; i < 2; i++) {
    header[28 + i] = (bitcount >> i * 8) & 0xFF;
  }
  /* compression 0 */
  for (size_t i = 0; i < 4; i++) {
    header[30 + i] = 0;
  }
  /* image byte size */
  uint32_t image_byte_size = image->width * image->height;
  for (size_t i = 0; i < 4; i++) {
    header[34 + i] = (image_byte_size >> i * 8) & 0xFF;
  }
  /* xpixmeter */
  for (size_t i = 0; i < 4; i++) {
    header[38 + i] = 0;
  }
  /* ypixmeter */
  for (size_t i = 0; i < 4; i++) {
    header[42 + i] = 0;
  }
  /* clrused */
  for (size_t i = 0; i < 4; i++) {
    header[46 + i] = 0;
  }
  /* important */
  for (size_t i = 0; i < 4; i++) {
    header[46 + i] = 0;
  }

  if (fwrite(header, sizeof(uint8_t), 54, fp) != 54) {
    perror("bmp writer header failed.");
    return false;
  }

  /* palette data (empty) */
  /* image data */
  size_t line_length = image->width * 3;
  uint8_t *line = (uint8_t*)malloc(sizeof(uint8_t) * line_length);
  if (line == NULL) {
    perror("line malloc failed.");
    return false;
  }
  for (size_t j = 0; j < image->height; j++) {
    for (size_t i = 0; i < image->width; i++) {
      for (size_t k = 0; k < 3; k++) {
        line[i * 3 + k] = ImageGet(image, i, j);
      }
    }
    if (fwrite(line, sizeof(uint8_t), line_length, fp) != line_length) {
      perror("writer line failed.");
      free(line);
      return false;
    }
  }
  return true;
}

void ImageTest(void) {
  Image *img = ImageInit(500, 700);
  if (img == NULL) {
    perror("img == NULL");
    return;
  }
  ImageGet(img, 200, 200) = 255;
  ImageGet(img, 300, 200) = 255;
  ImageGet(img, 200, 300) = 255;
  FILE *fp = fopen("test.bmp", "wb");
  if (fp == NULL) {
    perror("fp == NULL");
    ImageFree(img);
    return;
  }
  if (ImageToBMP(fp, img) == false) {
    perror("ImageToBMP failed.");
    ImageFree(img);
    fclose(fp);
    return;
  }
  ImageFree(img);
  fclose(fp);
  return;
}
