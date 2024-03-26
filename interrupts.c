
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
/* we run bottom halve before returning from irq Handler */
printk(KERN INFO "Inside Interrupt Handler. Running Bottom Half before return from the Handler.....\n"):
return IRO HANDLED;
dev t dev = 0;
static struct class *dev_class; static struct cdev chr_cdev;
static int
init cdd init(void);
static void
exit cdd_ exit(void);
/* File Operations for Device */
static int cdd open(struct inode *inode, struct file *file); static int cdd_release(struct inode *inode,
struct file *file);
static ssize t cdd
read (struct file *filp, char user *buf, size_t len,loff_t * off):
static ssize_t cdd write(struct file *filp, const char *buf, size_t len, loff_t * off);
static struct file operations fops =
.owner
= THIS MODULE,
. read
= cdd read,
•write
= cdd write,
• open
= cdd open,
• release
= cdd_release,
int cdd _open(struct inode *inode, struct file *file)( printk( "This is open file operation\n"):
pen tae, operat one tale
return
int cdd open(struct inode *inode, struct file *file){
printk("This is open file operation\n"):
return 0:
int cdd release(struct inode *inode, struct file *file)( printk("This is release file operation\n"):
return 0;
ssize t cdd
read (struct file *filp,char
user *buf, size_t len, loff_t * off)(
printk("This is read file operation\n");
return 0;
ssize_t cdd write(struct file *filp, const char *buf, size_t len, loff_t * off)(
printk("This is write file operation\n");
return 0;
Static int
init cdd init(void)
/*Allocating Major number*/
if( (alloc _chrdev region(&dev, 0, 1, "chr Dev")) <0)E
printk(KERN_INFO "Cannot allocate major number\n"):
return -1;
printk(KERN_INFO "Major = %d Minor = &d \n", MAJOR(dev), MINOR(dev)) :
/*Creating cdev structure*/ cdev_init (&chr_cdev,&fops):
static int

A
init cdd init(void)
/*Allocating Major number*/ if (alloc
chrdev region(&dev, 0, 1, "chr Dev")) <0)(
printk(KERN_INFO "Cannot allocate major number\n");
return -1;
printk(KERN_ INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR (dev)) ;
/*Creating
cdev structure*/
cdev_init (achr_cdev,&tops);
/*Adding
character device to the system*/
if (cdev add (&chr cdev, dev, 1)) < 0){
printk(KERN_INFO. "Cannot add the device to the system\n");
goto r_class;
/*creating
struct class/
if (dev class = class create(THIS MODULE, "chr class")) = NULLY
printk(KERN_INFO "Cannot create the struct class\n");
goto r_class;
}
/*Creating device*/
create(dev_class, NULL, dev, NULL, "chr _device")) = NULL)E
printk(KERN_INFO
"Cannot create the Device 1\n");
goto r device;
/*Adding character device to the system*/
if (cdev_add(&chr_cdev, dev, 1)) < 0){
printk(KERN_INFO "Cannot add the device to the system\n");
goto r class;
creating
struct class*/
if (dev class = class create(THIS MODULE, "chr class")) == NULLY
printk(KERN INFO "Cannot create the struct class\n");
goto r_class;
/*Creating device*/ if (device
create(dev class, NULL, dev, NULL, "chr _device")) = NULL)(
printk(KERN_INFO "Cannot create the Device I\n"): goto r device;
/* running irq Handler when interrupt occurs
on IRO NO
device */
if (request irq(IRO NO, irq handler, IRQF
SHARED, "chr device", (void *)(irq handler))) {
printk(KERN_INFO
"chr device: cannot register IRQ
goto irq:
printk(KERN _INFO "Device Driver Insert..Done!/\n");
return 0; irq:
free _irq(IRQ_NO, (void *)(irq_handler));
r_device:
class _destroy(dev_class):
goto irq;
printk(KERN_INFO "Device Driver Insert...Done!!/\n" ):
return 0;
irq:
free_irq(IRO_NO, (void *) (irq_handler));
device:
class _destroy (dev_class);
class:
unregister _chrdev region(dev,1);
cdev del (achr_cdev);
return -1;
void
exit cdd _exit(void)
free irq(IRQ NO, (void *) (irq handler)) ;
device destroy (dev class, dev):
class destroy(dev
class) :
cdev del (&chr cdev);
unregister_chrdev region(dev, 1):
printk(KERN_INFO "Device Driver Remove...Done!!/\n");
module init(cdd init); module_exit (cdd_exit):
MODULE LICENSE ("GPL");
MODULE AUTHOR( "XYZ"):
MODULE DESCRIPTION ("CharacterDevice Driver demonstrating Interrupts"):