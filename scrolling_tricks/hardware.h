#ifdef custom
#undef custom
#endif

#define mycustombase ((struct Custom *)0xdff000)
#define custom mycustombase

#define BPL0_USEBPLCON3_F  0x1
#define BPL0_COLOR_F       0x200
#define BPL0_BPU0_F        0x1000
#define BPL0_BPU3_F        0x10
#define BPL0_BPUMASK			0x7000

void kill_system(void);
void activate_system(void);
void wait_vbl(void);
void wait_vbeam(ULONG line);
void hard_wait_blit(void);

BOOL joy_left(void);
BOOL joy_right(void);
BOOL joy_up(void);
BOOL joy_down(void);

BOOL joy_fire(void);
BOOL lmb_down(void);

