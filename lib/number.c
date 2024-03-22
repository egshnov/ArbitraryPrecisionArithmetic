#include "number.h"
#include <stdlib.h>
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
static int first_non_null(const char *str, size_t len) {
    bool has_a_sign = (str[0] == '-' || str[0] == '+');
    int ind = has_a_sign;
    while (ind < len - 1 && str[ind] == '0') {
        ind++;
    }
    return ind;
}

//target must be a result of CreateNum()
static int8_t set_from_str_with_size(char const *str, size_t str_size, BigNum target) {
    if (str == NULL || target == NULL || strcmp(str, "") == 0) return ERROR;
    int first_non_null_digit = first_non_null(str, str_size);
    if (first_non_null_digit >= str_size) return ERROR;
    free(target->digits_); // if target is already initialised;

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
int8_t SetFromStr(BigNum target, char const *str) {
    return set_from_str_with_size(str, strlen(str), target);
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

static void set_sign_minus(BigNum tmp, int lhs) {
    if (lhs == -1 &&
        !(tmp->size_ == 1 &&
          tmp->digits_[0] == 0)) { //-> rhs.sign = +1 cause set_sign_minus is used when signs are different
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
        if (tmp->digits_ == NULL) {
            free(insurance);
            return ERROR;
        }
        tmp->digits_[tmp->size_ - 1] = overhead;
    }
    return SUCCESS;
}

static void set_sign_plus(BigNum tmp, int lhs) {
    tmp->sign_ = lhs;
}

static int8_t
apply_operation(BigNum lhs, BigNum rhs, BigNum tmp, char(*operation)(char, char, char *),
                int8_t(*rearrange)(BigNum, char), void(*set_sign)(BigNum, int), int sign_lhs) {
    char overhead = 0;
    for (size_t i = 0; i < rhs->size_; i++) {
        tmp->digits_[i] = operation(lhs->digits_[i], rhs->digits_[i], &overhead);
    }
    for (size_t i = rhs->size_; i < lhs->size_; i++) {
        tmp->digits_[i] = operation(lhs->digits_[i], 0, &overhead);
    }
    int8_t code = rearrange(tmp, overhead);
    if (code == SUCCESS) {
        set_sign(tmp, sign_lhs);
    }
    return code;
}

#define MAX(a, b) (((a)>(b))?(a):(b))

//res must be a result of CreateNum, lhs and rhs must be initialized
int8_t Add(BigNum lhs, BigNum rhs, BigNum res) {
    BigNum lhs_abs = CreateNum();
    BigNum rhs_abs = CreateNum();
    if (lhs == NULL || rhs == NULL || Abs(lhs, lhs_abs) == ERROR || Abs(rhs, rhs_abs) == ERROR) {
        FreeNum(lhs_abs);
        FreeNum(rhs_abs);
        return ERROR;
    }
    int8_t cmp = Compare(lhs_abs, rhs_abs);

    BigNum tmp = CreateNum();
    if (tmp == NULL) {
        FreeNum(lhs_abs);
        FreeNum(rhs_abs);
        return ERROR;
    }
    tmp->size_ = MAX(lhs->size_, rhs->size_);
    tmp->sign_ = 1;
    tmp->digits_ = (char *) malloc(sizeof(char) * tmp->size_);
    if (tmp->digits_ == NULL) {
        FreeNum(tmp);
        FreeNum(lhs_abs);
        FreeNum(rhs_abs);
        return ERROR;
    }

    int8_t code;
    if (lhs->sign_ == rhs->sign_) {
        code = cmp != -1 ? apply_operation(lhs_abs, rhs_abs, tmp, plus, rearrange_plus, set_sign_plus, lhs->sign_)
                         : apply_operation(rhs_abs, lhs_abs, tmp, plus, rearrange_plus, set_sign_plus, rhs->sign_);
    } else {
        code = cmp != -1 ? apply_operation(lhs_abs, rhs_abs, tmp, minus, rearrange_minus, set_sign_minus, lhs->sign_)
                         : apply_operation(rhs_abs, lhs_abs, tmp, minus, rearrange_minus, set_sign_minus, rhs->sign_);
    }

    if (code != ERROR) {
        SwapNums(tmp, res);
    }

    FreeNum(tmp);
    FreeNum(lhs_abs);
    FreeNum(rhs_abs);
    return code;
}

int8_t Sub(BigNum lhs, BigNum rhs, BigNum res) {
    rhs->sign_ = -rhs->sign_;
    int8_t code = Add(lhs, rhs, res);
    if (rhs != res) rhs->sign_ = -rhs->sign_; // if Sub(lhs,rhs,rhs) was called
    return code;
}

int8_t Mult(BigNum lhs, BigNum rhs, BigNum res) {
    BigNum tmp;
    tmp = CreateNum();
    if (tmp == NULL) return ERROR;
    tmp->digits_ = (char *) malloc(sizeof(char) * (lhs->size_ + rhs->size_));
    if (tmp->digits_ == NULL) {
        FreeNum(tmp);
        return ERROR;
    }
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
        FreeNum(tmp);
        return ERROR;
    }
    tmp->sign_ = lhs->sign_ == rhs->sign_ ? +1 : -1;
    SwapNums(tmp, res);
    FreeNum(tmp);
    return SUCCESS;
}


int8_t Abs(BigNum from, BigNum to) {
    BigNum tmp = CreateNum();
    if (tmp == NULL) return ERROR;
    if (CopyNum(from, tmp) == ERROR) {
        FreeNum(tmp);
        return ERROR;
    };
    tmp->sign_ = 1;
    SwapNums(tmp, to);
    FreeNum(tmp);
    return SUCCESS;
}

/*
  a / b:
  a = b * q + r    0 <= r < |b|
  abs(lhs) >= abs(rhs):
  ++ = default
  -- = default -> q = q + 1   r = abs(rhs) - r
  -+ = default -> q = -(q+1) r = abs(rhs) - r
  +- = default -> q = -q    r = r

  abs(lhs) <= abs(rhs)
  ++ = default = q = 0 r = lhs
  -- q = 1  r = abs(rhs) - abs(lhs)
  -+ q = -1 r = abs(rhs) - abs(lhs)
  +- q = 0 r = lhs
*/

static int8_t add_character_front(BigNum target, char c) {
    if (target->size_ == 1 && target->digits_[0] == 0) {
        target->digits_[0] = c;
    } else {
        char *new_digits = (char *) malloc(sizeof(char) * (target->size_ + 1));
        if (new_digits == NULL) return ERROR;
        new_digits[0] = c;
        for (int i = 1; i < target->size_ + 1; i++) {
            new_digits[i] = target->digits_[i - 1];
        }
        free(target->digits_);
        target->size_ += 1;
        target->digits_ = new_digits;
    }
    return SUCCESS;
}

static int8_t absolute_values_division(BigNum lhs, BigNum rhs, BigNum quotient, BigNum remainder) {
    remainder->sign_ = 1;
    quotient->sign_ = 1;
    for (int ind = lhs->size_ - 1; ind >= 0; ind--) {
        if (add_character_front(remainder, lhs->digits_[ind]) == ERROR) return ERROR;
        int8_t cmp = Compare(remainder, rhs);
        if (cmp == -1) continue;
        char cnt = 0;
        while (cmp > -1) {
            Sub(remainder, rhs, remainder);
            cnt++;
            cmp = Compare(remainder, rhs);
        }
        if (!(ind == 0 && cnt == 0)) {
            if (add_character_front(quotient, cnt) == ERROR) return ERROR;
        }
    }
    return SUCCESS;
}

#define release(a, b, c, d) \
    {                 \
        FreeNum((a));  \
        FreeNum((b));  \
        FreeNum((c));  \
        FreeNum((d));\
    }

int8_t DivMod(BigNum lhs, BigNum rhs, BigNum quotient, BigNum remainder) {
    if (quotient == NULL && remainder == NULL) return ERROR;
    if (rhs->size_ == 1 && rhs->digits_[0] == 0) return ERROR;

    BigNum lhs_abs = CreateNum();
    BigNum rhs_abs = CreateNum();
    BigNum tmp_quotient = CreateNum();
    BigNum tmp_remainder = CreateNum();;

    if (tmp_quotient == NULL || tmp_remainder == NULL || rhs_abs == NULL || lhs_abs == NULL ||
        Abs(lhs, lhs_abs) == ERROR || Abs(rhs, rhs_abs) == ERROR) {
        release(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder);
        return ERROR;
    }

    if (Compare(lhs_abs, rhs_abs) == -1) {
        if (lhs->sign_ == 1) {
            if (SetFromStr(tmp_quotient, "0") == ERROR || CopyNum(lhs, tmp_remainder) == ERROR) {
                release(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder);
                return ERROR;
            }
        } else {
            if (SetFromStr(tmp_quotient, rhs->sign_ == -1 ? "1" : "-1") == ERROR ||
                Sub(rhs_abs, lhs_abs, tmp_remainder) == ERROR) {
                release(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder);
                return ERROR;
            }
        }
    } else {
        if (absolute_values_division(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder) == ERROR) {
            release(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder);
            return ERROR;
        }
        if (lhs->sign_ == -1) {
            BigNum unit = CreateNum();
            if (unit == NULL || SetFromStr(unit, "1") == ERROR || Add(tmp_quotient, unit, tmp_quotient) == ERROR ||
                Sub(rhs_abs, tmp_remainder, tmp_remainder) == ERROR) {
                FreeNum(unit);
                release(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder);
                return ERROR;
            }
            FreeNum(unit);
            if (rhs->sign_ == 1) tmp_quotient->sign_ = -1;
        } else if (rhs->sign_ == -1) tmp_quotient->sign_ = -1;

    }
    if (quotient != NULL) {
        SwapNums(tmp_quotient, quotient);
    }
    if (remainder != NULL) {
        SwapNums(tmp_remainder, remainder);
    }
    release(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder);
    return SUCCESS;
}

int8_t Div(BigNum lhs, BigNum rhs, BigNum res) {
    return DivMod(lhs, rhs, res, NULL);
}

int8_t Mod(BigNum lhs, BigNum rhs, BigNum res) {
    return DivMod(lhs, rhs, NULL, res);
}

int8_t gcd(BigNum a, BigNum b, BigNum res) {
    if (b->size_ == 1 && b->digits_[0] == 0) {
        SwapNums(a, res);
        return SUCCESS;
    }
    Mod(a, b, a);
    return gcd(b, a, res);
}

int8_t GCD(BigNum lhs, BigNum rhs, BigNum res) {
    BigNum tmp_lhs = CreateNum();
    BigNum tmp_rhs = CreateNum();
    BigNum tmp_res = CreateNum();
    if (tmp_lhs == NULL || tmp_rhs == NULL || tmp_res == NULL ||
        Abs(lhs, tmp_lhs) == ERROR || Abs(rhs, tmp_rhs) == ERROR) {
        FreeNum(tmp_lhs);
        FreeNum(tmp_rhs);
        FreeNum(tmp_res);
        return ERROR;
    }
    int8_t code = gcd(tmp_lhs, tmp_rhs, tmp_res);
    if (code != ERROR) {
        SwapNums(tmp_res, res);
    }
    FreeNum(tmp_lhs);
    FreeNum(tmp_rhs);
    FreeNum(tmp_res);
    return code;
}

int8_t Compare(BigNum lhs, BigNum rhs) { // 0 = equal , 1 = lhs > rhs  -1 = lhs < rhs
    if (lhs->sign_ != rhs->sign_) return lhs->sign_ == 1 ? 1 : -1;
    if (lhs->size_ > rhs->size_) return lhs->sign_ == 1 ? 1 : -1;
    if (lhs->size_ < rhs->size_) return lhs->sign_ == 1 ? -1 : 1;
    for (int i = lhs->size_ - 1; i >= 0; i--) {
        if (lhs->digits_[i] != rhs->digits_[i]) return lhs->digits_[i] > rhs->digits_[i] ? 1 : -1;
    }
    return 0;
}

int8_t CopyNum(BigNum from, BigNum to) {
    if (to == NULL || from == NULL) return ERROR;
    free(to->digits_);
    to->size_ = from->size_;
    to->sign_ = from->sign_;
    to->digits_ = (char *) malloc(sizeof(char) * to->size_);
    if (to->digits_ == NULL) return ERROR;
    for (int i = 0; i < to->size_; i++) {
        to->digits_[i] = from->digits_[i];
    }
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
