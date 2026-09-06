// Copyright 2026 Luke Steuber. MIT License.
#include <pebble.h>
#define HP_PEBBLE
#include "geometry.h"

static Window *s_window;
static Layer *s_layer;
static GPoint s_points[HP_MAX_POINTS];
static GPath s_path={.points=s_points,.rotation=0,.offset={0,0}};
static uint32_t s_frame;
static int s_last_minute=-1;
static HPWorkspace s_work;

static void polygon(const HPPoint *p,int n,void *user){
  s_path.num_points=n;
  for(int i=0;i<n;i++)s_points[i]=GPoint(hp_round(p[i].x),hp_round(p[i].y));
  gpath_draw_filled((GContext *)user,&s_path);
}
static void dot(HPPoint p,float radius,void *user){
  graphics_fill_circle((GContext *)user,GPoint(hp_round(p.x),hp_round(p.y)),hp_round(radius));
}
static void fog(GContext *ctx,GRect b,HPLayout l){
  graphics_context_set_fill_color(ctx,GColorWhite);
  graphics_fill_rect(ctx,b,0,GCornerNone);
#ifdef PBL_COLOR
  // Static, lightly dithered backlight: keep ink edges crisp at this resolution.
  // Paint only the visible rows on circular screens; never assume full-width RAM.
  GBitmap *fb=graphics_capture_frame_buffer(ctx);
  if(!fb)return;
  static const int bayer[4][4]={{0,8,2,10},{12,4,14,6},{3,11,1,9},{15,7,13,5}};
  const int cx=(int)l.cx,cy=(int)l.cy;
  for(int y=0;y<b.size.h;y++){
    GBitmapDataRowInfo row=gbitmap_get_data_row_info(fb,y);
    int dy=(y-cy)*256/b.size.h;
    for(int x=row.min_x;x<=row.max_x;x++){
      int dx=(x-cx)*256/b.size.w;
      int threshold=2+24*(dx*dx+dy*dy)/65536;
      row.data[x]=(bayer[y%4][x%4]<threshold?GColorLightGray:GColorWhite).argb;
    }
  }
  graphics_release_frame_buffer(ctx,fb);
#else
  (void)l;
#endif
}
static void draw(Layer *layer,GContext *ctx){
  uint16_t begin_ms; time_t begin; time_ms(&begin,&begin_ms);
  GRect b=layer_get_bounds(layer);
  HPLayout l=hp_layout(b.size.w,b.size.h,PBL_IF_ROUND_ELSE(true,false));
  fog(ctx,b,l);
  graphics_context_set_antialiased(ctx,true);
  graphics_context_set_fill_color(ctx,GColorBlack);
  time_t epoch=time(NULL); struct tm now=*localtime(&epoch);
  float t=(float)(epoch%86400);
  hp_glyph(&s_work,l,now.tm_hour,now.tm_min,now.tm_sec,t,polygon,dot,ctx);
  char label[8]; hp_time(label,sizeof(label),now.tm_hour,now.tm_min,!clock_is_24h_style());
  graphics_context_set_text_color(ctx,GColorBlack);
  graphics_draw_text(ctx,label,fonts_get_system_font(b.size.w>=190?FONT_KEY_GOTHIC_24:FONT_KEY_GOTHIC_18),
    GRect(0,l.text_y,b.size.w,30),GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
  bool sample=s_frame++<3 || s_last_minute!=now.tm_hour*60+now.tm_min;
  s_last_minute=now.tm_hour*60+now.tm_min;
  if(sample){
    time_t end;uint16_t end_ms;time_ms(&end,&end_ms);
    APP_LOG(APP_LOG_LEVEL_INFO,"Heptapod %dx%d time=%02d:%02d:%02d frame_ms=%ld free=%lu",b.size.w,b.size.h,
      now.tm_hour,now.tm_min,now.tm_sec,
      (long)((end-begin)*1000+end_ms-begin_ms),(unsigned long)heap_bytes_free());
  }
}
static void tick(struct tm *now,TimeUnits changed){(void)now;(void)changed;layer_mark_dirty(s_layer);}
static void load(Window *window){
  s_layer=layer_create(layer_get_bounds(window_get_root_layer(window)));
  if(!s_layer)return;
  layer_set_update_proc(s_layer,draw);layer_add_child(window_get_root_layer(window),s_layer);
  tick_timer_service_subscribe(SECOND_UNIT,tick);
}
static void unload(Window *window){(void)window;tick_timer_service_unsubscribe();layer_destroy(s_layer);s_layer=NULL;}
int main(void){
  s_window=window_create();if(!s_window)return 1;
  window_set_window_handlers(s_window,(WindowHandlers){.load=load,.unload=unload});
  window_stack_push(s_window,false);app_event_loop();window_destroy(s_window);return 0;
}
