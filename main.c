#include <stdio.h>
#include <number.h>
#include <string.h>
#include <malloc.h>


int main() {
    printf("%d\n", '/' - '0');
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    SetFromStr("-11111", lhs);
    SetFromStr("231", rhs);
    Sub(lhs, rhs, rhs);
    char *str = ToStr(rhs);
    printf("%s\n", str);
    FreeNum(lhs);
    FreeNum(rhs);
    free(str);
    // printf("%d %d", FirstNonNull(str, strlen(str)), strlen(str));
}