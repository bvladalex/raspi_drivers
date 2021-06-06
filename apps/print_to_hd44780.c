/*
 * print_to_hd44780.c
 *
 *  Created on: Jun 1, 2021
 *      Author: bvlad
 */


#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define LCD_FILE "/dev/lcd"
#define MESSAGE "This is a test!"

//char msg[]="\fThis is a messag\ne! Please delete.";
char msg[]="\fIncl in degrees:";
//char msg[]="\fOverwrite!";

int main(void){
	int ret;
	//First, run mknod /dev/mydev c 202 0 to create the device /dev/mydev with right numbers
	int my_dev=open(LCD_FILE, O_WRONLY);
	if(my_dev<0){
		perror("Fail to open device file: /dev/lcd");
		}else{
			//ioctl(my_dev, 100, 110); //cmd =100, arg =110
			ret=write(my_dev,msg,(sizeof(msg)-1));
			if(ret<0){
				perror("Was not able to transfer all info.\n");
			}
			close(my_dev);
		}
	return 0;
}
