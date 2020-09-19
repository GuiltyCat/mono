#include "mono_image.h"
#include "mono_music.h"

int main(void) {
  FILE* fp = fopen("test_music.mono", "r");
  if (fp == NULL) {
    perror("file open failed.");
    return 0;
  }
  MonoMusic0* mm0 = mono_music0_parse(fp, 80000);
  if (mm0 == NULL) {
    perror("mm0 == NULL failed.");
    return 0;
  }
  fclose(fp);
  fp = fopen("test.wav", "wb");
  if (fp == NULL) {
    perror("wav file open failed.");
    return 0;
  }
  mono_music0_wav(fp, mm0);
  fclose(fp);

  return 0;
}
