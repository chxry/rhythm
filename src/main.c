#include <gint/gint.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <gint/cpu.h>
#include <stdlib.h>

extern bopti_image_t img_miku;

#define LANE_WIDTH DWIDTH / 6
#define MAX_TILES 20
#define TILE_HEIGHT 20
#define PX_TICKS 5
#define TARGET_PX 200
#define TARGET_TICKS TARGET_PX * PX_TICKS

typedef struct {
	char lane; // starts at 1
	int tick;
} tile;

typedef struct {
	int offset;
	int score;
	char* msg;
} window;

int main() {
	gint_setrestart(1);
	
	volatile int update_tick = 0; // 1000 hz
	volatile int render_tick = 0; // 10 hz - todo: calculate optimal fps
	unsigned int tick = 0;
	unsigned int frame = 0;
	timer_start(timer_configure(TIMER_TMU, 1000, GINT_CALL_SET(&update_tick)));
	timer_start(timer_configure(TIMER_TMU, 100000, GINT_CALL_SET(&render_tick)));

	char input = 0;
	tile tiles[MAX_TILES] = {{.lane = 0}};
	char map[] = {0b00000010, 0b00001000, 0b00000100, 0b00010000, 0, 0, 0b00001010, 0b000010100, 0b00001010};
	int score = 0;
	int combo = 0;
	char* msg = NULL;
	
	for (;;) {
		if (update_tick) {
			update_tick = 0;

			clearevents();
			if (keydown(KEY_EXIT)) {
				break;
			}

			input = 0;
			input |= keydown(KEY_F1);
			input |= keydown(KEY_F2) << 1;
			input |= keydown(KEY_F3) << 2;
			input |= keydown(KEY_F4) << 3;
			input |= keydown(KEY_F5) << 4;
			input |= keydown(KEY_F6) << 5;

			window windows[] = {
				{.offset = 15, .score = 300, .msg = "PERFECT"}, 
				{.offset = 30, .score = 200, .msg = "GREAT"},
				{.offset = 60, .score = 100, .msg = "GOOD"},
				{.offset = 90, .score = 50, .msg = "OK"},
				{.offset = 120, .score = 0, .msg = "MISS"}
			};
			#define MISS_WINDOW 4
			for (int t = 0; t < MAX_TILES; t++) {
				if (tiles[t].lane > 0) {
					int lane = (tiles[t].lane - 1);

					int delta = tick - (tiles[t].tick + TARGET_TICKS);
					if (delta > windows[MISS_WINDOW].offset) {
						tiles[t].lane = 0;
						msg = windows[MISS_WINDOW].msg;
						combo = 0;
						continue;
					}
					
					delta = abs(delta);
					if (input & (1 << lane)) {
						for (int w = 0; w <= MISS_WINDOW; w++) {
							if (delta < windows[w].offset) {
								tiles[t].lane = 0;
								score += windows[w].score;
								msg = windows[w].msg;
								if (w == MISS_WINDOW) {
									combo = 0;
								} else {
									combo++;
								}
								break;
							}
						}
					}
				}
			}

			int mspb = 162;
			if (tick % mspb == 0) {
				if (tick / mspb < sizeof(map)) {
					char row = map[tick / mspb];
					int min_free = 0;
					for (int l = 0; l < 6; l++) {
						if (row & (1 << l)) {
							for (int t = min_free; t < MAX_TILES; t++) {
								if (tiles[t].lane > 0) {
									continue;
								}
								tiles[t] = (tile){ .lane = l + 1, .tick = tick };
								min_free = t;
								break;
							}
						}
					}
				}
			}

			tick++;
		}
		if (render_tick) {
			render_tick = 0;
			
			// dclear(C_WHITE);
			dimage(0,0, &img_miku);
			dprint(1, 1, C_BLACK, "tick %u frame %u", tick, frame);
			dprint_opt(DWIDTH, 1, C_BLACK, C_NONE, DTEXT_RIGHT, DTEXT_TOP, "score %i", score);
			dprint_opt(DWIDTH, 10, C_BLACK, C_NONE, DTEXT_RIGHT, DTEXT_TOP, "combo %i", combo);
			dhline(TARGET_PX, C_LIGHT);
			// todo: slanted rendering
			for (int l = 0; l < 6; l++) {
					int x = l * LANE_WIDTH;
					
					if (l > 0) {
						dvline(x, C_LIGHT);
					}
					
					if (input & (1 << l)) {
						drect(x + 1, TARGET_PX - 1, x + LANE_WIDTH - 1, DHEIGHT, C_BLUE);
					}
			}
			for (int t = 0; t < MAX_TILES; t++) {
				if (tiles[t].lane > 0) {
					int x = (tiles[t].lane - 1) * LANE_WIDTH;
					int y = (tick - tiles[t].tick) / PX_TICKS;
					drect(x + 1, y, x + LANE_WIDTH - 1, y + TILE_HEIGHT, C_DARK);
				}
			}
			if (msg != NULL) {
				dtext_opt(DWIDTH / 2, DHEIGHT * 2 / 3, C_BLUE, C_NONE, DTEXT_CENTER, DTEXT_TOP, msg);
				msg = NULL;
			}
			
			dupdate();
			frame++;
		}
		sleep();
	}
	return 0;
}
