#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

#define max_buffer_size 512
char user_buffer[max_buffer_size];
dev_t Device_Num ;
struct cdev my_cdev;
/*holds the class pointer */
struct class *class_pcd;

struct device *device_pcd;
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
	loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);

	switch(whence)
	{
		case SEEK_SET:
			if((offset > max_buffer_size) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if((temp > max_buffer_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = max_buffer_size + offset;
			if((temp > max_buffer_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}
	
	pr_info("New value of the file position = %lld\n",filp->f_pos);

	return filp->f_pos;

}




ssize_t pcd_read(struct file *filp,char __user *buff,size_t count,loff_t *f_pos)
{

     pr_info("Read requested for %zu bytes \n",count);
    pr_info("In Function Read f_pos =%lld\n",*f_pos);
    if((*f_pos+count)>max_buffer_size)
    {
        count=max_buffer_size-*f_pos;
    }
    if(copy_to_user(buff,&user_buffer[*f_pos],count))
    {
        return -EFAULT;
    }



    *f_pos+=count;
    pr_info( "Number of bytes successfully read = %zu\n",count);
    pr_info("Updated file position =%lld\n",*f_pos);

    return count;
}
ssize_t pcd_write(struct file *filp,const char __user *buff,size_t count,loff_t *f_pos)
{

     pr_info("write requested for %zu bytes \n",count);
    pr_info("In Function write f_pos =%lld\n",*f_pos);
    if((*f_pos+count)>max_buffer_size)
    {
        count=max_buffer_size-*f_pos;
    }
    /*This to see that there is memory space to write or no*/
    if(!count)
    {
        pr_err("No space left on the device \n");
		return -ENOMEM;
    }
    
    if(copy_from_user(&user_buffer[*f_pos],buff,count))
    {
		pr_err("Errrrrrrrrrrrrrrrrrror!!!!!!!!!!!!!! \n");
        return -EFAULT;
    }



    *f_pos+=count;
    pr_info( "Number of bytes successfully wrriten = %zu\n",count);
    pr_info("Updated file position =%lld\n",*f_pos);

    return count;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	pr_info("open was successful\n");

	return 0;
}
int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("release was successful\n");

	return 0;
}



struct file_operations pcd_fops=
{
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.owner = THIS_MODULE
};

static int  __init hello_init(void)
{
    int ret;
    pr_info("Hello_From Init\n");
 /*Register a device */

   ret=alloc_chrdev_region(&Device_Num,0,1,"pcd_devices"); 
    if(ret<0)
    {
        pr_info("Feild to Dynamiclay allocate Device\n");
		goto out;
    }  
    /**
     * Need to print data about Major Number for my device 
     * 
    */
   pr_info("Device <Major>: <Minor>= %d:%d\n",MAJOR(Device_Num),MINOR(Device_Num));


    /**
     * Make kernel see this device driver
    */
    cdev_init(&my_cdev,&pcd_fops);
    my_cdev.owner = THIS_MODULE;
    ret=cdev_add(&my_cdev,Device_Num,1);
	if(ret < 0){
		pr_err("Cdev add failed\n");
        goto unreg_chrdev;
	}
	class_pcd = class_create(THIS_MODULE,"pcd_class");
	if(IS_ERR(class_pcd)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(class_pcd);
		goto cdev_del;
	  }
 device_pcd = device_create(class_pcd,NULL,Device_Num,NULL,"first");
	if(IS_ERR(device_pcd)){
		pr_err("Device create failed\n");
		ret = PTR_ERR(device_pcd);
			goto class_del;
	}

pr_info("Module init was successful\n");
return 0;

class_del:
	class_destroy(class_pcd);
cdev_del:
	cdev_del(&my_cdev);	
unreg_chrdev:
	unregister_chrdev_region(Device_Num,1);
out:
	pr_info("Module insertion failed\n");  
    return ret;
}


static void __exit hello_exit(void)
{
	device_destroy(class_pcd,Device_Num);
	class_destroy(class_pcd);
    /*clean up device*/
    cdev_del(&my_cdev);
	unregister_chrdev_region(Device_Num,1);
    pr_info("module unloaded\n");
    pr_info("Goodbye !\n");
}




module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AmrEssam2");
MODULE_DESCRIPTION("character driver");

