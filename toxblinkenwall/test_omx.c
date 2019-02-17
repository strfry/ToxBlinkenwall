#include "omx.h"

#include <assert.h>

// https://github.com/raspberrypi/userland/blob/master/host_applications/linux/apps/hello_pi/hello_encode/encode.c
#define WIDTH     320
#define HEIGHT    240
#define STRIDE    320

// generate an animated test card in YUV format
static int
generate_test_card(void *buf, uint32_t* filledLen, int frame)
{
   int i, j;
   char *y = buf;
   char *u = y + STRIDE * HEIGHT;
   char *v = u + (STRIDE >> 1) * (HEIGHT >> 1);

   for (j = 0; j < HEIGHT / 2; j++) {
      char *py = y + 2 * j * STRIDE;
      char *pu = u + j * (STRIDE>> 1);
      char *pv = v + j * (STRIDE>> 1);
      for (i = 0; i < WIDTH / 2; i++) {
         int z = (((i + frame) >> 4) ^ ((j + frame) >> 4)) & 15;
         py[0] = py[1] = py[STRIDE] = py[STRIDE + 1] = 0x80 + z * 0x8;
         pu[0] = 0x00 + z * 0x10;
         pv[0] = 0x80 + z * 0x30;
         py += 2;
         pu++;
         pv++;
      }
   }
   *filledLen = (STRIDE* HEIGHT * 3) >> 1;
   return 1;
}



int main()
{
    struct omx_state omx;
    void* pbuf;
    uint32_t buf_len, fill_len, i;
    int ret;

    i = 100;

    debug_log();

    ret = omx_init(&omx);
    assert(ret == 0);

    ret = omx_display_enable(&omx, WIDTH, HEIGHT, STRIDE);
    assert(ret == 0);

    while (i--) {
        ret = omx_display_input_buffer(&omx, &pbuf, &buf_len);
        assert(ret == 0);

        generate_test_card(pbuf, &fill_len, i);
        assert(fill_len == buf_len);

        ret = omx_display_flush_buffer(&omx);
        assert(ret == 0);
    }
    
}

