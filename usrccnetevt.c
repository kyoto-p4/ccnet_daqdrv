/* babirl/babidrvccnet/usrccnetevt.c
 * last modified : 08/09/06 21:45:46 
 *
 * CC/NET function for babildrv
 * 
 * Hidetada Baba (RIKEN)
 */


int usr_cmdbuf[USR_CMDLEN*2+2];
int usr_rplybuf[USR_CMDLEN*2+2];

int usr_ccnet_gen_init(int length){
  if(length <=0 || length > USR_CMDLEN) return 0;

  memset((char *)usr_cmdbuf, 0, length*sizeof(int));
  usr_cmdbuf[0] = length;

  return 1;
}

void usr_ccnet_gen_cc(int n, int a, int f, int data){
  usr_cmdbuf[usr_cmdbuf[1]*2+2] = data & 0xffffff;
  usr_cmdbuf[usr_cmdbuf[1]*2+3] = 0xe0000000 | CNAFGEN(0, n, a, f);
  usr_cmdbuf[1]++;
}

int usr_ccnet_segdata_pio(void){
  int len,i ;

  ccnet_exec_pio(usr_cmdbuf, usr_rplybuf);
  len = usr_rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp] = usr_rplybuf[2*i+2] & 0xffff;
    mp++;
    segmentsize++;
  }

  return segmentsize;
}

int usr_ccnet_segdata_pio24(void){
  int len,i ;

  ccnet_exec_pio(usr_cmdbuf, usr_rplybuf);
  len = usr_rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp]   =  usr_rplybuf[2*i+2] & 0xffff;
    data[mp+1] = (usr_rplybuf[2*i+2])>>16  & 0xff;
    mp+=2;
    segmentsize+=2;
  }

  return segmentsize;
}

int usr_ccnet_segdata_dma(void){
  int len, i;

  ccnet_exec_dma(usr_cmdbuf, usr_rplybuf);
  len = usr_rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp] = usr_rplybuf[2*i+2] & 0xffff;
    mp++;
    segmentsize++;
  }

  return segmentsize;
}

int usr_ccnet_segdata_dma24(void){
  int len, i ;

  ccnet_exec_dma(usr_cmdbuf, usr_rplybuf);
  len = usr_rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp]   =  usr_rplybuf[2*i+2] & 0xffff;
    data[mp+1] = (usr_rplybuf[2*i+2])>>16  & 0xff;
    mp+=2;
    segmentsize+=2;
  }

  return segmentsize;
}

int usr_ccnet_scrdata_dma24(int scrlen, int pos){
  int len, i ;
  int scrsize = 0;

  mp = blksize/2 - (scrlen * 2 * pos);

  ccnet_exec_dma(usr_cmdbuf, usr_rplybuf);
  len = usr_rplybuf[1];
  
  for(i=0;i<len;i++){
    data[mp]   =  usr_rplybuf[2*i+2] & 0xffff;
    data[mp+1] = (usr_rplybuf[2*i+2])>>16  & 0xff;
    mp+=2;
    scrsize+=2;
  }

  return scrsize;
}

#ifdef TDCMEMMODE
int usr_ccnet_3377mem(int n){
  int val = 0, i;
  short wdata, wordcnt;


  make compile error here


  wdata = 0x0001;
  write16(CNAFGEN(0,n,1,17),&wdata);       /* Change to CAMAC-mode */
  read16(CNAFGEN(0,n,0,1),&wordcnt);       /* Read word count */
  wdata = 0x0000;
  write16(CNAFGEN(0,n,0,17),&wdata);       /* Reset read buffer pointer */
  
  //data[mp++] = wordcnt;                  /* Write FERA word count */
  //segmentsize++;
  if(wordcnt > 0){
    usr_ccnet_gen_init(USR_CMDLEN);
    for(i=0;i<wordcnt;i++){
      usr_ccnet_gen_cc(n, 0, 0, val);
    }
    if(wordcnt > 2){
      usr_ccnet_segdata_dma();
    }else{
      usr_ccnet_segdata_pio();
    }
  }
  wdata = 0x0003;
  write16(CNAFGEN(0,n,1,17),&wdata);       /* Change to ECL-mode */ 

  return segmentsize;
}
#endif
