#include "mono_image.h"
#include "mono_music.h"
MonoMusic0* mono_music0_parse(FILE* fp, size_t sampling_freq);
MonoMusic0* mono_music0_parse2(FILE* fp, size_t sampling_freq);
int         mono_music0_wav(FILE* fp, MonoMusic0* mm0);

int main(void) {
  FILE* fp = fopen("test_music.mono", "r");
  if (fp == NULL) {
    perror("file open failed.");
    return 0;
  }
  MonoMusic0* mm0 = mono_music0_parse2(fp, 80000);
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
  // parse_test();
  // return 0;
  // WaveTest();
  // return 0;

  // test_note_freq();
  // return 0;
  // puts("ok");
  // ImageTest();
  // puts("ok");
  // puts("ok");

  // FILE* fp = fopen("test_music.mono", "r");
  // puts("ok");
  // if (fp == NULL) {
  //  perror("file open failed.");
  //  return 0;
  //}
  // puts("ok");
  // play_sheet(fp);
  // puts("ok");
  // fclose(fp);
  // puts("ok");

  // return 0;
}
