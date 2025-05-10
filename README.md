# sbuf
Scanline buffering for generic rendering engines on serial displays

## Highlights

- Scanline based buffering
- No dynamic memory allocation
- Minimal RAM needed (two lines)
- Double buffering to support DMA
- Overlappig segments supported
- Interlacing

## Configuration

name             | default | note
-----------------|---------|------------
SBF_SW           | 240     | width
SBF_SH           | 240     | height
SBF_MAX_SECTIONS |   8     | max number of sections in a scanline, depends on the complexity of the object

## RAM

- two scanlines -- SBF_SW * 2
- scanline buffer -- SBF_SH * SBF_MAX_SECTIONS * 4 bytes
- work array -- SBF_SH / 2 bytes

## Usage

1. define ```SBUF_IMPLEMENTATION```
2. include ```sbuf.h```
3. implement the callback function
   ```c
   void fill_scanline_type(uint8_t *buf, int y, int x0, int x1)
   {
     if( NULL == buf ) {
       // clear this scanline 'y' between x0 and x1 including x0 and x1
       // or render the background image to this area
     } else {
       // render the provided 'buf' to line 'y' from x0 to x1
     }
   }
   ```
5. call ```sbf_init()``` once
6. render your scene calling ```sbf_add_segment()``` as many times as needed
   _NOTE: No more than ```SBF_MAX_SECTIONS``` calls per scanline!_
8. call ```sbf_render_frame()``` to render the result
9. if interlacing is used, negate the flip marker

## Demo

https://github.com/user-attachments/assets/e198a00d-b2b0-4b1a-beb6-43dda7f9212d

_also using the [ninety3d](https://github.com/gega/ninety3d) library_
