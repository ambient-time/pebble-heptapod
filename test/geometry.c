#include <assert.h>
#include <string.h>
#ifdef HP_TEST_FIXED_TRIG
#include <math.h>
#include <stdint.h>
#define HP_PEBBLE
#define TRIG_MAX_ANGLE 65536
#define TRIG_MAX_RATIO 65535
static int32_t sin_lookup(int32_t phase){
  return (int32_t)roundf(sinf(phase*6.283185307179586f/TRIG_MAX_ANGLE)*TRIG_MAX_RATIO);
}
#endif
#include "../src/c/geometry.h"
static int w,h,count,dots;
static bool round_face;
static double signature;
static void check_point(HPPoint p,float radius){
  assert(isfinite(p.x)&&isfinite(p.y));
  assert(p.x-radius>=0&&p.x+radius<w&&p.y-radius>=0&&p.y+radius<h);
  if(round_face){float x=p.x-w/2.f,y=p.y-h/2.f;assert(sqrtf(x*x+y*y)+radius<w/2.f);}
}
static void poly(const HPPoint *p,int n,void *user){
  (void)user;assert(n>=3&&n<=HP_MAX_POINTS);count++;
  for(int i=0;i<n;i++){
    check_point(p[i],0);
    signature+=(i+1)*p[i].x+(n-i)*p[i].y;
  }
}
static void dot(HPPoint p,float r,void *user){(void)user;dots++;check_point(p,r);}
int main(void){
  HPWorkspace work;
  for(float value=.0001f;value<10000;value*=1.1f){
    assert(fabsf(hp_sqrt(value)-sqrtf(value))<sqrtf(value)*.00001f);
  }
  for(int arc=0;arc<6;arc++){
    int steps=hp_round((HP_ARCS[arc][1]-HP_ARCS[arc][0])*16);
    if(steps<10)steps=10;
    for(int i=0;i<=steps;i++){
      float u=(float)i/steps,reference=powf(fmaxf(0,sinf(HP_PI*u)),.55f);
      assert(fabsf(hp_pressure(u)-reference)<.003f);
    }
  }
  const int sizes[][2]={{144,168},{200,228},{180,180},{260,260}};
  char text[8];hp_time(text,sizeof(text),0,0,true);assert(!strcmp(text,"12:00"));
  hp_time(text,sizeof(text),0,0,false);assert(!strcmp(text,"00:00"));
  hp_time(text,sizeof(text),12,59,true);assert(!strcmp(text,"12:59"));
  hp_time(text,sizeof(text),23,59,false);assert(!strcmp(text,"23:59"));
  assert(fabsf(hp_hour_angle(0,0)+HP_PI/2)<.0001f);
  assert(fabsf(hp_hour_angle(3,0))<.0001f);
  assert(fabsf(hp_minute_angle(30,0)-HP_PI/2)<.0001f);
  assert(fabsf(hp_second_angle(45)-HP_PI)<.0001f);
  for(int d=0;d<4;d++){
    w=sizes[d][0];h=sizes[d][1];round_face=w==h;
    for(int second=0;second<86400;second+=37){
      count=dots=0;
      hp_glyph(&work,hp_layout(w,h,round_face),second/3600,second/60%60,second%60,(float)second,poly,dot,NULL);
      assert(count==21&&dots==6);
    }
  }
  w=200;h=228;round_face=false;signature=0;
  hp_glyph(&work,hp_layout(w,h,false),10,8,20,0,poly,dot,NULL);
  double first=signature;signature=0;
  hp_glyph(&work,hp_layout(w,h,false),10,8,21,0,poly,dot,NULL);
  assert(fabs(signature-first)>1); // Time markers move even with atmosphere frozen.
  puts("Heptapod geometry: 9,344 time/profile samples fit; 21 ribbons, six blots; time format and cardinal angles pass.");
}
