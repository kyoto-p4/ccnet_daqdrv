void startup(void){
  int val = 0;
  //  short sval = 0xf00f;

  /* Startup Function */
  ccnet_crate_reset();
  check_lam();

  /* Dummy LAM definition */
  crate_define_lam(LAMN);

  /* Create ccnet command frame */
  ccnetstartup();

  /* Create user command frame */
 
  //    usr_ccnet_gen_init(USR_CMDLEN);
  //    for(i=0;i<12;i++){
  //    usr_ccnet_gen_cc(NSCA, i, 0, val);
  //    }

  /* Clear modules */
  //control_mod(0, N, 0, 9);      // Clear command
  //write_data(0, N, 0, 17, VAL); // via out put register

  //control_mod(0,LAMN,12,11);
  control_mod(0,LAMN,3,11);
  //  control_mod(0,NQDC,0,9);
  control_mod(0,NTDC,0,9);
  control_mod(0,NSCA,0,9);
  //control_mod(0,LAMN,12,26);
  control_mod(0,LAMN,0,26);


  write_data(0,NOPR,0,17,OP_SCARST);
  write_data(0,NOPR,0,17,OP_STA|OP_VETO_STO);
  write_data(0,NOPR,0,16,OP_LEVEL);


  /* Set trigger mode */
  //ccnet_trig();                   // Interrupt by CC/NET front panel
  //ccnet_pulsemode();              // Define pulse mode
  //ccnet_pulse();                  // Output pulse from CC/NET front panel
}
