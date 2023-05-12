#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include <string>

void SetMultiplayerAircraftPosition(int index, float x, float y, float z)
{
    // Формируем имена DataRefs
    std::string xDataRefName = "sim/multiplayer/position/plane" + std::to_string(index) + "_x";
    std::string yDataRefName = "sim/multiplayer/position/plane" + std::to_string(index) + "_y";
    std::string zDataRefName = "sim/multiplayer/position/plane" + std::to_string(index) + "_z";

    // Получаем ссылки на DataRefs
    XPLMDataRef xRef = XPLMFindDataRef(xDataRefName.c_str());
    XPLMDataRef yRef = XPLMFindDataRef(yDataRefName.c_str());
    XPLMDataRef zRef = XPLMFindDataRef(zDataRefName.c_str());

    // Устанавливаем значения DataRefs
    XPLMSetDataf(xRef, x);
    XPLMSetDataf(yRef, y);
    XPLMSetDataf(zRef, z);
}

void FollowPlayerAircraft(int numMultiplayerAircraft)
{
    // Получаем доступ к многопользовательским самолетам
    XPLMPluginID multiplayerID = XPLMFindPluginBySignature("xplanesdk.examples.multiplayer");
    if (multiplayerID != XPLM_NO_PLUGIN_ID){
        XPLMSendMessageToPlugin(multiplayerID, MSG_ACQUIRE_PLANES, NULL);
    }
    else{
        printf("Error")
    }

    // Получаем ссылки на DataRefs
    XPLMDataRef xRef = XPLMFindDataRef("sim/flightmodel/position/local_x");
    XPLMDataRef yRef = XPLMFindDataRef("sim/flightmodel/position/local_y");
    XPLMDataRef zRef = XPLMFindDataRef("sim/flightmodel/position/local_z");

    // Получаем текущую позицию игрока
    float x = XPLMGetDataf(xRef);
    float y = XPLMGetDataf(yRef);
    float z = XPLMGetDataf(zRef);

    // Устанавливаем позиции многопользовательских самолетов
    for (int i = 0; i < numMultiplayerAircraft; i++){
        SetMultiplayerAircraftPosition(i + 1, x + (i + 1) * 2.0f, y, z);
    }
}

static bool gPluginEnabled = false;
static XPLMHotKeyID gHotKey = NULL;

// Функция обратного вызова для горячей клавиши
void MyHotKeyCallback(void *inRefcon)
{
    if (gPluginEnabled)
    {
        // Отключаем плагин и выводим сообщение на экран
        gPluginEnabled = false;
        XPLMSpeakString("Plugin disabled");
    }
    else
    {
        // Включаем плагин, вызываем функцию FollowPlayerAircraft и выводим сообщение на экран
        gPluginEnabled = true;
        FollowPlayerAircraft(1); // Указываем количество многопользовательских самолетов
        XPLMSpeakString("Plugin enabled");
    }
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    strcpy(outName, "Multiplayer Aircraft Position Setter");
    strcpy(outSig, "xpsdk.examples.positionsetter");
    strcpy(outDesc,
           "A plugin that sets the position of a multiplayer aircraft to follow the player aircraft.");

    // Регистрируем горячую клавишу F8 для активации плагина
    gHotKey = XPLMRegisterHotKey(XPLM_VK_F8, xplm_DownFlag,
                                 "Activate plugin", MyHotKeyCallback, NULL);

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    // Отменяем регистрацию горячей клавиши при остановке плагина
    XPLMUnregisterHotKey(gHotKey);
}

PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API int XPluginEnable(void) { return 1; }

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage,void *inParam) {}