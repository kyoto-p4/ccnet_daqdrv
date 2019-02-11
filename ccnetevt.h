#define DAQEXE_CTRL_READ         0x0
#define DAQEXE_CTRL_CLRBSY   0x10000
#define DAQEXE_CTRL_CLRCNT   0x20000
#define DAQEXE_CTRL_ENABLE   0x30000
#define DAQEXE_CTRL_DISABLE  0x40000
#define DAQEXE_CTRL_PLSOUT   0x50000
#define DAQEXE_CTRL_NOPLSOUT 0x60000

#define SEGCMDMSK   0xf0000000
#define SEGCMDINIT  0x10000000
#define SEGCMDSHORT 0x20000000
#define SEGCMDINT   0x30000000
#define SEGCMDEND   0x40000000
#define SEGCMDSKP   0x50000000

#define SEGIDGEN(device,focal,detector,module) ((((device<<6 | focal) << 6) | detector)<<8 | module) 

void ccnetstartup(void);
int ccnet_gen_init(int length);
void ccnet_gen_cc(int n, int a, int f, int data);
void ccnet_gen_clr(int n, int a, int f, int data);
int ccnet_segdata_pio(void);
int ccnet_segdata_dma(void);
int ccnet_dma(void);
int ccnet_segdata(void);
void ccnet_clear_pio(void);
void ccnet_clear_dma(void);
void ccnet_clear(void);
void ccnet_pulse(void);
void ccnet_trig(void);
void ccnet_pulsemode(void);
int ccnet_recseg(void);
