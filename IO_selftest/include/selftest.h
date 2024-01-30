/*
 * 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _SELFTEST_H_
#define _SELFTEST_H_



#ifdef __cplusplus
extern "C" {
#endif


#define MESSAGE_SIZE 64
#define QUEUE_SIZE 64 // set high for development


typedef struct {
char data[MESSAGE_SIZE];
} MESSAGE;

struct {   // global structure
    MESSAGE messages[QUEUE_SIZE];
    int begin;
    int end;
    int current_load;
} queue;


bool enque(MESSAGE *message);

#ifdef __cplusplus
}
#endif

#endif //

