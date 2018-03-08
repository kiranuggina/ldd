#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>

static unsigned int major; /* major number for device */
static struct class *scull_class;
static struct cdev scull_cdev[4];


int scull_open(struct inode * inode, struct file * filp)
{
    pr_info("Someone tried to open me\n");
    return 0;
}

int scull_release(struct inode * inode, struct file * filp)
{
    pr_info("Someone closed me\n");
    return 0;
}

ssize_t scull_read (struct file *filp, char __user * buf, size_t count,
                                loff_t * offset)
{
    pr_info("Nothing to read guy\n");
    return 0;
}


ssize_t scull_write(struct file * filp, const char __user * buf, size_t count,
                                loff_t * offset)
{
    pr_info("Can't accept any data guy\n");
    return count;
}

struct file_operations scull_fops = {
    open:       scull_open,
    release:    scull_release,
    read:       scull_read,
    write:      scull_write,
};

static int __init scull_char_init_module(void)
{
    int error,i,j;
    dev_t devt = 0;

    /* Get a range of minor numbers (starting with 0) to work with */
    error = alloc_chrdev_region(&devt, 0, 1, "scull_char");
    if (error < 0) {
        pr_err("Can't get major number\n");
        return error;
    }
    major = MAJOR(devt);
    pr_info("scull_char major number = %d\n",major);

    /* Create device class, visible in /sys/class */
    scull_class = class_create(THIS_MODULE, "scull_char_class");
    if (IS_ERR(scull_class)) {
        pr_err("Error creating scull char class.\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return PTR_ERR(scull_class);
    }

    for(i=0;i<4;i++){
        /* Initialize the char device and tie a file_operations to it */
        cdev_init(&scull_cdev[i], &scull_fops);
        scull_cdev[i].owner = THIS_MODULE;
        /* Now make the device live for the users to access */
        cdev_add(&scull_cdev[i], MKDEV(MAJOR(devt),MINOR(devt)+i), 1);

        if (IS_ERR(device_create(scull_class,NULL,MKDEV(MAJOR(devt),MINOR(devt)+i),NULL,"scull_char%d",i))) {
            pr_err("Error creating scull char device.\n");
            for(j=0;j<i;j++)
            {
                device_destroy(scull_class,MKDEV(MAJOR(devt),MINOR(devt)+j));
                cdev_del(&scull_cdev[j]);
            }
            class_destroy(scull_class);
            unregister_chrdev_region(devt, 1);
            return -1;
        }    
    }

    pr_info("scull char module loaded\n");
    return 0;
}

static void __exit scull_char_cleanup_module(void)
{
    int i;
    unregister_chrdev_region(MKDEV(major, 0), 1);
    for(i=0;i<4;i++)
    {
        device_destroy(scull_class, MKDEV(major, i));
        cdev_del(&scull_cdev[i]);
    }
    class_destroy(scull_class);

    pr_info("scull char module Unloaded\n");
}

module_init(scull_char_init_module);
module_exit(scull_char_cleanup_module);

MODULE_AUTHOR("Kiran Kumar Uggina <suryakiran104@gmail.com>");
MODULE_DESCRIPTION("scull device driver");
MODULE_LICENSE("GPL");
