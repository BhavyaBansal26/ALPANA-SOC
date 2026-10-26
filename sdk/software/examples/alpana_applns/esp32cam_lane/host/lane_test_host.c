/* lane_test_host.c : run lane_detect() (../lane.c) on the PC over a sequence of 80x60 frames
 *   gcc -O2 -I.. -o lane_test_host lane_test_host.c ../lane.c -lm
 *   ./lane_test_host frames.raw N   -> one line per frame:
 *   dir steer left_ok right_ok la lb ra rb ROWS (y lx rx)*ROWS            */
#include <stdio.h>
#include <stdlib.h>
#include "lane.h"
int main(int argc, char **argv) {
  static uint8_t img[LH][LW]; lane_result_t r;
  if (argc < 3) { fprintf(stderr, "usage: %s frames.raw nframes\n", argv[0]); return 1; }
  FILE *f = fopen(argv[1], "rb"); int n = atoi(argv[2]);
  if (!f) return 1;
  lane_init();
  for (int k = 0; k < n; k++) {
    if (fread(img, 1, sizeof img, f) != sizeof img) return 1;
    lane_detect(img, &r);
    printf("%d %d %d %d %f %f %f %f %d", r.dir, r.steer_deg, r.left_ok, r.right_ok, r.la, r.lb, r.ra, r.rb, LANE_ROWS);
    for (int i = 0; i < LANE_ROWS; i++) printf(" %d %d %d", r.ly[i], r.lx[i], r.rx[i]);
    printf("\n");
  }
  fclose(f);
  return 0;
}
