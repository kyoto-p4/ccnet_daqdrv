/* nbbqdrv.c

   Oct 18, 2006
   Hidetada Baba (baba@ribf.riken.jp)
*/


#include <linux/version.h>
#if LINUX_VERSION_CODE >=  0x020600
#if defined(USE_MODVERSIONS) && USE_MODVERSIONS
#  define MODVERSIONS
#  include <config/modversions.h>
#endif
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE >=  0x020600
#include <linux/init.h>
#include <linux/interrupt.h>
#endif
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/sched.h>
#if LINUX_VERSION_CODE <  0x020600
#include <linux/config.h>
#endif
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "bbmodules.h"
#include "nbbqctrl.h"
#include "nbbqdrv.h"
#include "nbbqio.h"

#if LINUX_VERSION_CODE >=  0x020410
MODULE_LICENSE("GPL");
#endif
#if LINUX_VERSION_CODE >=  0x020600
MODULE_AUTHOR("Hidetada Baba");
#ifndef MOD_IN_USE
#define MOD_IN_USE module_refcount(THIS_MODULE)
#endif
#ifndef MOD_INC_USE_COUNT
#define MOD_INC_USE_COUNT
#endif
#ifndef MOD_DEC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif
#endif


#include "./common.h"
#ifndef BLKSIZE
#define BLKSIZE 0x4000
#endif

#ifdef BBRL
#include "bbrl.c"
#endif
#ifdef BBDAQ
#include "bbdaq.c"
#endif

#ifdef CCNET
#include "ccnetevt.c"
#include "usrccnetevt.c"
#endif

#ifdef VME
#include "v775.c"
#include "v792.c"
#include "v785.c"
#include "v767.c"
#endif

#include "startup.c"
#include "evt.c"
#include "stop.c"
#include "sca.c"
#include "clear.c"

static int nbbqdrv_major = NBBQ_MAJOR;
static const char *nbbqdrv_name = "nbbqdrv";
#if LINUX_VERSION_CODE >= 0x020600
static int dev_id;
#else
static char *irq_name = "irq_nbbq";
#endif

ssize_t nbbqdrv_read(struct file *file,char *buff,size_t count,loff_t *pos);
static int nbbqdrv_open(struct inode* inode, struct file* filep);
static int nbbqdrv_release(struct inode* inode, struct file* filep);
static int nbbqdrv_ioctl(struct inode *inode, struct file *filep,
			 unsigned int cmd, unsigned long arg);
static unsigned int nbbqdrv_poll(struct file *file,poll_table *wait);
#if LINUX_VERSION_CODE >= 0x020600
static irqreturn_t nbbqdrv_interrupt(int irq, void* dev_id, struct pt_regs* regs);
#else
static void nbbqdrv_interrupt(int irq,void *dev_id,struct pt_regs* regs);
#endif

static int girq,chkbuff,chkblk;
#if LINUX_VERSION_CODE >=  0x020400
wait_queue_head_t nbbq_wait_queue;
#else
struct wait_queue *nbbq_wait_queue = NULL;
#endif


#if LINUX_VERSION_CODE >=  0x020600
static struct file_operations nbbqdrv_fops = {
  .read = nbbqdrv_read,
  .poll = nbbqdrv_poll,
  .ioctl = nbbqdrv_ioctl,
  .open = nbbqdrv_open,
  .release = nbbqdrv_release,
};
#else
#if LINUX_VERSION_CODE >=  0x020400
static struct file_operations nbbqdrv_fops = {
  read: nbbqdrv_read,
  poll: nbbqdrv_poll,
  ioctl: nbbqdrv_ioctl,
  open: nbbqdrv_open,
  release: nbbqdrv_release,
};
#else
static struct file_operations nbbqdrv_fops = {
  NULL,             // loff_t  llseek
  nbbqdrv_read,     // ssize_t read
  NULL,             // ssize_t write
  NULL,             // int     readdir
  nbbqdrv_poll,     // uint    poll
  nbbqdrv_ioctl,    // int     ioctl
  NULL,             // int     mmap
  nbbqdrv_open,     // int     open
  NULL,             // int     flush
  nbbqdrv_release,  // int     release
};
#endif
#endif

#ifdef DBUFF
volatile static int stopflag = 0;
volatile int drn = 0;
#endif

#if LINUX_VERSION_CODE >= 0x020600
int nbbqdrv_init_module(void){
#else
int init_module(void){
#endif
  int ret;

  ret = register_chrdev(nbbqdrv_major,nbbqdrv_name,&nbbqdrv_fops);
  if(ret < 0){
    printk("%s : can't regist.\n",nbbqdrv_name);
    return ret;
  }
  if(nbbqdrv_major == 0) {
    nbbqdrv_major = ret;
  }

  girq = get_irq();
  chkbuff = 0;

#if LINUX_VERSION_CODE >=  0x020400
  init_waitqueue_head(&nbbq_wait_queue);
#endif

  printk("%s : nbbqdrv was installed (irq %d).\n",nbbqdrv_name,girq);

  return 0;
}

#if LINUX_VERSION_CODE >= 0x020600
void nbbqdrv_cleanup_module(void){
#else
void cleanup_module(void){
#endif
  unregister_chrdev(nbbqdrv_major,nbbqdrv_name);
  printk("%s: nbbqdrv was unregistered.\n", nbbqdrv_name);
}

static int nbbqdrv_open(struct inode* inode, struct file* filep){
  int ret;

  if(MOD_IN_USE){
    return -EBUSY;
  }

  chkblk = 0;
  chkbuff = 0;
#ifdef DBUFF
  stopflag = 0;
  drn = 0;
#endif

  init_mem();

#if LINUX_VERSION_CODE >= 0x020600
  ret = request_irq(girq,nbbqdrv_interrupt,SA_INTERRUPT|SA_SHIRQ,nbbqdrv_name,&dev_id);
#else
  ret = request_irq(girq,nbbqdrv_interrupt,SA_INTERRUPT|SA_SHIRQ,nbbqdrv_name,irq_name);
#endif
  MOD_INC_USE_COUNT;

  startup();

#ifdef VMEINT
  vme_enable_interrupt();
#else
  rfs_enable_interrupt();
#endif

  return 0;
}

static int nbbqdrv_release(struct inode* inode, struct file* filep){

  chkbuff = 0;

#ifdef VMEINT
  vme_disable_interrupt();
#else
  rfs_disable_interrupt();
#endif

  MOD_DEC_USE_COUNT;

  return 0;
}

ssize_t nbbqdrv_read(struct file *file,char *buff,size_t count,loff_t *pos){
  unsigned long x;

#ifndef DBUFF
  chkbuff = 0;
  x = copy_to_user(buff,data,sizeof(data));
#else
  x = copy_to_user(buff,(char *)(databuff+BLKSIZE/2*drn),BLKSIZE);
  drn++;
  if(drn == 2) drn = 0;

  if(chkbuff == 2){
    if(stopflag == 0){
#ifdef VMEINT
      vme_enable_interrupt();
#else
      rfs_enable_interrupt();
#endif
    }
    clear();
    if(stopflag == 0){
#ifdef K2915
      rfs_enable_interrupt();
#endif
    }
  }

  chkbuff--;
  if(chkbuff < 0){
    chkbuff = 0;
  }
#endif

  return 1;
}


static int nbbqdrv_ioctl(struct inode* inode, struct file *filep,
			 unsigned int cmd, unsigned long arg){

  char flag;
  unsigned long x;

  switch(cmd){
  case NBBQ_STOPB:
#ifdef VMEINT
    vme_disable_interrupt();
#else
    rfs_disable_interrupt();
#endif
#ifdef DBUFF
    stopflag = 1;
#endif
    if(chkblk == 1){
      end_block();
#ifdef BBRL
#ifdef VMEINT
      /* none */
#else
      //crate_seti(0);
#endif
      sca();
#endif
#ifndef DBUFF
      flag = 1;
#else
      chkbuff++;
      flag = chkbuff;
#endif
      x = copy_to_user((void *)arg,&flag,1);
#ifndef DBUFF
      chkbuff = 1;
#endif
      chkblk = 0;
    }else{
      flag = 0;
      x = copy_to_user((void *)arg,&flag,1);
    }
    stop();
#if LINUX_VERSION_CODE >= 0x020600
    free_irq(girq,&dev_id);
#else
    free_irq(girq,irq_name);
#endif
    break;
  case NBBQ_EI:
#ifndef DBUFF
#ifdef VMEINT
    vme_define_intlevel(INTLEVEL);
    vme_enable_interrupt();
#else
    crate_define_lam(LAMN);
    rfs_enable_interrupt();
#endif
    clear();
#ifdef K2915
    rfs_enable_interrupt();
#endif
#endif
    break;
  }

  return 1;
}

static unsigned int nbbqdrv_poll(struct file *file,poll_table *wait){
  poll_wait(file,&nbbq_wait_queue,wait);
  if(chkbuff > 0){
    return POLLIN;
  }else{
    return 0;
  }
}

#if LINUX_VERSION_CODE >= 0x020600
static irqreturn_t nbbqdrv_interrupt(int irq, void* dev_id, struct pt_regs* regs){
#else
static void nbbqdrv_interrupt(int irq, void* dev_id, struct pt_regs* regs){
#endif

  if(dev_id == NULL){
#if LINUX_VERSION_CODE >= 0x020600
    return IRQ_NONE;
#else
    return;
#endif
  }

  if(chkblk == 0){
    init_block();
    chkblk = 1;
  }
  if(
#ifdef VMEINT
     vme_check_interrupt() != 0
#else
     check_lam() != 0
#endif
     ){
#ifdef VMEINT
    vme_disable_interrupt();
#else
    rfs_disable_interrupt();
#endif
    evt();
    if(end_event() > MAXBUFF){
      end_block();
#ifdef BBRL
#ifdef VMEINT
      /* none */
#else
      //crate_seti(0);
#endif
      sca();
#endif
      chkblk = 0;
#ifndef DBUFF
      chkbuff = 1;
#else
      chkbuff++;
      chmem();
      if(chkbuff != 2){
#ifdef VMEINT
	vme_enable_interrupt();
#else
	rfs_enable_interrupt();
#endif
	clear();
#ifdef K2915
	rfs_enable_interrupt();
#endif
      }
#endif
    }else{
#ifdef VMEINT
      vme_enable_interrupt();
#else
      rfs_enable_interrupt();
#endif
      clear();
#ifdef K2915
      rfs_enable_interrupt();
#endif
    }
  }else{
#if LINUX_VERSION_CODE >= 0x020600
    return IRQ_NONE;
#else
    return;
#endif
  }
  
  wake_up_interruptible(&nbbq_wait_queue);

#if LINUX_VERSION_CODE >= 0x020600
  return IRQ_HANDLED;
#endif
}

#if LINUX_VERSION_CODE >= 0x020600
module_init(nbbqdrv_init_module);
module_exit(nbbqdrv_cleanup_module);
#endif
