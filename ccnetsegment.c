void ccnetsegment(void){
  /* Event */

  init_segment(1);

  control_mod(0, LAMN, 0, 24);
  read_segndata(16,0,NQDC,0,0);
  read_segndata(16,0,NTDC,0,0);
  read_segndata(3,0,NSCA,0,0);
  //  control_mod(0,NQDC,0,9);
  //  control_mod(0,NTDC,0,9);

  end_segment();

  /*  init_segment(2);
  read_segndata(8,0,TDC1,0,0);
  end_segment();

  control_mod(0, QDC1, 0, 9);
  control_mod(0, TDC1, 0, 9);
  */
}
