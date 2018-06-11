/************************************************************
description
	ofxArtnetにて、common.hが使用されている。
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <stdio.h>

/************************************************************
************************************************************/
// #define SJ_DEBUG


/************************************************************
************************************************************/
enum{
	BUF_SIZE = 512,
};

#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);
#define WARNING_MSG(); printf("Warning in %s:%d\n", __FILE__, __LINE__);

/************************************************************
************************************************************/
extern void printMessage(const char* message);

