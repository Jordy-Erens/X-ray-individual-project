#include "doseAdmin.h"
#include <string.h>  // For strlen, strcmp, strncpy
#include <stdbool.h> // For bool type

// --- Sprint 1 Requirements ---
#define MAX_DOSES_PER_PATIENT 10
#define MAX_PATIENTS_PER_ENTRY 100
// -----------------------------

// --- Struct Definitions ---

// Struct for a single dose measurement
typedef struct {
	uint16_t dose;
	Date date;
} DoseData;

// Struct for a single patient
typedef struct {
    bool isActive; // Needed because we use a static array
	char patientName[MAX_PATIENTNAME_SIZE];
    DoseData doses[MAX_DOSES_PER_PATIENT];
    size_t doseCount; // Tracks how many of the 10 spots are used
} Patient;

// Struct for a Hash Entry
typedef struct {
    Patient patients[MAX_PATIENTS_PER_ENTRY];
    size_t patientCount; // Tracks how many of the 100 spots are used
} HashEntry;


// --- The actual Hash Table ---
// This is the "array of size 256"
static HashEntry hashTable[HASHTABLE_SIZE];


/**
 * @brief Calculates the hash index for a patient name.
 * @details Based on the PDF hint: sums the ASCII value of the
 * first 5 chars. Uses 255 for missing chars.
 * Then takes the modulo 256.
 * @param patientName The name of the patient.
 * @return The calculated hash index (0-255).
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
	return (uint8_t)(sum % HASHTABLE_SIZE); // HASHTABLE_SIZE is 256
}


void CreateHashTable(void)
{
    // Because we use a static array, "Create" simply means
    // "clearing" the existing data.
	RemoveAllDataFromHashTable();
}
 
void RemoveAllDataFromHashTable(void)
{
	// Loop through the entire table and set everything to "inactive"
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        hashTable[i].patientCount = 0;
        for (int j = 0; j < MAX_PATIENTS_PER_ENTRY; j++) {
            hashTable[i].patients[j].isActive = false;
            hashTable[i].patients[j].doseCount = 0;
        }
    }
}
 
int8_t AddPatient(char patientName[MAX_PATIENTNAME_SIZE])
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -3; // Name too long
    }

    if (IsPatientPresent(patientName) == 0) {
        return -1; // Patient already present
    }

    uint8_t hash = hashFunction(patientName);
    HashEntry* entry = &hashTable[hash];

    if (entry->patientCount >= MAX_PATIENTS_PER_ENTRY) {
        // "Allocation of memory failed"
        // For Sprint 1, this means the array of 100 is full.
        return -2;
    }

    // Find the first empty (inactive) slot in the array
    for (int i = 0; i < MAX_PATIENTS_PER_ENTRY; i++) {
        if (!entry->patients[i].isActive) {
            entry->patients[i].isActive = true;
            strncpy(entry->patients[i].patientName, patientName, MAX_PATIENTNAME_SIZE);
            entry->patients[i].doseCount = 0;
            entry->patientCount++;
            return 0; // Success
        }
    }

    // This shouldn't happen if patientCount is correct, but as a fallback:
    return -2; // No empty slot found
}
 
int8_t RemovePatient(char patientName[MAX_PATIENTNAME_SIZE])
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -2; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    HashEntry* entry = &hashTable[hash];

    for (int i = 0; i < MAX_PATIENTS_PER_ENTRY; i++) {
        if (entry->patients[i].isActive &&
            strcmp(entry->patients[i].patientName, patientName) == 0) {
            
            entry->patients[i].isActive = false;
            entry->patientCount--;
            return 0; // Success
        }
    }

	return -1; // Patient not present
}
 
int8_t IsPatientPresent(char patientName[MAX_PATIENTNAME_SIZE])
{
	if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -2; // Name too long
    }

    uint8_t hash = hashFunction(patientName);
    HashEntry* entry = &hashTable[hash];

    // Loop through the 100 slots in this hash entry
    for (int i = 0; i < MAX_PATIENTS_PER_ENTRY; i++) {
        if (entry->patients[i].isActive &&
            strcmp(entry->patients[i].patientName, patientName) == 0) {
            
            return 0; // Patient is present
        }
    }

	return -1; // Patient not present
}


// --- Functions for later Sprints (Stubs) ---

int8_t AddPatientDose(char patientName[MAX_PATIENTNAME_SIZE], 
			          Date* date, uint16_t dose)
{
	 (void)patientName; // Prevent compiler warnings
     (void)date;
     (void)dose;
	 return -1;
}
 
int8_t PatientDoseInPeriod(char patientName[MAX_PATIENTNAME_SIZE], 
                           Date* startDate, Date* endDate, uint32_t* totalDose)
{
	 (void)patientName; // Prevent compiler warnings
     (void)startDate;
     (void)endDate;
     (void)totalDose;
	 return -1;
}

int8_t GetNumberOfMeasurements(char patientName[MAX_PATIENTNAME_SIZE], 
                               size_t * nrOfMeasurements)
{
	 (void)patientName; // Prevent compiler warnings
     (void)nrOfMeasurements;
	 return -1;
}

void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation)
{
	 (void)totalNumberOfPatients; // Prevent compiler warnings
     (void)averageNumberOfPatients;
     (void)standardDeviation;
	 return;
}
				
int8_t WriteToFile(char filePath[MAX_FILEPATH_LEGTH])
{
     (void)filePath; // Prevent compiler warnings
	 return -1;
}

int8_t ReadFromFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 (void)filePath; // Prevent compiler warnings
	 return -1;
}
