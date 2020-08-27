#include "mono_image.h"
#include "mono_music.h"
int main(void) {
	puts("ok");
  ImageTest();
	puts("ok");
  WavTest();
	puts("ok");

  FILE *fp = fopen("test_music.mono", "r");
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
