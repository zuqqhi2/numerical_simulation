#ifndef _PPB_PPM_H
#define _PPB_PPM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PPM_MAGIC "P6"


typedef struct _rgb_t {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} rgb_t;

typedef struct _image_t {
  int width;
  int height;
  rgb_t *buf;
} image_t;

int read_ppm(image_t* img, char* fname) {
  char *token, *pc;
  char *buf;
  char *del = " \t\n";

  int filesize;
  int i, w, h, luma, pixs;

  struct stat st;

  rgb_t *dot;
  FILE *fp;

  stat(fname, &st);
  filesize = (int)st.st_size;

  buf = (char *)malloc(filesize * sizeof(char));

  if ((fp = fopen(fname, "r")) == NULL) {
    printf("Failed to open file\n");
    return -1;
  }

  fseek(fp, 0, SEEK_SET);
  fread(buf, filesize * sizeof(char), 1, fp);

  fclose(fp);

  token = (char *)strtok(buf, del);
  if (strncmp(token, PPM_MAGIC, 2) != 0) {
    return -1;
  }

  token = (char *)strtok(NULL, del);
  if (token[0] == '#') {
    token = (char *)strtok(NULL, "\n");
    token = (char *)strtok(NULL, del);
  }

  w = strtoul(token, &pc, 10);
  token = (char *)strtok(NULL, del);
  h = strtoul(token, &pc, 10);
  token = (char *)strtok(NULL, del);
  luma = strtoul(token, &pc, 10);

  token = pc + 1;
  pixs = w * h;

  img->buf = (rgb_t *)malloc(pixs * sizeof(rgb_t));

  dot = img->buf;
  for (i = 0; i < pixs; i++, dot++) {
    dot->r = *token++;
    dot->g = *token++;
    dot->b = *token++;
  }

  img->width = w;
  img->height = h;
  return 0;
}

int write_ppm(image_t* img, char* fname) {
  int i, j;
  int w = img->width;
  int h = img->height;

  FILE *fp;
  if ((fp = fopen(fname, "wb+")) == NULL) {
    fprintf(stderr, "failed to open file %s\n", fname);
    return -1;
  }

  fprintf(fp, "%s\n%d %d\n255\n", PPM_MAGIC, w, h);

  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      putc((int)img->buf[i*w+j].r, fp);
      putc((int)img->buf[i*w+j].g, fp);
      putc((int)img->buf[i*w+j].b, fp);
    }
  }
  fclose(fp);
  return 0;
}

void new_image(image_t* img, int w, int h) {
  img->width = w;
  img->height = h;
  img->buf = (rgb_t*)malloc(w*h*sizeof(rgb_t));
}

void delete_image(image_t *img) {
  free(img->buf);
}

#endif
