#include <stdio.h>
#include <number.h>
#include <string.h>
#include <malloc.h>


int main() {
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    BigNum quotient = CreateNum();
    BigNum remainder = CreateNum();
    BigNum result = CreateNum();
    SetFromStr("-5465465465468546354535556", lhs);
    SetFromStr("-36565654656568684868", rhs);
    Division(lhs, rhs, quotient, remainder);
    Mult(rhs, quotient, result);
    Add(result, remainder, result);
    printf("result is: %d", Compare(result, lhs));
    FreeNum(lhs);
    FreeNum(rhs);
    FreeNum(quotient);
    FreeNum(remainder);
    FreeNum(result);
    // printf("%d %d", FirstNonNull(str, strlen(str)), strlen(str));
}