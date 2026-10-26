/* lane.h : lane detection on an 80x60 "lane feature" frame (ALPANA_SOC, plain C + FPU)
 *
 * The ESP32-CAM sends an 80x60 image in which lane paint is bright:
 * luma + a boost for yellow paint (esp32cam_spi_slave.ino, LANE_FEATURE).
 * Dark tape on a light floor is detected automatically (polarity switch).
 */
#ifndef LANE_H
#define LANE_H
#include <stdint.h>

#define LW 80                     /* thumbnail width  */
#define LH 60                     /* thumbnail height */
#define LANE_ROWS 12              /* scan rows in the bottom part of the image */
#define LANE_Y_TOP 26             /* highest scan row (look-ahead) */
#define LANE_Y_BOT 58             /* lowest scan row (closest to the car) */
#define STEER_MAX_DEG 30

enum { DIR_STRAIGHT = 0, DIR_LEFT = 1, DIR_RIGHT = 2, DIR_LOST = 3 };

typedef struct {
  /* fitted lane lines as x = a + b*y (pixels)
     *_ok: 0 = none, 1 = measured in this frame, 2 = held from the last frames */
  int   left_ok, right_ok;
  float la, lb, ra, rb;
  /* lane centre at the look-ahead row and at the bottom row */
  float cx_top, cx_bot;
  /* raw detections per scan row (-1 = none) */
  int16_t ly[LANE_ROWS], lx[LANE_ROWS], rx[LANE_ROWS];
  /* output */
  int   steer_deg;                /* -30 (full left) .. +30 (full right) */
  int   dir;                      /* DIR_* */
  int   dark_lines;               /* 1 = dark tape on light floor, 0 = bright paint on dark road */
  int   thr;                      /* mean detection threshold (grey levels above the local road) */
  int   noise;                    /* mean road noise (grey levels) */
} lane_result_t;

void lane_init(void);
void lane_detect(const uint8_t img[LH][LW], lane_result_t *r);
#endif
