/*
 * This character device driver (/dev/dummy) is used
 * for read-only purpose. It will show you how much
 * you open this device and show the counter through
 * userland read operation (e.g., cat /dev/dummy)
 *
 * Copyright (c) 2025 Paulus Gandung Prakosa <rvn.plvhx@gmail.com>
 */

#include <linux/atomic.h>
#include <linux/compiler_types.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/sprintf.h>
#include <linux/types.h>
#include <linux/version.h>

#define CDEV_NAME "dummy"
#define CDEV_KBUF_LEN 128

enum {
    CDEV_IDLE,
    CDEV_ATOMIC_OPEN
};

static int major;
static struct class *klass;

static atomic_t devstate = ATOMIC_INIT(CDEV_IDLE);
static atomic_t devcount = ATOMIC_INIT(0);

static char kbuf[CDEV_KBUF_LEN + 1];

static int dummy_cdev_open(struct inode *inode, struct file *filp)
{
    if (atomic_cmpxchg(&devstate, CDEV_IDLE, CDEV_ATOMIC_OPEN))
        return -EBUSY;

    sprintf(kbuf, "%s has been opened %d times.\n", CDEV_NAME, atomic_read(&devcount));

    atomic_inc(&devcount);
    try_module_get(THIS_MODULE);
    return 0;
}

static int dummy_cdev_release(struct inode *inode, struct file *filp)
{
    atomic_set(&devstate, CDEV_IDLE);
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t dummy_cdev_read(struct file *filp, char __user *ubuf,
                               size_t ulen, loff_t *poff)
{
    ssize_t tbytes;
    atomic_t bcount = ATOMIC_INIT(0);
    const char *mptr = kbuf;

    // check if our buffer offset is reaching EOF.
    if (*(mptr + *poff) == '\0') {
        *poff = 0;
        return 0;
    }

    // move buffer ptr to current offset
    mptr += *poff;

    while (ulen && *mptr) {
        put_user(*(mptr++), ubuf++);
        ulen--;
        atomic_inc(&bcount);
    }

    tbytes = atomic_read(&bcount);
    *poff += tbytes;

    return tbytes;
}

static ssize_t dummy_cdev_write(struct file *filp, const char __user *ubuf,
                                size_t ulen, loff_t *poff)
{
    pr_alert("Sorry, this operation is not supported.\n");
    return 0;
}

static struct file_operations fops = {
    .open = dummy_cdev_open,
    .release = dummy_cdev_release,
    .read = dummy_cdev_read,
    .write = dummy_cdev_write
};

static int __init dummy_ctor(void)
{
    int ret;
    struct device *rdev;

    major = register_chrdev(0, CDEV_NAME, &fops);

    if (major < 0)
        return major;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    klass = class_create(CDEV_NAME);
#else
    klass = class_create(THIS_MODULE, CDEV_NAME);
#endif

    if (IS_ERR(klass)) {
        ret = PTR_ERR(klass);
        goto __release_chrdev;
    }

    rdev = device_create(klass, NULL,
                         MKDEV(major, 0), NULL, CDEV_NAME);

    if (IS_ERR(rdev)) {
        ret = PTR_ERR(rdev);
        goto __release_classdev;
    }

    return 0;

__release_classdev:
    class_destroy(klass);

__release_chrdev:
    unregister_chrdev(major, CDEV_NAME);
    return ret;
}

static void __exit dummy_dtor(void)
{
    device_destroy(klass, MKDEV(major, 0));
    class_destroy(klass);
    unregister_chrdev(major, CDEV_NAME);
    return;
}

module_init(dummy_ctor);
module_exit(dummy_dtor);

MODULE_AUTHOR("Paulus Gandung Prakosa <rvn.plvhx@gmail.com>");
MODULE_DESCRIPTION("Dummy character device driver that shows how much this device has been opened");
MODULE_LICENSE("GPL");
