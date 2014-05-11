/*
 * =====================================================================================
 *
 *       Filename:  fletcher.h
 *
 *    Description:  Header file for fletcher's checksum 
 *
 *        Version:  1.0
 *        Created:  04/05/2014 05:35:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef FLETCHER_H
#define FLETCHER_H

#include <stdint.h>     /*  Standard integer types */

/**
 * struct to hold values of fletcher checksum
 */
typedef struct fletcher_checksum {
    uint8_t sum1;
    uint8_t sum2;
} fletcher_checksum;

struct fletcher_checksum fletcher_checksum16 (unsigned char *data, size_t bytes);

#endif /* !FLETCHER_H */

