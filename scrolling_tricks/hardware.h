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

void KillSystem(void);
void ActivateSystem(void);
void WaitVBL(void);
void WaitVBeam(ULONG line);
void HardWaitBlit(void);
void HardWaitLMB(void);

BOOL JoyLeft(void);
BOOL JoyRight(void);
BOOL JoyUp(void);
BOOL JoyDown(void);
BOOL JoyFire(void);
BOOL LMBDown(void);

