/* babirl/babidrvccnet/ccnetevt.c
 * last modified : 08/09/06 21:45:46 
 *
 * CC/NET function for babildrv
 * 
 * Hidetada Baba (RIKEN)
 */


#include "ccnetevt.h"


int cmdbuf[CMDLEN*2+2], clrbuf[CMDLEN*2+2], rplybuf[CMDLEN*2+2];
int segcmd[1024], segcmdlen;

#include "ccnetfunc.c"

int ccnet_gen_init(int length){
  if(length <=0 || length > CMDLEN) return 0;

  memset((char *)cmdbuf, 0, length*sizeof(int));
  memset((char *)clrbuf, 0, length*sizeof(int));
  cmdbuf[0] = length;
  clrbuf[0] = length;

  return 1;
}

void ccnet_gen_cc(int n, int a, int f, int data){
  cmdbuf[cmdbuf[1]*2+2] = data & 0xffffff;
  cmdbuf[cmdbuf[1]*2+3] = 0xe0000000 | CNAFGEN(0, n, a, f);
  cmdbuf[1]++;
}

void ccnet_gen_clr(int n, int a, int f, int data){
  clrbuf[clrbuf[1]*2+2] = data & 0xffffff;
  clrbuf[clrbuf[1]*2+3] = 0xe0000000 | CNAFGEN(0, n, a, f);
  clrbuf[1]++;
}

int ccnet_segdata_pio(void){
  int len,i ;

  ccnet_exec_pio(cmdbuf, rplybuf);
  len = rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp] = rplybuf[2*i+2] & 0xffffff;
    mp++;
    segmentsize++;
  }

  return segmentsize;
}

int ccnet_segdata_dma(void){
  int len, i;

  ccnet_exec_dma(cmdbuf, rplybuf);
  len = rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp] = rplybuf[2*i+2] & 0xffffff;
    mp++;
    segmentsize++;
  }

  return segmentsize;
}

int ccnet_dma(void){
  int len;

  ccnet_exec_dma(cmdbuf, rplybuf);
  len = rplybuf[1];
  
  return len;
}

int ccnet_segdata(void){
  int len, i;

  ccnet_exec(cmdbuf, rplybuf);
  len = rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp] = rplybuf[2*i+2] & 0xffffff;
    mp++;
    segmentsize++;
  }

  return segmentsize;
}

void ccnet_clear_pio(void){
  ccnet_exec_pio(clrbuf, rplybuf);
}

void ccnet_clear_dma(void){
  ccnet_exec_pio(clrbuf, rplybuf);
}

void ccnet_clear(void){
  ccnet_exec(clrbuf, rplybuf);
}

void ccnet_pulse(void){
  int com;
  com = 0xf0000000 | DAQEXE_CTRL_CLRBSY;
  ccnet_control(com);
}

void ccnet_trig(void){
  int com = 0xf0000000 | DAQEXE_CTRL_ENABLE;
  ccnet_control(com);
}

void ccnet_pulsemode(void){
  int com = 0xf0000000 | DAQEXE_CTRL_PLSOUT;
  ccnet_control(com);
}

int ccnet_recseg(void){
  int cmd, val, idx, i, j;

  ccnet_dma();

  idx = 2;
  for(i=0;i<segcmdlen;i++){
    cmd = segcmd[i] & SEGCMDMSK;
    val = segcmd[i] & ~SEGCMDMSK;
    switch (cmd){
    case SEGCMDINIT:
      init_segment(val);
      break;
    case SEGCMDSHORT:
      for(j=0;j<val;j++){
	data[mp] = rplybuf[2*j+idx] & 0xffff;
	mp++;
	segmentsize++;
      }
      idx += val * 2;
      break;
    case SEGCMDINT:
      for(j=0;j<val;j++){
	data[mp]   = rplybuf[2*j+idx]  & 0xffff;
	data[mp+1] = (rplybuf[2*j+idx])>>16  & 0xff;
	mp += 2;
	segmentsize += 2;
      }
      idx += val * 2;
      break;
    case SEGCMDEND:
      end_segment();
      break;
    case SEGCMDSKP:
      idx += val * 2;
      break;
    }
  }

  return idx;
}

