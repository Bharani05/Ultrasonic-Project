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

fatal error: asm/cpu_device_id.h: No such file or directory
    4 | #include <asm/cpu_device_id.h>
///cpu

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/utsname.h>

#define BUFSIZE  100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *cpu_info = "CPU Model Name: ";
    char buffer[BUFSIZE];
    struct utsname sysinfo;
    int len;

    if (*offs > 0) {
        return 0; // End of file
    }

    uname(&sysinfo); // Get system information

    len = snprintf(buffer, sizeof(buffer), "%s%s\n", cpu_info, sysinfo.machine);
    if (len >= sizeof(buffer)) {
        len = sizeof(buffer) - 1; // Ensure it fits in the buffer
    }

    if (copy_to_user(user_buffer, buffer, len)) {
        return -EFAULT; // Error copying to user space
    }

    *offs += len;
    return len;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    char text[255];
    int to_copy, not_copied, delta;

    memset(text, 0, sizeof(text));

    to_copy = min(count, sizeof(text) - 1); // Leave space for null terminator

    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk("procfs_test - You have written %s to our proc entry\n", text);

    delta = to_copy - not_copied;
    return delta;
}

static struct proc_ops fops =
{
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL) {
        printk("procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL) {
        printk("procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        return -ENOMEM;
    }
    printk("procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk("procfs_test - Removing /proc/bharani/dummy\n");
    proc_remove(proc_file);
    proc_remove(proc_folder);
}

module_init(my_init);
module_exit(my_exit);
//error
 error: storage size of ‘sysinfo’ isn’t known
   22 |     struct utsname sysinfo;
      |                    ^~~~~~~
/home/lg-bharani/drivers/bharani/procfile.c:29:5: error: implicit declaration of function ‘uname’; did you mean ‘utsname’? [-Werror=implicit-function-declaration]
   29 |     uname(&sysinfo); // Get system information
      |     ^~~~~
// update
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUFSIZE  100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *proc_cpuinfo_path = "/proc/cpuinfo";
    struct file *cpuinfo_file;
    char *cpuinfo_buf;
    loff_t pos = 0;
    ssize_t ret;
    mm_segment_t oldfs;

    if (*offs > 0) {
        return 0; // End of file
    }

    cpuinfo_file = filp_open(proc_cpuinfo_path, O_RDONLY, 0);
    if (IS_ERR(cpuinfo_file)) {
        printk(KERN_ERR "procfs_test - Error opening %s\n", proc_cpuinfo_path);
        return PTR_ERR(cpuinfo_file);
    }

    cpuinfo_buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!cpuinfo_buf) {
        filp_close(cpuinfo_file, NULL);
        return -ENOMEM;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    ret = kernel_read(cpuinfo_file, cpuinfo_buf, BUFSIZE - 1, &pos);
    set_fs(oldfs);
    filp_close(cpuinfo_file, NULL);

    if (ret < 0) {
        kfree(cpuinfo_buf);
        return ret;
    }

    cpuinfo_buf[ret] = '\0'; // Null-terminate the buffer

    if (copy_to_user(user_buffer, cpuinfo_buf, ret)) {
        kfree(cpuinfo_buf);
        return -EFAULT;
    }

    kfree(cpuinfo_buf);
    *offs += ret;
    return ret;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    char text[255];
    int to_copy, not_copied, delta;

    memset(text, 0, sizeof(text));

    to_copy = min(count, sizeof(text) - 1);

    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk(KERN_INFO "procfs_test - You have written %s to our proc entry\n", text);

    delta = to_copy - not_copied;
    return delta;
}

static struct proc_ops fops =
{
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        return -ENOMEM;
    }
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "procfs_test - Removing /proc/bharani/dummy\n");
    proc_remove(proc_file);
    proc_remove(proc_folder);
}

module_init(my_init);
module_exit(my_exit);
//error
error: implicit declaration of function ‘get_fs’; did you mean ‘sget_fc’? [-Werror=implicit-function-declaration]
   44 |     oldfs = get_fs();
      |             ^~~~~~
      |             sget_fc
/home/lg-bharani/drivers/bharani/procfile.c:44:13: error: incompatible types when assigning to type ‘mm_segment_t’ from type ‘int’
/home/lg-bharani/drivers/bharani/procfile.c:45:5: error: implicit declaration of function ‘set_fs’; did you mean ‘sget_fc’? [-Werror=implicit-function-declaration]
   45 |     set_fs(KERNEL_DS);
      |     ^~~~~~
      |     sget_fc
/home/lg-bharani/drivers/bharani/procfile.c:45:12: error: ‘KERNEL_DS’ undeclared (first use in this function); did you mean ‘KERNFS_NS’?
   45 |     set_fs(KERNEL_DS);
      |            ^~~~~~~~~
      |            KERNFS_NS
//update
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUFSIZE  1024  // Increase buffer size to accommodate more content

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *proc_cpuinfo_path = "/proc/cpuinfo";
    struct file *cpuinfo_file;
    char *cpuinfo_buf;
    loff_t pos = 0;
    ssize_t ret;
    mm_segment_t oldfs;

    if (*offs > 0) {
        return 0; // End of file
    }

    cpuinfo_file = filp_open(proc_cpuinfo_path, O_RDONLY, 0);
    if (IS_ERR(cpuinfo_file)) {
        printk(KERN_ERR "procfs_test - Error opening %s\n", proc_cpuinfo_path);
        return PTR_ERR(cpuinfo_file);
    }

    cpuinfo_buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!cpuinfo_buf) {
        filp_close(cpuinfo_file, NULL);
        return -ENOMEM;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    ret = kernel_read(cpuinfo_file, cpuinfo_buf, BUFSIZE - 1, &pos);
    set_fs(oldfs);
    filp_close(cpuinfo_file, NULL);

    if (ret < 0) {
        kfree(cpuinfo_buf);
        return ret;
    }

    cpuinfo_buf[ret] = '\0'; // Null-terminate the buffer

    if (copy_to_user(user_buffer, cpuinfo_buf, ret)) {
        kfree(cpuinfo_buf);
        return -EFAULT;
    }

    kfree(cpuinfo_buf);
    *offs += ret;
    return ret;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    char text[255];
    int to_copy, not_copied, delta;

    memset(text, 0, sizeof(text));

    to_copy = min(count, sizeof(text) - 1);

    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk(KERN_INFO "procfs_test - You have written %s to our proc entry\n", text);

    delta = to_copy - not_copied;
    return delta;
}

static struct proc_ops fops =
{
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        return -ENOMEM;
    }
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "procfs_test - Removing /proc/bharani/dummy\n");
    proc_remove(proc_file);
    proc_remove(proc_folder);
}

module_init(my_init);
module_exit(my_exit);

// error
implicit declaration of function ‘sget_fs’; did you mean ‘sget_fc’? [-Werror=implicit-function-declaration]
   44 |     oldfs = sget_fs();
      |             ^~~~~~~
      |             sget_fc
/home/lg-bharani/drivers/bharani/procfile.c:44:13: error: incompatible types when assigning to type ‘mm_segment_t’ from type ‘int’
/home/lg-bharani/drivers/bharani/procfile.c:45:5: error: implicit declaration of function ‘set_fs’; did you mean ‘sget_fc’? [-Werror=implicit-function-declaration]
   45 |     set_fs(KERNEL_DS);
      |     ^~~~~~
      |     sget_fc
/home/lg-bharani/drivers/bharani/procfile.c:45:12: error: ‘KERNEL_DS’ undeclared (first use in this function); did you mean ‘KERNFS_NS’?
   45 |     set_fs(KERNEL_DS);
      |            ^~~~~~~~~
      |            KERNFS_NS
//update

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUFSIZE  1024  // Increase buffer size to accommodate more content

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *proc_cpuinfo_path = "/proc/cpuinfo";
    struct file *cpuinfo_file;
    char *cpuinfo_buf;
    loff_t pos = 0;
    ssize_t ret;

    if (*offs > 0) {
        return 0; // End of file
    }

    cpuinfo_file = filp_open(proc_cpuinfo_path, O_RDONLY, 0);
    if (IS_ERR(cpuinfo_file)) {
        printk(KERN_ERR "procfs_test - Error opening %s\n", proc_cpuinfo_path);
        return PTR_ERR(cpuinfo_file);
    }

    cpuinfo_buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!cpuinfo_buf) {
        filp_close(cpuinfo_file, NULL);
        return -ENOMEM;
    }

    ret = kernel_read(cpuinfo_file, cpuinfo_buf, BUFSIZE - 1, &pos);
    filp_close(cpuinfo_file, NULL);

    if (ret < 0) {
        kfree(cpuinfo_buf);
        return ret;
    }

    cpuinfo_buf[ret] = '\0'; // Null-terminate the buffer

    if (copy_to_user(user_buffer, cpuinfo_buf, ret)) {
        kfree(cpuinfo_buf);
        return -EFAULT;
    }

    kfree(cpuinfo_buf);
    *offs += ret;
    return ret;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    char text[255];
    int to_copy, not_copied, delta;

    memset(text, 0, sizeof(text));

    to_copy = min(count, sizeof(text) - 1);

    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk(KERN_INFO "procfs_test - You have written %s to our proc entry\n", text);

    delta = to_copy - not_copied;
    return delta;
}

static struct proc_ops fops =
{
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        return -ENOMEM;
    }
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "procfs_test - Removing /proc/bharani/dummy\n");
    proc_remove(proc_file);
    proc_remove(proc_folder);
}

module_init(my_init);
module_exit(my_exit);
// error 
 cannot create /home/lg-bharani/drivers/bharani/.procfile.o.cmd: Permission denied

//explain

Certainly! Here is a detailed line-by-line explanation of your Linux kernel module code:

### Header Inclusions

```c
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
```

- **`#include <linux/module.h>`**: Includes functions and macros for defining a kernel module, such as `module_init()` and `module_exit()`.
- **`#include <linux/moduleparam.h>`**: Provides functionality for module parameters.
- **`#include <linux/init.h>`**: Includes macros for module initialization and cleanup.
- **`#include <linux/kernel.h>`**: Includes kernel-specific functions and macros, like `printk()`.
- **`#include <linux/proc_fs.h>`**: Provides definitions for interacting with the `/proc` filesystem.
- **`#include <linux/uaccess.h>`**: Functions for copying data between user space and kernel space.
- **`#include <linux/fs.h>`**: File operations and related structures.
- **`#include <linux/slab.h>`**: Memory allocation functions like `kmalloc()` and `kfree()`.

### Define Constants

```c
#define BUFSIZE  1024  // Increase buffer size to accommodate more content
```

- **`#define BUFSIZE 1024`**: Defines a buffer size of 1024 bytes for reading data from `/proc/cpuinfo`.

### Module Metadata

```c
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");
```

- **`MODULE_LICENSE("GPL")`**: Specifies that the module is licensed under the GNU General Public License.
- **`MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>")`**: Provides the author’s name and email address.
- **`MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks")`**: Describes the purpose of the module.

### Global Variables

```c
static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;
```

- **`static struct proc_dir_entry *proc_folder`**: Pointer to the `/proc` directory entry for the module’s folder.
- **`static struct proc_dir_entry *proc_file`**: Pointer to the `/proc` file entry created by the module.

### Read Function

```c
static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *proc_cpuinfo_path = "/proc/cpuinfo";
    struct file *cpuinfo_file;
    char *cpuinfo_buf;
    loff_t pos = 0;
    ssize_t ret;

    if (*offs > 0) {
        return 0; // End of file
    }

    cpuinfo_file = filp_open(proc_cpuinfo_path, O_RDONLY, 0);
    if (IS_ERR(cpuinfo_file)) {
        printk(KERN_ERR "procfs_test - Error opening %s\n", proc_cpuinfo_path);
        return PTR_ERR(cpuinfo_file);
    }

    cpuinfo_buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!cpuinfo_buf) {
        filp_close(cpuinfo_file, NULL);
        return -ENOMEM;
    }

    ret = kernel_read(cpuinfo_file, cpuinfo_buf, BUFSIZE - 1, &pos);
    filp_close(cpuinfo_file, NULL);

    if (ret < 0) {
        kfree(cpuinfo_buf);
        return ret;
    }

    cpuinfo_buf[ret] = '\0'; // Null-terminate the buffer

    if (copy_to_user(user_buffer, cpuinfo_buf, ret)) {
        kfree(cpuinfo_buf);
        return -EFAULT;
    }

    kfree(cpuinfo_buf);
    *offs += ret;
    return ret;
}
```

- **`static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)`**: Defines a function to read data from the proc file.
- **`static const char *proc_cpuinfo_path = "/proc/cpuinfo";`**: Path to the `/proc/cpuinfo` file, which contains CPU information.
- **`struct file *cpuinfo_file;`**: File pointer used to open `/proc/cpuinfo`.
- **`char *cpuinfo_buf;`**: Buffer to store data read from `/proc/cpuinfo`.
- **`loff_t pos = 0;`**: Offset for reading from the file.
- **`ssize_t ret;`**: Number of bytes read or error code.

**`if (*offs > 0) { return 0; }`**: Check if the end of the file has been reached.

- **`cpuinfo_file = filp_open(proc_cpuinfo_path, O_RDONLY, 0);`**: Opens `/proc/cpuinfo` for reading.
- **`if (IS_ERR(cpuinfo_file)) { ... }`**: Checks if opening the file failed.
- **`cpuinfo_buf = kmalloc(BUFSIZE, GFP_KERNEL);`**: Allocates memory for the buffer.
- **`if (!cpuinfo_buf) { ... }`**: Checks if memory allocation failed.
- **`ret = kernel_read(cpuinfo_file, cpuinfo_buf, BUFSIZE - 1, &pos);`**: Reads data from `/proc/cpuinfo` into `cpuinfo_buf`.
- **`filp_close(cpuinfo_file, NULL);`**: Closes the file after reading.
- **`if (ret < 0) { ... }`**: Checks if reading failed.
- **`cpuinfo_buf[ret] = '\0';`**: Null-terminates the buffer.
- **`if (copy_to_user(user_buffer, cpuinfo_buf, ret)) { ... }`**: Copies data from kernel space to user space.
- **`*offs += ret;`**: Updates the offset for the next read.

### Write Function

```c
static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    char text[255];
    int to_copy, not_copied, delta;

    memset(text, 0, sizeof(text));

    to_copy = min(count, sizeof(text) - 1);

    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk(KERN_INFO "procfs_test - You have written %s to our proc entry\n", text);

    delta = to_copy - not_copied;
    return delta;
}
```

- **`static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)`**: Defines a function to handle writes to the proc file.
- **`char text[255];`**: Buffer to store data written by the user.
- **`int to_copy, not_copied, delta;`**: Variables for managing the copy operation.
- **`memset(text, 0, sizeof(text));`**: Initializes the `text` buffer.
- **`to_copy = min(count, sizeof(text) - 1);`**: Determines the number of bytes to copy.
- **`not_copied = copy_from_user(text, user_buffer, to_copy);`**: Copies data from user space to kernel space.
- **`printk(KERN_INFO "procfs_test - You have written %s to our proc entry\n", text);`**: Logs the written data.
- **`delta = to_copy - not_copied;`**: Calculates the number of bytes successfully copied.
- **`return delta;`**: Returns the number of bytes written.

### File Operations Structure

```c
static struct proc_ops fops =
{
    .proc_read = my_read,
    .proc_write = my_write,
};
```

- **`static struct proc_ops fops`**: Defines file operations for the proc file.
- **`.proc_read = my_read,`**: Assigns the `my_read` function to handle read operations.
- **`.proc_write = my_write,`**: Assigns the `my_write` function to handle write operations.

### Module Initialization

```c
static int __init my_init(void)
{
    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        return -ENOMEM;
    }
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}
```

- **`static int __init my_init(void)`**: Initialization function for the module.
- **`proc_folder = proc_mkdir("bharani", NULL);`**: Creates a new directory `/proc/bharani`.
- **`if (proc_folder == NULL) { ... }`**: Checks if the directory creation failed.
- **`proc_file = proc_create("dummy", 0666, proc_folder,

// final code

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUFSIZE  1024  // Increase buffer size to accommodate more content

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks");

static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *proc_cpuinfo_path = "/proc/cpuinfo";
    struct file *cpuinfo_file;
    char *cpuinfo_buf;
    loff_t pos = 0;
    ssize_t ret;

    if (*offs > 0) {
        return 0; // End of file
    }

    cpuinfo_file = filp_open(proc_cpuinfo_path, O_RDONLY, 0);
    if (IS_ERR(cpuinfo_file)) {
        printk(KERN_ERR "procfs_test - Error opening %s\n", proc_cpuinfo_path);
        return PTR_ERR(cpuinfo_file);
    }

    cpuinfo_buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!cpuinfo_buf) {
        filp_close(cpuinfo_file, NULL);
        return -ENOMEM;
    }

    ret = kernel_read(cpuinfo_file, cpuinfo_buf, BUFSIZE - 1, &pos);
    filp_close(cpuinfo_file, NULL);

    if (ret < 0) {
        kfree(cpuinfo_buf);
        return ret;
    }

    cpuinfo_buf[ret] = '\0'; // Null-terminate the buffer

    if (copy_to_user(user_buffer, cpuinfo_buf, ret)) {
        kfree(cpuinfo_buf);
        return -EFAULT;
    }

    kfree(cpuinfo_buf);
    *offs += ret;
    return ret;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    char text[255];
    int to_copy, not_copied, delta;

    memset(text, 0, sizeof(text));

    to_copy = min(count, sizeof(text) - 1);

    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk(KERN_INFO "procfs_test - You have written %s to our proc entry\n", text);

    delta = to_copy - not_copied;
    return delta;
}

static struct proc_ops fops =
{
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        return -ENOMEM;
    }
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "procfs_test - Removing /proc/bharani/dummy\n");
    proc_remove(proc_file);
    proc_remove(proc_folder);
}

module_init(my_init);
module_exit(my_exit);


could not insert module processentry.ko: Cannot allocate memory

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUFSIZE  1024  // Buffer size for reading proc data

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name <your.email@example.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs, implements read and write callbacks, and includes personal information");

// Define the buffer for storing data written to the proc file
static char *data_buffer;
static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

// Define personal information
static const char *personal_info = "Personal Information:\nName: Your Name\nEmail: your.email@example.com\n";

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    ssize_t ret;

    // Check if end of file has been reached
    if (*offs > 0)
        return 0;

    // Allocate buffer for reading
    char *buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    // Format the buffer with personal information and proc data
    ret = snprintf(buf, BUFSIZE - 1, "%s\nData: %s\n", personal_info, data_buffer);

    // Copy the buffer to user space
    if (copy_to_user(user_buffer, buf, ret)) {
        kfree(buf);
        return -EFAULT;
    }

    kfree(buf);
    *offs += ret;
    return ret;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    int to_copy, not_copied, delta;

    // Prepare buffer for writing
    memset(data_buffer, 0, BUFSIZE);

    to_copy = min(count, (size_t)(BUFSIZE - 1));
    not_copied = copy_from_user(data_buffer, user_buffer, to_copy);

    delta = to_copy - not_copied;
    data_buffer[delta] = '\0'; // Null-terminate the buffer

    printk(KERN_INFO "procfs_test - You have written: %s\n", data_buffer);

    return delta;
}

static struct proc_ops fops = {
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    // Allocate buffer
    data_buffer = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!data_buffer)
        return -ENOMEM;
    
    // Initialize buffer
    memset(data_buffer, 0, BUFSIZE);

    // Create proc folder and file
    proc_folder = proc_mkdir("bharani", NULL);
    if (!proc_folder) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (!proc_file) {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        kfree(data_buffer);
        return -ENOMEM;
    }

    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "procfs_test - Removing /proc/bharani/dummy\n");
    proc_remove(proc_file);
    proc_remove(proc_folder);
    kfree(data_buffer);
}

module_init(my_init);
module_exit(my_exit);
//final
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/stat.h>

#define BUFSIZE  100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharani<bhrnbala@gmail.com>");
MODULE_DESCRIPTION("Module creates a folder and file in procfs and implements read and write callbacks and implement the thread function");

static char *data_buffer;//pointer to a buffer to hold a data written to the file in /proc
static struct proc_dir_entry *proc_folder;//pointer to the /proc directory entry created for the module
static struct proc_dir_entry *proc_file;//poointer to the /pro file entry inside the /proc directory
static struct task_struct *procfs_thread;//pointer to the kernel thread structure
static bool running = true;//boolean flag to control the kernel threadrunning status

static ssize_t my_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
    static const char *personal_info = "Personal Information:\nName: Bharani\nEmail: bharani.v@lge.com\n";
    ssize_t ret;
    char *buf;

    printk(KERN_INFO "procfs_test - my_read called, offset = %lld\n", *offs);


    if (*offs > 0)
        return 0;

    buf = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    ret = snprintf(buf, BUFSIZE - 1, "%s\nData:%s\n", personal_info, data_buffer);

    if (copy_to_user(user_buffer, buf, ret))
    {
        kfree(buf);
        return -EFAULT;
    }

    kfree(buf);
    *offs += ret;

    return ret;
}

static ssize_t my_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
    int to_copy, not_copied, delta;

    memset(data_buffer, 0, BUFSIZE);

    to_copy = min(count, (size_t)(BUFSIZE - 1));

    not_copied = copy_from_user(data_buffer, user_buffer, to_copy);

    delta = to_copy - not_copied;
    data_buffer[delta] = '\0';

    printk(KERN_INFO "procfs_test - You have written: %s\n", data_buffer);

    return delta;
}

static struct proc_ops fops = {
    .proc_read = my_read,
    .proc_write = my_write,
};

static int thread_func(void *data)
{
    while (running)
    {
        printk(KERN_INFO "procfs_test - Thread running - PID %d\n", current->pid);
        msleep(1000);
    }
    return 0;
}

static int __init my_init(void)
{
    data_buffer = kmalloc(BUFSIZE, GFP_KERNEL);
    if (!data_buffer)
        return -ENOMEM;

    memset(data_buffer, 0, BUFSIZE);

    proc_folder = proc_mkdir("bharani", NULL);
    if (proc_folder == NULL)
    {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani\n");
        kfree(data_buffer);
        return -ENOMEM;
    }

    proc_file = proc_create("dummy", 0666, proc_folder, &fops);
    if (proc_file == NULL)
    {
        printk(KERN_ERR "procfs_test - Error creating /proc/bharani/dummy\n");
        proc_remove(proc_folder);
        kfree(data_buffer);
        return -ENOMEM;
    }
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy\n");

    procfs_thread = kthread_create(thread_func, NULL, "procfs_thread");

    if (IS_ERR(procfs_thread))
    {
        printk(KERN_ERR "procfs_thread creation failed\n");
        proc_remove(proc_file);
        proc_remove(proc_folder);
        kfree(data_buffer);
        return PTR_ERR(procfs_thread);
    }
    wake_up_process(procfs_thread);
    printk(KERN_INFO "procfs_test - Created /proc/bharani/dummy and started thread\n");

    return 0;
}

static void __exit my_exit(void)
{
    running = false;
    if (procfs_thread)
        kthread_stop(procfs_thread);

    proc_remove(proc_file);
    proc_remove(proc_folder);
    kfree(data_buffer);

    printk(KERN_INFO "procfs_test - Cleaned up /proc/bharani/dummy\n");
}

module_init(my_init);
module_exit(my_exit);


