/*
  bbrl.c

  Sep 19, 2005
  Hidetada Baba (RIKEN)
  baba@rarfaxp.riken.jp
*/

void init_mem(void);
void init_block(void);
void init_event(short fid);
void init_segment(short segid);
int end_block(void);
int end_event(void);
int end_segment(void);
#ifdef CAMAC
int read_segdata(short c,short n,short a,short f);
int read_segndata(short len,short c,short n,short a,short f);
int read_segsdata(short len,short c,short n,short a,short f);
int read_segbdata(short len,short c,short n,short a,short f);
int read_segqsdata(short len,short c,short n,short a,short f);
int read_segqrdata(short len,short c,short n,short a,short f);
int read_segmod(short len,short segid,short c,short n,short a,short f);
int read_segsmod(short len,short segid,short c,short n,short a,short f);
int read_segbmod(short len,short segid,short c,short n,short a,short f);
int read_segmemmod(short segid,short c,short n);
int read_scaler(short pos,short len,short c,short n);
int read_sscaler(short pos,short len,short c,short n);
int read_pedestal(short type,short c,short n,short vsn);
void write_mod(short c,short n,short a,short f,short *data);
void write_data(short c,short n,short a,short f,short data);
void control_mod(short c,short n,short a,short f);
#endif
int get_mp(void);
void delay_us(void);
#ifdef VME
int dmasegdata(unsigned long maddr,int rsize);
#ifdef SBS620
#ifdef ISERIES
int dmasegdata_i(unsigned long maddr,int rsize, int dn);
#endif
#endif
#endif

#define DMAMAX 4096
#define DMADELAYMAX 10000 // > 10ms
#ifdef VME
static unsigned int dmadata[DMAMAX];
#endif
#ifndef DBUFF
static unsigned short data[BLKSIZE/2];
#else
unsigned short *data;
static unsigned short databuff[BLKSIZE];
volatile int dn=0;
#endif

static int mp,eventid,eventhmp,eventsize,segmenthmp,segmentsize;
int i;
int blksize = BLKSIZE;

#ifndef DBUFF
void init_mem(void){
  memset(data,0,blksize);                   /* Memory set to '0' */
}
#else
void chmem(void){
  if(dn==0){
    data = databuff + BLKSIZE/2;
    dn = 1;
  }else{
    data = databuff;
    dn = 0;
  }
}

void init_mem(void){
  dn = 1;
  chmem();
  memset(databuff,0,sizeof(databuff));             /* Memory set to '0' */
}
#endif

void end_driver(void){
}

void init_block(void){
  memset(data,0,4);                       /* Memory set to '0' */
  mp = 4;
  eventid = 0;
}

void init_pblock(void){
  data[0] = 0x0010;
  memset(data+1,0,3);                       /* Memory set to '0' */
  mp = 4;
  eventid = 0;
}

void init_event(short fid){
  eventid++;                               /* Count up event ID */
  eventhmp = mp;                           /* Store mp for event size */
  eventsize = 3;                           /* Initialize event size */
  mp++;
  data[mp++] = fid;                        /* Write FID */
  data[mp++] = eventid;                    /* Write event ID */
}

void init_segment(short segid){
  segmenthmp = mp;                         /* Store mp for segment size */
  segmentsize = 2;                         /* Initialize segment size */
  mp++;
  data[mp++] = segid;                      /* Write segment ID */
}

int end_block(void){
  data[mp++] = 0xffff;                     /* Write END */
  data[mp++] = 0xffff;                     /* Write END */

  return mp;
}

int end_event(void){
  data[eventhmp] = eventsize | 0x8000;     /* Write event size */
  return mp;
}

int end_segment(void){
  data[segmenthmp] = segmentsize;          /* Write segment size */
  eventsize += segmentsize;                /* Count up event size */

  return mp;
}

#ifdef CAMAC
int read_segdata(short c,short n,short a,short f){
  read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
  mp++;
  segmentsize++;                           /* Count up segment size */

  return segmentsize;
}

int read_segndata(short len,short c,short n,short a,short f){
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
    a++;
    mp++;
    segmentsize++;                           /* Count up segment size */
  }

  return segmentsize;
}

int read_segsdata(short len,short c,short n,short a,short f){
#ifdef K2915
  block_read16(QSCAN,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  segmentsize += len;
#else
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
    a++;
    mp++;
    segmentsize++;                           /* Count up segment size */
  }
#endif

  return segmentsize;
}

int read_segbdata(short len,short c,short n,short a,short f){
  block_read16(QIGNORE,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  segmentsize += len;

  return segmentsize;
}

int read_segqrdata(short len,short c,short n,short a,short f){
  int rlen;

  rlen = block_read16(QREPEAT,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += rlen;
  segmentsize += rlen;

  return segmentsize;
}

int read_segqsdata(short len,short c,short n,short a,short f){
  int rlen;

  rlen = block_read16(QSTOP,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += rlen;
  segmentsize += rlen;

  return segmentsize;
}

int read_segmod(short len,short segid,short c,short n,short a,short f){
  data[mp++] = len+2;                      /* Write word count */
  data[mp++] = segid;                      /* Write segment ID */
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);      /* Read data */
    mp++;
    a++;
  }

  eventsize += len + 2;                    /* Count up event size */

  return len + 2;
}

int read_segsmod(short len,short segid,short c,short n,short a,short f){
  segmenthmp = mp;
  mp++;
  data[mp++] = segid;                      /* Write segment ID */
  block_read16(QSCAN,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  data[segmenthmp] = len + 2;              /* Write word count */
  eventsize += len + 2;                    /* Count up event size */

  return len + 2;
}

int read_segbmod(short len,short segid,short c,short n,short a,short f){
  segmenthmp = mp;
  mp++;
  data[mp++] = segid;                      /* Write segment ID */
  block_read16(QIGNORE,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  data[segmenthmp] = len + 2;              /* Write word count */
  eventsize += len + 2;                    /* Count up event size */

  return len + 2;
}


int read_segmemmod(short segid,short c,short n){
  short wdata,wordcnt;

  wdata = 0x0001;
  write16(CNAFGEN(c,n,1,17),&wdata);       /* Change to CAMAC-mode */
  read16(CNAFGEN(c,n,0,1),&wordcnt);       /* Read word count */
  wdata = 0x0000;
  write16(CNAFGEN(c,n,0,17),&wdata);       /* Reset read buffer pointer */
  data[mp++] = wordcnt+3;                  /* Write word count */
  data[mp++] = segid;                      /* Write segment ID */
  data[mp++] = wordcnt;                    /* Write FERA word count */
  if(wordcnt != 0){
    block_read16(QIGNORE,CNAFGEN(c,n,0,0),data+mp,wordcnt); /* Read */
  }
  mp += wordcnt;
  wdata = 0x0003;
  write16(CNAFGEN(c,n,1,17),&wdata);       /* Change to ECL-mode */

  eventsize += wordcnt + 3;

  return wordcnt + 3;
}

int read_scaler(short pos,short len,short c,short n){
  mp = blksize/2 - (len * 2 * pos);           /* Set mp */
  for(i=0;i<len;i++){
    read24(CNAFGEN(c,n,i,0),data+mp);      /* Read Scaler data */
    mp += 2;
  }

  return len * 2;
}

int read_sscaler(short pos,short len,short c,short n){
  mp = blksize/2 - (len * 2 * pos);           /* Set mp */

  block_read24(QSCAN,CNAFGEN(c,n,0,0),(unsigned int*)(data+mp),len);
  /* Read data 24Bit Block Transfer Mode*/

  return len * 2;
}

int read_pedestal(short type,short c,short n,short vsn){
  short len;
  /* type 0 : 4300B
          1 : 3351
  */
  data[mp++] = type;
  data[mp++] = (c<<8)|n;
  data[mp++] = vsn;
  if(type == 0){
    len = 16;
  }else{
    len = 8;
  }
  block_read16(QIGNORE,CNAFGEN(c,n,0,2),data+mp,len); /* Read */
  mp += len;

  return mp;
}

void write_mod(short c,short n,short a,short f,short *data){
  write16(CNAFGEN(c,n,a,f),data);
}

void write_data(short c,short n,short a,short f,short data){
  write16(CNAFGEN(c,n,a,f),&data);
}

void control_mod(short c,short n,short a,short f){
  control(CNAFGEN(c,n,a,f));
}
#endif

int get_mp(void){
  return mp;
}

void delay_us(void){
  outb(1,0x80);
}

#ifdef VME
int dmasegdata(unsigned long maddr,int rsize){
  int wordcnt, csize;
  volatile int dmadelay, dmaflag;
  
  wordcnt = 0;
  dmaflag = 0;
  csize = rsize * 4;  /* long word -> char size */
  vme_dma_vread32_start(maddr, csize);
  delay_us();  /* delay about 1us */
  dmadelay = 0;
  for(dmadelay=0;dmadelay<DMADELAYMAX;dmadelay++){
    if(vme_dma_vread32_store((char *)dmadata,csize)){
      dmadelay = DMADELAYMAX + 100;
      dmaflag = 1;
    }else{
      delay_us();
    }
  }
  
  if(dmaflag){
    memcpy((char *)(data+mp),(char *)(dmadata),csize);
    wordcnt += rsize;
    segmentsize += rsize*2;
    mp += rsize*2;
  }else{
    data[mp++] = 0xfeee;
    data[mp++] = dmadelay & 0xffff;
    wordcnt++;
    segmentsize += 2;
  }

  return segmentsize;
}
#ifdef SBS620
#ifdef ISERIES
int dmasegdata_i(unsigned long maddr,int rsize, int dn){
  int wordcnt, csize;
  volatile int dmadelay, dmaflag;
  
  wordcnt = 0;
  dmaflag = 0;
  csize = rsize * 4;  /* long word -> char size */
  vme_dma_vread32_start_i(maddr, csize, dn);
  delay_us();  /* delay about 1us */
  dmadelay = 0;
  for(dmadelay=0;dmadelay<DMADELAYMAX;dmadelay++){
    if(vme_dma_vread32_store_i((char *)dmadata,csize,dn)){
      dmadelay = DMADELAYMAX + 100;
      dmaflag = 1;
    }else{
      delay_us();
    }
  }
  
  if(dmaflag){
    memcpy((char *)(data+mp),(char *)(dmadata),csize);
    wordcnt += rsize;
    segmentsize += rsize*2;
    mp += rsize*2;
  }else{
    data[mp++] = 0xfeee;
    data[mp++] = dmadelay & 0xffff;
    wordcnt++;
    segmentsize += 2;
  }

  return segmentsize;
}
#endif
#endif
#endif
