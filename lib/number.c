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
static int8_t set_from_str_with_size(char const *str, size_t str_size, BigNum target) {
    if (str == NULL || target == NULL || strcmp(str, "") == 0) return ERROR;
    free(target->digits_); // if target is already initialised;
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
    if (tmp == NULL) return ERROR;
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
    if (tmp == NULL) return ERROR;
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


int8_t Abs(BigNum to, BigNum from) {
    printf("from in abs %s\n\n\n", ToStr(from));
    BigNum tmp = CreateNum();
    if (tmp == NULL) return ERROR;
    if (CopyNum(tmp, from) == ERROR) return ERROR;
    tmp->sign_ = 1;
    SwapNums(tmp, to);
    printf("in abs %s\n", ToStr(to));
    FreeNum(tmp);
    return SUCCESS;
}

/*
  a / b:
  a = b * q + r    0 <= r < |b|
  abs(lhs) >= abs(rhs):
  ++ = default
  -- = default -> q += 1   r = -r -> r += rhs
  -+  , +- = default q = -q   r = r


  abs(lhs) <= abs(rhs)
  ++ = default = q = 0 r = lhs
  -- q = 1  r = abs(lhs) + rhs
  -+ q = -1 r = lhs + rhs
  +- q = 0 r = lhs
*/

static int8_t add_character_front(BigNum target, char c) {
    char *new_digits = (char *) malloc(sizeof(char) * (target->size_ + 1));
    if (new_digits == NULL) return ERROR;
    new_digits[0] = c;
    for (int i = 1; i < target->size_ + 1; i++) {
        new_digits[i] = target->digits_[i - 1];
    }
    free(target->digits_);
    target->size_ += 1;
    target->digits_ = new_digits;
    return SUCCESS;
}

static int8_t add_character_back(BigNum target, char c) {
    char *new_digits = (char *) malloc(sizeof(char) * (target->size_ + 1));
    if (new_digits == NULL) return ERROR;
    for (int i = 0; i < target->size_ + 1; i++) {
        new_digits[i] = target->digits_[i - 1];
    }
    free(target->digits_);
    new_digits[target->size_] = c;
    target->size_ += 1;
    target->digits_ = new_digits;
    return SUCCESS;
}

static int8_t absolute_values_division(BigNum lhs, BigNum rhs, BigNum quotient, BigNum remainder) {
    printf("lhs: %s\n", ToStr(lhs));
    printf("rhs: %s\n", ToStr(rhs));

    for (int ind = lhs->size_ - 1; ind >= 0; ind--) {
        if (add_character_front(remainder, lhs->digits_[ind]) == ERROR) return ERROR;
        printf("added charcater %d\n", lhs->digits_[ind]);
        int8_t cmp = Compare(remainder, rhs);
        printf("compare is %d\n", cmp);
        printf("remainder: %s\n", ToStr(remainder));
        printf("rhs: %s\n", ToStr(rhs));
        if (cmp == -1) continue;
        char cnt = 0;
        while (cmp != -1) {
            Sub(remainder, rhs, remainder);
            cnt++;
            cmp = Compare(remainder, rhs);
        }
        add_character_back(quotient, cnt);
    }
    return SUCCESS;
}

static const int maintainer_size = 4;

static void release_maintainer(BigNum *maintainer) {
    for (int i = 0; i < maintainer_size; i++) {
        FreeNum(maintainer[i]);
    }
}

static void add_to_maintainer(BigNum *maintainer, BigNum target) {
    static int ind = 0;
    maintainer[ind++] = target;
}

int8_t Division(BigNum lhs, BigNum rhs, BigNum quotient, BigNum remainder) {
    printf("in div\n");
    printf("lhs: %s\n", ToStr(lhs));
    printf("rhs: %s\n", ToStr(rhs));
    printf("\n\n\n");

    if (rhs->size_ == 1 && rhs->digits_[0] == 0) return ERROR;
    BigNum maintainer[maintainer_size];
    for (int i = 0; i < maintainer_size; i++) {
        maintainer[i] = NULL;
    }

    BigNum lhs_abs = CreateNum();
    BigNum rhs_abs = CreateNum();
    BigNum tmp_quotient = CreateNum();
    BigNum tmp_remainder = CreateNum();

    add_to_maintainer(maintainer, lhs_abs);
    add_to_maintainer(maintainer, rhs_abs);
    add_to_maintainer(maintainer, tmp_quotient);
    add_to_maintainer(maintainer, tmp_remainder);

    if (tmp_quotient == NULL || tmp_remainder == NULL || rhs_abs == NULL || lhs_abs == NULL ||
        Abs(lhs_abs, lhs) == ERROR || Abs(rhs_abs, rhs) == ERROR) {
        release_maintainer(maintainer);
        return ERROR;
    }
    printf("after abs\n");
    printf("lhs: %s\n", ToStr(lhs_abs));
    printf("rhs: %s\n", ToStr(rhs_abs));
    printf("\n\n\n");

    if (Compare(lhs_abs, rhs_abs) == -1) {
          printf("lhs < rhs\n");
        if (lhs->sign_ == 1) {
            if (SetFromStr("0", tmp_quotient) == ERROR || CopyNum(tmp_remainder, lhs) == ERROR) {
                release_maintainer(maintainer);
                return ERROR;
            }
             printf("++ or +-\n");
        } else {
            printf("-- or ++ \n");
            if (SetFromStr(rhs->sign_ == -1 ? "1" : "-1", tmp_quotient) == ERROR ||
                Add(lhs_abs, rhs, tmp_remainder) == ERROR) {
                release_maintainer(maintainer);
                return ERROR;
            }
        }
    } else {
       printf("lhs >= rhs\n");
        printf("getting absolute\n");
        if (absolute_values_division(lhs_abs, rhs_abs, tmp_quotient, tmp_remainder) == ERROR) {
            release_maintainer(maintainer);
            return ERROR;
        }

         printf("got absolute\n");
        if (lhs->sign_ == -1 && rhs->sign_ == -1) {
            //  printf("--\n");
            BigNum unit = CreateNum();
            tmp_remainder->sign_ = -1;
            if (unit == NULL || SetFromStr("1", unit) == ERROR || Add(tmp_quotient, unit, tmp_quotient) == ERROR ||
                Add(tmp_remainder, rhs, tmp_remainder) == ERROR) {
                FreeNum(unit);
                release_maintainer(maintainer);
                return ERROR;
            }
            FreeNum(unit);
            printf("-- fine \n");
        }
        if (lhs->sign_ != rhs->sign_) {
             printf("-+ or +- \n");
            tmp_quotient->sign_ = -1;
        }

    }
    SwapNums(tmp_quotient, quotient);
    SwapNums(tmp_remainder, remainder);
    printf("swapped nums\n");
    printf("releasing\n");
    FreeNum(tmp_quotient);
    FreeNum(tmp_remainder);
    FreeNum(lhs_abs);
    FreeNum(rhs_abs);
    return SUCCESS;
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

int8_t CopyNum(BigNum to, BigNum from) {
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
//
//BigNum NumFromInt(int num) {
//    char int_str[20];
//    sprintf(int_str, "%d", num);
//    return NumFromStr(int_str);
//}
//
