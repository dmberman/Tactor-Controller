/*
 * proto_defs.h
 *
 *  Created on: Mar 22, 2014
 *      Author: Dan
 */

#ifndef PROTO_DEFS_H_
#define PROTO_DEFS_H_

#define N_PARAMS 10 //Max number of parameters for a function
#define MAX_STR_LEN 200 //Max length in characters of parameter or function name value
#define Tx_MAX_PACKET_LENGTH 25//Max length of transmit packet

#define MAX_PARAM_LENGTH 16
#define MAX_FUNC_LENGTH 16

//Motor Controls
#define MOTOR_RUN 'R'
#define MOTOR_STOP 'S'
#define ALL_STOP 'A'
#define ALL_RUN 'T'

//Communication
#define ACK 'K'
#define NACK 'N'
#define LED 'L'
#define COM_CHECK 'H'
#define RESET 'X'
#define END_OF_PACKET 10

//Waveforms
#define SET_WAVEFORM 'W'
#define GET_WAVEFORM 'G'
#define SET_SAMPLE_RATE 'P'
#define GET_SAMPLE_RATE 'M'
#define GET_CURRENT_MS 'Q'

#endif /* PROTO_DEFS_H_ */
