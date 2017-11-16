/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   chardev.c
 * Author: root
 *
 * Created on November 15, 2017, 3:13 AM
 */

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/pfn.h>
#include <linux/mman.h>


#include "../utils/log.h"
#include "../page_mapper.h"


#include "chardev.h"

static const struct vm_operations_struct mmap_mem_ops = {

};

static int mmap_pages(struct file *file, struct vm_area_struct *vma)
{
    size_t size = vma->vm_end - vma->vm_start;
    /*
    if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
            return -EINVAL;

    if (!private_mapping_ok(vma))
            return -ENOSYS;

    if (!range_is_allowed(vma->vm_pgoff, size))
            return -EPERM;

    if (!phys_mem_access_prot_allowed(file, vma->vm_pgoff, size,
                                            &vma->vm_page_prot))
            return -EINVAL;
    
    vma->vm_page_prot = phys_mem_access_prot(file, vma->vm_pgoff,
                                             size,
                                             vma->vm_page_prot);
*/
    LOG("MMAP called. Start %llu off %llu size %llu perm %llu\n", 
            vma->vm_start, 
            vma->vm_pgoff, 
            size, 
            vma->vm_page_prot.pgprot);
    
    vma->vm_ops = &mmap_mem_ops;

    /* Remap-pfn-range will mark the range VM_IO */
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        vma->vm_pgoff,
                        size,
                        vma->vm_page_prot)) 
    {
        return -EAGAIN;
    }
    
    return 0;
}

/*
* @brief  this is called, when file open
*
* @param  struct inode *inode : inode file desc
*         struct file *filp : private data
*
* @return 0: success
*/
int chardev_open(struct inode *inode, struct file *filp)
{

    LOG("Open called \n");
    
    struct page_mapper_dev *pm; /// device information
    int rc;

    /// Find the device
    pm = container_of(inode->i_cdev, struct page_mapper_dev, chardev);
    if (!pm)
    {
	LOG_ERROR("container_of error \n");

	return -1;
    }

    /// get  current mode
    rc = filp->f_flags & O_ACCMODE;
    switch (rc)
    {
	case O_WRONLY:
	break;

	case O_RDONLY:
	break;

	case O_RDWR:
	break;

	default:
	    //ts->cur_fd_state = OPENED_NONE;
	break;


    }

    /// and use filp->private_data to point to the device data
    filp->private_data = pm;

    return 0;
}

/*
* @brief  this is called, when close
*
* @param  struct inode *inode : inode file desc
*         struct file *filp : private data
*
* @return 0: success
*/
int chardev_release(struct inode *inode,
		    struct file *filp)
{

    LOG("Release called \n");
    
    struct page_mapper_dev *pm = filp->private_data;
    if (!pm)
    {
	LOG_ERROR("private_data NULL \n");

	return -1;
    }

    /// Close tslot
    //ts->cur_fd_state = CLOSE;

    //LOG("called \n");
    
    /// set atomic size to 0
    //clear_sw_rx_buffer_current_size(&ts->rx_buf);

    return 0;
}


/*
* @brief  this is called, while smbd writes dev
*
* @param  struct file *filp : file desc
*         const char __user *buf : here copy info, from data to user
*	  size_t count : size of copied data
*         loff_t *f_pos : offset of data
*
* @return count of read bytes
*/
ssize_t chardev_write(struct file *filp,
		    const char __user *buf,
		    size_t count,
		    loff_t *f_pos)
{

    LOG("Write called \n");
    
    struct page_mapper_dev *pm = filp->private_data;
    if (!pm)
    {
	LOG_ERROR("private_data NULL \n");

	return -1;
    }

    /*
    if (copy_from_user(ts->tx_buf.tx_buffer, buf, count))
    {
	LOG_ERROR("copy_from_user() error. count = %lu \n", count);

	return -EFAULT;
    }
    */

    return count;
}


/*
* @brief  this is called, while smbd read dev
*
* @param  struct file *filp : file desc
*         char __user *buf : here copy info, from data to user
*	  size_t count : size of copied data
*         loff_t *f_pos : offset of data
*
* @return count of read bytes
*/
ssize_t chardev_read(struct file *filp,
	char __user *buf,
	size_t count,
	loff_t *f_pos)
{

    int rc = 0;
    
    LOG("Read called \n");
    
    struct page_mapper_dev *pm = filp->private_data;
    if (!pm)
    {
	LOG_ERROR("private_data NULL \n");

	return -1;
    }
    /*
    /// copy data to user
    if (copy_to_user(buf, ts->rx_buf.rxb[buf_num].data, rc))
    {
	LOG_ERROR("copy_to_user() error. count = %d \n", rc);

	return -EFAULT;
    }
    */
    
    return rc;
}


struct file_operations chardev_fops = {
	.owner = THIS_MODULE,
	.read =	chardev_read,
	.write = chardev_write,
	.open =	chardev_open,
	.release = chardev_release,
        .mmap =	mmap_pages,

};


/*
* @brief  sutup char device
*
* @param  struct ts_struct *ts : timeslot mod
*	  int id : id of timeslot
*	  u32 major_num : device major num
*
* @return 1: success
*	  -1: error
*/
int setup_char_dev(struct page_mapper_dev *pm_dev, u32 major_num)
{
    if (!pm_dev)
    {
	LOG_ERROR("get params error \n");

	return -1;
    }

    int rc;
    int devno = MKDEV(major_num, 0);

    cdev_init(&pm_dev->chardev, &chardev_fops);
    pm_dev->chardev.owner = THIS_MODULE;
    pm_dev->chardev.ops = &chardev_fops;
    
    rc = cdev_add(&pm_dev->chardev, devno, 1);
    if (rc)
    {
	LOG_ERROR("cdev_add() error %d adding %d ts \n", rc, 0);

	return -1;
    }

    LOG("Created char device MAJOR = %d MINOR = %d \n",
	    MAJOR(devno),
	    MINOR(devno));

    return 1;
}

/*
* @brief  close main char device
*
* @param  struct stm_dev *stm : main dev struct
*
* @return void;
*/
void remove_char_device(struct page_mapper_dev *pm_dev)
{
    if (!pm_dev)
    {
	LOG_ERROR("get params error \n");

	return;
    }

     /// destroy associated class
    device_destroy(pm_dev->cdev_class, MKDEV(pm_dev->major_number,
                                            pm_dev->minor_number));

    /// destroy char device
    cdev_del(&pm_dev->chardev);

    /// Unregister char device region
    unregister_chrdev_region(MKDEV(pm_dev->major_number, 0),
			    1);

    LOG("Chardev MAJOR = %d released \n",
	pm_dev->major_number);

}

/*
* @brief  init char device region
*
* @param  struct stm_dev *stm : main dev struct
*
* @return 1: success
*	  -1: error
*/
int create_char_device(struct page_mapper_dev *pm_dev)
{
    if (!pm_dev)
    {
	LOG_ERROR("get params error \n");

	return -1;
    }

    int rc;

    /// copy major num
    pm_dev->major_number = MAJOR_NUMBER;

    dev_t dev = MKDEV(pm_dev->major_number, 0);

    char region_name[16] = {0}; /// name of char region. Can see in /proc/devices

    sprintf(region_name, "%s", DEV_NAME);

    /// register your major, and accept a dynamic number
    rc = alloc_chrdev_region(&dev,
	    0,
	    1,
	    region_name);

    if (rc < 0)
    {
	LOG_ERROR("alloc_chrdev_region error \n");

	return -1;
    }

    pm_dev->cdev_class = class_create(THIS_MODULE, DEV_NAME);
    
    /// copy new major, minor numbers
    pm_dev->major_number = MAJOR(dev);
    pm_dev->minor_number = MINOR(dev);

    LOG("Allocated chardev region MAJOR = %d MINOR = %d\n",
	pm_dev->major_number, pm_dev->minor_number);

          /// create char device
    rc = setup_char_dev(pm_dev,
		    pm_dev->major_number);

    if (rc == -1)
    {
	LOG_ERROR("setup_char_dev() error \n");

	return -1;
    }

    dev_t devno = MKDEV(pm_dev->major_number, pm_dev->minor_number);

    /// create udev rule
    pm_dev->sysfs_dev = device_create(pm_dev->cdev_class,
                                NULL,
                                devno,
                                NULL,
                                DEV_NAME);
    if (!pm_dev->sysfs_dev)
    {
	LOG_ERROR("device_create() error \n");

	return -1;
    }
    
    return 1;
}


MODULE_LICENSE("GPL");