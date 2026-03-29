#include "project/tests/file_repository_test.c"
#include "project/tests/record_repository_test.c"
#include "project/tests/header_repository_test.c"
#include "project/tests/data_base_repository_test.c"

int main() {
   file_repository_test();
   record_repository_test();
   header_repository_test();
   test_database_repository();
   return 0;
}