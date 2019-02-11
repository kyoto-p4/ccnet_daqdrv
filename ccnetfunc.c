/* Created by mkccnetsegment */

void ccnetstartup(void){
  int i, val=0;

  ccnet_gen_init(CMDLEN);
  ccnet_gen_cc(5, 0, 24, val);
  for(i=0;i<16;i++){
    ccnet_gen_cc(5, i+0, 0, val);
  }
  for(i=0;i<16;i++){
    ccnet_gen_cc(18, i+0, 0, val);
  }
  for(i=0;i<3;i++){
    ccnet_gen_cc(22, i+0, 0, val);
  }

  segcmdlen = 6;
  segcmd[0] = 0x10000001;
  segcmd[1] = 0x50000001;
  segcmd[2] = 0x20000010;
  segcmd[3] = 0x20000010;
  segcmd[4] = 0x20000003;
  segcmd[5] = 0x40000000;
}
