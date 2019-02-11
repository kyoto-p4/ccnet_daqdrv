void clear(void){
  /*
  short sval = 0xf00f;
  write_data(0, OREG, 0, 17, sval);

  ccnet_pulse(); 
  */

  control_mod(0,NQDC,3,11);
  control_mod(0,NTDC,0,9);
  //control_mod(0,LAMN,12,26);
  control_mod(0,NSCA,0,9);

  control_mod(0,LAMN,0,26);
  write_data(0,NOPR,0,17,OP_VETO_STO);

}
