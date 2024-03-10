#ifndef ARBITARYPRECISIONARITHMETICS_NUMBER_H
#define ARBITARYPRECISIONARITHMETICS_NUMBER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

//little-endian
struct BigNum { //0 - 49 9 - 57
    char *digits_;
    size_t size_; //amount of digits
    int sign_; //-1 0 1
};
typedef struct BigNum *BigNum;

#define SUCCESS 0
#define ERROR 1

BigNum CreateNum();

int8_t SetFromStr(BigNum target, char const *str);

char *ToStr(BigNum num);

int8_t Add(BigNum lhs, BigNum rhs, BigNum res);

int8_t Sub(BigNum lhs, BigNum rhs, BigNum res);

int8_t Mult(BigNum lhs, BigNum rhs, BigNum res);

int8_t DivMod(BigNum lhs, BigNum rhs, BigNum quotient, BigNum remainder);


int8_t CopyNum(BigNum from, BigNum to);

int8_t Abs(BigNum from, BigNum to);

int8_t Compare(BigNum lhs, BigNum rhs); // -1 = lhs<rhs 0 = lhs==rhs 1 = lhs==rhs
int8_t Div(BigNum lhs, BigNum rhs, BigNum res);

int8_t Mod(BigNum lhs, BigNum rhs, BigNum res);

int8_t GCD(BigNum lhs, BigNum rhs, BigNum res);

void FreeNum(BigNum num);

void SwapNums(BigNum lhs, BigNum rhs);

#endif //ARBITARYPRECISIONARITHMETICS_NUMBER_H
