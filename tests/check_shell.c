#include <check.h>
#include <stdlib.h>

START_TEST(test_shell_starts)
{
    ck_assert_int_eq(0,0);    
}
END_TEST

START_TEST(test_shell_fails)
{
    ck_assert_int_eq(1,0);
}
END_TEST


static Suite *shell_suite(void)
{
    Suite *s;
    TCase *tc_core;
    TCase *tc_fails;

    s = suite_create("Shell");

    tc_core = tcase_create("Core");
    tc_fails = tcase_create("Fail");

    tcase_add_test(tc_fails, test_shell_fails);
    tcase_add_test(tc_core, test_shell_starts);

    suite_add_tcase(s, tc_fails);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = shell_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}