#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>               /* file_operations, register_chrdev */
#include <linux/uaccess.h>          /* copy_to_user, copy_from_user */
#include <linux/cdev.h>             /* cdev struct */
#include <linux/kernel.h>           /* printk */

#define DEVICE_NAME "mychardev"
#define BUF_SIZE 1024
static int major_number;
static char kernel_buffer[BUF_SIZE] = "Hello from kernel!\n";
static int buffer_len = 19;
// ■■ OPEN & RELEASE FUNCTIONS ■■
static int mydev_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "mychardev: device opened\n");
	return 0; /* 0 = success */
}

static int mydev_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "mychardev: device closed\n");
	return 0;
}
// ■■ READ FUNCTION — most important ■■
static ssize_t mydev_read(struct file *file, char __user *user_buf, size_t count, loff_t *offset) {
	int bytes_to_read = min((int)count, buffer_len);
	/* copy_to_user: cannot use memcpy — kernel/user memory is separate */
	if (copy_to_user(user_buf, kernel_buffer, bytes_to_read)) {
		return -EFAULT; /* EFAULT = bad address error */
	}
	printk(KERN_INFO "mychardev: sent %d bytes to user\n", bytes_to_read);
	return bytes_to_read;
}
// ■■ WRITE FUNCTION ■■
static ssize_t mydev_write(struct file *file, const char __user *user_buf,
	size_t count, loff_t *offset) {
	int bytes = min((int)count, BUF_SIZE - 1);
	if (copy_from_user(kernel_buffer, user_buf, bytes)) {
		return -EFAULT;
	}
	buffer_len = bytes;
	kernel_buffer[bytes] = '\0';
	printk(KERN_INFO "mychardev: received %d bytes from user\n", bytes);
	return bytes;
}
// ■■ FILE_OPERATIONS & INIT/EXIT ■■
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = mydev_open,
	.release = mydev_release,
	.read = mydev_read,
	.write = mydev_write,
};
static int __init mydev_init(void) {
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "Failed to register device\n");
		return major_number;
	}
	printk(KERN_INFO "mychardev registered with major %d\n", major_number);
	return 0;
}
static void __exit mydev_exit(void) {
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "mychardev unregistered\n");
}
module_init(mydev_init);
module_exit(mydev_exit);
MODULE_LICENSE("GPL");