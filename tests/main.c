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

void
test_operation(char const *s_lhs, char const *s_rhs, char const *s_res, int8_t (*operation)(BigNum, BigNum, BigNum)) {
    BigNum lhs = CreateNum();
    BigNum rhs = CreateNum();
    mu_check(SetFromStr(s_lhs, lhs) == SUCCESS);
    mu_check(SetFromStr(s_rhs, rhs) == SUCCESS);
    BigNum res = CreateNum();
    mu_check(operation(lhs, rhs, res) == SUCCESS);
    char *str = ToStr(res);
    int check = strcmp(str, s_res);
    mu_assert(check == 0, check);
    FreeNum(lhs);
    FreeNum(rhs);
    FreeNum(res);
    free(str);
}

MU_TEST(addition) {
    test_operation("1", "23", "24", Add);
    test_operation("-1", "-23", "-24", Add);
    test_operation("1", "-23", "-22", Add);
    test_operation("-1", "23", "22", Add);
    test_operation("1", "-23", "-22", Add);
    test_operation("-1", "23", "22", Add);
    test_operation("1232421424", "142412124", "1374833548", Add);
    test_operation("-124612421", "-54135145345", "-54259757766", Add);
    test_operation("543543435435543533143514212532354514544154", "435435141321412313213541231325143",
                   "543543435870978674464926525745895745869297", Add);
    test_operation("-64646854354354548797896445615212311383513122225465454531534",
                   "-544154351321312532132131251343514351351354135",
                   "-64646854354355092952247766927744443514764465739816805885669", Add);
    test_operation("5151514544", "51515351161564", "51520502676108", Add);

    test_operation("-11111", "231", "-10880", Add);
    test_operation("1435135135135", "-1435135135135", "0", Add);
    test_operation("-5151514544", "51515351161564", "51510199647020", Add);
    test_operation("64646854354354548797896445615212311383513122225465454531534",
                   "-544154351321312532132131251343514351351354135",
                   "64646854354354004643545124302680179252261778711114103177399", Add);
}

MU_TEST(subtraction) {
    test_operation("12412442", "12412442", "0", Sub);
    test_operation("7989797997", "4546854", "7985251143", Sub);
    test_operation("-554164", "-85454", "-468710", Sub);
    test_operation("5465146546514165468468486486561351351547564684863135114564151",
                   "-54646468546854646546513515153143544464546548484343513186498878",
                   "60111615093368812014982001639704895816094113169206648301063029", Sub);
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

MU_TEST(multiplication) {
    test_operation("251513513513511315135215154684614351213211132135151546854654654513212123121351",
                   "656545465615313513514354354646465451231320",
                   "165130056838271733874550062816663544175130286803316585629648093679063295879808463606683623666665532526561930264731913320",
                   Mult);
    test_operation("684654564654654", "-545445465416545564", "-373441727667620137223357315654856", Mult);
    test_operation("-546498846468654684864684484646845648654658455151514614515614",
                   "-531355151531335263652151819511927497425628514952561452156195814418422562652921587419562531952362923694125639741956198145369741985194148529581256214591285222692914965",
                   "290384977377051935517151314231343767412507114888149372512081187374921275348477395342044523608749546577901986435325410912016536415118365005086885867791461200706959951942591306532467535868072502817206560543771065270383166763510",
                   Mult);

}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(string_conversion_test);
    MU_RUN_TEST(string_conversion_error_handling);
    MU_RUN_TEST(addition);
    MU_RUN_TEST(subtraction);
    MU_RUN_TEST(rep_arguments);
    MU_RUN_TEST(multiplication);
}

int main() {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
