/*
 * motors.h
 *
 *  Created on: Mar 25, 2013
 *      Author: Peter
 */

#ifndef MOTORS_H_
#define MOTORS_H_

void stoptofull(void);
void slowtostop(void);
void stoptopartial(void);

alt_u32 returnHbridgeIn(int id);

void turnLeft_simple();

void turnLeft(void);
void turnRight(void);
void turn180(void);

void printHistory(move_obj *TRVL, int TRVL_INDEX);
void executeHistory(move_obj *TRVL, int TRVL_INDEX);
bool evalHistory(move_obj *TRVL, int TRVL_INDEX, char dir, double distance);

char newDir(char oldDir, char move);
char oppositeDir(char dir);

void motorReverse(void);
void motorGo(int leftin, int rightin, double duty, double distance);
void motorGoForward(int leftin, int rightin, double leftduty, double rightduty, double distance);

int M_checkMotion(void);
void M_allStop(void);

#endif /* MOTORS_H_ */

