// Copyright 2026 Luke Steuber. MIT License.
#include <pebble.h>
#include <string.h>
#define DEFAULT_DIGITAL true
#include "settings.h"
#define HP_PEBBLE
#include "geometry.h"

static Window *s_window;
static Layer *s_layer;
static GPoint s_points[HP_MAX_POINTS];
static GPath s_path={.points=s_points,.rotation=0,.offset={0,0}};
static uint32_t s_frame;
static int s_last_second=-1;
static HPWorkspace s_work;

static void polygon(const HPPoint *p,int n,void *user){
  s_path.num_points=n;
  for(int i=0;i<n;i++)s_points[i]=GPoint(hp_round(p[i].x),hp_round(p[i].y));
  gpath_draw_filled((GContext *)user,&s_path);
}
static void dot(HPPoint p,float radius,void *user){
  graphics_fill_circle((GContext *)user,GPoint(hp_round(p.x),hp_round(p.y)),hp_round(radius));
}
// Half-resolution smoke masks keep all six targets within bounded memory.
#define MW ((PBL_DISPLAY_WIDTH+1)/2)
#define MH ((PBL_DISPLAY_HEIGHT+1)/2)
static uint8_t s_mask[MW*MH],s_blur[MW*MH],s_temp[MW*MH];
static AppTimer *s_timer;
static bool s_focus=true;
static void smoke_polygon(const HPPoint *p,int n,void *user){
  (void)user;
  float cross[HP_MAX_POINTS];
  int lo=MH-1,hi=0;
  for(int i=0;i<n;i++){int y=(int)(p[i].y/2);if(y<lo)lo=y;if(y>hi)hi=y;}
  if(lo<0)lo=0;
  if(hi>=MH)hi=MH-1;
  for(int y=lo;y<=hi;y++){
    float row=y*2+1;int count=0;
    for(int i=0,j=n-1;i<n;j=i++)if((p[i].y>row)!=(p[j].y>row)){
      float x=(p[i].x+(row-p[i].y)*(p[j].x-p[i].x)/(p[j].y-p[i].y))/2;
      int k=count++;while(k>0 && cross[k-1]>x){cross[k]=cross[k-1];k--;}cross[k]=x;
    }
    for(int i=0;i+1<count;i+=2){int left=(int)cross[i],right=(int)cross[i+1];
      if(left<0)left=0;
      if(right>=MW)right=MW-1;
      for(int x=left;x<=right;x++)s_mask[y*MW+x]=255;
    }
  }
}
static void smoke_dot(HPPoint p,float radius,void *user){
  (void)user;int cx=p.x/2,cy=p.y/2,r=hp_max(1,radius/2);
  for(int y=cy-r;y<=cy+r;y++)for(int x=cx-r;x<=cx+r;x++)
    if(x>=0&&x<MW&&y>=0&&y<MH&&(x-cx)*(x-cx)+(y-cy)*(y-cy)<=r*r)s_mask[y*MW+x]=255;
}
static void blur(int radius){
  int span=radius*2+1;
  for(int y=0;y<MH;y++){
    int sum=0;for(int x=0;x<radius;x++)sum+=s_mask[y*MW+x];
    for(int x=0;x<MW;x++){
      if(x+radius<MW)sum+=s_mask[y*MW+x+radius];
      if(x-radius-1>=0)sum-=s_mask[y*MW+x-radius-1];
      s_temp[y*MW+x]=sum/span;
    }
  }
  for(int x=0;x<MW;x++){
    int sum=0;for(int y=0;y<radius;y++)sum+=s_temp[y*MW+x];
    for(int y=0;y<MH;y++){
      if(y+radius<MH)sum+=s_temp[(y+radius)*MW+x];
      if(y-radius-1>=0)sum-=s_temp[(y-radius-1)*MW+x];
      s_blur[y*MW+x]=sum/span;
    }
  }
}
static void fog(GContext *ctx,GRect b,HPLayout l,float t){
  GBitmap *fb=graphics_capture_frame_buffer(ctx);if(!fb)return;
  static const int bayer[4][4]={{0,8,2,10},{12,4,14,6},{3,11,1,9},{15,7,13,5}};
  int clouds[4][3];
  for(int i=0;i<4;i++){
    clouds[i][0]=b.size.w*(.2f+i*.2f+.12f*hp_sin(t/(4.3f+i)+i));
    clouds[i][1]=b.size.h*(.2f+(i%2)*.5f+.1f*hp_cos(t/(3.4f+i)+i));
    clouds[i][2]=b.size.w*(.5f+i*.06f);
  }
  for(int y=0;y<b.size.h;y++){
    GBitmapDataRowInfo row=gbitmap_get_data_row_info(fb,y);
    int dy=(y-l.cy)*256/b.size.h;
    for(int x=row.min_x;x<=row.max_x;x++){
      int dx=(x-l.cx)*256/b.size.w;
      int light=239-85*(dx*dx+dy*dy)/65536;
      for(int i=0;i<4;i++){
        int xx=x-clouds[i][0],yy=y-clouds[i][1],rr=clouds[i][2]*clouds[i][2];
        int density=rr-xx*xx-yy*yy;if(density>0)light-=18*density/rr;
      }
      // Broad smoke wash plus a denser, soft penumbra under the actual ink.
      light-=s_blur[(y/2)*MW+x/2]*3/4;
      if(light<0)light=0;
      if(light>255)light=255;
      int threshold=bayer[y%4][x%4];
#ifdef PBL_COLOR
      int shade=light*3/255;if(shade<3 && (light*3%255)*16>threshold*255)shade++;
      row.data[x]=0xc0|shade*21;
#else
      uint8_t bit=1u<<(x&7);
      if(light*16>threshold*255)row.data[x>>3]|=bit;else row.data[x>>3]&=(uint8_t)~bit;
#endif
    }
  }
  graphics_release_frame_buffer(ctx,fb);
}
static void draw(Layer *layer,GContext *ctx){
  uint16_t begin_ms; time_t begin; time_ms(&begin,&begin_ms);
  GRect b=layer_get_bounds(layer);
  HPLayout l=hp_layout(b.size.w,b.size.h,PBL_IF_ROUND_ELSE(true,false));
  l.cy=s_digital?(b.size.h-(b.size.w>=200?38:32))*.5f:b.size.h*.5f;
  float t=s_calm?0:(float)(begin%86400)+begin_ms/1000.f;
  time_t epoch=begin; struct tm now=*localtime(&epoch);
  memset(s_mask,0,sizeof(s_mask));
  hp_glyph(&s_work,l,now.tm_hour,now.tm_min,now.tm_sec,t,smoke_polygon,smoke_dot,NULL);
  blur(b.size.w>=200?4:3);
  fog(ctx,b,l,t);
  graphics_context_set_antialiased(ctx,true);
  graphics_context_set_fill_color(ctx,GColorBlack);
  hp_glyph(&s_work,l,now.tm_hour,now.tm_min,now.tm_sec,t,polygon,dot,ctx);
  digital_draw(ctx,b,&now,GColorBlack);
  bool sample=s_frame++<3 || s_last_second!=now.tm_hour*3600+now.tm_min*60+now.tm_sec;
  s_last_second=now.tm_hour*3600+now.tm_min*60+now.tm_sec;
  if(sample){
    time_t end;uint16_t end_ms;time_ms(&end,&end_ms);
    APP_LOG(APP_LOG_LEVEL_INFO,"Heptapod %dx%d time=%02d:%02d:%02d frame_ms=%ld free=%lu digital=%d calm=%d frames=%lu",b.size.w,b.size.h,
      now.tm_hour,now.tm_min,now.tm_sec,
      (long)((end-begin)*1000+end_ms-begin_ms),(unsigned long)heap_bytes_free(),s_digital,s_calm,(unsigned long)s_frame);
  }
}
static void cancel_timer(void){if(s_timer){app_timer_cancel(s_timer);s_timer=NULL;}}
static void animate(void *context){(void)context;s_timer=NULL;if(!s_focus||s_calm)return;if(s_layer)layer_mark_dirty(s_layer);s_timer=app_timer_register(250,animate,NULL);}
static void settings_changed(void){cancel_timer();if(s_focus&&s_layer){layer_mark_dirty(s_layer);if(!s_calm)s_timer=app_timer_register(250,animate,NULL);}}
static void focus(bool focused){s_focus=focused;settings_changed();}
static void tick(struct tm *now,TimeUnits changed){(void)now;(void)changed;if(s_focus&&s_layer&&s_calm)layer_mark_dirty(s_layer);}
static void load(Window *window){
  s_layer=layer_create(layer_get_bounds(window_get_root_layer(window)));
  if(!s_layer)return;
  layer_set_update_proc(s_layer,draw);layer_add_child(window_get_root_layer(window),s_layer);
  tick_timer_service_subscribe(SECOND_UNIT,tick);app_focus_service_subscribe(focus);settings_changed();
}
static void unload(Window *window){(void)window;cancel_timer();app_focus_service_unsubscribe();tick_timer_service_unsubscribe();layer_destroy(s_layer);s_layer=NULL;}
int main(void){settings_init();
  s_window=window_create();if(!s_window)return 1;
  window_set_window_handlers(s_window,(WindowHandlers){.load=load,.unload=unload});
  window_stack_push(s_window,false);app_event_loop();window_destroy(s_window);return 0;
}
