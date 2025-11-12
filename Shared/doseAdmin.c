#include "doseAdmin.h"
#include <string.h>  // For strlen, strcmp, strncpy
#include <stdbool.h> // For bool type
#include <stdlib.h>  // For malloc, free
#include <math.h>    // For GetHashPerformance (sqrt)

#define MAX_DOSES_PER_PATIENT 10 // Sprint 2: Still a fixed array of 10

// Represents a single dose measurement
typedef struct {
	uint16_t dose;
	Date date;
} DoseData;

// Represents a patient (dynamically allocated)
typedef struct {
	char patientName[MAX_PATIENTNAME_SIZE];
    DoseData doses[MAX_DOSES_PER_PATIENT];
    size_t doseCount; // Tracks used dose spots
} Patient;


// --- The Hash Table ---
// Sprint 2: An array of POINTERS to patients.
// Max 1 patient per entry (NULL if empty).
static Patient* hashTable[HASHTABLE_SIZE];


/**
 * @brief Calculates the hash index (0-255) for a patient name.
 * @details Sums first 5 chars (uses 255 for missing chars) and takes modulo 256.
 */
static uint8_t hashFunction(char patientName[MAX_PATIENTNAME_SIZE]){
	unsigned long sum = 0;
    size_t len = strlen(patientName);

    for (int i = 0; i < 5; i++) {
        if (i < len) {
            sum += (unsigned char)patientName[i];
        } else {
            sum += 255; // Value for missing characters
        }
    }
	return (uint8_t)(sum % HASHTABLE_SIZE);
}


void CreateHashTable(void)
{
    // Initialize all pointers in the hash table to NULL.
	for (int i = 0; i < HASHTABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
}
 
void RemoveAllDataFromHashTable(void)
{
	// Loop through the table and free any allocated patient data
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        if (hashTable[i] != NULL) {
            free(hashTable[i]);
            hashTable[i] = NULL;
        }
    }
}
 
int8_t AddPatient(char patientName[MAX_PATIENTNAME_SIZE])
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -3; // Name too long
    }

    uint8_t hash = hashFunction(patientName);

    if (hashTable[hash] != NULL) {
        // Slot is full. Check if it's the *same* patient.
        if (strcmp(hashTable[hash]->patientName, patientName) == 0) {
            return -1; // Patient already present
        }
        // Different patient, hash collision. Slot is full for Sprint 2.
        return -2; // Allocation failed
    }

    // Allocate memory for the new patient
    Patient* newPatient = (Patient*)malloc(sizeof(Patient));
    if (newPatient == NULL) {
        return -2; // Allocation of memory failed
    }

    // Initialize the new patient
    strncpy(newPatient->patientName, patientName, MAX_PATIENTNAME_SIZE);
    newPatient->doseCount = 0;
    
    hashTable[hash] = newPatient;
    
    return 0; // Success
}
 
int8_t RemovePatient(char patientName[MAX_PATIENTNAME_SIZE])
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -2; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    Patient* patient = hashTable[hash];

    if (patient != NULL && strcmp(patient->patientName, patientName) == 0) {
        free(patient); // Free the dynamically allocated memory
        hashTable[hash] = NULL;
        return 0; // Success
    }

	return -1; // Patient not present
}
 
int8_t IsPatientPresent(char patientName[MAX_PATIENTNAME_SIZE])
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -2; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    Patient* patient = hashTable[hash];

    if (patient != NULL && strcmp(patient->patientName, patientName) == 0) {
        return 0; // Patient is present
    }

	return -1; // Patient not present
}

/**
 * @brief Helper function to check if a date is within a given period.
 */
static bool isDateInRange(Date* date, Date* startDate, Date* endDate)
{
    // Convert dates to YYYYMMDD for simple integer comparison
    uint32_t dateVal = date->year * 10000 + date->month * 100 + date->day;
    uint32_t startVal = startDate->year * 10000 + startDate->month * 100 + startDate->day;
    uint32_t endVal = endDate->year * 10000 + endDate->month * 100 + endDate->day;

    return (dateVal >= startVal) && (dateVal <= endVal);
}

int8_t AddPatientDose(char patientName[MAX_PATIENTNAME_SIZE], 
			          Date* date, uint16_t dose)
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -3; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    Patient* patient = hashTable[hash];

    if (patient == NULL || strcmp(patient->patientName, patientName) != 0) {
        return -1; // Patient unknown
    }

    if (patient->doseCount >= MAX_DOSES_PER_PATIENT) {
        return -2; // Dose array is full
    }

    // Add the dose
    patient->doses[patient->doseCount].date = *date;
    patient->doses[patient->doseCount].dose = dose;
    patient->doseCount++;

	return 0; // Success
}
 
int8_t PatientDoseInPeriod(char patientName[MAX_PATIENTNAME_SIZE], 
                           Date* startDate, Date* endDate, uint32_t* totalDose)
{
    *totalDose = 0; // Initialize output parameter

	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -2; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    Patient* patient = hashTable[hash];

    if (patient == NULL || strcmp(patient->patientName, patientName) != 0) {
        return -1; // Patient unknown
    }

    // Iterate through the patient's doses
    for (size_t i = 0; i < patient->doseCount; i++) {
        if (isDateInRange(&patient->doses[i].date, startDate, endDate)) {
            *totalDose += patient->doses[i].dose;
        }
    }
	 
	return 0; // Success
}

int8_t GetNumberOfMeasurements(char patientName[MAX_PATIENTNAME_SIZE], 
                               size_t * nrOfMeasurements)
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -2; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    Patient* patient = hashTable[hash];

    if (patient == NULL || strcmp(patient->patientName, patientName) != 0) {
        return -1; // Patient not present
    }

    *nrOfMeasurements = patient->doseCount;
	return 0; // Success
}

void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation)
{
    size_t totalPatients = 0;
    double sumOfSquares = 0.0; // Sum of (entries_in_slot)^2

    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        if (hashTable[i] != NULL) {
            totalPatients++;
            sumOfSquares += 1.0; // (1 patient)^2 is 1
        }
    }

    *totalNumberOfPatients = totalPatients;
    *averageNumberOfPatients = (double)totalPatients / HASHTABLE_SIZE;

    // Calculate variance and standard deviation
    double meanOfSquares = sumOfSquares / HASHTABLE_SIZE;
    double variance = meanOfSquares - (*averageNumberOfPatients * *averageNumberOfPatients);
    *standardDeviation = sqrt(variance);
}
				
int8_t WriteToFile(char filePath[MAX_FILEPATH_LEGTH])
{
     (void)filePath; // Not implemented in Sprint 2
	 return -1;
}

int8_t ReadFromFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 (void)filePath; // Not implemented in Sprint 2
	 return -1;
}
