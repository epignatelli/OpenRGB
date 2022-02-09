#include "Detector.h"
#include "NZXTHue1Controller.h"
#include "RGBController.h"
#include "RGBController_NZXTHue1.h"
#include <hidapi/hidapi.h>

#define NZXT_VID                     0x1E71
#define CRYORIG_H7_QUAD_LUMI_PID     0x1712

static void spawn_hue(hid_device_info* info, const std::string& name, int rgb_channels, int fan_channels)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        NZXTHue1Controller*     controller     = new NZXTHue1Controller(dev, rgb_channels, fan_channels, info->path);
        RGBController_NZXTHue1* rgb_controller = new RGBController_NZXTHue1(controller);
        rgb_controller->name                   = name;

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

void DetectCryorigH7QuadLumi(hid_device_info* info, const std::string& name)
{
    spawn_hue(info, name, 2, 0);
}


REGISTER_HID_DETECTOR("CRYORIG H7 Quad Lumi",      DetectCryorigH7QuadLumi,   NZXT_VID, CRYORIG_H7_QUAD_LUMI_PID);
