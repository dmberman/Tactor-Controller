/*
 * str2num.c
 *
 *  Created on: May 12, 2014
 *      Author: Dan
 */

#include "math.h"
#include "string.h"

unsigned long str2ul(unsigned char *str){
	signed long slCount;
	unsigned long result,len,multiplier;
	len = strlen((char *)str);
	if(len > 1 && str[0] == '0' && str[1] == 'x'){ //Check for hex
		for(slCount = 2; slCount < len; slCount++){
			//Check for hex value
			if(str[slCount] < '0' || str[slCount > 'F']){
				result = 0;
				return result;
			}
			result *= 16;
			result += (unsigned long)(str[slCount]-'0');
		}
		return result;
	}

	multiplier = 1;
	result = 0;
	for(slCount = len-1;slCount >= 0 ;slCount--){
		if(str[slCount]<'0' || str[slCount]>'9'){ //Check for numeric input
			result = 0;
			result --;
			return result;
		}
		else{
			result += (str[slCount]-'0')*multiplier;
			multiplier *= 10;
		}
	}
	return result;
}

double str2dbl(unsigned char *str){
	unsigned long ulCount, frac,neg,divisor,len;
	double result;

	len = strlen((char *) str);

	//Check for single digit
	if(len > 1){
		//Check for negative
		if(str[0] == '-'){
			neg = 1;
		}
		else{
			neg = 0;
		}

		//Check for hex value
		if(str[0] == '0' && str[1] == 'x'){
			for(ulCount = 2; ulCount < len; ulCount++){
				//Check for hex value
				if(str[ulCount] < '0' || str[ulCount > 'F']){
					result = 0;
					return result;
				}
				result *= 16;
				result += (double)(str[ulCount]-'0');
			}
			return result;
		}

	}
	frac = 0;
	result = 0;
	divisor = 10;
	for(ulCount = neg; ulCount < len; ulCount ++){
		if(str[ulCount] == '.'){
			frac = 1;
		}
		else{
			//Check for numeric value
			if(str[ulCount] < '0' || str[ulCount > '9']){
				result = 0;
				return result;
			}

			if(frac){
				result += (str[ulCount]-'0')/divisor;
				divisor *= 10;
			}

			else{
				result *= 10;
				result += str[ulCount]-'0';
			}
		}
	}
	return result;
}
