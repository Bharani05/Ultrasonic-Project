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
