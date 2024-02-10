#ifndef systolic//conditional compilation
#define systolic

#include <ap_int.h>

typedef ap_int<16> DataType;

#define SIDE_LEN 4// The length of a side of the systolic array

void systolic_array(DataType din_a[], DataType din_b[], DataType res[]);

#endif