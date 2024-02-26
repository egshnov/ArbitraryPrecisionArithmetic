#include "number.h"
#include <malloc.h>
#include <ctype.h>
#include <string.h>

const int base = 10;
#define swap(T, x, y) \
    {                 \
        T obj = (x);  \
        (x) = (y);    \
        (y) = obj;    \
    }

void SwapNums(BigNum lhs, BigNum rhs) {
    swap(size_t, lhs->size_, rhs->size_);
    swap(char*, lhs->digits_, rhs->digits_);
    swap(int, lhs->sign_, rhs->sign_);
}

//double pointer?
BigNum CreateNum() {
    BigNum tmp = (BigNum) (malloc(sizeof(struct BigNum)));
    if (tmp != NULL) {
        tmp->digits_ = NULL;
        tmp->size_ = 0;
        tmp->sign_ = 0;
    }
    return tmp;
}

//gets non-empty null-terminated string
static int FirstNonNull(const char *str, size_t len) {
    bool has_a_sign = (str[0] == '-' || str[0] == '+');
    int ind = has_a_sign;
    while (ind < len - 1 && str[ind] == '0') {
        ind++;
    }
    return ind;
}

//target must be a result of CreateNum()
static int8_t SetFromStrWithSize(char const *str, size_t str_size, BigNum target) {
    if (str == NULL || target == NULL || strcmp(str, "") == 0) return ERROR;

    int first_non_null_digit = FirstNonNull(str, str_size);
    if (first_non_null_digit >= str_size) return ERROR;


    target->size_ = str_size - first_non_null_digit;

    target->digits_ = (char *) malloc(sizeof(char) * target->size_);

    if (target->digits_ == NULL) return ERROR;

    int ind = str_size - 1;
    for (size_t dig_ind = 0; ind >= first_non_null_digit; ind--) {
        if (!isdigit(str[ind])) return ERROR;
        target->digits_[dig_ind++] = str[ind] - '0';
    }

    if (isdigit(str[0])) {
        target->sign_ = 1;
    } else if (str[0] == '+' || str[0] == '-') {
        target->sign_ = str[0] == '+' || target->digits_[target->size_ - 1] == 0 ? 1 : -1;
    } else return ERROR;

    return SUCCESS;
}

//undefined behaviour if str is not a null terminated string
int8_t SetFromStr(char const *str, BigNum target) {
    return SetFromStrWithSize(str, strlen(str), target);
}

//null if couldn't alloc , ub if num was initialised incorrectly
char *ToStr(BigNum num) {
    bool is_negative = num->sign_ == -1;
    int additional_cells = (is_negative ? 2 : 1);
    char *str = (char *) malloc(sizeof(char) * (num->size_ + additional_cells));
    if (str == NULL) return str;
    if (is_negative) str[0] = '-';

    for (size_t i = 0; i < num->size_; i++) {
        str[i + is_negative] = num->digits_[num->size_ - 1 - i] + '0';
    }

    str[num->size_ + is_negative] = '\0';
    return str;
}

static char minus(char l, char r, char *overhead) {
    bool need_additional = (l - *overhead) < r;
    char additional = need_additional ? base : 0;
    char result = l + additional - r - *overhead;
    *overhead = need_additional ? 1 : 0;
    return result;
}

static int8_t rearrange_minus(BigNum tmp, char overhead) {
    size_t i = tmp->size_ - 1;
    while (i > 0 && tmp->digits_[i] == 0) {
        i--;
    }
    tmp->size_ = i + 1;
    char *insurance = tmp->digits_;
    tmp->digits_ = (char *) realloc(tmp->digits_, sizeof(char *) * tmp->size_);
    if (tmp->digits_ == NULL) {
        free(insurance);
        return ERROR;
    }
    return SUCCESS;
}

static void set_sign_minus(BigNum tmp, BigNum lhs, BigNum rhs) {
    if (lhs->sign_ == -1) { //-> rhs.sign = +1 cause set_sign_minus is used whe signs are different
        tmp->sign_ = -1;
    } else {
        tmp->sign_ = +1;
    }
}

static char plus(char l, char r, char *overhead) {
    char intermediate = l + r + *overhead;
    *overhead = intermediate / base;
    return intermediate % base;
}

static int8_t rearrange_plus(BigNum tmp, char overhead) {
    char *insurance = NULL;
    if (overhead != 0) {
        tmp->size_ += 1;
        insurance = tmp->digits_;
        tmp->digits_ = (char *) realloc(tmp->digits_, sizeof(char) * tmp->size_);
        tmp->digits_[tmp->size_ - 1] = overhead;
    }
    if (tmp->digits_ == NULL) {
        free(insurance);
        return ERROR;
    }
    return SUCCESS;
}

static void set_sign_plus(BigNum tmp, BigNum lhs, BigNum rhs) {
    tmp->sign_ = lhs->sign_;
}

static int8_t
apply_operation(BigNum lhs, BigNum rhs, BigNum tmp, char(*operation)(char, char, char *),
                int8_t(*rearrange)(BigNum, char), void(*set_sign)(BigNum, BigNum, BigNum)) {
    char overhead = 0;
    for (size_t i = 0; i < rhs->size_; i++) {
        tmp->digits_[i] = operation(lhs->digits_[i], rhs->digits_[i], &overhead);
    }
    for (size_t i = rhs->size_; i < lhs->size_; i++) {
        tmp->digits_[i] = operation(lhs->digits_[i], 0, &overhead);
    }
    int8_t code = rearrange(tmp, overhead);
    if (code == SUCCESS) {
        set_sign(tmp, lhs, rhs);
    }
    return code;
}

static int8_t apply_addition(BigNum lhs, BigNum rhs, BigNum res) {
    BigNum tmp = CreateNum();
    tmp->size_ = lhs->size_;
    tmp->sign_ = 1;
    tmp->digits_ = (char *) malloc(sizeof(char) * tmp->size_);
    int8_t code;
    if (tmp->digits_ == NULL) return ERROR;
    if (lhs->sign_ == rhs->sign_) {
        code = apply_operation(lhs, rhs, tmp, plus, rearrange_plus, set_sign_plus);
    } else {
        code = apply_operation(lhs, rhs, tmp, minus, rearrange_minus, set_sign_minus);
    }
    if (code == ERROR) return ERROR;
    SwapNums(tmp, res);
    FreeNum(tmp);
    return SUCCESS;
}

// res must be a result of CreateNum, lhs and rhs must be initialized
int8_t Add(BigNum lhs, BigNum rhs, BigNum res) {
    if (lhs->size_ >= rhs->size_) return apply_addition(lhs, rhs, res);
    return apply_addition(rhs, lhs, res);
}

int8_t Sub(BigNum lhs, BigNum rhs, BigNum res) {
    rhs->sign_ = -rhs->sign_;
    int8_t code = lhs->size_ >= rhs->size_ ? apply_addition(lhs, rhs, res) : apply_addition(rhs, lhs, res);
    if (rhs != res) rhs->sign_ = -rhs->sign_; // if Sub(lhs,rhs,rhs) was called
    return code;
}

int8_t Mult(BigNum lhs, BigNum rhs, BigNum res) {
    BigNum tmp;
    tmp = CreateNum();
    tmp->digits_ = (char *) malloc(sizeof(char) * (lhs->size_ + rhs->size_));
    if (tmp->digits_ == NULL) return ERROR;
    for (int i = 0; i < lhs->size_ + rhs->size_; i++) {
        tmp->digits_[i] = 0;
    }
    for (int i = 0; i < lhs->size_; i++) {
        for (int j = 0, overhead = 0; j < rhs->size_ || overhead; j++) {
            char intermediate =
                    tmp->digits_[i + j] + lhs->digits_[i] * (j < rhs->size_ ? rhs->digits_[j] : 0) + overhead;
            tmp->digits_[i + j] = intermediate % base;
            overhead = intermediate / base;
        }
    }
    int ind = lhs->size_ + rhs->size_ - 1;
    while (ind > 0 && tmp->digits_[ind] == 0) {
        ind--;
    }
    tmp->size_ = ind + 1;
    char *insurance = tmp->digits_;
    tmp->digits_ = (char *) (realloc(tmp->digits_, sizeof(char) * tmp->size_));
    if (tmp->digits_ == NULL) {
        free(insurance);
        return ERROR;
    }
    tmp->sign_ = lhs->sign_ == rhs->sign_ ? +1 : -1;
    SwapNums(tmp, res);
    FreeNum(tmp);
    return SUCCESS;
}

void FreeNum(BigNum num) {
    if (num != NULL) {
        num->size_ = 0;
        num->sign_ = 0;
        free(num->digits_);
    }
    free(num);
}
//
//BigNum NumFromInt(int num) {
//    char int_str[20];
//    sprintf(int_str, "%d", num);
//    return NumFromStr(int_str);
//}
//
