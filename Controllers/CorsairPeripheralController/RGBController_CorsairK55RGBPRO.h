#ifndef RGBCONTROLLER_CORSAIRK55RGBPRO_H
#define RGBCONTROLLER_CORSAIRK55RGBPRO_H

#include "RGBController.h"
#include "CorsairK55RGBPROController.h"

class RGBController_CorsairK55RGBPRO : public RGBController
{
public:
    RGBController_CorsairK55RGBPRO(CorsairK55RGBPROController* controller_ptr);
    ~RGBController_CorsairK55RGBPRO();

    void SetupZones();
    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();

    void KeepaliveThread();

private:
    CorsairK55RGBPROController*                         controller;

    std::thread*                                        keepalive_thread;
    std::atomic<bool>                                   keepalive_thread_run;
    std::chrono::time_point<std::chrono::steady_clock>  last_update_time;

};

#endif // RGBCONTROLLER_CORSAIRK55RGBPRO_H
