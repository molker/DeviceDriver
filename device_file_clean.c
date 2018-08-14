/*Author: Matthew Olker
 *Class: CSC4210
 *Date: 3/15/2018
 *Summary: A simple device driver that has the current file operation to read from it.
 *Should return "Hello world from the kernel mode!"
 */

#include "device_file.h"
#include <linux/fs.h> 	     /* file stuff */
#include <linux/kernel.h>    /* printk() */
#include <linux/errno.h>     /* error codes */
#include <linux/module.h>  /* THIS_MODULE */
#include <linux/cdev.h>      /* char device stuff */
#include <asm/uaccess.h>  /* copy_to_user() */
#include <linux/kdev_t.h> /*gives dev_t type*/
#include <linux/cdev.h>	  /*gives cdev struct*/

static const char    g_s_Hello_World_string[] = "Hello world from kernel mode!\n\0";
static const ssize_t g_s_Hello_World_size = sizeof(g_s_Hello_World_string);
//parameters for new way of registering/unregistering
static dev_t dev;
static struct cdev cDev;
static int cnt = 1;

/*===============================================================================================*/
static ssize_t device_file_read(
                           struct file *file_ptr
                        , char __user *user_buffer
                        , size_t count
                        , loff_t *possition)
{
   printk( KERN_NOTICE "Simple-driver: Device file is read at offset = %i, read bytes count = %u"
            , (int)*possition
            , (unsigned int)count );
   //make sure offset isn't larger than the string size
   if( *possition >= g_s_Hello_World_size )
      return 0;

   //make sure the offset+count isn't larger than string size
   if( *possition + count > g_s_Hello_World_size )
      count = g_s_Hello_World_size - *possition;

   //make sure reading the message to the user doesn't fail
   if( copy_to_user(user_buffer, g_s_Hello_World_string + *possition, count) != 0 )
      return -EFAULT;

   //update the offset
   *possition += count;
   return count;
}
/*===============================================================================================*/
static struct file_operations simple_driver_fops = 
{
   .owner   = THIS_MODULE,
   .read    = device_file_read,
};

static int device_file_major_number = 0;

static const char device_name[] = "simple-driver";

/*===============================================================================================*/
int register_device(void)
{
      int result = 0;
	  int first_minor = 0;
	  int cdev_added = 0;

      printk( KERN_NOTICE "Simple-driver: register_device() is called." );

	  /* dynamically allocate space for the character device
	   * dev - dev type that holds the major and minor number after alloc_chrdev_region
	   * first_minor - tells the function which minor number to use
	   * cnt - the number of device numbers to return
	   * device_name - name associated with major number
	   */
	  result = alloc_chrdev_region(&dev,first_minor,cnt,device_name);
	
	  //initalize character device/cdev struct and set file operations 
	  cdev_init(&cDev,&simple_driver_fops);
	  //add the struct to the system
	  cdev_added = cdev_add(&cDev,dev,cnt);
	  
	  //Error checking
	  if( cdev_added < 0 ){
	  	 printk( KERN_WARNING "Simple-driver: failed to add the character device to the system");
	  	 return cdev_added;
	  }

      if( result < 0 )
      {
         printk( KERN_WARNING "Simple-driver:  can\'t register character device with errorcode = %i", result );
         return result;
      }

      device_file_major_number = result;
      printk( KERN_NOTICE "Simple-driver: registered character device with major number = %i and minor numbers 0...255"
                  , device_file_major_number );

      return 0;
}
/*-----------------------------------------------------------------------------------------------*/
void unregister_device(void)
{
   printk( KERN_NOTICE "Simple-driver: unregister_device() is called" );
   if(device_file_major_number != 0)
   {
   	  //delete the character device and unregister the major number/device
      cdev_del(&cDev);
	  unregister_chrdev_region(dev,cnt);
   }
}
