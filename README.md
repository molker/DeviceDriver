# DeviceDriver
Linux Device Driver for the High Point University Operating Systems class

Tutorial for setting up and running a simple device driver as a base for further operating system customization. This driver will include the ability to read and write from the driver. For the best results, use the files provided in the repository. 


##### Setting Up and Running
###### Loading the Driver
The first important command you'll be using for running this driver is 
```
#>make load
```
which calls your make file to run what we previously specified. Doing this will set up your driver in /proc/modules. Once this is done, before you can move on you need to make sure you know what major number your driver is set up with, because the next step will connect a node to that major number. To do this, cd (change directory) over to the /proc/modules directory and perform the ls -l command. Then, find your driver which should be called "simple-driver" and find what major number was assigned to it. 
###### Connecting the Driver
The second important command you'll need to know for running this driver is 
```
#>mknod NAME TYPE [MAJOR MINOR]
```
mknod is used for creating block or character special files, in this case you will be using character. In the NAME section, provide the name for the node which you'll be using for calling your driver. In the TYPE section, since this is a character driver, put the letter c. In major number, provide the major number associated with your driver that you found in the last step. The minor number can be anything between 0-255, so I would recommend putting 0.
###### Calling your Driver
The driver file provided is set up with 2 options for interaction, reading and writing. There is a default message in this driver which you can find by using the "cat" command followed by the name of the driver. To write to the driver you will need to use the "echo" command with the string you want to put in the driver and followed by the a > and the name of the driver. Make sure to use these commands while in the /dev/ directory. 
###### Read
```
$> cat simple-driver
```
###### Write
```
$> echo "Hello World" > simple-driver
```
###### References: 
* For more specific information on how creating each individual part of the code was done, refer to [Driver Help](https://www.apriorit.com/dev-blog/195-simple-driver-for-linux-os)
