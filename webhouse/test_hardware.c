/*
 * test_hardware.c
 * Ein Testprogramm, um alle GPIO-Funktionen des Webhauses zu überprüfen.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Für sleep()
#include "Webhouse.h"

// Hilfsfunktion für schöneren Output
void printStatus(const char* component, const char* status) {
    printf("  [TEST] %-15s -> %s\n", component, status);
    fflush(stdout);
}

int main() {
    printf("========================================\n");
    printf("   START HARDWARE TEST (Webhaus)\n");
    printf("========================================\n");
    
    // 1. Initialisierung
    printf("\n[INIT] Starte Webhouse Umgebung...\n");
    initWebhouse();
    sleep(1);

    // -------------------------------------------------
    // TEST: LEDs (Lichter)
    // -------------------------------------------------
    printf("\n--- Teste LEDs ---\n");
    
    // LED 1
    turnLED1On();
    printStatus("LED 1", (getLED1State() ? "AN (OK)" : "FEHLER (Sollte AN sein)"));
    sleep(1);
    turnLED1Off();
    printStatus("LED 1", (!getLED1State() ? "AUS (OK)" : "FEHLER (Sollte AUS sein)"));

    // LED 2
    turnLED2On();
    printStatus("LED 2", (getLED2State() ? "AN (OK)" : "FEHLER (Sollte AN sein)"));
    sleep(1);
    turnLED2Off();
    printStatus("LED 2", (!getLED2State() ? "AUS (OK)" : "FEHLER (Sollte AUS sein)"));


    // -------------------------------------------------
    // TEST: TV
    // -------------------------------------------------
    printf("\n--- Teste TV ---\n");
    turnTVOn();
    printStatus("Fernseher", (getTVState() ? "AN (OK)" : "FEHLER"));
    sleep(2);
    turnTVOff();
    printStatus("Fernseher", (!getTVState() ? "AUS (OK)" : "FEHLER"));


    // -------------------------------------------------
    // TEST: Heizung & Temperatur
    // -------------------------------------------------
    printf("\n--- Teste Heizung & Sensor ---\n");
    float startTemp = getTemp();
    printf("  [INFO] Start-Temperatur: %.2f C\n", startTemp);
    
    turnHeatOn();
    printStatus("Heizung", (getHeatState() ? "AN (OK)" : "FEHLER"));
    
    printf("  [WAIT] Warte 3 Sekunden (Heize auf)...\n");
    sleep(3);
    
    float endTemp = getTemp();
    printf("  [INFO] End-Temperatur:   %.2f C\n", endTemp);
    
    if(endTemp > startTemp) {
        printf("  [SUCCESS] Temperatur ist gestiegen!\n");
    } else {
        printf("  [WARNING] Temperatur nicht gestiegen (Simulation läuft evtl. langsam).\n");
    }
    
    turnHeatOff();
    printStatus("Heizung", "AUS");


    // -------------------------------------------------
    // TEST: Dimmbare Lampen (Roof & Stand)
    // -------------------------------------------------
    printf("\n--- Teste Dimmbare Lampen (0%% -> 100%% -> 0%%) ---\n");
    
    printf("  [ACTION] Dimme Roof Lamp (Deckenlampe) hoch...\n");
    for(int i=0; i<=100; i+=10) {
        dimRLamp(i);
        usleep(100000); // 100ms warten
    }
    sleep(1);
    dimRLamp(0);
    printStatus("Roof Lamp", "Wieder AUS");

    printf("  [ACTION] Dimme Stand Lamp (Stehlampe) hoch...\n");
    for(int i=0; i<=100; i+=10) {
        dimSLamp(i);
        usleep(100000); 
    }
    sleep(1);
    dimSLamp(0);
    printStatus("Stand Lamp", "Wieder AUS");


    // -------------------------------------------------
    // TEST: Alarm Sensor
    // -------------------------------------------------
    printf("\n--- Teste Alarm Sensor ---\n");
    int alarm = getAlarmState();
    if(alarm) {
        printf("  [INFO] Alarm-Status: ALARM AKTIV (High)\n");
    } else {
        printf("  [INFO] Alarm-Status: RUHIG (Low)\n");
    }


    // -------------------------------------------------
    // Abschluss
    // -------------------------------------------------
    printf("\n========================================\n");
    printf("   TEST ABGESCHLOSSEN\n");
    printf("========================================\n");
    
    closeWebhouse();
    
    return 0;
}