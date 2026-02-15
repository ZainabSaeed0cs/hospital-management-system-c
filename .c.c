#include "raylib.h"

// IMPORTANT: You must have raygui.h in your project directory
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Data Structures & Constants ---
#define MAX_PATIENTS 50
#define MAX_DOCTORS 5
#define MAX_APPOINTMENTS 100

struct Patient {
    char name[30];
    int id;
    int age;
    char disease[30];
};

struct Doctor {
    char name[30];
    int available;   
};

struct Appointment {
    int patientId;
    char patientName[30]; // Storing name here for easier display
    char doctorName[30];
    char date[20];
};

// --- Global Data ---
struct Patient patients[MAX_PATIENTS];
int patientcount = 0;

struct Doctor doctors[MAX_DOCTORS] = {
    {"Dr. Zainab", 1},
    {"Dr. Isma", 0},
    {"Dr. Ali", 0},
    {"Dr. Nadia", 1},
    {"Dr. Hamza", 1}
};

struct Appointment appointments[MAX_APPOINTMENTS];
int appointmentCount = 0;

// --- Application States ---
typedef enum { 
    MENU, 
    ADD_PATIENT, 
    VIEW_PATIENTS, 
    DELETE_PATIENT, 
    DOCTORS_VIEW,
    BOOK_APPOINTMENT,   // New State
    VIEW_APPOINTMENTS   // New State
} AppState;

// --- Input Buffers (for Text Boxes) ---
char idInput[30] = "";
char nameInput[30] = "";
char ageInput[30] = "";
char diseaseInput[30] = "";
char deleteIdInput[30] = "";

// Buffers for Appointment
char bookPatientIdInput[30] = "";
char bookDateInput[30] = "";
int  selectedDoctorIndex = 0; // For the spinner (0 to 4)

// State variables for Text Box Edit Modes
bool idEditMode = false;
bool nameEditMode = false;
bool ageEditMode = false;
bool diseaseEditMode = false;
bool deleteIdEditMode = false;
bool bookIdEditMode = false;
bool bookDateEditMode = false;
bool doctorSpinnerEditMode = false;

// Scrolling
float scrollOffset = 0.0f;

// --- File Operations ---
void loadPatients() {
    FILE *fp = fopen("patients.txt", "r");
    if (fp == NULL) return;
    patientcount = 0;
    while (fscanf(fp, "%d %s %d %s",
           &patients[patientcount].id,
            patients[patientcount].name,
           &patients[patientcount].age,
            patients[patientcount].disease) != EOF) {
        patientcount++;
    }
    fclose(fp);
}

void savePatients() {
    FILE *fp = fopen("patients.txt", "w");
    if (fp == NULL) return;
    for (int i = 0; i < patientcount; i++) {
        fprintf(fp, "%d %s %d %s\n",
                patients[i].id,
                patients[i].name,
                patients[i].age,
                patients[i].disease);
    }
    fclose(fp);
}

void loadAppointments() {
    FILE *fp = fopen("appointments.txt", "r");
    if (fp == NULL) return;
    appointmentCount = 0;
    while (fscanf(fp, "%d %s %s %s",
           &appointments[appointmentCount].patientId,
            appointments[appointmentCount].patientName,
            appointments[appointmentCount].doctorName,
            appointments[appointmentCount].date) != EOF) {
        appointmentCount++;
    }
    fclose(fp);
}

void saveAppointments() {
    FILE *fp = fopen("appointments.txt", "w");
    if (fp == NULL) return;
    for (int i = 0; i < appointmentCount; i++) {
        fprintf(fp, "%d %s %s %s\n",
                appointments[i].patientId,
                appointments[i].patientName,
                appointments[i].doctorName,
                appointments[i].date);
    }
    fclose(fp);
}

// --- Logic Helpers ---
void ClearInputs() {
    strcpy(idInput, "");
    strcpy(nameInput, "");
    strcpy(ageInput, "");
    strcpy(diseaseInput, "");
    strcpy(bookPatientIdInput, "");
    strcpy(bookDateInput, "");
    idEditMode = false;
    nameEditMode = false;
    ageEditMode = false;
    diseaseEditMode = false;
    bookIdEditMode = false;
    bookDateEditMode = false;
}

// Helper to find patient name by ID
int FindPatientIndex(int id) {
    for(int i=0; i<patientcount; i++) {
        if(patients[i].id == id) return i;
    }
    return -1;
}

// --- Main Program ---
int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Hospital Management System - Raylib GUI");
    SetTargetFPS(60);

    // Load data
    loadPatients();
    loadAppointments();
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    AppState currentState = MENU;
    char statusMessage[100] = ""; 

    while (!WindowShouldClose()) {
        
        // Logic for scrolling
        if (currentState == VIEW_PATIENTS || currentState == VIEW_APPOINTMENTS) {
            scrollOffset += GetMouseWheelMove() * 20.0f;
            if (scrollOffset > 0) scrollOffset = 0;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("HOSPITAL MANAGEMENT SYSTEM", 220, 20, 24, DARKBLUE);
        DrawLine(20, 50, 780, 50, GRAY);

        switch (currentState) {
            
            // ------------------ MAIN MENU ------------------
            case MENU:
                // Left Column
                if (GuiButton((Rectangle){150, 150, 220, 60}, "Add Patient")) {
                    currentState = ADD_PATIENT;
                    ClearInputs();
                    strcpy(statusMessage, "");
                }
                if (GuiButton((Rectangle){150, 230, 220, 60}, "View Patients")) {
                    currentState = VIEW_PATIENTS;
                    scrollOffset = 0;
                }
                if (GuiButton((Rectangle){150, 310, 220, 60}, "Delete Patient")) {
                    currentState = DELETE_PATIENT;
                    strcpy(deleteIdInput, "");
                    strcpy(statusMessage, "");
                }

                // Right Column
                if (GuiButton((Rectangle){430, 150, 220, 60}, "Book Appointment")) {
                    currentState = BOOK_APPOINTMENT;
                    ClearInputs();
                    strcpy(statusMessage, "");
                }
                if (GuiButton((Rectangle){430, 230, 220, 60}, "View Appointments")) {
                    currentState = VIEW_APPOINTMENTS;
                    scrollOffset = 0;
                }
                if (GuiButton((Rectangle){430, 310, 220, 60}, "Doctor Status")) {
                    currentState = DOCTORS_VIEW;
                }

                if (GuiButton((Rectangle){300, 450, 200, 50}, "EXIT")) {
                    CloseWindow();
                    return 0;
                }
                break;

            // ------------------ ADD PATIENT ------------------
            case ADD_PATIENT:
                DrawText("Add New Patient", 50, 70, 20, DARKGRAY);

                DrawText("ID:", 200, 125, 20, BLACK);
                if (GuiTextBox((Rectangle){300, 120, 200, 30}, idInput, 30, idEditMode)) idEditMode = !idEditMode;

                DrawText("Name:", 200, 175, 20, BLACK);
                if (GuiTextBox((Rectangle){300, 170, 200, 30}, nameInput, 30, nameEditMode)) nameEditMode = !nameEditMode;

                DrawText("Age:", 200, 225, 20, BLACK);
                if (GuiTextBox((Rectangle){300, 220, 200, 30}, ageInput, 30, ageEditMode)) ageEditMode = !ageEditMode;

                DrawText("Disease:", 200, 275, 20, BLACK);
                if (GuiTextBox((Rectangle){300, 270, 200, 30}, diseaseInput, 30, diseaseEditMode)) diseaseEditMode = !diseaseEditMode;

                if (GuiButton((Rectangle){300, 350, 200, 40}, "SAVE PATIENT")) {
                    if (patientcount < MAX_PATIENTS && strlen(idInput) > 0) {
                        patients[patientcount].id = atoi(idInput);
                        strcpy(patients[patientcount].name, nameInput);
                        patients[patientcount].age = atoi(ageInput);
                        strcpy(patients[patientcount].disease, diseaseInput);
                        patientcount++;
                        savePatients();
                        strcpy(statusMessage, "Patient Saved Successfully!");
                        ClearInputs();
                    } else {
                        strcpy(statusMessage, "List Full or Invalid ID!");
                    }
                }
                DrawText(statusMessage, 300, 400, 20, GREEN);
                if (GuiButton((Rectangle){20, 530, 100, 40}, "Back")) currentState = MENU;
                break;

            // ------------------ VIEW PATIENTS ------------------
            case VIEW_PATIENTS:
                DrawText("Patient Records", 50, 70, 20, DARKGRAY);
                BeginScissorMode(0, 100, screenWidth, 400);
                int startY = 110 + (int)scrollOffset;
                if (patientcount == 0) DrawText("No records found.", 300, 200, 20, GRAY);

                for (int i = 0; i < patientcount; i++) {
                    char displayBuffer[200];
                    sprintf(displayBuffer, "ID: %-4d | Name: %-15s | Age: %-3d | Disease: %s", 
                            patients[i].id, patients[i].name, patients[i].age, patients[i].disease);
                    DrawRectangle(50, startY - 5, 700, 30, (i % 2 == 0) ? LIGHTGRAY : RAYWHITE);
                    DrawText(displayBuffer, 60, startY, 20, BLACK);
                    startY += 40;
                }
                EndScissorMode();
                if (GuiButton((Rectangle){20, 530, 100, 40}, "Back")) currentState = MENU;
                break;

            // ------------------ DELETE PATIENT ------------------
            case DELETE_PATIENT:
                DrawText("Delete Patient Record", 50, 70, 20, DARKGRAY);
                DrawText("Enter ID to Delete:", 200, 155, 20, BLACK);
                if (GuiTextBox((Rectangle){400, 150, 100, 30}, deleteIdInput, 30, deleteIdEditMode)) deleteIdEditMode = !deleteIdEditMode;

                if (GuiButton((Rectangle){300, 220, 200, 40}, "DELETE")) {
                    int targetId = atoi(deleteIdInput);
                    int found = 0;
                    for (int i = 0; i < patientcount; i++) {
                        if (patients[i].id == targetId) {
                            for (int j = i; j < patientcount - 1; j++) {
                                patients[j] = patients[j + 1];
                            }
                            patientcount--;
                            savePatients();
                            found = 1;
                            strcpy(statusMessage, "Patient Deleted Successfully!");
                            break;
                        }
                    }
                    if (!found) strcpy(statusMessage, "Patient ID Not Found!");
                }
                DrawText(statusMessage, 300, 300, 20, MAROON);
                if (GuiButton((Rectangle){20, 530, 100, 40}, "Back")) currentState = MENU;
                break;

            // ------------------ DOCTORS LIST ------------------
            case DOCTORS_VIEW:
                DrawText("Doctor Availability Management", 50, 70, 20, DARKGRAY);
                int docY = 120;
                for (int i = 0; i < MAX_DOCTORS; i++) {
                    DrawText(doctors[i].name, 200, docY + 5, 20, BLACK);
                    bool isAvailable = doctors[i].available;
                    if (GuiCheckBox((Rectangle){400, docY, 20, 20}, isAvailable ? "Available" : "Busy", &isAvailable)) {
                        doctors[i].available = isAvailable;
                    }
                    docY += 50;
                }
                if (GuiButton((Rectangle){20, 530, 100, 40}, "Back")) currentState = MENU;
                break;

            // ------------------ BOOK APPOINTMENT (NEW) ------------------
            case BOOK_APPOINTMENT:
                DrawText("Book New Appointment", 50, 70, 20, DARKGRAY);

                // 1. Enter Patient ID
                DrawText("Patient ID:", 150, 140, 20, BLACK);
                if (GuiTextBox((Rectangle){300, 135, 100, 30}, bookPatientIdInput, 30, bookIdEditMode)) bookIdEditMode = !bookIdEditMode;

                // 2. Select Doctor (Spinner)
                DrawText("Select Doctor:", 150, 200, 20, BLACK);
                if (GuiSpinner((Rectangle){300, 195, 100, 30}, NULL, &selectedDoctorIndex, 1, 5, doctorSpinnerEditMode)) doctorSpinnerEditMode = !doctorSpinnerEditMode;
                
                // Show Doctor Name and Status dynamically next to spinner
                int docIdx = selectedDoctorIndex - 1; // Array is 0-4, Spinner is 1-5
                if(docIdx >= 0 && docIdx < MAX_DOCTORS) {
                    DrawText(doctors[docIdx].name, 420, 200, 20, DARKBLUE);
                    DrawText(doctors[docIdx].available ? "(Available)" : "(Busy)", 
                             600, 200, 20, doctors[docIdx].available ? GREEN : RED);
                }

                // 3. Enter Date
                DrawText("Date (DD/MM):", 150, 260, 20, BLACK);
                if (GuiTextBox((Rectangle){300, 255, 200, 30}, bookDateInput, 30, bookDateEditMode)) bookDateEditMode = !bookDateEditMode;

                // 4. Book Button
                if (GuiButton((Rectangle){300, 340, 200, 50}, "BOOK NOW")) {
                    int pID = atoi(bookPatientIdInput);
                    int pIndex = FindPatientIndex(pID);
                    docIdx = selectedDoctorIndex - 1;

                    if (pIndex == -1) {
                        strcpy(statusMessage, "Error: Patient ID not found!");
                    } 
                    else if (!doctors[docIdx].available) {
                        strcpy(statusMessage, "Error: Selected Doctor is Busy!");
                    }
                    else if (appointmentCount >= MAX_APPOINTMENTS) {
                        strcpy(statusMessage, "Error: Appointment list full!");
                    }
                    else {
                        // Success Logic
                        appointments[appointmentCount].patientId = pID;
                        strcpy(appointments[appointmentCount].patientName, patients[pIndex].name);
                        strcpy(appointments[appointmentCount].doctorName, doctors[docIdx].name);
                        strcpy(appointments[appointmentCount].date, bookDateInput);
                        
                        appointmentCount++;
                        saveAppointments();
                        strcpy(statusMessage, "Appointment Booked Successfully!");
                    }
                }

                DrawText(statusMessage, 200, 420, 20, (statusMessage[0] == 'E') ? RED : GREEN);

                if (GuiButton((Rectangle){20, 530, 100, 40}, "Back")) currentState = MENU;
                break;

            // ------------------ VIEW APPOINTMENTS (NEW) ------------------
            case VIEW_APPOINTMENTS:
                DrawText("Scheduled Appointments", 50, 70, 20, DARKGRAY);
                
                BeginScissorMode(0, 100, screenWidth, 400);
                int appY = 110 + (int)scrollOffset;
                
                // Draw Table Header
                DrawText("Patient Name", 60, appY, 20, DARKBLUE);
                DrawText("Doctor", 300, appY, 20, DARKBLUE);
                DrawText("Date", 550, appY, 20, DARKBLUE);
                DrawLine(50, appY + 25, 750, appY + 25, BLACK);
                appY += 40;

                if (appointmentCount == 0) DrawText("No appointments booked.", 300, 200, 20, GRAY);

                for (int i = 0; i < appointmentCount; i++) {
                    DrawRectangle(50, appY - 5, 700, 30, (i % 2 == 0) ? LIGHTGRAY : RAYWHITE);
                    DrawText(appointments[i].patientName, 60, appY, 20, BLACK);
                    DrawText(appointments[i].doctorName, 300, appY, 20, BLACK);
                    DrawText(appointments[i].date, 550, appY, 20, BLACK);
                    appY += 40;
                }
                EndScissorMode();

                if (GuiButton((Rectangle){20, 530, 100, 40}, "Back")) currentState = MENU;
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}