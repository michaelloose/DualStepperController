/*
* functions.c
*
* Created: 26.07.2021 17:24:27
*  Author: michaelloose
*/

#include "headers.h"

void double2str(char* buf, double val){

	char *tmpSign = (val < 0) ? "-" : "";
	double tmpVal = (val < 0) ? -val : val;

	int tmpInt1 = tmpVal;                  // Get the integer (678).
	double tmpFrac = tmpVal - tmpInt1;      // Get fraction (0.0123).
	int tmpInt2 = trunc(tmpFrac * 10000);  // Turn into integer (123).

	// Print as parts, note that you need 0-padding for fractional bit.

	sprintf (buf, "%s%d.%04d", tmpSign, tmpInt1, tmpInt2);
	
}