#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#define BUFSIZE  100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static ssize_t my_read(struct file *file,  char *user_buffer,size_t count, loff_t *offs)
{
        char text[] = "Hello from a procfs file\n";
        int to_copy, not_copied, delta;

        to_copy = min(count, sizeof(text));


        not_copied = copy_to_user(text, user_buffer, to_copy);
        printk("procfs_test - You have read %s to me\n",text);

        delta = to_copy - not_copied;

        return delta;
}

static ssize_t my_write(struct file *file,const char *user_buffer,size_t count, loff_t *offs)
{
        char text[255];
        int to_copy, not_copied,delta;

        memset(text, 0, sizeof(text));

        to_copy = min(count, sizeof(text));

        not_copied = copy_from_user(text, user_buffer, to_copy);
        printk("procfs_test - You have written %s to our proc entry\n",text);

        delta = to_copy - not_copied;
        return delta;
}

static struct proc_ops fops =
{
        .proc_read   = my_read,
        .proc_write  = my_write,
};

static int __init my_init(void)
{
        proc_folder = proc_mkdir("bharani", NULL);
        if(proc_folder == NULL)
        {
                printk("procfs_test - Error creating /proc/bharani\n");
                return -ENOMEM;
        }

        proc_file = proc_create("dummy", 0666, proc_folder, &fops);
        if(proc_file == NULL)
        {
                printk("procfs_test - Error crating /proc/bharani/dummy \n");
                proc_remove(proc_folder);
                return -ENOMEM;
        }
        printk("procfs_test - Created /proc/bharani/dummy \n");
        return 0;
}

static void __exit my_exit(void)
{
        printk("procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_file);
        proc_remove(proc_folder);
}

module_init(my_init);
module_exit(my_exit);


// arch/x86/kernel/cpu/proc.c
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/cpu_device_id.h>

static int cpuinfo_proc_show(struct seq_file *m, void *v) {
    // Example information; real implementation queries hardware
    seq_printf(m, "processor       : %d\n", smp_processor_id());
    seq_printf(m, "vendor_id       : %s\n", "GenuineIntel");
    seq_printf(m, "cpu family      : %d\n", 6);
    seq_printf(m, "model           : %d\n", 158);
    seq_printf(m, "model name      : Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz\n");
    seq_printf(m, "cpu MHz         : %lu\n", 1992);
    seq_printf(m, "cache size      : %d KB\n", 8192);
    // Add more fields as necessary
    return 0;
}

static int cpuinfo_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, cpuinfo_proc_show, NULL);
}

static const struct file_operations cpuinfo_proc_fops = {
    .owner   = THIS_MODULE,
    .open    = cpuinfo_proc_open,
    .read    = seq_read,
    .release = single_release,
};

void cpuinfo_proc_init(void) {
    proc_create("cpuinfo", 0, NULL, &cpuinfo_proc_fops);
}

void cpuinfo_proc_exit(void) {
    remove_proc_entry("cpuinfo", NULL);
}

module_init(cpuinfo_proc_init);
module_exit(cpuinfo_proc_exit);

MODULE_LICENSE("GPL");
