#define ALT_LEGACY_INTERRUPT_API_PRESENT

#include "lib/robot_includes.h"
#include "lib/flash.h"
#include "lib/adc_spi_read.h"
#include "lib/I2C.h"
#include "lib/LSM303.h"
#include "lib/motors.h"
#include "includes.h"

OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task3_stk[TASK_STACKSIZE];
OS_STK    task4_stk[TASK_STACKSIZE];
OS_STK    task5_stk[TASK_STACKSIZE];
OS_STK    task6_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK_ENCODER_PRIORITY      	1
#define TASK_NAVI_PRIORITY      	2
#define TASK_SERVO_PRIORITY 		3
#define TASK_HISTORY_PRIORITY        4
#define TASK_LSM303_PRIORITY      	5

/* Queues */
OS_EVENT* MainQueue;
void* MainQueueBaseAddress[QUEUE_SIZE];

/* System status */
int SYSTEM_STATUS;
int MOTOR_0_STATUS;
int MOTOR_1_STATUS;

bool FRONT_CLEAR;
bool LEFT_CLEAR;
bool RIGHT_CLEAR;

/* Function declarations */
bool isSensorClear(float volt);
void navigateFront(angle_obj FRONT[]);
void EncoderCalibrate(void);
void SensorsCalibrate(void);
void checkLeftRight();
void servoRotate(angle_obj *FRONT_SERVO);

angle_obj FRONT_SERVO[3];
move_obj TRVL[400]; //remalloc later;
int TRVL_INDEX;
double ENCODER_INIT;

float LEFT_VOLT, RIGHT_VOLT, FRONT_VOLT;
double ENCODER_0A, ENCODER_1A;

/* Navigation task */
void Navigation_Task(void *pData){
    int status = 0;
    INT8U err;
    //char * msg = malloc(sizeof(char) * MAX_BUFFER_SIZE);

    while (1)
    {
    	status = (int) OSQPend(MainQueue, 0, &err); //wait on messages from queue and act accordingly

    	switch (status){
    		case FRONT_IS_BLOCKED:
    			printf("Stop motors! System Status: %i \n", SYSTEM_STATUS);
    			M_allStop();

    			if (TRVL[TRVL_INDEX].move == 'F'){
    				TRVL[TRVL_INDEX].dir = newDir(TRVL[TRVL_INDEX-1].dir, 'F');
    				TRVL[TRVL_INDEX].count = ENCODER_0A - ENCODER_INIT;
    				TRVL[TRVL_INDEX].angle = 0;
    				TRVL_INDEX++;
    			}

    			if (SYSTEM_STATUS == SYS_BUSY){
					/* suspend servo task */
					err = OSTaskSuspend(TASK_SERVO_PRIORITY);
					if (err == OS_NO_ERR){
						//printf("Servo task suspended \n");
						OSTimeDlyHMSM(0, 0, 0, 100);
						IOWR(SERVO_BASE, 0, 0.1 * SERVO_MAX);
					}else{
						//printf("Error! Could not suspend servo task \n");
					}
    			}

    			OSTimeDlyHMSM(0, 0, 0, 1500);
    			checkLeftRight();
    			printf("Front status: %i & Left Volt and Status: %.2fV, %i & Right Volt and Status:  %.2fV, %i \n", FRONT_CLEAR, LEFT_VOLT, LEFT_CLEAR, RIGHT_VOLT, RIGHT_CLEAR);

    			/*
    			if (RIGHT_CLEAR){
    				if (RIGHT_VOLT > LEFT_VOLT && TRVL[TRVL_INDEX - 1].dir != 'R'){
    					printf("Turn right \n");
						LAST_TURN = "right";
						turnRight();
    				}else if (RIGHT_VOLT > LEFT_VOLT && TRVL[TRVL_INDEX - 1].dir == 'R'){
    					printf("Turn left, last turn was right \n");
						LAST_TURN = "right";
						turnRight();
    				}
    			}
    			*/
    			if (!FRONT_CLEAR){
					if (RIGHT_CLEAR || LEFT_CLEAR){
						if (RIGHT_VOLT > LEFT_VOLT){ //left side is farther away
							if (LEFT_CLEAR){
								printf("Turn left \n");
								turnLeft();
								TRVL[TRVL_INDEX].move = 'L';
								TRVL[TRVL_INDEX].dir = newDir(TRVL[TRVL_INDEX-1].dir, 'L');
								TRVL[TRVL_INDEX].angle = 90;
								TRVL_INDEX++;
							}
						}else{
							printf("Turn right \n");
							turnRight();
							TRVL[TRVL_INDEX].move = 'R';
							TRVL[TRVL_INDEX].dir = newDir(TRVL[TRVL_INDEX-1].dir, 'R');
							TRVL[TRVL_INDEX].angle = 90;
							TRVL_INDEX++;
						}
					}else{ //no where to go
						printf("Reverse! \n");
						motorReverse();
						OSTimeDlyHMSM(0, 0, 2, 0);
						turn180();
						TRVL[TRVL_INDEX].move = 'B';
						TRVL[TRVL_INDEX].dir = newDir(TRVL[TRVL_INDEX-1].dir, 'B');
						TRVL[TRVL_INDEX].angle = 180;
						TRVL_INDEX++;
						//follow mapping...
					}
    			}

    			servoRotate(FRONT_SERVO);
    			navigateFront(FRONT_SERVO); //since SYSTEM_STATUS is busy, will only update FRONT_CLEAR
    			//usleep(1 * SECOND_DELAY);

    			if (FRONT_CLEAR){
    				SYSTEM_STATUS = SYS_FREE;

    	   			/* resume servo task */
					err = OSTaskResume(TASK_SERVO_PRIORITY);
					if (err == OS_NO_ERR){
						//printf("Servo task resumed \n");
					}
    			}
    			else{
    				OSQPost(MainQueue, (void *) FRONT_IS_BLOCKED);
    			}

    			break;
    		case FRONT_IS_CLEAR:
    			printf("Start motors! \n");

    			TRVL[TRVL_INDEX].move = 'F';
    			ENCODER_INIT = ENCODER_0A;

    			stoptopartial();
    			OSTimeDlyHMSM(0, 0, 0, 500);

    			if ((MOTOR_0_STATUS & MOTOR_1_STATUS) == MOTOR_FULL){
					SYSTEM_STATUS = SYS_FREE;

					/* resume servo task */
					err = OSTaskResume(TASK_SERVO_PRIORITY);
					if (err == OS_NO_ERR){
						//printf("Servo task resumed \n");
					}
    			}else{
    			    M_allStop();
    				printf("Error! Front should be clear but can't go forward \n");
    				//usleep(1 * SECOND_DELAY);
    				servoRotate(FRONT_SERVO);
    				navigateFront(FRONT_SERVO);
    				OSQPost(MainQueue, (void *) FRONT_IS_CLEAR);
    			}

				break;
    		case UNKNOWN: //check front, sides to decide what to do next
    			servoRotate(FRONT_SERVO);
    			navigateFront(FRONT_SERVO); //since SYSTEM_STATUS is busy, will only update FRONT_CLEAR
    			checkLeftRight();
    			break;
    		default:
    			break;
    	}
    }
}

void LSM303_Task(void *pData){
	alt_u8 *data;
	// set clock as output
    //IOWR_ALTERA_AVALON_PIO_DIRECTION(COMPASS_I2C_SCL_BASE, ALTERA_AVALON_PIO_DIRECTION_OUTPUT);
    //IOWR(COMPASS_I2C_SCL_BASE, 0, 0x01);
    //if (I2C_MultipleRead(I2C_SCL_BASE, I2C_SDA_BASE, DeviceAddr, ControlAddr, szBuf, sizeof(szBuf))){

	usleep(2 * SECOND_DELAY);
	//LSM303 compass;
	//compassInit(&compass);

	//enableDefault();
	//alt_u8 new = LSM303_OUT_X_H_M;
	//printf("Sending: %02x\n", new);
	//alt_u8 temp = readMagReg(LSM303_OUT_X_H_M + 1);
	//readMag(&compass);
	//LSM303_calibrate(&compass);

	//try enabling it directly
	/* CRA_REG_M:  | TEMP_EN  |  0  |  0  | DO2 | DO1 | DO0 |  0  |  0  |
	 * 	DO2, DO1, DO0 = 000 = 0.75 Hz output rate
	 * 	= 100 = 15Hz output rate
	 * 	default is 00010000
	 * */
     if (I2C_Write(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, 0x3C, LSM303_MR_REG_M, 0x00)){
		  printf("Initialize success!\n");
	  }
	  else{
	        printf("Failed to enable magnetometer\r\n");
	  }

	  if (I2C_Read(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, 0x3C, LSM303_OUT_X_H_M, data)){
	  	 printf("Success! %02x \n", (int) &data);
	  }else{
	     printf("Failed to read magnetometer register\r\n");
	  }

}

void History_Task(void* pData){
	INT8U err;
    bool retval = FALSE;
	OSTimeDlyHMSM(0, 0, 1, 0);

 while(1){

    retval = evalHistory(TRVL, TRVL_INDEX, 'N', COURSE_LENGTH);

    if (retval){
		 if (TRVL[TRVL_INDEX].move == 'F'){
			TRVL[TRVL_INDEX].dir = newDir(TRVL[TRVL_INDEX-1].dir, 'F');
			TRVL[TRVL_INDEX].count = ENCODER_0A - ENCODER_INIT;
			TRVL[TRVL_INDEX].angle = 0;
		  }

		 SYSTEM_STATUS = SYS_BUSY;

		 executeHistory(TRVL, TRVL_INDEX);
		 err = OSTaskSuspend(TASK_NAVI_PRIORITY);
		 err = OSTaskSuspend(TASK_SERVO_PRIORITY);
		 M_allStop();
    }
    	/*
        //SensorsCalibrate();
        //debug, flash LED
        if (LED_on){
        	IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0x00);
        	LED_on = 0;
        } else{
        	IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0x01);
        	LED_on = 1;
        }
        */
        OSTimeDlyHMSM(0, 0, 10, 0);
    }
}

void checkLeftRight(){
	alt_u16 data16;
	float volt = 0;
	int ch = 0;

	for (ch = 0; ch < 8; ch++){
		data16 = ADC_Read(ch); // 12-bits resolution
		volt = (float)data16 * 3.3 / 4095.0;

		if (ch == CH_LEFT)
			LEFT_VOLT = volt;
		else if (ch == CH_RIGHT)
			RIGHT_VOLT = volt;
		//printf("CH%i ADC is = %.2f V\r\n", ch, volt);
	}

	if (isSensorClear(LEFT_VOLT)){
		LEFT_CLEAR = TRUE;
	}else{
		LEFT_CLEAR = FALSE;
	}

	if (isSensorClear(RIGHT_VOLT)){
		RIGHT_CLEAR = TRUE;
	}else{
		RIGHT_CLEAR = FALSE;
	}
}

void Servo_Task(void* pData){
//send PWM signals to Servo for continuous rotation +/- 20 degrees
	//printf("Testing Servo! \n");

	FRONT_SERVO[0].quadrant = "left";
	FRONT_SERVO[0].angle = -15;
	FRONT_SERVO[1].quadrant = "center";
	FRONT_SERVO[1].angle = 0;
	FRONT_SERVO[2].quadrant = "right";
	FRONT_SERVO[2].angle = 15;
	IOWR(SERVO_BASE, 0, 0.1 * SERVO_MAX);
	OSTimeDlyHMSM(0, 0, 1, 0);

	while(1){
		servoRotate(FRONT_SERVO);
		navigateFront(FRONT_SERVO);
	}
}

void servoRotate(angle_obj *FRONT_SERVO){
	//return if any of these return a too close object
	IOWR(SERVO_BASE, 0, SERVO_LEFT * SERVO_MAX);
	OSTimeDlyHMSM(0, 0, 0, 250);
	FRONT_SERVO[0].distance = (float) ADC_Read(CH_FRONT) * 3.3 / 4095.0;

	IOWR(SERVO_BASE, 0, SERVO_CENTER * SERVO_MAX);
	OSTimeDlyHMSM(0, 0, 0, 250);
	FRONT_SERVO[1].distance = (float) ADC_Read(CH_FRONT) * 3.3 / 4095.0;

	if (!isSensorClear(FRONT_SERVO[1].distance)){
		FRONT_SERVO[2].distance = 2.1;
		return;
	}

	IOWR(SERVO_BASE, 0, SERVO_RIGHT * SERVO_MAX);
	OSTimeDlyHMSM(0, 0, 0, 250);
	FRONT_SERVO[2].distance = (float) ADC_Read(CH_FRONT) * 3.3 / 4095.0;
	if (!isSensorClear(FRONT_SERVO[2].distance)){
		return;
	}

	IOWR(SERVO_BASE, 0, SERVO_CENTER * SERVO_MAX);
	OSTimeDlyHMSM(0, 0, 0, 250);
	FRONT_SERVO[1].distance = (float) ADC_Read(CH_FRONT) * 3.3 / 4095.0;

	OSTimeDlyHMSM(0, 0, 0, 200);
}

void navigateFront(angle_obj FRONT[]){

	if ((MOTOR_0_STATUS & MOTOR_1_STATUS) == MOTOR_STOP){
		if (isSensorClear(FRONT[0].distance) && isSensorClear(FRONT[1].distance)  && isSensorClear(FRONT[2].distance))
		{
			FRONT_CLEAR = TRUE;

			if (SYSTEM_STATUS == SYS_FREE){
				SYSTEM_STATUS = SYS_BUSY;
				printf("Clear! Front Sensor detects %f, %f, %f \n", FRONT[0].distance, FRONT[1].distance, FRONT[2].distance);
				OSQPost(MainQueue, (void *) FRONT_IS_CLEAR);
			}
		}
	}else if ((MOTOR_0_STATUS & MOTOR_1_STATUS) == MOTOR_FULL) //close to an object and moving
	{
		if (!isSensorClear(FRONT[0].distance) || !isSensorClear(FRONT[1].distance)  || !isSensorClear(FRONT[2].distance)){
			FRONT_CLEAR = FALSE;

			if (SYSTEM_STATUS == SYS_FREE){
				printf("Blocked! Front Sensor detects %f, %f, %f \n", FRONT[0].distance, FRONT[1].distance, FRONT[2].distance);
				M_allStop();
				SYSTEM_STATUS = SYS_BUSY;
				OSQPost(MainQueue, (void *) FRONT_IS_BLOCKED);
			}
		}
	}

	/*
	if ((MOTOR_0_STATUS & MOTOR_1_STATUS) == MOTOR_STOP){
		if (isSensorClear(FRONT[0].distance) && isSensorClear(FRONT[1].distance)  && isSensorClear(FRONT[2].distance))
		{
			SYSTEM_STATUS = SYS_BUSY;
			printf("Clear! Front Sensor detects %f, %f, %f \n", FRONT[0].distance, FRONT[1].distance, FRONT[2].distance);
			OSQPost(MainQueue, (void *) FRONT_IS_CLEAR);
			FRONT_CLEAR = TRUE;

		}
	}else if ((MOTOR_0_STATUS & MOTOR_1_STATUS) == MOTOR_FULL) //close to an object and moving
	{
		if (!isSensorClear(FRONT[0].distance) || !isSensorClear(FRONT[1].distance)  || !isSensorClear(FRONT[2].distance))
		{
		//if (FRONT_CLEAR){
			printf("Blocked! Front Sensor detects %f, %f, %f \n", FRONT[0].distance, FRONT[1].distance, FRONT[2].distance);
			SYSTEM_STATUS = SYS_BUSY;
			OSQPost(MainQueue, (void *) FRONT_IS_BLOCKED);
			FRONT_CLEAR = FALSE;
		//}
		}
	}
	*/

	/*
	else if (!isSensorClear(FRONT[0].distance) && isSensorClear(FRONT[1].distance)  && isSensorClear(FRONT[2].distance))
			{
				SYSTEM_STATUS = SYS_BUSY;
				OSQPost(MainQueue, (void *) FRONT_LEFT_IS_BLOCKED);
				FRONT_CLEAR = FALSE;
			}else if (isSensorClear(FRONT[0].distance) && isSensorClear(FRONT[1].distance)  && !isSensorClear(FRONT[2].distance))
			{
				SYSTEM_STATUS = SYS_BUSY;
				OSQPost(MainQueue, (void *) FRONT_RIGHT_IS_BLOCKED);
				FRONT_CLEAR = FALSE;
			}
	*/
}

bool isSensorClear(float volt){
	bool retval = TRUE;

	if (volt > 1.9){
		retval = FALSE;
	}
	else if (volt <= 1.9 && volt > 0){
		//
	}

	return retval;
}

void Encoder_Task(void* pData){
	//read from pins like for PWM
	//double encode_0_a, encode_1_a; //encode_b;
	double counts_0_new, counts_0_old, counts_1_new, counts_1_old;
	counts_0_old = 0;
	counts_1_old = 0;

	//ENCODER_0A, ENCODER_1A increment forever... but slowly. Reset values after a specific amount of time

	OSTimeDlyHMSM(0, 0, 1, 0);

	//EncoderCalibrate();
/*  64 counts per revolution, @ 100% is 80rev/min
	H-bridge slows voltage output, motors see around 8V max
	(64 counts per revolution of motor shaft) * (131 revolutions of motor shaft per revolutions of output shaft) * (1 revolution of output shaft per 2*PI*4 distance)
    = 210712.9025
    43 increments (12V), 1.3333 revs per second

	Counting only the rising edges of channel A = quarter of counting the rising&falling edges of channel A&B
	So 64 counts becomes => 16 counts per revolution
	16 counts/rev * 131 rev/rev * 1 rev/ ( 2 * pi * 4cm) = 2096 counts / 25.13cm
*/
	while (1)
	{
		//1st motor
		counts_0_new = (double) IORD(ENCODER_0A_BASE, 0);

		if ( counts_0_new >= counts_0_old ){
			ENCODER_0A += (counts_0_new - counts_0_old);
		}
		else{ //counter reset
			ENCODER_0A += counts_0_new;
		}

		if ((counts_0_new - counts_0_old) < 1){
			MOTOR_0_STATUS = MOTOR_STOP;
		}
		else{
			MOTOR_0_STATUS = MOTOR_FULL;
		}

		//printf("ENCODER: old value is: %f, new value is : %f \n", counts_0_old, counts_0_new);
		counts_0_old = counts_0_new;

		//2nd motor
		counts_1_new = (double) IORD(ENCODER_1A_BASE, 0);
		if ( counts_1_new >= counts_1_old ){
			ENCODER_1A += (counts_1_new - counts_1_old);
		}
		else{ //counter reset
			ENCODER_1A += counts_1_new;
			//printf("counter reset, temp value is: %f, motor 1 value is : %f \n", temp, motor_1);
		}

		if ((counts_1_new - counts_1_old) < 2){
			MOTOR_1_STATUS = MOTOR_STOP;
		}
		else{
			MOTOR_1_STATUS = MOTOR_FULL;
		}

		counts_1_old = counts_1_new;

		//if ((ENCODER_0A - ENCODER_1A) > 100 || (ENCODER_1A - ENCODER_0A) > 100 )
		//	printf("Error, one motor is moving faster than the other! %f %f \n", ENCODER_0A, ENCODER_1A);

		//printf("Motor 0 encoder A value is :( %f ) Revolutions : (%f) \n", encode_0_a,  encode_0_a / 16);
		//printf("Motor 1 encoder A value is :( %f ) Revolutions : (%f) \n", encode_1_a,  encode_1_a / 16);
		//printf("Current encoder B value is :( %f ) \n", encode_b / 32);
		//printf("Counts 0 new :( %f ) Counts 0 old : (%f) \n", counts_0_new,  counts_0_old);
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}


void EncoderCalibrate(){
	double encode_0_a, encode_1_a;
	double counts_1_old, counts_0_old;

	IOWR(PWM_0_BASE, 0, 0.75 * 31250);
	IOWR(PWM_2_BASE, 0, 0.75 * 31250);

	counts_0_old = IORD(ENCODER_0A_BASE, 0);
	counts_1_old = IORD(ENCODER_1A_BASE, 0);

	OSTimeDlyHMSM(0, 0, 2, 0);

	IOWR(PWM_0_BASE, 0, 0);
	IOWR(PWM_2_BASE, 0, 0);
	encode_0_a = ( IORD(ENCODER_0A_BASE, 0) - counts_0_old);
	encode_1_a = ( IORD(ENCODER_1A_BASE, 0) - counts_1_old);

	printf("Motor 0 encoder A value is :( %f ) \n", encode_0_a);
	printf("Motor 1 encoder A value is :( %f ) \n", encode_1_a);
}

void SensorsCalibrate(){
	alt_u16 data16;
	float volt = 0;
	int ch = 0;

	while(1){
		if (ch > 7)
			ch = 0;

		data16 = ADC_Read(ch); // 12-bits resolution
		volt = (float)data16 * 3.3 / 4095.0;
		printf("CH%i ADC is = %.2f V\r\n", ch, volt);
		ch++;
		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}

int main(void){
	//Initialize queues
	MainQueue = OSQCreate(&MainQueueBaseAddress[0], QUEUE_SIZE);

	//Initialize tasks
	OSTaskCreateExt(Encoder_Task,
					  NULL,
					  (void *)&task1_stk[TASK_STACKSIZE-1],
					  TASK_ENCODER_PRIORITY,
					  TASK_ENCODER_PRIORITY,
					  task1_stk,
					  TASK_STACKSIZE,
					  NULL,
					  0);
	OSTaskCreateExt(LSM303_Task,
					   NULL,
					   (void *)&task4_stk[TASK_STACKSIZE-1],
					   TASK_LSM303_PRIORITY,
					   TASK_LSM303_PRIORITY,
					   task4_stk,
					   TASK_STACKSIZE,
					   NULL,
					   0);

	OSTaskCreateExt(Navigation_Task,
				   NULL,
				   (void *)&task5_stk[TASK_STACKSIZE-1],
				   TASK_NAVI_PRIORITY,
				   TASK_NAVI_PRIORITY,
				   task5_stk,
				   TASK_STACKSIZE,
				   NULL,
				   0);
	OSTaskCreateExt(Servo_Task,
				   NULL,
				   (void *)&task6_stk[TASK_STACKSIZE-1],
				   TASK_SERVO_PRIORITY ,
				   TASK_SERVO_PRIORITY ,
				   task6_stk,
				   TASK_STACKSIZE,
				   NULL,
				   0);

	OSTaskCreateExt(History_Task,
					  NULL,
					  (void *)&task3_stk[TASK_STACKSIZE-1],
					  TASK_HISTORY_PRIORITY,
					  TASK_HISTORY_PRIORITY,
					  task3_stk,
					  TASK_STACKSIZE,
					  NULL,
					  0);

	M_allStop();
	LEFT_CLEAR = RIGHT_CLEAR = FRONT_CLEAR = TRUE;
	ENCODER_0A = ENCODER_1A = 0;

	//assume starting position is 'North'
	ENCODER_INIT = ENCODER_0A;
	TRVL[0].dir = 'N';
	TRVL[0].move = 'F';
	TRVL[0].angle = 0;
	TRVL[0].count = 1;
	TRVL_INDEX = 1;

	SYSTEM_STATUS = SYS_FREE; //system is free to receive new commands
	OSStart();

	return 0;
}
