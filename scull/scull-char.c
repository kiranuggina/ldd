#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "scull.h"

static unsigned int major; /* major number for device */
static struct class *scull_class;

struct  scull_dev my_scull[4];

int scull_open(struct inode * inode, struct file * filp)
{
    struct scull_dev *dev; /* device information */

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev; /* for other methods */
    pr_info("Someone tried to open me\n");
    return 0;
}

int scull_release(struct inode * inode, struct file * filp)
{
    pr_info("Someone closed me\n");
    return 0;
}

ssize_t scull_read (struct file *filp, char __user * buf, size_t count, loff_t * offset)
{
    struct scull_dev *dev = filp->private_data;
    int error_count = 0;
    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    error_count = copy_to_user(buf, dev->message, dev->size_of_message);

    if (error_count==0){            // if true then have success
        pr_info("scull_char: Sent %d characters to the user\n", dev->size_of_message);
        return (dev->size_of_message=0);  // clear the position to the start and return 0
    }
    else {
        pr_info("scull_char: Failed to send %d characters to the user\n", error_count);
    return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
    }
    return 0;
}


ssize_t scull_write(struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
    struct scull_dev *dev = filp->private_data;
    //sprintf(message, "%s(%zu letters)", buf, count);   // appending received string with its length
    if(copy_from_user(dev->message,buf,count)){
        return -EFAULT;
    }
    dev->size_of_message = strlen(dev->message);                 // store the length of the stored message
    pr_info("scull_char: Received %zu characters from the user\n", count);
    return count;
}

struct file_operations scull_fops = {
    owner:      THIS_MODULE,
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
        cdev_init(&my_scull[i].cdev, &scull_fops);
        my_scull[i].cdev.owner = THIS_MODULE;
        /* Now make the device live for the users to access */
        cdev_add(&my_scull[i].cdev, MKDEV(MAJOR(devt),MINOR(devt)+i), 1);

        if (IS_ERR(device_create(scull_class,NULL,MKDEV(MAJOR(devt),MINOR(devt)+i),NULL,"scull_char%d",i))) {
            pr_err("Error creating scull char device.\n");
            for(j=0;j<i;j++)
            {
                device_destroy(scull_class,MKDEV(MAJOR(devt),MINOR(devt)+j));
                cdev_del(&my_scull[j].cdev);
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
        cdev_del(&my_scull[i].cdev);
    }
    class_destroy(scull_class);

    pr_info("scull char module Unloaded\n");
}

module_init(scull_char_init_module);
module_exit(scull_char_cleanup_module);

MODULE_AUTHOR("Kiran Kumar Uggina <suryakiran104@gmail.com>");
MODULE_DESCRIPTION("scull device driver");
MODULE_LICENSE("GPL");
