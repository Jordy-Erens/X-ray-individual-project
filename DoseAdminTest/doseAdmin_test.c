#include <string.h>
#include "doseAdmin.h"
#include "unity.h"
#include <stdlib.h>

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)

// --- Test Data ---
static char name1[] = "Alice";
static char name2[] = "Bob";
static char nameTooLong[MAX_PATIENTNAME_SIZE + 5]; // Buffer that is definitely too long

// --- setUp & tearDown ---

void setUp(void)
{
    // This is run before EACH test
    // We create a fresh, empty table for every test.
    CreateHashTable();
}

void tearDown(void)
{
    // This is run after EACH test
    // This cleans up all malloc'd memory from the test.
    RemoveAllDataFromHashTable();
}

// --- Test Cases ---

void test_AddPatient_And_IsPresent_Success(void)
{
    // Patient should not exist yet
    TEST_ASSERT_EQUAL_INT(-1, IsPatientPresent(name1));
    
    // Add patient
    TEST_ASSERT_EQUAL_INT(0, AddPatient(name1));

    // Patient should now exist
    TEST_ASSERT_EQUAL_INT(0, IsPatientPresent(name1));
}

void test_AddPatient_Error_Duplicate(void)
{
    TEST_ASSERT_EQUAL_INT(0, AddPatient(name1));
    // Try to add the same patient again
    TEST_ASSERT_EQUAL_INT(-1, AddPatient(name1));
}

void test_AddPatient_Error_NameTooLong(void)
{
    // Fill the name buffer with 'A's to make it too long
    memset(nameTooLong, 'A', MAX_PATIENTNAME_SIZE + 4);
    nameTooLong[MAX_PATIENTNAME_SIZE + 4] = '\0';
    
    TEST_ASSERT_EQUAL_INT(-3, AddPatient(nameTooLong));
}

void test_RemovePatient_Success(void)
{
    AddPatient(name1);
    TEST_ASSERT_EQUAL_INT(0, IsPatientPresent(name1));

    // Remove patient
    TEST_ASSERT_EQUAL_INT(0, RemovePatient(name1));
    
    // Patient should be gone
    TEST_ASSERT_EQUAL_INT(-1, IsPatientPresent(name1));
}

void test_RemovePatient_Error_NotFound(void)
{
    AddPatient(name1);
    // Try to remove a patient that doesn't exist
    TEST_ASSERT_EQUAL_INT(-1, RemovePatient(name2));
}

void test_AddDose_And_GetMeasurements(void)
{
    AddPatient(name1);
    Date date = {1, 1, 2025};
    size_t measurements = 0;

    // Should have 0 measurements initially
    TEST_ASSERT_EQUAL_INT(0, GetNumberOfMeasurements(name1, &measurements));
    TEST_ASSERT_EQUAL_INT(0, measurements);

    // Add a dose
    TEST_ASSERT_EQUAL_INT(0, AddPatientDose(name1, &date, 100));

    // Should have 1 measurement now
    TEST_ASSERT_EQUAL_INT(0, GetNumberOfMeasurements(name1, &measurements));
    TEST_ASSERT_EQUAL_INT(1, measurements);
}

void test_AddDose_Error_PatientNotFound(void)
{
    Date date = {1, 1, 2025};
    // Try to add dose to a patient that doesn't exist
    TEST_ASSERT_EQUAL_INT(-1, AddPatientDose(name1, &date, 100));
}

void test_AddDose_Error_DoseArrayFull(void)
{
    AddPatient(name1);
    Date date = {1, 1, 2025};

    // Add 10 doses (which is MAX_DOSES_PER_PATIENT)
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(0, AddPatientDose(name1, &date, 50));
    }

    // Check count
    size_t measurements = 0;
    GetNumberOfMeasurements(name1, &measurements);
    TEST_ASSERT_EQUAL_INT(10, measurements);

    // Try to add the 11th dose
    TEST_ASSERT_EQUAL_INT(-2, AddPatientDose(name1, &date, 50));
}

void test_PatientDoseInPeriod_Calculation(void)
{
    AddPatient(name1);
    uint32_t totalDose = 0;

    // Dates
    Date d1 = {5, 1, 2025};
    Date d2 = {10, 1, 2025};
    Date d3 = {15, 1, 2025};
    
    Date start = {1, 1, 2025};
    Date end = {12, 1, 2025}; // Should include d1 and d2, but not d3

    // Add doses
    AddPatientDose(name1, &d1, 100); // In period
    AddPatientDose(name1, &d2, 200); // In period
    AddPatientDose(name1, &d3, 400); // Out of period

    // Calculate dose
    TEST_ASSERT_EQUAL_INT(0, PatientDoseInPeriod(name1, &start, &end, &totalDose));
    
    // Expected: 100 + 200 = 300
    TEST_ASSERT_EQUAL_UINT32(300, totalDose);
}

void test_PatientDoseInPeriod_NoDoses(void)
{
    AddPatient(name1);
    uint32_t totalDose = 0;
    Date start = {1, 1, 2025};
    Date end = {12, 1, 2025};

    // Calculate dose
    TEST_ASSERT_EQUAL_INT(0, PatientDoseInPeriod(name1, &start, &end, &totalDose));
    
    // Expected: 0
    TEST_ASSERT_EQUAL_UINT32(0, totalDose);
}

void test_GetHashPerformance(void)
{
    AddPatient(name1); // hash("Alice")
    AddPatient(name2); // hash("Bob")

    size_t totalPatients = 0;
    double avg = 0.0;
    double stdDev = 0.0;

    GetHashPerformance(&totalPatients, &avg, &stdDev);

    TEST_ASSERT_EQUAL_INT(2, totalPatients);
    // 2 patients / 256 buckets = 0.0078125
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.0078125, avg);
}

// add here all your dose admin testcases, and call them in main!! Remove the given testcases

int main(void)
{
    UnityBegin();

    // Run all the new tests
    MY_RUN_TEST(test_AddPatient_And_IsPresent_Success);
    MY_RUN_TEST(test_AddPatient_Error_Duplicate);
    MY_RUN_TEST(test_AddPatient_Error_NameTooLong);
    MY_RUN_TEST(test_RemovePatient_Success);
    MY_RUN_TEST(test_RemovePatient_Error_NotFound);
    MY_RUN_TEST(test_AddDose_And_GetMeasurements);
    MY_RUN_TEST(test_AddDose_Error_PatientNotFound);
    MY_RUN_TEST(test_AddDose_Error_DoseArrayFull);
    MY_RUN_TEST(test_PatientDoseInPeriod_Calculation);
    MY_RUN_TEST(test_PatientDoseInPeriod_NoDoses);
    MY_RUN_TEST(test_GetHashPerformance);

    // You can keep these original tests if you want
    // MY_RUN_TEST(test_FailTest);
    // MY_RUN_TEST(test_LeakTest);
    // MY_RUN_TEST(test_OutOfRangeTest);

    UnityEnd();
}
