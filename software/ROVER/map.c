/*
 * mapping.c
 *
 *  Created on: Mar 13, 2013
 *      Author: Peter
 */

#include "map.h"
#include <stdio.h>
#define OPEN 50
#define CLOSED 0

int bug, counter;
int newRightOpening;
int newLeftOpening;
int oldLeftOpening;
int oldRightOpening;



void GenerateTest()
{
	double front, left, right;
	int angles, test;
	bug = 0;
	counter = 0;
	int length = 200;
	int a;
	int commandList[] = {LEFT,LEFT,RIGHT,RIGHT};

	oldLeftOpening=CLOSED;
	oldRightOpening=CLOSED;
	for (a=0;a<sizeof(commandList)/sizeof(commandList[0]);a++)
	{
	//GenerateForward(front,left,right,angles,test,100);

		if (a != 0) {
			if ( commandList[a - 1] == LEFT) {
				oldLeftOpening = OPEN;
				oldRightOpening = CLOSED;
			}  else{
				oldLeftOpening = CLOSED;
				oldRightOpening = OPEN;
			}
		}

		if(a+1!=sizeof(commandList)/sizeof(commandList[0])){
			if(commandList[a+1]==LEFT)
			{

				newLeftOpening=OPEN;
				newRightOpening=CLOSED;

			}
			else
			{

				newLeftOpening=CLOSED;
				newRightOpening=OPEN;

			}
		}

	    if(commandList[a]==LEFT)
	    {
	        printf("%d %d %d %d LEFT\n",oldLeftOpening,oldRightOpening,newLeftOpening,newRightOpening);
	        //GenerateLeft(front,left,right,angles,test,length,length);
	    }
	    else
	    {
	        printf("%d %d %d %d RIGHT\n",oldLeftOpening,oldRightOpening,newLeftOpening,newRightOpening);
	        //GenerateRight(front,left,right,angles,test,length,length);
	    }
	}


}


void GenerateForward(double *front,double *left, double *right, int *angles, int *test,int length)
{

	int a = 0;
	bug=counter;
	for (a = 0; a <= length; a++) {
		angles[ bug+a] = angles;
		front[ bug+a] = length+50 - a;
		right[bug + a] = 25;
		left[bug + a] = 25;
		test[bug + a] = FORWARD;
	}
	counter+=length;
	bug=counter;
}

/*

void GenerateLeft(double *front,double *left, double *right, int *angles, int *test,int sLength, int bLength)
{
    int j = 0;


    double Rangle=acos((25.00/(sLength-newRightOpening)))*180/PI;
    double Langle=acos(25.00/(sLength-newLeftOpening))*180/PI;
    double OLangle=acos(25.00/(bLength-oldLeftOpening))*180/PI;
    double ORangle=acos(25.00/(bLength-oldRightOpening))*180/PI;


    bug=counter;
    for (j = 0; j <= 25; j++) {

        angles[bug + j] = angle;
        front[bug + j] = 50 - j;
        right[bug + j] = 25;
        left[bug + j] = sLength;
        test[bug + j] = FORWARD;
    }


    counter+=25;
    bug=counter;

    for (j = 0; j <= 90; j++) {
        angles[bug + j] = angle + j;
//front sensor
        if (j <= Rangle) {
            front[bug + j] = 25 / cos(j * PI / 180);
        } else {
            front[bug + j] = sLength / cos((90 - j) * PI / 180);
        }
//right sensor
        if(newRightOpening==OPEN)
        {

        }
        if (j < 45) {
            right[bug + j] = 25 / cos(j * PI/ 180);
        } else {
            right[bug + j] = 25 / cos((90 - j) * PI / 180);
        }
//Left sensor
        if (j < (90-Langle)) {
            left[bug + j] = sLength / cos(j * 3.14 / 180);
        } else if (j < 45) {
            left[bug + j] = 25 / cos((90 - j) * 3.14 / 180);
        } else if (j <OLangle) {
            left[bug + j] = 25 / sin((90 - j) * 3.14 / 180);
        } else {
            left[bug + j] = bLength / cos((90 - j) * 3.14 / 180);
        }
        test[bug + j] = LEFT;
    }
    test[bug + 90] = FORWARD;
    counter+=91;
    bug=counter;
    angle+=90;

    for (j = 0; j <= 25; j++) {
        angles[bug + j] = angle;
        front[bug + j] = sLength - j;
        right[bug +j] = 25;
        left[bug + j] = bLength;
        test[bug + j] = FORWARD;
    }
    counter+=25;
    bug=counter;



}
void GenerateRight(double *front,double *left, double *right, int *angles, int *test,int sLength, int bLength)
{

    int j = 0;


    double Rangle=acos((25.00/(sLength-newRightOpening)))*180/PI;
    double Langle=acos(25.00/(sLength-newLeftOpening))*180/PI;
    double OLangle=acos(25.00/(bLength-oldLeftOpening))*180/PI;
    double ORangle=acos(25.00/(bLength-oldRightOpening))*180/PI;

    bug=counter;
    for (j = 0; j <= 25; j++) {

        angles[bug + j] = angle;
        front[bug + j] = 50 - j;
        left[bug + j] = 25;
        right[bug + j] = sLength;
        test[bug + j] = FORWARD;
    }


    counter+=25;
    bug=counter;

    for (j = 0; j <= 90; j++) {
        angles[bug + j] = angle - j;
//front sensor
        if (j <= Rangle) {
            front[bug + j] = 25 / cos(j * PI / 180);
        } else {
            front[bug + j] = sLength / cos((90 - j) * PI / 180);
        }
//left sensor
        if (j < 45) {
            left[bug + j] = 25 / cos(j * PI/ 180);
        } else {
            left[bug + j] = 25 / cos((90 - j) * PI / 180);
        }
//right sensor

        if (j < (90-Rangle)) {
            right[bug + j] =  sLength / cos(j * 3.14 / 180);
        } else if (j < 45) {
            right[bug + j] = 25 / cos((90 - j) * 3.14 / 180);
        } else if (j <ORangle) {
            right[bug + j] = 25 / sin((90 - j) * 3.14 / 180);
        } else {
            right[bug + j] = bLength / cos((90 - j) * 3.14 / 180);
        }
        test[bug + j] = RIGHT;
    }
    test[bug + 90] = FORWARD;
    counter+=91;
    bug=counter;
    angle+=-90;

    for (j = 0; j <= 25; j++) {
        angles[bug + j] = angle;
        front[bug + j] = sLength - j;
        left[bug +j] = 25;
        right[bug + j] = bLength;
        test[bug + j] = FORWARD;
    }
    counter+=25;
    bug=counter;



}
*/
