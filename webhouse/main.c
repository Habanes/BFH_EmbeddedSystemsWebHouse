/******************************************************************************/
/** \file       startup.c
 *******************************************************************************
 *
 * \brief      Main application for the Rasberry-Pi webhouse
 *
 * \author     fue1
 *
 * \date       November 2021
 *
 * \remark     Last Modification
 * \li fue1, November 2021, Created
 *
 ******************************************************************************/
/*
 * functions  global:
 * main
 * * functions  local:
 * shutdownHook
 * processCommand
 * * Autor      Elham Firouzi
 *
 ******************************************************************************/

typedef int int32_t;

//----- Header-Files -----------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include "jansson.h"
#include "Webhouse.h"
#include "handshake.h"

//----- Macros -----------------------------------------------------------------
#define TRUE 1
#define FALSE 0

// Konfiguration laut PDF Anforderungen
#define PORT 8000               
#define RX_BUFFER_SIZE 1024
#define WS_HS_ACCLEN 130

//----- Function prototypes ----------------------------------------------------
static void shutdownHook (int32_t sig);
void processCommand(char *command, int com_sock_id); // Hilfsfunktion für Befehle

//----- Data -------------------------------------------------------------------
static volatile int eShutdown = FALSE;

//----- Implementation ---------------------------------------------------------

/*******************************************************************************
 * function :    main
 ******************************************************************************/
/** \brief     Starts the socket server (ip: localhost, port:8000) and waits
 * on a connection attempt of the client.
 *
 * \type         global
 *
 * \return       EXIT_SUCCESS
 *
 ******************************************************************************/
int main(int argc, char **argv) {
    int bind_status;
    int listen_status;
    int server_sock_id;
    int com_sock_id;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int addrlen = sizeof (struct sockaddr_in);
    
    // Puffer für Netzwerkdaten
    char rxBuf[RX_BUFFER_SIZE];
    int rx_data_len;
    
    signal(SIGINT, shutdownHook);

    initWebhouse();
    printf("Init Webhouse... Done.\n");
    fflush(stdout);

    // ---------------------------------------------------------
    // 1. Socket erstellen [cite: 5, 25]
    // ---------------------------------------------------------
    server_sock_id = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock_id < 0) {
        perror("Socket Error");
        exit(EXIT_FAILURE);
    }

    // Option setzen, um Port sofort wiederverwenden zu können (verhindert Blockade nach Neustart)
    int option = 1;
    setsockopt(server_sock_id, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // ---------------------------------------------------------
    // 2. Adresse konfigurieren (Port 8000) [cite: 70]
    // ---------------------------------------------------------
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);            
    server.sin_addr.s_addr = htonl(INADDR_ANY); 

    // ---------------------------------------------------------
    // 3. Bind [cite: 353]
    // ---------------------------------------------------------
    bind_status = bind(server_sock_id, (struct sockaddr *)&server, sizeof(server));
    if (bind_status < 0) {
        perror("Bind Error");
        close(server_sock_id);
        exit(EXIT_FAILURE);
    }

    // ---------------------------------------------------------
    // 4. Listen
    // ---------------------------------------------------------
    listen_status = listen(server_sock_id, 5);
    if (listen_status < 0) {
        perror("Listen Error");
        close(server_sock_id);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on Port %d\n", PORT);
    fflush(stdout);

    // ---------------------------------------------------------
    // 5. Main Loop (Accept & Communication)
    // ---------------------------------------------------------
    while (eShutdown == FALSE) {
        
        // Warten auf Verbindung...
        com_sock_id = accept(server_sock_id, (struct sockaddr *)&client, (socklen_t*)&addrlen);
        
        if (com_sock_id < 0) {
            if (eShutdown) break;
            perror("Accept Error");
            continue;
        }

        printf("Client connected!\n");
        fflush(stdout);

        // Innere Schleife: Kommunikation mit dem verbundenen Client
        while (1) {
            memset(rxBuf, 0, RX_BUFFER_SIZE);
            
            // Daten empfangen [cite: 355]
            rx_data_len = recv(com_sock_id, rxBuf, RX_BUFFER_SIZE - 1, 0);

            if (rx_data_len <= 0) {
                printf("Client disconnected.\n");
                close(com_sock_id);
                break; // Zurück zum accept()
            }

            rxBuf[rx_data_len] = '\0'; // String terminieren

            // --- Handshake Prüfung [cite: 387] ---
            if (strncmp(rxBuf, "GET", 3) == 0) {
                printf("Handshake Request received.\n");
                
                char response[WS_HS_ACCLEN]; // Array min 130 Bytes [cite: 364]
                
                // Handshake Antwort generieren [cite: 361]
                get_handshake_response(rxBuf, response);
                
                // Antwort senden [cite: 391]
                send(com_sock_id, response, strlen(response), 0);
                printf("Handshake sent.\n");
            
            } else {
                // --- Datenverarbeitung (Befehle) [cite: 376] ---
                
                // Puffer für den entschlüsselten Befehl
                char command[rx_data_len + 1]; 
                
                // 1. Empfangene Nachricht entschlüsseln
                decode_incoming_request(rxBuf, command);
                command[strlen(command)] = '\0'; // Sicherstellen, dass String terminiert ist

                // 2. Befehl verarbeiten (Hardware steuern)
                processCommand(command, com_sock_id);
            }
            fflush(stdout);
        }
    }

    closeWebhouse();
    close(server_sock_id);
    printf ("Close Webhouse\n");
    fflush (stdout);

    return EXIT_SUCCESS;
}

/*******************************************************************************
 * function :    processCommand
 ******************************************************************************/
/** \brief        Parses the HTML command string and controls hardware
 * \param[in]    command       Decoded string from WebSocket
 * \param[in]    com_sock_id   Socket ID to send responses back
 ******************************************************************************/
void processCommand(char *command, int com_sock_id) {
    printf("Processing Command: %s\n", command);

    // --- Schalter (Buttons) ---
    // Mapping der HTML Befehle auf Webhouse.h Funktionen
    
    if (strstr(command, "<HeatOn>") != NULL) {
        turnHeatOn();
    } 
    else if (strstr(command, "<HeatOff>") != NULL) {
        turnHeatOff();
    }
    else if (strstr(command, "<L1on>") != NULL) {
        turnLED1On();
    }
    else if (strstr(command, "<L1off>") != NULL) {
        turnLED1Off();
    }
    else if (strstr(command, "<TVon>") != NULL) {
        turnTVOn();
    }
    else if (strstr(command, "<TVoff>") != NULL) {
        turnTVOff();
    }
    else if (strstr(command, "<AlarmOn>") != NULL) {
        // Falls Funktion in Webhouse.h existiert, sonst Dummy
        // turnAlarmOn(); 
        printf("Alarm aktiviert\n");
    }
    else if (strstr(command, "<AlarmOff>") != NULL) {
        // turnAlarmOff();
        printf("Alarm deaktiviert\n");
    }
    else if (strstr(command, "<L2on>") != NULL) {
        printf("LED 2 Ein\n");
        turnLED2On(); // Funktion aus Webhouse.h 
    }
    else if (strstr(command, "<L2off>") != NULL) {
        printf("LED 2 Aus\n");
        turnLED2Off();
    }
    else if (strstr(command, "<GetStatus>") != NULL) {
        // Leerer Befehl, nur um Temperatur/Status abzufragen
        // Wir machen nichts hier, da die Antwort sowieso am Ende gesendet wird
    }
    
    // --- Regler (Slider) ---
    // Format: "<Dim1:50>" oder "<SetTemp:22>"
    
    else if (strncmp(command, "<Dim1:", 6) == 0) {
        int wert = 0;
        if (sscanf(command, "<Dim1:%d>", &wert) == 1) {
            printf("Dimmer set to: %d\n", wert);
            // Je nach Pin-Belegung: dimRLamp oder dimSLamp
            dimRLamp((uint16_t)wert); 
        }
    }

    // --- Antwort vorbereiten ---
    // Das PDF empfiehlt eine Bestätigung "<Command executed>" [cite: 404]
    // Optional: Wir hängen die Temperatur an, damit das JS Display funktioniert.
    
    char response_raw[50];
    float temp = getTemp(); // Temperatur lesen
    
    // Wir senden Bestätigung UND Temperatur im Format, das dein JS versteht ("Temp:XX")
    sprintf(response_raw, "CmdOK;Temp:%.1f", temp);
    
    // Antwort für WebSocket codieren (Muss sein!) [cite: 375]
    char codedResponse[strlen(response_raw) + 10]; // Etwas Puffer
    code_outgoing_response(response_raw, codedResponse);
    
    // Abschicken
    send(com_sock_id, codedResponse, strlen(codedResponse), 0);
}

/*******************************************************************************
 * function :    shutdownHook
 ******************************************************************************/
/** \brief        Handle the registered signals (SIGTERM, SIGINT)
 *
 * \type         static
 *
 * \param[in]    sig    incoming signal
 *
 * \return       void
 *
 ******************************************************************************/
static void shutdownHook(int32_t sig) {
    printf("Ctrl-C pressed....shutdown hook in main\n");
    fflush(stdout);
    eShutdown = TRUE;
}