#include "device_file.h"
#include <linux/fs.h> 	     /* file stuff */
#include <linux/kernel.h>    /* printk() */
#include <linux/errno.h>     /* error codes */
#include <linux/module.h>  /* THIS_MODULE */
#include <linux/cdev.h>      /* char device stuff */
#include <asm/uaccess.h>  /* copy_to_user() */
#include <linux/kdev_t.h> /*gives dev_t type*/
#include <linux/cdev.h>	  /*gives cdev struct*/
#include <linux/string.h> /*strlen()*/
#include <linux/slab.h>
#include <linux/semaphore.h>
/*
	Basically: 
	use the kernel semaphore. it uses structs instead of sem_t
	use a bounded buffer of structs that contain a pointer to the word and an int with the length
*/

#define BUFFER_SIZE 5

static dev_t dev;
static struct cdev * cDev;
static int cnt = 1;
static char 		*memory_buffer = "Hello world from kernel mode!\n\0";
static ssize_t 		memory_buffer_size = sizeof(memory_buffer);
static int readCount = 0;
static int inBufferPt = 0, removeBufferPt = 0;

static struct input{
	char *word;
	int word_length;
};

static input buffer[BUFFER_SIZE];


/*===============================================================================================*/
static ssize_t device_file_read(
                           struct file *file_ptr
                        , char __user *user_buffer
                        , size_t count
                        , loff_t *possition)
{

	/*sem_wait(&S);
		readCount++;
		if(readCount == 1){
			printk( KERN_NOTICE "Simple-driver: Waiting on W");
			sem_wait(&W);
		}
	sem_post(&S);*/

   printk( KERN_NOTICE "Simple-driver: Device file is read at offset = %i, read bytes count = %u\n"
            , (int)*possition
            , (unsigned int)count );

   if( *possition >= memory_buffer_size )
      return 0;

   if( *possition + count > memory_buffer_size )
      count = memory_buffer_size - *possition;

   if( copy_to_user(user_buffer, memory_buffer + *possition, memory_buffer_size) != 0 )
	 return -EFAULT;   

   *possition += count;
   kfree(memory_buffer);

	/*sem_wait(&S);
		readCount--;
		if(readCount == 0){
			printk( KERN_NOTICE "Simple-driver: Waiting on W");
			sem_post(&W);
		}
	sem_post(&S);*/

   return count;
}
/*===============================================================================================*/
static ssize_t device_file_write(
							struct file *file_ptr,
							const char __user *user_buffer,
							size_t count,
							loff_t *possition)
{
	//sem_wait(&W);
	memory_buffer_size = count;
	memory_buffer = kmalloc(count*sizeof(char),GFP_KERNEL);	
	printk( KERN_NOTICE "Simple-driver: Device file is writing at offset = %i, written bytes count = %u\n"
			, (int)*possition
			, (unsigned int)count );

	printk( KERN_NOTICE "Simple-driver: User-buffer: %s\n",user_buffer);

	if(strlen(user_buffer) == 0)
		return 0;

	if(count == 0){
		printk( KERN_NOTICE "Simple-driver: nothing being written to driver");
		return 0;
	}

	if( copy_from_user(memory_buffer+*possition, user_buffer, count) != 0)
		return -EFAULT;
	
	//sem_post(&W);

	return count;
}
/*===============================================================================================*/
static struct file_operations simple_driver_fops = 
{
   .owner   = THIS_MODULE,
   .read    = device_file_read,
   .write	= device_file_write,
};

static int device_file_major_number = 0;

static const char device_name[] = "simple-driver";

/*===============================================================================================*/
int register_device(void)
{
      int result = 0;
	  int first_minor = 0;
	  int cdev_added = 0;

      printk( KERN_NOTICE "Simple-driver: register_device() is called.\n" );

	  /* dynamically allocate space for the character device
	   * dev - dev type that holds the major and minor number after alloc_chrdev_region
	   * first_minor - tells the function which minor number to use
	   * cnt - the number of device numbers to return
	   * device_name - name associated with major number
	   */
	  cDev = cdev_alloc();

	  if (cDev == NULL){
		  printk( KERN_WARNING "Simple-driver: error allocating space for cdev\n");
		  return -1;
	  }

	  result = alloc_chrdev_region(&dev,first_minor,cnt,device_name);
	
	  //initalize character device/cdev struct and set file operations 
	  cdev_init(cDev,&simple_driver_fops);
	  //add the struct to the system
	  cdev_added = cdev_add(cDev,dev,cnt);
	
	  device_file_major_number = MAJOR(dev);

	  if( cdev_added < 0 ){
	  	 printk( KERN_WARNING "Simple-driver: failed to add the character device to the system\n");
	  	 return cdev_added;
	  }

      if( device_file_major_number < 0 )
      {
         printk( KERN_WARNING "Simple-driver:  can\'t register character device with errorcode = %i\n", result );
         return result;
      }

	  //error check semaphore
    if(sem_init(&S,0,1) != 0){
        fprintf(stderr,"S initialized incorrectly");
        return -1;
    }
    fprintf(stdout,"semaphore S initalized: 0\n");
    
    //error check semaphore
    if(sem_init(&W,0,1) != 0){
        fprintf(stderr,"W initalized incorrectly");
        return -1;
    }
	fprintf(stdout,"semaphore W initalized: 0\n");

      
      printk( KERN_NOTICE "Simple-driver: registered character device with major number = %i and minor numbers 0...255\n"
                  , device_file_major_number );

      return 0;
}
/*-----------------------------------------------------------------------------------------------*/
void unregister_device(void)
{
   printk( KERN_NOTICE "Simple-driver: unregister_device() is called\n" );
   if(device_file_major_number != 0)
   {
      cdev_del(cDev);
	  unregister_chrdev_region(dev,cnt);
   }
}
