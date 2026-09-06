// Copyright 2026 Luke Steuber. MIT License.
// Pure native interpretation of the supplied heptapod/index.html geometry.
// No perfect-freehand implementation or third-party glyph assets are copied.
#pragma once
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define HP_PI 3.14159265358979323846f
#define HP_TAU (2.0f * HP_PI)
#define HP_MAX_POINTS 80
// Use the firmware's fixed-point trig on Pebble. Desktop libm's large-angle
// reduction is not a safe embedded dependency (faulted in __kernel_rem_pio2f).
static inline float hp_sin(float angle) {
#ifdef HP_PEBBLE
  int32_t phase=(int32_t)(angle*(TRIG_MAX_ANGLE/HP_TAU));
  return sin_lookup(phase%TRIG_MAX_ANGLE)/(float)TRIG_MAX_RATIO;
#else
  return sinf(angle);
#endif
}
static inline float hp_cos(float angle) {return hp_sin(angle+HP_PI/2);}
static inline float hp_min(float a,float b){return a<b?a:b;}
static inline float hp_max(float a,float b){return a>b?a:b;}
static inline int hp_round(float v){return (int)(v+(v<0?-.5f:.5f));}
// Positive square root without libm's non-relocatable constant pointers.
static inline float hp_sqrt(float value){
  if(value<=0)return 0;
  union {float f;uint32_t bits;} seed={.f=value};
  seed.bits=(seed.bits>>1)+0x1fc00000;
  float x=seed.f;
  for(int i=0;i<4;i++)x=.5f*(x+value/x);
  return x;
}
// Samples of the original sin(pi*u)^.55 brush pressure and (1-u)^.7 taper.
// Arc pressure interpolates at 1/64 intervals; flourish samples are exact.
static const float HP_PRESSURE[65]={
  0.00000000f,0.19051866f,0.27875080f,0.34800556f,0.40703303f,0.45926565f,0.50646946f,0.54969067f,
  0.58960589f,0.62667973f,0.66124532f,0.69354949f,0.72377976f,0.75208149f,0.77856920f,0.80333424f,
  0.82645032f,0.84797736f,0.86796448f,0.88645210f,0.90347369f,0.91905699f,0.93322507f,0.94599711f,
  0.95738902f,0.96741398f,0.97608283f,0.98340441f,0.98938579f,0.99403251f,0.99734872f,0.99933732f,
  1.00000000f,0.99933732f,0.99734872f,0.99403251f,0.98938579f,0.98340441f,0.97608283f,0.96741398f,
  0.95738902f,0.94599711f,0.93322507f,0.91905699f,0.90347369f,0.88645210f,0.86796448f,0.84797736f,
  0.82645032f,0.80333424f,0.77856920f,0.75208149f,0.72377976f,0.69354949f,0.66124532f,0.62667973f,
  0.58960589f,0.54969067f,0.50646946f,0.45926565f,0.40703303f,0.34800556f,0.27875080f,0.19051866f,
  0.00000000f
};
static const float HP_TAPER[25]={
  1.00000000f,0.97064767f,0.94090983f,0.91076354f,0.88018331f,0.84914069f,0.81760377f,0.78553651f,
  0.75289796f,0.71964119f,0.68571201f,0.65104720f,0.61557221f,0.57919794f,0.54181638f,0.50329416f,
  0.46346306f,0.42210525f,0.37892914f,0.33352711f,0.28529498f,0.23325825f,0.17561966f,0.10810658f,
  0.00000000f
};
static inline float hp_pressure(float u){
  float x=u*64;int i=(int)x;
  if(i>=64)return HP_PRESSURE[64];
  return HP_PRESSURE[i]+(HP_PRESSURE[i+1]-HP_PRESSURE[i])*(x-i);
}
typedef struct { float x,y; } HPPoint;
typedef struct { float cx,cy,r; int text_y; } HPLayout;
// Caller-owned scratch storage keeps Pebble's small task stack clear. The
// callbacks consume each polygon immediately; rendering is not recursive.
typedef struct { HPPoint line[40],outline[HP_MAX_POINTS];float width[40]; } HPWorkspace;
typedef void (*HPPolygon)(const HPPoint *,int,void *);
typedef void (*HPDot)(HPPoint,float,void *);

static const float HP_ARCS[6][4] = {
  {-.42f,1.30f,.95f,.16f},{1.52f,2.66f,.5f,-.5f},{2.55f,4.22f,.82f,.42f},
  {4.10f,4.55f,.4f,.9f},{4.74f,5.55f,.62f,-.28f},{5.42f,6.18f,.78f,.3f}
};
static const float HP_FLOUR[12][5] = {
  {.42f,.52f,.85f,.046f,1},{.86f,.24f,-1.9f,.03f,1},{1.62f,.40f,.6f,.04f,1},
  {2.08f,.16f,1.7f,.026f,-1},{2.72f,.56f,-.72f,.05f,1},{3.30f,.30f,1.35f,.034f,1},
  {3.86f,.22f,-2.f,.03f,1},{4.50f,.48f,.7f,.045f,1},{5.16f,.18f,1.6f,.028f,-1},
  {5.74f,.42f,-.8f,.04f,1},{1.05f,.30f,1.1f,.03f,1},{3.05f,.14f,1.9f,.024f,-1}
};
static const float HP_DOTS[6][3] = {
  {.95f,.17f,.022f},{1.95f,-.12f,.015f},{2.85f,.2f,.018f},
  {4.05f,.15f,.02f},{4.95f,-.14f,.013f},{5.6f,.22f,.016f}
};

static inline HPLayout hp_layout(int w,int h,bool round) {
  float r=hp_min(w*(round?.253f:.28f),(h-42)/3.35f);
  return (HPLayout){w*.5f,h*(round?.46f:.44f),r,h-(w>=190?34:29)};
}
static inline float hp_hour_angle(int h,int m) { return ((h%12)+m/60.f)*HP_TAU/12.f-HP_PI/2; }
static inline float hp_minute_angle(int m,int s) { return (m+s/60.f)*HP_TAU/60.f-HP_PI/2; }
static inline float hp_second_angle(int s) { return s*HP_TAU/60.f-HP_PI/2; }
static inline void hp_time(char *out,unsigned size,int h,int m,bool twelve) {
  if(twelve)snprintf(out,size,"%d:%02d",h%12?h%12:12,m);
  else snprintf(out,size,"%02d:%02d",h,m);
}

// Pressure-expanded ribbon. Center-line tangent controls the width normal;
// unlike a chain of discs this retains a sharp lifted-brush tip.
static inline void hp_ribbon(HPWorkspace *work,int count,HPPolygon emit,void *user) {
  const HPPoint *line=work->line; const float *width=work->width;
  HPPoint *outline=work->outline;
  for(int i=0;i<count;i++){
    int a=i?i-1:i,b=i+1<count?i+1:i;
    float dx=line[b].x-line[a].x,dy=line[b].y-line[a].y;
    float length=hp_max(.001f,hp_sqrt(dx*dx+dy*dy));
    float nx=-dy/length*width[i],ny=dx/length*width[i];
    outline[i]=(HPPoint){line[i].x+nx,line[i].y+ny};
    outline[count*2-i-1]=(HPPoint){line[i].x-nx,line[i].y-ny};
  }
  emit(outline,count*2,user);
}
static inline void hp_arc(HPWorkspace *work,HPLayout l,const float *d,float t,HPPolygon emit,void *user) {
  HPPoint *line=work->line; float *width=work->width;
  int steps=hp_round((d[1]-d[0])*16); if(steps<10)steps=10;
  for(int i=0;i<=steps;i++){
    float u=(float)i/steps,a=d[0]+(d[1]-d[0])*u;
    float wob=.018f*hp_sin(a*3+d[3]*5)+.011f*hp_sin(a*7+1.3f)+.006f*hp_sin(a*5-t/.95f+d[3]*4);
    float rr=l.r*(1+d[3]*.05f+wob);
    float pressure=d[2]*(.22f+.78f*hp_pressure(u));
    line[i]=(HPPoint){l.cx+hp_cos(a)*rr,l.cy+hp_sin(a)*rr};
    width[i]=l.r*.165f*(.19f+.62f*pressure);
    if(i==0||i==steps)width[i]*=.35f;
  }
  hp_ribbon(work,steps+1,emit,user);
}
static inline void hp_flourish(HPWorkspace *work,HPLayout l,float a,float length,float curl,float size,float dir,float t,HPPolygon emit,void *user) {
  HPPoint *line=work->line; float *width=work->width;
  for(int i=0;i<=24;i++){
    float u=i/24.f;
    float aa=a+curl*u+.045f*hp_sin(t/1.25f+a*1.7f)*u;
    float rr=l.r+dir*length*u+l.r*.01f*hp_sin(t/1.7f+a)*u;
    line[i]=(HPPoint){l.cx+hp_cos(aa)*rr,l.cy+hp_sin(aa)*rr};
    width[i]=size*(.1f+.8f*HP_TAPER[i]);
    if(i==24)width[i]=0;
  }
  hp_ribbon(work,25,emit,user);
}
static inline void hp_glyph(HPWorkspace *work,HPLayout l,int h,int m,int s,float t,HPPolygon emit,HPDot dot,void *user) {
  l.r*=1+.006f*hp_sin(t/1.6f);
  l.cx+=l.r*(.03f*hp_sin(t/4.2f)+.013f*hp_sin(t/2.3f+1.1f));
  l.cy+=l.r*(.028f*hp_cos(t/3.7f)+.012f*hp_cos(t/2.6f+.6f));
  for(int i=0;i<6;i++)hp_arc(work,l,HP_ARCS[i],t,emit,user);
  for(int i=0;i<12;i++){
    const float *d=HP_FLOUR[i];
    hp_flourish(work,l,d[0],l.r*d[1],d[2],l.r*d[3],d[4],t,emit,user);
  }
  for(int i=0;i<6;i++){
    const float *d=HP_DOTS[i]; float rr=l.r*(1+d[1]);
    dot((HPPoint){l.cx+hp_cos(d[0])*rr,l.cy+hp_sin(d[0])*rr},hp_max(1,l.r*d[2]),user);
  }
  hp_flourish(work,l,hp_hour_angle(h,m),l.r*.6f,-.5f,l.r*.05f,1,t,emit,user);
  hp_flourish(work,l,hp_minute_angle(m,s),l.r*.46f,.55f,l.r*.042f,1,t,emit,user);
  hp_flourish(work,l,hp_second_angle(s),l.r*.32f,-.7f,l.r*.032f,1,t,emit,user);
}
