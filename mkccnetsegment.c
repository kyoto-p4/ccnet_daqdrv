/* babirl/babidrv/mkccnetsegment.c
 *
 * evt.c auto maker for CCNET
 * 
 * Hidetada Baba (RIKEN)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CCNETFILE "ccnetfunc.c"

#define SEGCMDMSK   0xf0000000
#define SEGCMDINIT  0x10000000
#define SEGCMDSHORT 0x20000000
#define SEGCMDINT   0x30000000
#define SEGCMDEND   0x40000000
#define SEGCMDSKP   0x50000000

#define SEGIDGEN(device,focal,detector,module) ((((device<<6 | focal) << 6) | detector)<<8 | module) 

/* Prototype */
void init_segment(int segid);
void read_segdata(int c, int n, int a, int f);
void read_segndata(int len, int c, int n, int a, int f);
void read_segdata24(int c, int n, int a, int f);
void read_segndata24(int len, int c, int n, int a, int f);
void end_segment(void);
void control_mod(int c, int n, int a, int f);
void write_data(int c, int n, int a, int f, int val);

/* Common variables */
int segcmd[1024], segcmdlen;
FILE *fd;


/* User include */
#ifdef BIGRIPSDAQ	
#include <bigrips_zds.h>
#endif

#include "bbmodules.h"
#include "ccnetsegment.c"
/* End of user indlude */



void init_segment(int segid){
  segcmd[segcmdlen++] = SEGCMDINIT | segid;
}

void read_segdata(int c, int n, int a, int f){
  fprintf(fd, "  ccnet_gen_cc(%d, %d, %d, val);\n", n, a, f);
  segcmd[segcmdlen++] = SEGCMDSHORT | 1;
}

void read_segndata(int len, int c, int n, int a, int f){
  len &= 0x0fffffff;

  fprintf(fd, "  for(i=0;i<%d;i++){\n", len);
  fprintf(fd, "    ccnet_gen_cc(%d, i+%d, %d, val);\n", n, a, f);
  fprintf(fd, "  }\n");
  segcmd[segcmdlen++] = SEGCMDSHORT | len;
 
}

void read_segdata24(int c, int n, int a, int f){
  fprintf(fd, "  ccnet_gen_cc(%d, %d, %d, val);\n", n, a, f);
  segcmd[segcmdlen++] = SEGCMDINT | 1;
}

void read_segndata24(int len, int c, int n, int a, int f){
  len &= 0x0fffffff;

  fprintf(fd, "  for(i=0;i<%d;i++){\n", len);
  fprintf(fd, "    ccnet_gen_cc(%d, i+%d, %d, val);\n", n, a, f);
  fprintf(fd, "  }\n");
  segcmd[segcmdlen++] = SEGCMDINT | len;
}

void end_segment(void){
  segcmd[segcmdlen++] = SEGCMDEND;
}

void control_mod(int c, int n, int a, int f){
  fprintf(fd, "  ccnet_gen_cc(%d, %d, %d, val);\n", n, a, f);
  segcmd[segcmdlen++] = SEGCMDSKP | 1;
}

void write_data(int c, int n, int a, int f, int val){
  fprintf(fd, "  ccnet_gen_cc(%d, %d, %d, %d);\n", n, a, f, val);
  segcmd[segcmdlen++] = SEGCMDSKP | 1;
}


int main(int argc, char *argv[]){
  int i;

  // Initialize 
  memset((char *)segcmd, 0, sizeof(segcmd));
  segcmdlen = 0;

  if((fd = fopen(CCNETFILE, "w")) == NULL){
    printf("Can't open %s\n", CCNETFILE);
    exit(1);
  }
  
  fprintf(fd, "/* Created by mkccnetsegment */\n\n");
  fprintf(fd, "void ccnetstartup(void){\n");
  fprintf(fd, "  int i, val=0;\n\n");
  fprintf(fd, "  ccnet_gen_init(CMDLEN);\n");
  ccnetsegment();

  fprintf(fd, "\n  segcmdlen = %d;\n", segcmdlen);
  for(i=0; i<segcmdlen; i++){
    fprintf(fd, "  segcmd[%d] = 0x%08x;\n", i, segcmd[i]);
  }
  fprintf(fd, "}\n");


  fclose(fd);

  return 0;
}
