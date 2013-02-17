#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include "ppb_ppm.h"
#include "ppb_raytrace.h"
#include "timer.h"

void set_color(rgb_t* buf, unsigned char r, unsigned char g, unsigned char b){
  buf->r = r;
  buf->g = g;
  buf->b = b;
  return;
}

void mult_color(rgb_t* b, rgb_t* a, float t) {
  b->r = (char)(t * a->r);
  b->g = (char)(t * a->g);
  b->b = (char)(t * a->b);
  return;
}

void add_color(rgb_t* c, rgb_t* a, rgb_t* b) {
  c->r = a->r + b->r;
  c->g = a->g + b->g;
  c->b = a->b + b->b;
  return;
}

void draw_pixel(rgb_t* buf, rgb_t* color) {
  buf->r = color->r;
  buf->g = color->g;
  buf->b = color->b;
  return;
}

void set_vector(vector_t* v, float x, float y, float z) {
  v->x = x;
  v->y = y;
  v->z = z;
  return;
}

void add_vector(vector_t* c, vector_t* a, vector_t* b) {
  c->x = a->x + b->x;
  c->y = a->y + b->y;
  c->z = a->z + b->z;
  return;
}

void max_vector(vector_t* c, float n) {
  c->x = Max(c->x, n);
  c->y = Max(c->y, n);
  c->z = Max(c->z, n);
  return;
}

void sub_vector(vector_t* c, vector_t* a, vector_t* b) {
  c->x = a->x - b->x;
  c->y = a->y - b->y;
  c->z = a->z - b->z;
  return;
}

void mult_vector(vector_t* b, vector_t* a, float t) {
  b->x = t * a->x;
  b->y = t * a->y;
  b->z = t * a->z;
  return;
}

void norm_vector(vector_t* a) {
  float d = sqrtf(Sq(a->x) + Sq(a->y) + Sq(a->z));
  a->x /= d;
  a->y /= d;
  a->z /= d;
  return;
}

float innerproduct_vector(vector_t* a, vector_t* b) {
  return (a->x*b->x + a->y*b->y + a->z*b->z);
}

void set_workarea(workarea_t* warea, int sx, int ex, int sy, int ey) {
  warea->startx = sx;
  warea->starty = sy;
  warea->endx = ex;
  warea->endy = ey;
  return;
}

void new_scene(scene_data_t* scene) {
  set_color(&(scene->ball).color, SPHERE_COLOR);
  set_vector(&(scene->ball).center, SPHERE_CENTER);
  (scene->ball).radius = SPHERE_RAD;

  set_color(&(scene->bgcolor), BG_COLOR);
  set_vector(&(scene->viewpoint), VIEW_POINT);
  set_vector(&(scene->light), LIGHT);
  return;
}

float intersect(sphere_t* ball, vector_t* viewpoint, vector_t* view) {
  float t;
  vector_t vtmp;

  sub_vector(&vtmp, viewpoint, &ball->center);
  float b = innerproduct_vector(view, &vtmp);
  float c = innerproduct_vector(&vtmp, &vtmp) - Sq(ball->radius);
  float d = Sq(b) - c;

  if (d < 0) return INFINITY2;
  float det = sqrtf(d);
  t = -b - det;
  if (t < 0.05f) t = -b + det;
  if (t < 0) return INFINITY2;
  return t;
}

rgb_t shading(vector_t* view, vector_t* light, vector_t* n, rgb_t* color, float s) {
  float kd = 0.7f, ks = 0.7f, ke = 0.3f;

  rgb_t white;
  set_color(&white, 255, 255, 255);
  rgb_t c, c0, c1;

  float ln = innerproduct_vector(light, n);
  float lv = innerproduct_vector(light, view);
  float nv = innerproduct_vector(n, view);
  float cos_alpha = Max((-1.0f * ln), 0.0f);
  float cos_beta = Max((2.0f * ln * nv - lv), 0.0f);
  float cos_beta_pow20;
  
  Pow(cos_beta_pow20, cos_beta, 20.0);
  mult_color(&c0, color, (s * kd * cos_alpha + ke));
  mult_color(&c1, &white, (s * ks * cos_beta_pow20));
  add_color(&c, &c0, &c1);
  return c;
}

void trace_ray(image_t* img, scene_data_t* scene, workarea_t* warea) {
  int x, y;
  rgb_t color;

  int w = img->width;
  int h = img->height;
  vector_t view = scene->view;
  vector_t viewpoint = scene->viewpoint;
  vector_t light = scene->light;
  sphere_t ball = scene->ball;
  rgb_t bgcolor = scene->bgcolor;
  float dv, t;
  int index;

  for (y = warea->starty; y < warea->endy; ++y) {
    viewpoint.y = y*2.0f/h-1.0f;
    for (x = warea->startx; x < warea->endx; ++x) {
      viewpoint.x = x*2.0f/w-1.0f;

      dv = sqrtf(Sq(viewpoint.x)+Sq(viewpoint.y)+Sq(viewpoint.z));
      set_vector(&view, x, y, -dv);
      sub_vector(&view, &view, &viewpoint);
      norm_vector(&view);
      index = y*w+x;

      t = intersect(&ball, &viewpoint, &view);
      if (t < INFINITY2) {
        vector_t p, tv, n, L;
        mult_vector(&tv, &view, t);
        add_vector(&p, &viewpoint, &tv);
        sub_vector(&n, &p, &ball.center);
        norm_vector(&n);
        sub_vector(&L, &p, &light);
        norm_vector(&L);
        color = shading(&view, &L, &n, &ball.color, 0.5f);
        draw_pixel(&img->buf[index], &color);
      } else {
        draw_pixel(&img->buf[index], &bgcolor);
      }
    }
  }
}

void trace_ray_thread(thread_arg_t* arg) {
  image_t* img = arg->img;
  scene_data_t* scene = arg->scene;
  workarea_t* warea = &arg->warea;

#ifdef __CPU_ZERO
  cpu_set_t mask;
  __CPU_ZERO(&mask);
  __CPU_SET(arg->cpuid, &mask);
  if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
    printf("WARNING: faild to set CPU affinity...(cpuid=%d)\n");
#endif

  trace_ray(img, scene, warea);
  return;
}

void init_trace_thread(int thread_num, thread_arg_t* targ, workarea_t* warea, image_t* img, scene_data_t* scene, int w, int h) {
  int i;

  int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
  for (i = 0; i < thread_num; ++i) {
    set_workarea(&warea[i], 0, w, i*h/thread_num, (i+1)*h/thread_num);
    
    targ[i].cpuid = i %cpu_num;

    targ[i].img = img;
    targ[i].scene = scene;
    targ[i].warea = warea[i];
  }
  return;
}

void start_trace_thread(int thread_num, pthread_t* tid, thread_arg_t* targ) {
  int i;

  for (i = 0; i < thread_num; ++i) {
    pthread_create(&tid[i], NULL, (void*)trace_ray_thread, (void*)&targ[i]);
  }
  return;
}

void wait_trace_thread(int thread_num, pthread_t* tid) {
  int i;
  for (i = 0; i < thread_num; ++i) {
    pthread_join(tid[i], NULL);
  }
  return;
}

int main(int argc, char* argv[]) {
  unsigned int t, time;
  image_t img;
  scene_data_t scene;
  int w = IMG_WIDTH;
  int h = IMG_HEIGHT;
  workarea_t warea[THREAD_NUM];
  thread_arg_t targ[THREAD_NUM];
  pthread_t tid[THREAD_NUM];

  new_image(&img, w, h);
  new_scene(&scene);

  start_timer(&t);

  init_trace_thread(THREAD_NUM, targ, warea, &img, &scene, w, h);
  start_trace_thread(THREAD_NUM, tid, targ);
  wait_trace_thread(THREAD_NUM, tid);

  time = stop_timer(&t);
  print_timer(time);

  write_ppm(&img, OUTPUT);
  delete_image(&img);
  return 0;
}
