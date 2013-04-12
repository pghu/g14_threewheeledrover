#ifndef ROBOT_INCLUDES_H_
#define ROBOT_INCLUDES_H_

#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <string.h>
#include <stddef.h>
#include <unistd.h>  // usleep
#include <math.h>
#include "sys/alt_flash.h"
#include "sys/alt_flash_types.h"
#include "io.h"
#include "alt_types.h"  // alt_u32
#include "altera_avalon_pio_regs.h" //IOWR_ALTERA_AVALON_PIO_DATA
//#include <sys/alt_irq.h>//interrupts
#include <priv/alt_legacy_irq.h> //use the old alt_irq_register() function
#include "sys/alt_alarm.h" // time tick function (alt_nticks(), alt_ticks_per_second())
#include "sys/alt_timestamp.h" 
#include "sys/alt_stdio.h"
#include "system.h"
#include <fcntl.h>
//#include <debug.h>

#define DEBUG_DUMP  /*printf */ 

#define   TASK_STACKSIZE       2048
#define   QUEUE_SIZE 		   10
#define   MAX_BUFFER_SIZE      255
#define   SECOND_DELAY 		   1000000
#define   PI                   3.14159265

#define   SERVO_MAX 		   1000000
#define   SERVO_LEFT           0.117
#define   SERVO_CENTER         0.1
#define   SERVO_RIGHT          0.09

#define   CH_FRONT 			   1
#define   CH_LEFT			   4
#define   CH_RIGHT             6

#define   COURSE_LENGTH        20000

/* Message Statuses */
#define FRONT_IS_BLOCKED 0
#define FRONT_IS_CLEAR 1

#define UNKNOWN 90
#define FRONT_LEFT_IS_BLOCKED 93
#define FRONT_RIGHT_IS_BLOCKED 94

#define NOTMOVING 1

#define MOTOR_STOP 0
#define MOTOR_FULL 1
#define MOTOR_HALF 2
#define MOTOR_75 3

#define MOTOR_LEFT 11
#define MOTOR_RIGHT 12
#define MOTOR_REVERSE 13
#define MOTOR_180 14

#define SYS_BUSY 1
#define SYS_FREE 0

/* bool definition */
typedef int bool;
#define TRUE    1
#define FALSE   0


typedef struct{
	char * quadrant;
	double angle;
	double distance;
} angle_obj;

typedef struct{
	char move;
	char dir;
	double count; //encoder counts
	double angle;

} move_obj;




#endif
