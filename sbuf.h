#ifndef SBUF_H
#define SBUF_H

/* config area
 */

#ifndef SBF_SH
#ifdef N3D_SH
#define SBF_SH N3D_SH
#else
#define SBF_SH (240)
#endif
#endif

#ifndef SBF_SW
#ifdef N3D_SW
#define SBF_SW N3D_SH
#else
#define SBF_SW (240)
#endif
#endif

#ifndef SBF_MAX_SECTIONS
#define SBF_MAX_SECTIONS (8)
#endif

/* end of config area
 */


#include <stdint.h>


#ifdef N3D_SW
_Static_assert(N3D_SW==SBF_SW&&N3D_SH==SBF_SH,"nighty3d dimensions are different than sbuf dimensions!");
#endif


typedef void fill_scanline_type(uint8_t *buf, int y, int x0, int x1);


void sbf_init(void);
void sbf_render_frame(fill_scanline_type *fill_scanline, int flip);
void sbf_add_segment(int y, int x, int w, uint16_t col, int flip);


#ifdef SBUF_IMPLEMENTATION


#define SBF_SET(x,w,col) ( (((uint32_t)(x))&0xff)<<24 | ((uint32_t)(w)&0xff)<<16 | (((uint32_t)(col))&0xffff) )
#define SBF_GETX(sl) ((uint8_t)(((sl)>>24)&0xff))
#define SBF_GETW(sl) ((uint8_t)(((sl)>>16)&0xff))
#define SBF_GETCOL(sl) ((uint16_t)((sl)&0xffff))
static uint32_t sbf_scanline[SBF_SH][SBF_MAX_SECTIONS];

#if SBF_MAX_SECTIONS<16

static uint8_t sbf_section_cnt[SBF_SH/2]; // one nibble for scanline
#define SBF_GETCNT(slnc,y) ((slnc[(y)/2]>>(((y)%2)!=0?0:4))&0x0f)
#define SBF_SETCNT(slnc,y,len) do { static uint8_t mask[2]={0x0f,0xf0};  slnc[(y)/2]&=mask[(y)%2]; slnc[(y)/2]|=(len)<<(4*!(y%2)); } while(0)

#elif SBF_MAX_SECTIONS<256

static uint8_t sbf_section_cnt[SBF_SH];
#define SBF_GETCNT(slnc,y) (slnc[(y)])
#define SBF_SETCNT(slnc,y,len) do { slnc[(y)]=(len); } while(0)

#else

#error SBF_MAX_SECTIONS too large

#endif

void sbf_init(void)
{
  memset(sbf_section_cnt,0,sizeof(sbf_section_cnt));
}

static void sbf_render_line(int y, fill_scanline_type *fill_scanline, int flip)
{
  static uint16_t sbf_line_buf[2][SBF_SW];
  static int sbf_line_buf_idx=0;
  int x0=SBF_SW, x1=-1;
  int x, xx;
  uint16_t col;

  if(y<0||y>SBF_SH) return;
  if((y%2)!=flip) sbf_line_buf_idx^=1;
  int cnt=SBF_GETCNT(sbf_section_cnt,y);
  for(int i=0; i<cnt; i++)
  {
    x=SBF_GETX(sbf_scanline[y][i]);
    xx=x+SBF_GETW(sbf_scanline[y][i]);
    if(x<x0) x0=x;
    if(xx>x1) x1=xx;
    if((y%2)==flip) continue;
    col=SBF_GETCOL(sbf_scanline[y][i]);
    for(;x<(xx-1);x++) sbf_line_buf[sbf_line_buf_idx][x]=col;
  }
  if(x1>=0)
  {
    if((y%2)!=flip)
    {
      // render horizontal line from line_buf[x0..x1]
      fill_scanline((uint8_t *)(&sbf_line_buf[sbf_line_buf_idx][x0]), y, x0, x1);
      sbf_scanline[y][0]=SBF_SET(x0,x1-x0+1,0);
      SBF_SETCNT(sbf_section_cnt,y,1);
    }
    else
    {
      // clear from x0..x1
      fill_scanline(NULL, y, x0, x1);
      SBF_SETCNT(sbf_section_cnt,y,0);
    }
  }
}

void sbf_render_frame(fill_scanline_type *fill_scanline, int flip)
{
  for(int y=0;y<SBF_SH/2;y++)
  {
    sbf_render_line(y, fill_scanline, flip);
    sbf_render_line(SBF_SH-y-1, fill_scanline, flip);
  }
}

void sbf_add_segment(int y, int x, int w, uint16_t col, int flip)
{
  if((y%2)!=flip)
  {
    int cnt=SBF_GETCNT(sbf_section_cnt,y);
    if(cnt>=SBF_MAX_SECTIONS) return;
    SBF_SETCNT(sbf_section_cnt,y,cnt+1);
    sbf_scanline[y][cnt]=SBF_SET(x,w,col);
  }
}


#endif
#endif
