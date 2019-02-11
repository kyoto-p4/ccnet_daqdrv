#include "rpv130.h"

int rpv130_write( unsigned long maddr, unsigned short val){
  set_amsr(0x29);
  vwrite16(maddr, &val);
  set_amsr(0x09);
  return 1;
}

int rpv130_output(unsigned long maddr, unsigned short val){
  set_amsr(0x29);
  rpv130_write(maddr+RPV130_PULSE, val);
  set_amsr(0x09);

  return 1;
}

int rpv130_segdata(unsigned long maddr, int mode){
  set_amsr(0x29);
  vread16(maddr+mode,(short *)(data+mp));
  set_amsr(0x09);
  mp += 1;
  segmentsize += 1;
  
  return segmentsize;
}

int rpv130_segdata_v(unsigned long maddr, int mode, unsigned short *sval){
  set_amsr(0x29);
  vread16(maddr+mode,(short *)(data+mp));
  set_amsr(0x09);

  memcpy((char *)sval,(char *)(data+mp),2);

  mp += 1;
  segmentsize += 1;
  
  return segmentsize;
}

int rpv130_clear(unsigned long maddr){
  short sval;

  sval = RPV130_CLEAR1OR2 | RPV130_CLEAR3 | RPV130_MASK1OR2;
  set_amsr(0x29);
  vwrite16(maddr+RPV130_CTL1,&sval);
  vwrite16(maddr+RPV130_CTL2,&sval);
  set_amsr(0x09);

  return 1;
}
