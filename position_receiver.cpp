//
// Created by tehnokrat on 4/29/23.
//
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
#include "math.h"
#include <sys/socket.h>
#include <unistd.h>

#define PORT 3400


// Объявление функции обратного вызова для цикла полета
void MyHotKeyCallback(void *inRefcon);
float MyFlightLoopCallback(float inElapsedSinceLastCall,
                           float inElapsedTimeSinceLastFlightLoop,
                           int inCounter,
                           void *inRefcon);

static XPLMDataRef gPlaneX = NULL;
static XPLMDataRef gPlaneY = NULL;
static XPLMDataRef gPlaneZ = NULL;
static XPLMDataRef gPlaneQ = NULL;
static XPLMHotKeyID	gHotKey = NULL;


int server_fd = 0;
float pi  = 3.14159265359;
struct sockaddr_in serv_addr;

// Функция обратного вызова для цикла полета
float MyFlightLoopCallback(float inElapsedSinceLastCall,
                           float inElapsedTimeSinceLastFlightLoop,
                           int inCounter, void *inRefcon){
    // Буфер для приема данных
    char buffer[1024] = {0};
    // Принимаем данные через сокетное соединение
    int valread = read(server_fd, buffer, 1024);
    // Проверяем результат чтения данных
    if (valread > 0){
        // Данные были прочитаны - парсим их
        float x, y, z, psi, theta, phi;
        float q[4] = {0};
        int n = sscanf(buffer, "%f,%f,%f,%f,%f,%f\n", &x, &y, &z, &psi, &theta, &phi);
        if (n == 6) {
            printf("Прочитано данных %d\n", n);
            printf("x = %f,\n y=%f,\n z=%f,\n\n", x, y, z);
            printf("psi = %f,\n theta=%f,\n phi=%f,\n\n", psi, theta, phi);
            // Устанавливаем новое положение самолета
            XPLMSetDataf(gPlaneX, x);
            XPLMSetDataf(gPlaneY, y);
            XPLMSetDataf(gPlaneZ, z);
            float psi_rad = pi / 360 * psi;
            float theta_rad = pi / 360 * theta;
            float phi_rad = pi / 360 * phi;
            q[0] =  cos(psi_rad) * cos(theta_rad) * cos(phi_rad) + sin(psi_rad) * sin(theta_rad) * sin(phi_rad);
            q[1] =  cos(psi_rad) * cos(theta_rad) * sin(phi_rad) - sin(psi_rad) * sin(theta_rad) * cos(phi_rad);
            q[2] =  cos(psi_rad) * sin(theta_rad) * cos(phi_rad) + sin(psi_rad) * cos(theta_rad) * sin(phi_rad);
            q[3] = -cos(psi_rad) * sin(theta_rad) * sin(phi_rad) + sin(psi_rad) * cos(theta_rad) * cos(phi_rad);
            XPLMSetDatavf(gPlaneQ, q, 0, 4);
        }
        else{ printf("НЕВЕРНО ОТПРАВЛЕННЫ ДАННЫЕ %d\n", n); }
    }
    // Возвращаем время до следующего вызова функции (в секундах)
    return -1.0;}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    // Задаем имя, сигнатуру и описание плагина
    strcpy(outName, "PositionReceiver");
    strcpy(outSig, "xpsdk.examples.PositionReceiver");
    strcpy(outDesc,
           "A plugin that receives the plane's position over a socket connection.");

    // Находим ссылки на данные положения самолета
    gPlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
    gPlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
    gPlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
    gPlaneQ = XPLMFindDataRef("sim/flightmodel/position/q");



    // Создаем сокет сервера
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 0;
    }

    // Задаем параметры сервера
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу сервера
    if (bind(server_fd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr))<0) {
        perror("bind failed");
        return 0;
    }

    // Начинаем прослушивание соединений
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return 0;
    }

    // Принимаем соединение
    int addrlen = sizeof(serv_addr);
    if ((server_fd = accept(server_fd, (struct sockaddr *)&serv_addr,
                       (socklen_t*)&addrlen))<0) {
        perror("accept");
        return 0;
    }
    char buffer[1024] = {0};
    sprintf(buffer, "Awaiting for data from sender...\n");
    printf("Awaiting for data from sender...\n");
    XPLMDebugString(buffer);
    send(server_fd, buffer, strlen(buffer), 0);
    // Регистрируем функцию обратного вызова для цикла полета
    gHotKey = XPLMRegisterHotKey(XPLM_VK_F8, xplm_DownFlag, "Activate plugin", MyHotKeyCallback, NULL);
    //XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, -1.0, NULL);

    return 1;
}


void MyHotKeyCallback(void *inRefcon) {
    XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, -1, NULL);
}

PLUGIN_API void XPluginStop(void) {
    XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
    close(server_fd);
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) { return 1; }

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage,
                                      void *inParam) {}