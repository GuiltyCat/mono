#include "mono_image.h"
#include "mono_music.h"
int main(void) {
	parse_test();
	return 0;
  WaveTest();
  return 0;

  test_note_freq();
  return 0;
  puts("ok");
  ImageTest();
  puts("ok");
  puts("ok");

  FILE* fp = fopen("test_music.mono", "r");
  puts("ok");
  if (fp == NULL) {
    perror("file open failed.");
    return 0;
  }
  puts("ok");
  play_sheet(fp);
  puts("ok");
  fclose(fp);
  puts("ok");

  return 0;
}
