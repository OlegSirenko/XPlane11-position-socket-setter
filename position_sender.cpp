#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMCamera.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 3400

// Объявление функции обратного вызова для цикла полета
float MyFlightLoopCallback(float inElapsedSinceLastCall,
                           float inElapsedTimeSinceLastFlightLoop,
                           int inCounter,
                           void *inRefcon);


static XPLMDataRef gPlaneX = NULL;
static XPLMDataRef gPlaneY = NULL;
static XPLMDataRef gPlaneZ = NULL;

int sock = 0;
struct sockaddr_in serv_addr;

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    strcpy(outName, "PositionSender");
    strcpy(outSig, "xpsdk.examples.positionsender");
    strcpy(outDesc,
           "A plugin that sends the plane's position over a socket connection.");

    gPlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
    gPlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
    gPlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return 0;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 0;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return 0;
    }
    XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, -1.0, NULL);
    return 1;
}

// Функция обратного вызова для цикла полета
float MyFlightLoopCallback(float inElapsedSinceLastCall,
                           float inElapsedTimeSinceLastFlightLoop,
                           int inCounter, void *inRefcon){
    // Получаем текущее положение самолета
    float x = XPLMGetDataf(gPlaneX);
    float y = XPLMGetDataf(gPlaneY);
    float z = XPLMGetDataf(gPlaneZ);

    // Отправляем данные через сокетное соединение
    char buffer[1024];
    sprintf(buffer, "%f,%f,%f\n", x, y, z);
    XPLMDebugString(buffer);
    send(sock, buffer, strlen(buffer), 0);
    return -1.0;
}

PLUGIN_API void XPluginStop(void) {
    XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
    close(sock);
}

PLUGIN_API void XPluginDisable(void) { /* */
    XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
    close(sock);
}

PLUGIN_API int XPluginEnable(void) { return 1; }

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage,
                                      void *inParam) {}

