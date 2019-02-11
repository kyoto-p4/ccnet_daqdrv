void stop(void){
  //control_mod(0, LAMN, 12, 24);
  control_mod(0, LAMN, 0, 24);
  write_data(0,NOPR,0,17,OP_STO);
  control_mod(0, NOPR, 0, 9);

}
