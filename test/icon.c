#include "../src/c/geometry.h"
#include <stdlib.h>
static void polygon(const HPPoint *points,int n,void *user){
  FILE *out=user;fputs("<polygon points=\"",out);
  for(int i=0;i<n;i++)fprintf(out,"%.3f,%.3f ",points[i].x,points[i].y);
  fputs("\"/>\n",out);
}
static void dot(HPPoint p,float r,void *user){fprintf(user,"<circle cx=\"%.3f\" cy=\"%.3f\" r=\"%.3f\"/>\n",p.x,p.y,r);}
int main(int argc,char **argv){
  if(argc!=2)return 1;FILE *out=fopen(argv[1],"w");if(!out)return 2;
  fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\"><rect width=\"100\" height=\"100\" fill=\"white\"/><g fill=\"black\">\n",out);
  HPWorkspace work;
  hp_glyph(&work,(HPLayout){50,50,28,0},10,8,20,0,polygon,dot,out);
  fputs("</g></svg>\n",out);fclose(out);return 0;
}
