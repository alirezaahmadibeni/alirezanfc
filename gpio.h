#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <linux/rtc.h>
#include <linux/kernel.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/poll.h>
#include <signal.h>

#define TTP229	0b1010111
#define gpio_int	363			//GPIOL11

#define gpio_high	1
#define gpio_low	0
#define gpio_input	0
#define gpio_output 1



struct pollfd	pfd ;
int i2cdev ;
size_t  readn;
char buf[10] , stdstr[1024];
bool ThreadContinue = true ;
//########################################################################################################

//########################################################################################################
int init_inthandler(void)	{
	
	char	file_addr[128] ;
	char	str[10] ;
	int		fp ;
	int		ret , len ;

	//init key interrupt gpio
	sprintf(file_addr,"/sys/class/gpio/unexport") ;
	fp = open(file_addr, O_WRONLY);
	len = snprintf(str, sizeof(str), "%u", gpio_int) ;
	write( fp, str, len) ;
	close( fp ) ;
	
	sprintf(file_addr,"/sys/class/gpio/export") ;
	fp = open(file_addr, O_WRONLY);
	len = snprintf(str, sizeof(str), "%u", gpio_int) ;
	write( fp, str, len);
	close( fp ) ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/direction", gpio_int) ;
	fp = open(file_addr, O_WRONLY) ;
	write( fp, "in", 3) ;
	close( fp ) ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/edge", gpio_int) ;
	fp = open(file_addr, O_WRONLY) ;
	write( fp, "falling", 8) ;
	close( fp ) ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/value",gpio_int) ;
	pfd.fd = open(file_addr, O_RDONLY  | O_NONBLOCK) ;
	if(pfd.fd == -1)	{
		printf("error opening the gpio%u value file for int handler",gpio_int) ;
		return -1 ;
	}
	
	pfd.events = POLLPRI | POLLIN ;
	pfd.revents = 0 ;
	lseek(pfd.fd, 0, SEEK_SET) ;
	readn = read(pfd.fd, buf, sizeof(buf) ) ;
	//sigfillset(&sigmask) ;
	ret = poll(&pfd, 1, 10000) ; //, &sigmask) ;
	if (ret == -1) {
		perror("poll error") ;
		return -1;
	}
	
	if (ret == 0) {
		printf("poll timeout") ;
	}
	
	if (pfd.revents & POLLPRI)	{
		lseek(pfd.fd, 0, SEEK_SET);
		readn = read(pfd.fd, buf, sizeof(buf) );
		buf[readn ? readn - 1 : 0] = 0;
		printf("gpio int is readable: %s\n",buf) ;
		//pfd.revents = 0;
	}
	
	return 0;
}
//########################################################################################################
void i2cinit(unsigned char i2c_slave_addr)	{
	char *i2cdevaddr = (char*)"/dev/i2c-0" ;
	i2cdev = open(i2cdevaddr, O_RDWR  | O_NONBLOCK) ;
	if (i2cdev < 0)	{
		perror("Failed to open the i2c bus");
	}else{
		if(ioctl(i2cdev, I2C_SLAVE, i2c_slave_addr) < 0) {
			printf("I2C Slave Error on TTP229\n") ;
		}
	}
}
//########################################################################################################
void i2cclose(void) {
	close(i2cdev) ;
}
//########################################################################################################
bool gpio_init(unsigned short pin_number , unsigned char gpio_mode)	{
	char	file_addr[128] ;
	char	str[10] ;
	int		fp , len;
	bool	res = true ;
	
	sprintf(file_addr,"/sys/class/gpio/unexport") ;
	fp = open(file_addr, O_WRONLY);
	len = snprintf(str, sizeof(str), "%u", pin_number) ;
	write( fp, str, len) ;
	close( fp ) ;
	
	sprintf(file_addr,"/sys/class/gpio/export") ;
	fp = open(file_addr, O_WRONLY);
	len = snprintf(str, sizeof(str), "%u", pin_number) ;
	write( fp, str, len);
	close( fp ) ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/direction", pin_number) ;
	fp = open(file_addr, O_WRONLY) ;
	if( gpio_mode == gpio_output )	{
		write( fp, "out", 4) ;
	}else	{
		write( fp, "in", 3) ;
	}	
	close( fp ) ;
	return res ;
}
//########################################################################################################
bool gpio_write(unsigned short pin_number , unsigned char gpio_state)	{
	char	file_addr[128] ;
	int		fp ;
	bool	res = true ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/value", pin_number) ;
	fp = open( file_addr, O_WRONLY) ;
	if( gpio_state == gpio_high )	{
		write( fp, "1", 2);
	}else	{
		write( fp, "0", 2);
	}
	close( fp ) ;
	return res ;
}
//########################################################################################################
char gpio_read(unsigned short pin_number)	{
	char	file_addr[128] ;
	char	pin_value = -1 ;
	int		fp ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/value",pin_number) ;
	fp = open( file_addr, O_RDONLY  | O_NONBLOCK) ;
	
	close( fp ) ;
	return pin_value ;
}
//########################################################################################################
void led_on(unsigned short led_pin_number)	{
	char	file_addr[128] ;
	int		fp ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/value", led_pin_number) ;
	fp = open( file_addr, O_WRONLY) ;
	write( fp, "1", 2);
	close( fp ) ;
}
//########################################################################################################
void led_off(unsigned short led_pin_number)	{
	char	file_addr[128] ;
	int		fp ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/value", led_pin_number) ;
	fp = open( file_addr, O_WRONLY) ;
	write( fp, "0", 2);
	close( fp ) ;
}
//########################################################################################################
void led_toggle(unsigned short led_pin_number)	{
	char	file_addr[128] ;
	char	str[10] ;
	unsigned int led_status ;
	int		fp ;
	
	sprintf(file_addr,"/sys/class/gpio/gpio%u/value",led_pin_number) ;
	fp = open( file_addr, O_RDONLY  | O_NONBLOCK) ;
	read( fp, str, 2);
	close( fp ) ;
	
	led_status = atoi(str) ;
	
	fp = open( file_addr, O_WRONLY) ;
	if( led_status == 1 ){
		write( fp, "0", 2);
	}else{
		write( fp, "1", 2);
	}
	close( fp ) ;
}
//########################################################################################################