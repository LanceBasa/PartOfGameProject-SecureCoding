//////////////////////////
// DO NOT SUBMIT THIS FILE
//////////////////////////

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <p_and_p.h>

#include "test_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// useful if you want to coredump
// yourself -- see `man raise`
//#include <signal.h>

int open_with_fileno(const char * infile_path) {
  FILE *ifp = fopen(infile_path, "rb");
  if (ifp == NULL)
     die_perror(__FILE__, __LINE__, "couldn't open file")

  int fd = fileno(ifp);

  if (fd == -1)
    die_perror(__FILE__, __LINE__, "couldn't get fd for file");

  return fd;
}


#suite p_and_p_tests


#tcase arithmetic_testcase
// ^ this isn't a "real" test-case -- it's just used as an illustration
// of how test-cases are written.

#test arithmetic_works
    int m = 3;
    int n = 4;
    int expected = 7;
    int actual = m + n;
    ck_assert_int_eq(actual, expected);

//////////////////////////////////////////////////
// Uncomment the following to see AddressSanitizer
// in action:
//////////////////////////////////////////////////

//#tcase deliberately_bad_testcase
//
//#test go_out_of_bounds
//    char buf[]            = "supercallifragilistic";
//    const size_t buf_size = sizeof(buf)/sizeof(buf[0]);
//    buf[buf_size-1]       = 'X';
//    // ^ buf is now not nul-terminated
//    size_t len            = strlen(buf);
//    // we expect the strlen-calculated size
//    // to be greater than 0.
//    ck_assert_uint_gt(len, 0);

//#tcase loadItemDetails_items001_works_testcase
//
//    struct ItemDetails items001_expectedItems[] = {
//      { .itemID = 16602759796824695000UL, .name = "telescope",      .desc = "brass with wooden tripod, 25x30x60 in." },
//      { .itemID = 13744653742375254000UL, .name = "rope",           .desc = "hemp, 50 ft." },
//      { .itemID = 3400701144194139000UL,  .name = "music-box",      .desc = "brass gears with tin-plated case, 6 in., cursed" },
//      { .itemID = 734628920708863200UL,   .name = "billiard-ball",  .desc = "ivory, 2 in., set of 16 in wooden case, of mysterious origin" },
//      { .itemID = 14734865628490349000UL, .name = "sword-cane",     .desc = "steel-bladed, concealed in Malacca-wood walking-cane, 36 in." },
//      { .itemID = 14324391740292973000UL, .name = "dynamite-stick", .desc = "with paper wrapping, 1 in. diameter x 12 in." },
//      { .itemID = 7562791295545618000UL,  .name = "Epsom-salts",    .desc = "6 oz, in glass bottle with cork stopper" },
//      { .itemID = 13658877949582529000UL, .name = "camp-stool",     .desc = "canvas and wood, 12 in. seat height" },
//      { .itemID = 2390949174291477500UL,  .name = "slide-rule",     .desc = "wood and brass, 12 in., cursed" },
//    };
//
//    size_t items001_expectedSize = sizeof(items001_expectedItems)/sizeof(struct ItemDetails);
//
//
//#test loadItemDetails_items001_size_is_correct
//
//    const char * infile_path = "test-data/items001.dat";
//    int fd = open_with_fileno(infile_path);
//
//    size_t numItems = 0;
//    struct ItemDetails * itemsArr = NULL;
//    int res = loadItemDetails(&itemsArr, &numItems, fd);
//
//    if (res != 0)
//      die_perror(__FILE__, __LINE__, "loadItems failed");
//
//    // check we got the expected number of items
//    ck_assert_msg(numItems == items001_expectedSize, "numItems should equal items001_expectedSize");
//
//    free(itemsArr);
//
//static_assert( sizeof(items001_expectedItems)/sizeof(struct ItemDetails) == 9 ,
//               "expect correct number of file 001 items" );
//
//// test each item is correct, from idx 0 through to 8
//// loop index is available as `_i`
//
//#test-loop (0,9) loadItemDetails_items001_items_are_equal
//
//    const char * infile_path = "test-data/items001.dat";
//    int fd = open_with_fileno(infile_path);
//
//    size_t numItems = 0;
//    struct ItemDetails * itemsArr = NULL;
//    int res = loadItemDetails(&itemsArr, &numItems, fd);
//
//    if (res != 0)
//      die_perror(__FILE__, __LINE__, "loadItems failed");
//
//    // pre-requisite: we got the expected number of items
//    assert(numItems == items001_expectedSize);
//
//    fprintf(stderr, "checking equality of item %d, with expected itemID %zu\n",
//            _i, items001_expectedItems[_i].itemID
//    );
//    assert_itemDetails_are_equal(itemsArr + _i, &(items001_expectedItems[_i]));
//
//    free(itemsArr);
//
//// ///////////////////////////////////////////////////////
//// // Uncomment the following if you have a isValidName()
//// // implementation to test:
//// ///////////////////////////////////////////////////////
////
//#tcase isValidName_testcase
//
//// datatypes and data used for this testcase
//
//struct NameAndValidity {
//  char name[DEFAULT_BUFFER_SIZE];
//  int validity;
//};
//
//struct NameAndValidity isValidName_test_data[] = {
//  { .name = "a b", .validity = 0 },
//  { .name = "a-b", .validity = 1 },
//  { .name = "a'b", .validity = 1 }
//};
//
//#test isValidName_simple_case
//    const char *name = "bob";
//    int expected_result = 1;
//    int actual_result = isValidName(name);
//    ck_assert_int_eq(actual_result, expected_result);
//
//#test-loop (0,3) isValidName_more_cases
//
//    struct NameAndValidity nv = isValidName_test_data[_i];
//    const char *name = nv.name;
//    int expected_result = nv.validity;
//    int actual_result = isValidName(name);
//    ck_assert_int_eq(actual_result, expected_result);
//
/////////////////////////////////////////////////////////
//// Uncomment the following if you have a saveItemDetails()
//// implementation to test:
/////////////////////////////////////////////////////////
//
//#tcase saveItemDetails_testcase
//
//#test saveItemDetails_works
//
//    struct ItemDetails itemArr[] = {
//      { .itemID = 16602759796824695000UL, .name = "telescope",      .desc = "brass with wooden tripod, 25x30x60 in." }
//    };
//    size_t itemArr_size = sizeof(itemArr)/sizeof(struct ItemDetails);
//
//    char* file_conts = NULL;
//    size_t file_size = 0;
//
//    FILE *ofp = fopen("tmp.dat", "wb");
//    assert(ofp != NULL);
//    int fd = fileno(ofp);
//    assert(fd != -1);
//    int res = saveItemDetails(itemArr, itemArr_size, fd);
//    assert(res == 0);
//    fclose(ofp);
//
//    res = slurp_file("tmp.dat", "rb", &file_conts, &file_size);
//    assert(res == 0);
//
//    const size_t expected_size = sizeof(uint64_t) + sizeof(struct ItemDetails);
//
//    fprintf(stderr, "%s:%d: actual file_size = %zu\n",
//            __FILE__, __LINE__, file_size);
//
//    ck_assert_msg(file_size == expected_size, "size of written file should eq expected size");
//
//    // metadata should be `1`
//    size_t actual_read_metadata = 0;
//    memcpy(&actual_read_metadata, file_conts, sizeof(size_t));
//    ck_assert_msg(actual_read_metadata == itemArr_size, "size of written metadata should be as expected");
//
//    // following the metadata should be our struct
//    struct ItemDetails actual_read_item = { 0 };
//    memcpy(&actual_read_item, file_conts + sizeof(size_t), sizeof(struct ItemDetails));
//    ck_assert_msg(actual_read_metadata == itemArr_size, "size of written metadata should be as expected");
//
//    assert_itemDetails_are_equal(&actual_read_item, &(itemArr[0]));
//
//    if (file_conts != NULL)
//      free(file_conts);


// vim: syntax=c :
