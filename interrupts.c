
Linux 20.04 (Running]
#include<linux/kobject.h>
#include «linux/interrupt.h>
#include <asm/io.h>
#include<linux/kthread.h>
#include<linux/delay.h›
#include<linux/jiffies.h>
#include<linux/sysfs.h>
#include<linux/timer.h>
#define IRO NO 1
unsigned int i =0;
unsigned long handler_run_count =0;
DEFINE_SPINLOCK(cdd_spinlock);
static irgreturn t irq handler (int irg, void *dev_id) {
spin_lock irq(&cdd_spinlock);
handler run count++;
printk(KERN_INFO "Interrupt Occurred and executing ISR routine/Interrupt Handler. Handler Run Count: &lu\n", handler_ru n_ count);
spin unlock irq(&cdd
spinlock) :
/* we run bottom halve before returning from irq Handler */
printk(KERN_ INFO "Inside Interrupt Handler. Running Bottom Half before return from the Handler..... \n"):
return IRQ HANDLEDE
dev t dev = 0;
static struct static struct
class
*dev class;
cdev
chr _cdev;
0 0