/*
 * accel_read_cli.c
 *
 *  Created on: May 15, 2021
 *      Author: bvlad
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define ACCEL_FILE_ACC_X "/sys/bus/iio/devices/iio:device0/in_accel_x_raw"
#define ACCEL_FILE_ACC_Y "/sys/bus/iio/devices/iio:device0/in_accel_y_raw"
#define ACCEL_FILE_ACC_Z "/sys/bus/iio/devices/iio:device0/in_accel_z_raw"

#define LCD_FILE "/dev/lcd"

#define MAX_BUFF 64

#define ACC_FS_SENSITIVITY 16384
#define FULL_SCALE_G_VALUE 2

int main(void){

	int fd_x,fd_y,fd_z,ret;
	int acc_values[3];
	double acc_x, acc_y, acc_z;
	char buff_x[4]={0,0,0,0};
	char buff_y[2],buff_z[2];
	long unsigned energy_uj = 0;
	char buf_desc[MAX_BUFF];
	int c;
	char Vo[10], x_to_print[10];
	char msg[]="\fIncl in degrees:";
	char special_chars[]="\f\n";

	double acc_val_x;
	char *ret_char_ptr;


/*
	while(1){

		snprintf(buf_desc, sizeof(buf_desc),ACCEL_FILE_ACC_X);

		if((fd_x=open(buf_desc,O_RDONLY))<0){
				perror("Could not open X acc file");
				return 1;
			}

		ret=read(fd_x,buff_x,4);
		if(ret<=0){
			perror("Could not read from accel x regs");
			return 1;
		}

		printf("buff_x[0]=%i\n",buff_x[0]);
		printf("buff_x[1]=%i\n",buff_x[1]);
		printf("buff_x[2]=%i\n",buff_x[2]);
		printf("buff_x[3]=%i\n",buff_x[3]);

		acc_values[0]=( ((buff_x[2]) << 8) + buff_x[3]);
		acc_x = (double) acc_values[0] / ACC_FS_SENSITIVITY;

		printf("Acceleration on X axis (raw value) is: %i\n", acc_values[0]);
		printf("Acceleration on X axis (g value) is: %d\n", acc_x);

		close(fd_x);
		sleep(1);

	}
	*/

	FILE *proc = fopen (ACCEL_FILE_ACC_Y, "r");
	if (!proc) {
		        fprintf (stderr, "error: process open failed.\n");
		        return 1;
		}
	int i=0;
	while(1) {
	      c = fgetc(proc);
	      if( feof(proc) ) {
	         break ;
	      }
	      printf("%c", c);
	if (c != '\0'){
	  Vo[i] = c;
	++i;
	}
	Vo[i+1]= '\0';
	//      Vo = c;
	   }

	   fclose(proc);
	//Vo[i] = system(command);

	printf("Analog Reading: %s", Vo);

	//This is the printing on LCD part

	int my_dev=open(LCD_FILE, O_WRONLY);
		if(my_dev<0){
			perror("Fail to open device file: /dev/lcd");
			}else{
				acc_val_x=strtod(Vo,&ret_char_ptr);
				gcvt(acc_val_x/16384,5,x_to_print);
				//snprintf(x_to_print,(sizeof(Vo)+1),"\f%s",Vo);
				//snprintf(x_to_print,(sizeof(x_to_print)+1),"\f%s",x_to_print);
				ret=write(my_dev,special_chars,1);
				ret=write(my_dev,x_to_print,7);
				if(ret<0){
					perror("Was not able to transfer all info.\n");
				}
				close(my_dev);
				printf("X to print is: %s\n", x_to_print);
			}

	return 0;
}
