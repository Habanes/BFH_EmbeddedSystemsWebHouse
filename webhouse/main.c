/******************************************************************************/
/** \file       main.c
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

#define PORT 8000               
#define RX_BUFFER_SIZE 1024
#define WS_HS_ACCLEN 130

//----- Function prototypes ----------------------------------------------------
static void shutdownHook (int32_t sig);
void processCommand(char *command, int com_sock_id);

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
    
    char rxBuf[RX_BUFFER_SIZE];
    int rx_data_len;
    
    signal(SIGINT, shutdownHook);

    initWebhouse();
    printf("Init Webhouse... Done.\n");
    fflush(stdout);

    // Create TCP socket
    server_sock_id = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock_id < 0) {
        perror("Socket Error");
        exit(EXIT_FAILURE);
    }

    // Allow port reuse
    int option = 1;
    setsockopt(server_sock_id, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Configure server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);            
    server.sin_addr.s_addr = htonl(INADDR_ANY); 

    // Bind socket to address
    bind_status = bind(server_sock_id, (struct sockaddr *)&server, sizeof(server));
    if (bind_status < 0) {
        perror("Bind Error");
        close(server_sock_id);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    listen_status = listen(server_sock_id, 5);
    if (listen_status < 0) {
        perror("Listen Error");
        close(server_sock_id);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on Port %d\n", PORT);
    fflush(stdout);

    // Main loop: accept and handle connections
    while (eShutdown == FALSE) {
        
        com_sock_id = accept(server_sock_id, (struct sockaddr *)&client, (socklen_t*)&addrlen);
        
        if (com_sock_id < 0) {
            if (eShutdown) break;
            perror("Accept Error");
            continue;
        }

        printf("Client connected!\n");
        fflush(stdout);

        // Communication loop with connected client
        while (1) {
            memset(rxBuf, 0, RX_BUFFER_SIZE);
            
            // Receive data from client
            rx_data_len = recv(com_sock_id, rxBuf, RX_BUFFER_SIZE - 1, 0);

            if (rx_data_len <= 0) {
                printf("Client disconnected.\n");
                close(com_sock_id);
                break;
            }

            rxBuf[rx_data_len] = '\0';

            // Handle WebSocket handshake
            if (strncmp(rxBuf, "GET", 3) == 0) {
                printf("Handshake Request received.\n");
                
                char response[WS_HS_ACCLEN];
                get_handshake_response(rxBuf, response);
                send(com_sock_id, response, strlen(response), 0);
                printf("Handshake sent.\n");
            
            } else {
                // Decode and process command
                char command[rx_data_len + 1];
                decode_incoming_request(rxBuf, command);
                command[strlen(command)] = '\0';
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

    // Process button commands
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
        armAlarm();
    }
    else if (strstr(command, "<AlarmOff>") != NULL) {
        disarmAlarm();
    }
    else if (strstr(command, "<L2on>") != NULL) {
        printf("LED 2 On\n");
        turnLED2On();
    }
    else if (strstr(command, "<L2off>") != NULL) {
        printf("LED 2 Off\n");
        turnLED2Off();
    }
    else if (strstr(command, "<GetStatus>") != NULL) {
        // Status request only - response sent at end
    }
    
    // Process slider commands
    else if (strncmp(command, "<Dim1:", 6) == 0) {
        int value = 0;
        if (sscanf(command, "<Dim1:%d>", &value) == 1) {
            printf("Dimmer 1 set to: %d\n", value);
            dimRLamp((uint16_t)value); 
        }
    }
    else if (strncmp(command, "<Dim2:", 6) == 0) {
        int value = 0;
        if (sscanf(command, "<Dim2:%d>", &value) == 1) {
            printf("Dimmer 2 set to: %d\n", value);
            dimSLamp((uint16_t)value);
        }
    }
    else if (strncmp(command, "<SetTemp:", 9) == 0) {
        int targetTemp = 0;
        if (sscanf(command, "<SetTemp:%d>", &targetTemp) == 1) {
            printf("Target temperature set: %d°C\n", targetTemp);
            // Simple bang-bang temperature control
            float currentTemp = getTemp();
            if (currentTemp < (float)targetTemp) {
                turnHeatOn();
            } else {
                turnHeatOff();
            }
        }
    }

    // Send response with current status
    char response_raw[100];
    float temp = getTemp();
    int alarmArmed = getAlarmArmedState();
    int alarmTriggered = getAlarmState();
    
    sprintf(response_raw, "Temp:%.1f;AlarmArmed:%d;AlarmTriggered:%d", temp, alarmArmed, alarmTriggered);
    
    char codedResponse[strlen(response_raw) + 10];
    code_outgoing_response(response_raw, codedResponse);
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