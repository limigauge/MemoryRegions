/**
 * main.c
 *
 * COMP 2160 SECTION A01
 * INSTRUCTOR    NIKNAM
 * ASSIGNMENT    Assignment 4, question 1
 * AUTHOR        Michelle Li, 7866927
 * DATE          2021-12-12
 *
 * PURPOSE: Main program for testing memory regions implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "regions.h"

// this code should run to completion with the output shown
// you must think of additional cases of correct use and misuse for your testing

static int number_of_tests = 0;
static int failed_tests = 0;

void test_rinit(char *name, rsize_t size, Boolean expected){
    if(rinit(name, size) == expected){
        printf("rinit test succeeded. ");

        if(expected == TRUE){
            printf("Added %s to Regions.\n", name);
        } else {
            printf("%s was not added to Regions.\n", name);
        }
    } else {
        printf("rinit() test failed.\n");
        failed_tests++;
    }
    number_of_tests++;
}

void test_ralloc( rsize_t size, Boolean expected){
    if(TRUE == expected){
        if(ralloc(size) != 0){
            printf("Allocation of size %d succeeded. ", size);
        } else {
          printf("ralloc() test failed.\n");
          failed_tests++;
        }
    } else {
        if(ralloc(size) != 0){
            printf("Memory allocation for size %d was successful, but should not have been.\n", size);
            failed_tests++;
        } else {
            printf("ralloc() failed.\n");
        }
    }
    number_of_tests++;
}

void test_rsize(void *ptr, rsize_t expected){
    if(rsize(ptr) == expected){
        printf("rsize succeeded. Returned size: %d\n", expected);
    } else {
        printf("rsize failed. Returned size %d\n", rsize(ptr));
        failed_tests++;
    }
    number_of_tests++;
}

void test_rchoose(char *name, Boolean expected){
    rchoose(name);

    if(strcmp(rchosen(), name) == 0 && expected == TRUE){
      printf("rchoose succeeded. Current region is: %s\n", rchosen());
    } else if (expected == FALSE && strcmp(rchosen(), name) != 0) {
      printf("rchoose suceeded. Did not choose the region.\n");
    } else {
      printf("rchoose failed.\n");
      failed_tests++;
    }
    number_of_tests++;
}

void test_rchosen(char *expected){
    if(strcmp(rchosen(), expected) == 0){
      printf("rchosen test succeeded. Current region is %s\n", expected);
    } else {
      printf("rchosen test failed.\n");
      failed_tests++;
    }
    number_of_tests++;
}


int main()
{
    printf("Processing...\n");

    char *p1, *p2, *p3;

    test_rinit("first", 600, TRUE);
    test_rinit("second", 999, TRUE);
    test_rinit("third", 100, TRUE);
    test_rinit("fourth", 888, TRUE);

    //tests that should fail for rinit
    test_rinit("first", 599, FALSE); //repeat name
    test_rinit("fifth", 0, FALSE); //0 memory
    test_rinit("", 230, FALSE); //no name

    test_rchosen("fourth"); //make sure last created region is set as chosen
    test_rchoose("does not exist!", FALSE);

    test_rchoose("second", TRUE);
    test_ralloc(700, TRUE); //rounds up
    test_ralloc(300, FALSE); //rounds up too, shoudn't fit
    
    p1 = ralloc(296); //perfect fit
    assert(p1 != NULL);
    test_rsize(p1, 296);
    rfree(p1);
    test_ralloc(290, TRUE); //should round up to fit again as 296

    test_rchoose("third", TRUE);
    p2 = ralloc(64);
    p3 = ralloc(20); 
    test_rsize(p3, 24); //rounded up
    rfree(p2);
    test_ralloc(70, FALSE); //can't fit in front or back of p3
    p2 = ralloc(50); //insert at front of memory
    assert(p2 != NULL);
    test_ralloc(12, TRUE); //perfect fit at end of memory buffer
    assert(p2+64 == p3);

    rdump();

    printf("Current region is: %s\n", rchosen());
    rdestroy("third");
    printf("Current region is: %s\n", rchosen()); //should be previous region: second

    rdestroy("fourth");
    rdestroy("second");
    rdestroy("region that doesn't exist"); //shouldn't crash or anything

    test_rchosen("first");
    test_rinit("second", 1, TRUE); //can be recreated because was destroyed
    test_rinit("final", 1, TRUE);

    rdump();

    rchoose("first");
    rdestroy("first");

    test_rchosen("second"); //if list top is destroyed, it goes to the next top rather than the previous region
    rdestroy("second");
    rdestroy("final");
    rdump(); //nothing

    printf("\nPrinting test results...\n");
  	printf("Number of tests completed: %d\n", number_of_tests);
  	printf("Number of tests failed: %d\n", failed_tests);

  fprintf(stderr,"\nEnd of processing.\n");

  return EXIT_SUCCESS;
}