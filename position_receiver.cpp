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

int server_fd = 0;
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
        float x, y, z;
        int n = sscanf(buffer, "%f,%f,%f\n", &x, &y, &z);
        if (n == 3) {
            printf("Прочитано данных %d\n", n);
            printf("x = %f,\n y=%f,\n z=%f,\n\n", x, y, z);
            // Устанавливаем новое положение самолета
            XPLMSetDataf(gPlaneX, x);
            XPLMSetDataf(gPlaneY, y);
            XPLMSetDataf(gPlaneZ, z);
        }
        else{
            printf("НЕВЕРНО ОТПРАВЛЕННЫ ДАННЫЕ %d\n", n);
        }
    }
    // Возвращаем время до следующего вызова функции (в секундах)
    return -1.0;
}

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
    XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, -1.0, NULL);

    return 1;
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