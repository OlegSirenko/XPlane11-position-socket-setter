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

void MyHotKeyCallback(void *inRefcon);
float MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
                           int inCounter, void *inRefcon);
static XPLMDataRef gPlaneX = NULL;
static XPLMDataRef gPlaneY = NULL;
static XPLMDataRef gPlaneZ = NULL;
static XPLMDataRef gPlanePsi = NULL;
static XPLMDataRef gPlaneTheta = NULL;
static XPLMDataRef gPlanePhi = NULL;
static XPLMHotKeyID	gHotKey = NULL;
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
    gPlanePsi = XPLMFindDataRef("sim/flightmodel/position/psi");
    gPlaneTheta = XPLMFindDataRef("sim/flightmodel/position/theta");
    gPlanePhi = XPLMFindDataRef("sim/flightmodel/position/phi");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return 0;}
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "192.168.1.152", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 0;}
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return 0;}
    gHotKey = XPLMRegisterHotKey(XPLM_VK_F8, xplm_DownFlag, "Activate plugin", MyHotKeyCallback, NULL);
    return 1;}
// Функция обратного вызова для горячей клавиши
void MyHotKeyCallback(void *inRefcon) { XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, 0.05, NULL);}
// Функция обратного вызова для цикла полета
float MyFlightLoopCallback(float inElapsedSinceLastCall,float inElapsedTimeSinceLastFlightLoop,
                           int inCounter, void *inRefcon){
    // Получаем текущее положение самолета
    float x = XPLMGetDataf(gPlaneX);
    float y = XPLMGetDataf(gPlaneY);
    float z = XPLMGetDataf(gPlaneZ);
    float psi = XPLMGetDataf(gPlanePsi);
    float theta = XPLMGetDataf(gPlaneTheta);
    float phi = XPLMGetDataf(gPlanePhi);
    // Отправляем данные через сокетное соединение
    char buffer[1024];
    sprintf(buffer, "%f,%f,%f,%f,%f,%f\n", x, y, z, psi, theta, phi);
    send(sock, buffer, strlen(buffer), 0);
    return 0.05;}

PLUGIN_API void XPluginStop(void) {XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);close(sock);}
PLUGIN_API void XPluginDisable(void) { /* */
    XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
    close(sock);}
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage,void *inParam) {}

