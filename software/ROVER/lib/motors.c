/*
 * motors.c
 *
 *  Created on: Mar 25, 2013
 *      Author: Peter
 */

#include "includes.h"
#include "robot_includes.h"
#include "motors.h"

void stoptofull(void){
	if (M_checkMotion() == TRUE){ //motors are still moving
		M_allStop();
	}

	IOWR(PWM_0_BASE, 0, 0.25 * 31250);
	IOWR(PWM_2_BASE, 0, 0.25 * 31250);
	usleep(0.5 * SECOND_DELAY);
	IOWR(PWM_0_BASE, 0, 0.50 * 31250);
	IOWR(PWM_2_BASE, 0, 0.50 * 31250);
	usleep(1 * SECOND_DELAY);
	IOWR(PWM_0_BASE, 0, 0.75 * 31250);
	IOWR(PWM_2_BASE, 0, 0.75 * 31250);
	usleep(1 * SECOND_DELAY);
	IOWR(PWM_0_BASE, 0, 1 * 31250);
	IOWR(PWM_2_BASE, 0, 1 * 31250);
	usleep(1 * SECOND_DELAY);
}

void stoptopartial(void){
	if (M_checkMotion() == TRUE){ //motors are still moving
		M_allStop();
	}

	IOWR(PWM_0_BASE, 0, 0.5 * 31250);
	IOWR(PWM_2_BASE, 0, 0.5 * 31250);
	usleep(0.5 * SECOND_DELAY);

	IOWR(PWM_0_BASE, 0, 0.63 * 31250);
	IOWR(PWM_2_BASE, 0, 0.65 * 31250);
	usleep(0.5 * SECOND_DELAY);

	IOWR(PWM_0_BASE, 0, 0.859 * 31250); //0.859 for 90%
	IOWR(PWM_2_BASE, 0, 0.9 * 31250); //0.9 for 90%
	usleep(1 * SECOND_DELAY);

}

void slowtostop(void){

	IOWR(PWM_0_BASE, 0, 0.75 * 31250);
	IOWR(PWM_2_BASE, 0, 0.75 * 31250);
	usleep(1 * SECOND_DELAY);

	IOWR(PWM_0_BASE, 0, 0.50 * 31250);
	IOWR(PWM_2_BASE, 0, 0.50 * 31250);
	usleep(0.75 * SECOND_DELAY);

	IOWR(PWM_0_BASE, 0, 0.25 * 31250);
	IOWR(PWM_2_BASE, 0, 0.25 * 31250);
	usleep(0.25 * SECOND_DELAY);

	IOWR(PWM_0_BASE, 0, 0);
	IOWR(PWM_2_BASE, 0, 0);
	usleep(1 * SECOND_DELAY);
}

/* Read the previous values of the PWM registers, make sure zero is being sent */
int M_checkMotion(void){

	int i;
	bool isMoving = FALSE;
	//double test = 0;
	int L298_REGISTERS[] = {PWM_0_BASE, PWM_1_BASE, PWM_2_BASE, PWM_3_BASE};

	int L298size = (int) sizeof(L298_REGISTERS)/sizeof(L298_REGISTERS[0]);
	//printf("The size is: %d \n", L298size);
	for (i = 0; i < L298size; i++){
		//test = IORD(L298_REGISTERS[i], 0);
		//printf("Register value is : %f , Register is: %d \n", test, L298_REGISTERS[i]);
		if (IORD(L298_REGISTERS[i], 0) > 0){
			isMoving = TRUE;
			break;
		}
	}

	return isMoving;
}

void M_allStop(void){
	//Signal all L298 inputs to go to 0 (coast)
	//Brakes = 1 1
	IOWR(PWM_0_BASE, 0, 1);
	IOWR(PWM_1_BASE, 0, 1);
	IOWR(PWM_2_BASE, 0, 1);
	IOWR(PWM_3_BASE, 0, 1);
	usleep(1 * SECOND_DELAY);
}

void turnLeft_Simple(void){
	//turn for a time limited amount
	IOWR(PWM_1_BASE, 0, 0.50 * 31250); //tell left motor to go reverse
    IOWR(PWM_2_BASE, 0, 0.50 * 31250);
	usleep(0.5 * SECOND_DELAY);
	IOWR(PWM_1_BASE, 0, 0.75 * 31250); //tell left motor to go reverse
    IOWR(PWM_2_BASE, 0, 0.75 * 31250);
	usleep(1 * SECOND_DELAY);
	IOWR(PWM_1_BASE, 0, 0);
	IOWR(PWM_2_BASE, 0, 0);
}

void turnLeft(){
	motorGo(1, 2, 0.75, 103);
}

void turnRight(){
	motorGo(0, 3, 0.75, 100);
}

void turn180(){
	motorGo(0, 3, 0.9, 218);
}

void motorReverse(){
	motorGo(1, 3, 0.9, 120);
}

void motorGo(int leftin, int rightin, double duty, double distance){
	alt_u32 left, right;
	double motor_0, motor_0_total, temp_0;
	double motor_1, motor_1_total, temp_1;

	left = returnHbridgeIn(leftin);
	right = returnHbridgeIn(rightin);

	IOWR(left, 0, duty * 31250);
	IOWR(right, 0, duty * 31250);

	motor_0 = IORD(ENCODER_0A_BASE, 0);
	motor_0_total = 0;

	motor_1 = IORD(ENCODER_1A_BASE, 0);
	motor_1_total = 0;

	while (1){
		temp_0 = IORD(ENCODER_0A_BASE, 0);
		temp_1 = IORD(ENCODER_1A_BASE, 0);

		if ( temp_0 >= motor_0 ){
			motor_0_total += (temp_0 - motor_0);
		}
		else{ //counter reset
			motor_0_total += temp_0;
			//printf("counter reset, temp value is: %f, motor_0 value is : %f \n", temp_0, motor_0);
		}

		if ( temp_1 >= motor_0 ){
			motor_1_total += (temp_1 - motor_1);
		}
		else{ //counter reset
			motor_1_total += temp_1;
			//printf("counter reset, temp value is: %f, motor_1 value is : %f \n", temp_1, motor_1);
		}

		if ((motor_0_total >= (16 * distance) - (0.05 * 16 * distance)) || (motor_1_total >= (16 * distance) - (0.05 * 16 * distance))){
			//printf("DONE! total is: %f  and temp value is: %f \n", motor_0_total, temp_0);
			break;
		}

		motor_0 = temp_0;
		motor_1 = temp_1;
		OSTimeDlyHMSM(0, 0, 0, 10);
	}

	//stop
	IOWR(left, 0, 0);
	IOWR(right, 0, 0);
}

void motorGoForward(int leftin, int rightin, double leftduty, double rightduty, double distance){
	alt_u32 left, right;
	double motor_0, motor_0_total, temp_0;
	double motor_1, motor_1_total, temp_1;

	left = returnHbridgeIn(leftin);
	right = returnHbridgeIn(rightin);
	IOWR(left, 0, leftduty * 31250);
	IOWR(right, 0, rightduty * 31250);

	motor_0 = IORD(ENCODER_0A_BASE, 0);
	motor_0_total = 0;

	motor_1 = IORD(ENCODER_1A_BASE, 0);
	motor_1_total = 0;

	while (1){
		temp_0 = IORD(ENCODER_0A_BASE, 0);
		temp_1 = IORD(ENCODER_1A_BASE, 0);

		if ( temp_0 >= motor_0 ){
			motor_0_total += (temp_0 - motor_0);
		}
		else{ //counter reset
			motor_0_total += temp_0;
			//printf("counter reset, temp value is: %f, motor_0 value is : %f \n", temp_0, motor_0);
		}

		if ( temp_1 >= motor_0 ){
			motor_1_total += (temp_1 - motor_1);
		}
		else{ //counter reset
			motor_1_total += temp_1;
			//printf("counter reset, temp value is: %f, motor_1 value is : %f \n", temp_1, motor_1);
		}

		if ((motor_0_total >= (16 * distance) - (0.05 * 16 * distance)) || (motor_1_total >= (16 * distance) - (0.05 * 16 * distance))){
			//printf("DONE! total is: %f  and temp value is: %f \n", motor_0_total, temp_0);
			break;
		}

		motor_0 = temp_0;
		motor_1 = temp_1;
		OSTimeDlyHMSM(0, 0, 0, 10);
	}

	//stop
	IOWR(left, 0, 0);
	IOWR(right, 0, 0);
}


void printHistory(move_obj *TRVL, int TRVL_INDEX){
	int i;

	for (i = 0; i < TRVL_INDEX; i++){
		printf("Step %i : (%c, %c, %f, %f) \n", i, TRVL[i].dir, TRVL[i].move, TRVL[i].count, TRVL[i].angle);
	}

}

bool evalHistory(move_obj *TRVL, int TRVL_INDEX, char dir, double distance){
	bool reachedGoal = FALSE;
	double dist = 0;
	int i;

	for (i = 0; i <= TRVL_INDEX; i++){
		if (TRVL[i].dir == 'N'){
			dist += TRVL[i].count;
		}else if (TRVL[i].dir == 'S'){
			dist -= TRVL[i].count;
		}

		if (dist >= COURSE_LENGTH - (0.05 * COURSE_LENGTH)){ //do approximation
			reachedGoal = TRUE;
			printf("Reached end of course! %f \n", dist);

			break;
		}else{
			printf("Current course progress is: %f \n", dist);
		}
	}

	return reachedGoal;
}

void executeHistory(move_obj *TRVL, int TRVL_INDEX){
	int i;

	printf("Go back in history to origin! \n");
	M_allStop();
	//motorReverse();
	turn180();
	OSTimeDlyHMSM(0, 0, 2, 0);

	for (i = 0; i < TRVL_INDEX; i++){
		switch (TRVL[i].move){
		case 'F':
			motorGoForward(0, 2, 0.859, 0.9, TRVL[i].count);
			break;
		case 'L':
			turnRight();
			break;
		case 'R':
			turnLeft();
			break;
		case 'B':
			//do nothing
			break;
		default:
			break;
		}
		//printf("Step %i : (%c, %c, %f, %f) \n", i, TRVL[i].dir, TRVL[i].move, TRVL[i].count, TRVL[i].angle);
		OSTimeDlyHMSM(0, 0, 1, 0);
		M_allStop();
		OSTimeDlyHMSM(0, 0, 1, 0);
	}

	printf("Done! \n");

}

char newDir(char oldDir, char move){
	char x = 'N';

	if (oldDir == 'N'){
		switch (move){
			case 'F':
				x = 'N';
				break;
			case 'L':
				x = 'W';
				break;
			case 'R':
				x = 'E';
				break;
			case 'B':
				x = 'S';
				break;
			default:
				break;
		}
	}else if (oldDir == 'W'){
		switch (move){
			case 'F':
				x = 'W';
				break;
			case 'L':
				x = 'S';
				break;
			case 'R':
				x = 'N';
				break;
			case 'B':
				x = 'E';
				break;
			default:
				break;
		}
	}else if (oldDir == 'E'){
		switch (move){
			case 'F':
				x = 'E';
				break;
			case 'L':
				x = 'N';
				break;
			case 'R':
				x = 'S';
				break;
			case 'B':
				x = 'W';
				break;
			default:
				break;
		}
	}else if (oldDir == 'S'){
		switch (move){
			case 'F':
				x = 'S';
				break;
			case 'L':
				x = 'E';
				break;
			case 'R':
				x = 'W';
				break;
			case 'B':
				x = 'N';
				break;
			default:
				break;
		}
	}else{
		printf("Error!!!! \n");
	}

	return x;
}

char oppositeDir(char dir){
	char x;

	return x;
}

alt_u32 returnHbridgeIn(int id){
	alt_u32 retval = 0;

	switch(id){
		case 0:
			retval = PWM_0_BASE;
			break;
		case 1:
			retval = PWM_1_BASE;
			break;
		case 2:
			retval = PWM_2_BASE;
			break;
		case 3:
			retval = PWM_3_BASE;
			break;
		default:
			break;
	}

	return retval;
}
