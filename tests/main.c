#include <number.h>
#include "malloc.h"
#include "minunit.h"
#include <string.h>

void test_conversion(char const *given, char const *expected, char sign) {
    BigNum num = CreateNum();
    int code = SetFromStr(given, num);
    mu_assert(code == SUCCESS, code);
    mu_check(num->sign_ == sign);
    char *str2 = ToStr(num);
    mu_assert(strcmp(str2, expected) == 0, str2);
    free(str2);
    FreeNum(num);
}

MU_TEST(string_conversion_test) {
    test_conversion("12345", "12345", 1);
    test_conversion("+12345", "12345", 1);
    test_conversion("+000012345", "12345", 1);
    test_conversion("-12345", "-12345", -1);
    test_conversion("-000012345", "-12345", -1);
    test_conversion("0", "0", 1);
    test_conversion("+0", "0", 1);
    test_conversion("-0", "0", 1);
}

void test_sc_error_handling(char const *given) {
    BigNum num = CreateNum();
    int code = SetFromStr(given, num);
    mu_assert(code == ERROR, code);
    FreeNum(num);
}

MU_TEST(string_conversion_error_handling) {
    test_sc_error_handling("asasfaf");
    test_sc_error_handling("0basdasd");
    test_sc_error_handling("+asasfaf");
    test_sc_error_handling("-asasfaf");
    test_sc_error_handling("a1234");
    test_sc_error_handling("+1655a");
    test_sc_error_handling("+00sasdfaf");
    test_sc_error_handling("asasfaf");
    test_sc_error_handling("00000a");
    test_sc_error_handling("123oa1314");
    test_sc_error_handling("");
    test_sc_error_handling("a");
    test_sc_error_handling("-");
    test_sc_error_handling("+");
}

void test_addition(char const *s_lhs, char const *s_rhs, char const *s_res) {
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    mu_check(SetFromStr(s_lhs, lhs) == SUCCESS);
    mu_check(SetFromStr(s_rhs, rhs) == SUCCESS);
    BigNum res = CreateNum();
    mu_check(Add(lhs, rhs, res) == SUCCESS);
    char *str = ToStr(res);
    int check = strcmp(str, s_res);
    mu_assert(check == 0, check);
    FreeNum(lhs);
    FreeNum(rhs);
    FreeNum(res);
    free(str);
}

MU_TEST(addition) {
    test_addition("1", "23", "24");
    test_addition("-1", "-23", "-24");
    test_addition("1", "-23", "-22");
    test_addition("-1", "23", "22");
    test_addition("1", "-23", "-22");
    test_addition("-1", "23", "22");
    test_addition("1232421424", "142412124", "1374833548");
    test_addition("-124612421", "-54135145345", "-54259757766");
    test_addition("543543435435543533143514212532354514544154", "435435141321412313213541231325143",
                  "543543435870978674464926525745895745869297");
    test_addition("-64646854354354548797896445615212311383513122225465454531534",
                  "-544154351321312532132131251343514351351354135",
                  "-64646854354355092952247766927744443514764465739816805885669");
    test_addition("5151514544", "51515351161564", "51520502676108");

    test_addition("-11111", "231", "-10880");
    test_addition("1435135135135", "-1435135135135", "0");
    test_addition("-5151514544", "51515351161564", "51510199647020");
    test_addition("64646854354354548797896445615212311383513122225465454531534",
                  "-544154351321312532132131251343514351351354135",
                  "64646854354354004643545124302680179252261778711114103177399");
}

void test_subtraction(char const *s_lhs, char const *s_rhs, char const *s_res) {
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    mu_check(SetFromStr(s_lhs, lhs) == SUCCESS);
    mu_check(SetFromStr(s_rhs, rhs) == SUCCESS);
    BigNum res = CreateNum();
    mu_check(Sub(lhs, rhs, res) == SUCCESS);
    char *str = ToStr(res);
    int check = strcmp(str, s_res);
    mu_assert(check == 0, check);
    FreeNum(lhs);
    FreeNum(rhs);
    FreeNum(res);
    free(str);
}

MU_TEST(subtraction) {
    test_subtraction("12412442", "12412442", "0");
    test_subtraction("7989797997", "4546854", "7985251143");
    test_subtraction("-554164", "-85454", "-468710");
    test_subtraction("5465146546514165468468486486561351351547564684863135114564151",
                     "-54646468546854646546513515153143544464546548484343513186498878",
                     "60111615093368812014982001639704895816094113169206648301063029");
}

void test_arguments_lhs(char const *s_lhs, char const *s_rhs, char const *s_res) {
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    mu_check(SetFromStr(s_lhs, lhs) == SUCCESS);
    mu_check(SetFromStr(s_rhs, rhs) == SUCCESS);
    mu_check(Sub(lhs, rhs, rhs) == SUCCESS);
    char *str = ToStr(rhs);
    int check = strcmp(str, s_res);
    mu_assert(check == 0, check);
    FreeNum(lhs);
    FreeNum(rhs);
    free(str);
}

void test_arguments_rhs(char const *s_lhs, char const *s_rhs, char const *s_res) {
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    mu_check(SetFromStr(s_lhs, lhs) == SUCCESS);
    mu_check(SetFromStr(s_rhs, rhs) == SUCCESS);
    mu_check(Sub(lhs, rhs, lhs) == SUCCESS);
    char *str = ToStr(lhs);
    int check = strcmp(str, s_res);
    mu_assert(check == 0, check);
    FreeNum(lhs);
    FreeNum(rhs);
    free(str);
}


MU_TEST(rep_arguments) {
    test_arguments_lhs("1", "23", "-22");

    test_arguments_lhs("12412442", "12412442", "0");
    test_arguments_lhs("7989797997", "4546854", "7985251143");
    test_arguments_lhs("-554164", "-85454", "-468710");
    test_arguments_lhs("5465146546514165468468486486561351351547564684863135114564151",
                     "-54646468546854646546513515153143544464546548484343513186498878",
                     "60111615093368812014982001639704895816094113169206648301063029");

    test_arguments_rhs("12412442", "12412442", "0");
    test_arguments_rhs("7989797997", "4546854", "7985251143");
    test_arguments_rhs("-554164", "-85454", "-468710");
    test_arguments_rhs("5465146546514165468468486486561351351547564684863135114564151",
                       "-54646468546854646546513515153143544464546548484343513186498878",
                       "60111615093368812014982001639704895816094113169206648301063029");
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(string_conversion_test);
    MU_RUN_TEST(string_conversion_error_handling);
    MU_RUN_TEST(addition);
    MU_RUN_TEST(subtraction);
    MU_RUN_TEST(rep_arguments);
}

int main() {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
