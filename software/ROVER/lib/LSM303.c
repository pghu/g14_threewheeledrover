#include "robot_includes.h"
#include "LSM303.h"
#include "I2C.h"

// The Arduino two-wire interface uses a 7-bit number for the address, 
// and sets the last bit correctly based on reads and writes
#define MAG_ADDRESS            0x3C
#define ACC_ADDRESS_SA0_A_LOW  0x30
#define ACC_ADDRESS_SA0_A_HIGH 0x32

// Timeout Methods //////////////////////////////////////////////////////////////
bool timeoutOccurred(LSM303 *compass)
{
  return compass->did_timeout;
}

void setTimeout(LSM303 *compass, unsigned int timeout)
{
  compass->io_timeout = timeout;
}

unsigned int getTimeout(LSM303 *compass)
{
  return compass->io_timeout;
}

// Initialize the LSM303
void compassInit(LSM303 *compass)
{  
	compass->acc_address = ACC_ADDRESS_SA0_A_HIGH;
    compass->_device = LSM303DLHC_DEVICE;

    /*
    compass->m_max.x = +540;
    compass->m_max.y = +500;
    compass->m_max.z = 180;

    compass->m_min.x = -520;
    compass->m_min.y = -570;
    compass->m_min.z = -770;
*/
    compass->io_timeout = 0;  // 0 = no timeout
    compass->did_timeout = FALSE;
}

// Turns on the LSM303's accelerometer and magnetometers and places them in normal
// mode.
void enableDefault(void)
{
  // Enable Accelerometer
  // 0x27 = 0b00100111
  // Normal power mode, all axes enabled
 // writeAccReg(LSM303_CTRL_REG1_A, 0x27);
	//if (_device == LSM303DLHC_DEVICE)
	//    writeAccReg(LSM303_CTRL_REG4_A, 0x08); // DLHC: enable high resolution mode
  
  // Enable Magnetometer
  // 0x00 = 0b00000000
  // Enter Continuous conversion mode from sleep mode
 // writeMagReg(LSM303_MR_REG_M, 0x00);

  //if (I2C_Write(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, ACC_ADDRESS_SA0_A_HIGH, LSM303_CTRL_REG1_A, 0x27)){
//	  printf("Initialize success!\n");
 // }
 // else{		printf("Failed to enable accelerometer\r\n");
 // }

  //For magnetic sensors the default (factory) 7-bit slave address is 0011110xb. R = 1, W = 0 so READ = 0x00111101 = 0x3D

  if (I2C_Write(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, MAG_ADDRESS, LSM303_MR_REG_M, 0x00)){
	  printf("Initialize success!\n");
  }
  else{
      printf("Failed to enable magnetometer\r\n");
  }
}

// Writes a magnetometer register
void writeMagReg(alt_u8 reg, alt_u8 value)
{
  if (I2C_Write(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, MAG_ADDRESS, reg, value)){
	  printf("Success!\n");
  }else{
        printf("Failed to write magnetometer\r\n");
  }
}

// Reads a magnetometer register
alt_u8 readMagReg(alt_u8 reg)
{
  alt_u8 *value;
  
  if (I2C_Read(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, MAG_ADDRESS, reg, value)){
	  printf("Success! %02x \n", (int) &value);
  }else{
        printf("Failed to read magnetometer register\r\n");
        value = 0;
  }

  return *value;
}

void setMagGain(alt_u8 value)
{
	//check to see if value is in maggain
  if (I2C_Write(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, MAG_ADDRESS, LSM303_CRB_REG_M, value)){
	  printf("Success!\n");
  }else{
        printf("Failed to write magnetometer gain\r\n");
  }
}

// Reads the 3 magnetometer channels and stores them in vector m
void readMag(LSM303 *compass)
{
	alt_u16 xhm, xlm, yhm, ylm, zhm, zlm;
	alt_u8 szBuf[5]; //need 6 values, magnetometer only refreshes data once these 6 values have been read
	int Num, i;

	if (I2C_MultipleRead(COMPASS_I2C_SCL_BASE, COMPASS_I2C_SDA_BASE, MAG_ADDRESS, LSM303DLHC_OUT_X_H_M , szBuf, sizeof(szBuf)))
	{
		Num = sizeof(szBuf)/sizeof(szBuf[0]);
        for(i=0;i<Num;i++){
            printf("Addr[%d] = %02xh\r\n", i, szBuf[i]);
        }

        xhm = szBuf[0];
        xlm = szBuf[1];
        zhm = szBuf[2];
        zlm = szBuf[3];
        yhm = szBuf[4];
        ylm = szBuf[5];
    }
	else{
        printf("Failed to read magnetometer\r\n");
    }

  // combine high and low bytes
  compass->m.x = (alt_u16)(xhm << 8 | xlm);
  compass->m.y = (alt_u16)(yhm << 8 | ylm);
  compass->m.z = (alt_u16)(zhm << 8 | zlm);
}

// Reads all 6 channels of the LSM303 and stores them in the object variables
void LSM303_read(LSM303 *compass)
{
  //readAcc();
  readMag(compass);
}

void LSM303_calibrate(LSM303 *compass)
{
  vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};
  running_min.x = fmin(running_min.x, compass->m.x);
  running_min.y = fmin(running_min.y, compass->m.y);
  running_min.z = fmin(running_min.z, compass->m.z);

  running_max.x = fmax(running_max.x, compass->m.x);
  running_max.y = fmax(running_max.y, compass->m.y);
  running_max.z = fmax(running_max.z, compass->m.z);

  printf("M min \n");
  printf("X: %d \n", (int)running_min.x);
  printf("Y: %d \n", (int)running_min.y);
  printf("Z: %d \n", (int)running_min.z);

  printf("M max \n");
  printf("X: %d \n", (int)running_max.x);
  printf("Y: %d \n", (int)running_max.y);
  printf("Z: %d \n", (int)running_max.z);
}

// Returns the number of degrees from the -Y axis that it
// is pointing.
int heading_Y(LSM303 *compass)
{
	vector new = {0, -1, 0};
	return heading(compass, new);
}

// Returns the number of degrees from the From vector projected into
// the horizontal plane is away from north.
// 
// Description of heading algorithm: 
// Shift and scale the magnetic reading based on calibration data to
// to find the North vector. Use the acceleration readings to
// determine the Down vector. The cross product of North and Down
// vectors is East. The vectors East and North form a basis for the
// horizontal plane. The From vector is projected into the horizontal
// plane and the angle between the projected vector and north is
// returned.
int heading(LSM303 *compass, vector from)
{
    // shift and scale
	compass->m.x = (compass->m.x - compass->m_min.x) / (compass->m_max.x - compass->m_min.x) * 2 - 1.0;
	compass->m.y = (compass->m.y - compass->m_min.y) / (compass->m_max.y - compass->m_min.y) * 2 - 1.0;
	compass->m.z = (compass->m.z - compass->m_min.z) / (compass->m_max.z - compass->m_min.z) * 2 - 1.0;

    vector temp_a = compass->a;
    // normalize
    vector_normalize(&temp_a);
    //vector_normalize(&m);

    // compute E and N
    vector E;
    vector N;
    vector_cross(&compass->m, &temp_a, &E);
    vector_normalize(&E);
    vector_cross(&temp_a, &E, &N);
  
    // compute heading
    int heading = round(atan2(vector_dot(&E, &from), vector_dot(&N, &from)) * 180 / M_PI);
    if (heading < 0) heading += 360;
  return heading;
}

void vector_cross(const vector *a,const vector *b, vector *out)
{
  out->x = a->y*b->z - a->z*b->y;
  out->y = a->z*b->x - a->x*b->z;
  out->z = a->x*b->y - a->y*b->x;
}

float vector_dot(const vector *a,const vector *b)
{
  return a->x*b->x+a->y*b->y+a->z*b->z;
}

void vector_normalize(vector *a)
{
  float mag = sqrt(vector_dot(a,a));
  a->x /= mag;
  a->y /= mag;
  a->z /= mag;
}

// Private Methods //////////////////////////////////////////////////////////////

/*
byte detectSA0_A(void)
{
  Wire.beginTransmission(ACC_ADDRESS_SA0_A_LOW);
  Wire.write(LSM303_CTRL_REG1_A);
  last_status = Wire.endTransmission();
  Wire.requestFrom(ACC_ADDRESS_SA0_A_LOW, 1);
  if (Wire.available())
  {
    Wire.read();
    return LSM303_SA0_A_LOW;
  }
  else
    return LSM303_SA0_A_HIGH;
}
*/
