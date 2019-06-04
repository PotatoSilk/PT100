#include <STC89C5xRC.H>

#ifndef STRUCT_H
#define STRUCT_H

#define uchar unsigned char
#define uint unsigned int

typedef enum _ENUM_DISPLAYMODE
{
	SCAN_ROUND_MODE = 1,
	QUERY_MODE      = 2,
	SETTING_MODE    = 3,
	SCAN_MAX_MODE   = 4
} ENUM_DISPLAYMODE;

/**
 * LP == LongPress
 */
typedef enum _EMUM_KeyReturnValue
{
	KeyNone        = 0,
	KeyQuerySet    = 1,
	KeyDown        = 2,
	KeyMaxFan      = 3,
	KeyUp          = 4,
	
	KeyQuerySetLP1 = 1 | 0x10,
	KeyDownLP1     = 2 | 0x10,
	KeyMaxFanLP1   = 3 | 0x10,
	KeyUpLP1       = 4 | 0x10,
	
	KeyQuerySetLP2 = 1 | 0x20,
	KeyDownLP2     = 2 | 0x20,
	KeyMaxFanLP2   = 3 | 0x20,
	KeyUpLP2       = 4 | 0x20
} EMUM_KeyReturnValue;

typedef enum _EMUM_Flag
{
	NO  = 0,
	YES = 1
} EMUM_Flag;

typedef enum _ENUM_SYSFLAG
{
	SysTest   = 0,
	FanOFF    = 1,
	FanON     = 2,
	OverHeat  = 3,
	OHProtect = 4,
	ForceMute = 5
} ENUM_SYSFLAG;

typedef struct _STR_DISPLAYSTATE
{
	uchar ucDisplayChannel;
	uchar ucDisplayChannelCount;
	int iDisplayNum;
	ENUM_DISPLAYMODE eDisplayMode;
	uchar ucRefreshDisplayFlag;
} STR_DISPLAYSTATE;

typedef struct _STR_KEYSTATE
{
	EMUM_KeyReturnValue eKeyReturnVal;
	uchar ucKeyQueryCount;
} STR_KEYSTATE;

typedef union _UN_INT
{
	int iX;
	uchar ucS[2];
} UN_INT;

typedef struct _STR_SYSTEMSTATE
{
	EMUM_Flag eForceTurnOnFan;
	EMUM_Flag eTimingTurnOnFan;
	EMUM_Flag eForceMute;
	EMUM_Flag eAlarmTest;
	uchar ucChErrorReg;
} STR_SYSTEMSTATE;

typedef struct _STR_MAXTEMPERATURE
{
	uchar ucChannel;
	UN_INT  unMAXTemperature;
} STR_MAXTEMPERATURE;

#endif
