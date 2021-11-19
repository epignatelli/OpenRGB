/*-------------------------------------------------------------------*\
|  XPGSpectrixS40GControllerDetect.cpp                                |
|                                                                     |
|  Driver for XPG's Spectrix S40G NVMe                                |
|                                                                     |
|  NicolasNewman          25th Mar 2021                               |
|                                                                     |
\*-------------------------------------------------------------------*/

#include "Detector.h"
#include "XPGSpectrixS40GController.h"
#include "RGBController.h"
#include "RGBController_XPGSpectrixS40G.h"
#include <vector>
#include <hidapi/hidapi.h>

#define DEVBUFSIZE (128 * 1024)
#include <windows.h>
#include <fileapi.h>

int Search(wchar_t *dev_name)
{
    wchar_t buff[DEVBUFSIZE]  = L"";
    int wchar_count;

    wchar_count = QueryDosDeviceW(NULL, buff, DEVBUFSIZE);

    if(wchar_count == 0)
    {
        return 0;
    }

    for(int i = 0; i < wchar_count; i++)
    {
        if(wcsstr(buff + i, L"SCSI#Disk&Ven_NVMe&Prod_XPG_SPECTRIX_S40#"))
        {
            wcsncpy(dev_name, buff + i, MAX_PATH);
            (dev_name)[MAX_PATH - 1] = '\0';
            return 1;
        }

        i += wcslen(buff + i);
    }

    return 0;
}

/******************************************************************************************\
*                                                                                          *
*   DetectXPGSpectrixS40GControllers                                                       *
*                                                                                          *
*       Tests for the existance of a file descriptor matching                              *
*       SCSI#Disk&Ven_NVMe&Prod_XPG_SPECTRIX_S40# on Windows machines                      *
*                                                                                          *
\******************************************************************************************/

void DetectSpectrixS40GControllers(std::vector<RGBController*>& rgb_controllers)
{
    XPGSpectrixS40GController* new_xpg_s40g;
    RGBController_XPGSpectrixS40G* new_controller;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/install/identifiers-for-scsi-devices
    wchar_t dev_name[MAX_PATH];
    
    if(Search(dev_name))
    {
        new_xpg_s40g = new XPGSpectrixS40GController(dev_name, 0x67);

        new_controller = new RGBController_XPGSpectrixS40G(new_xpg_s40g);

        new_controller->name    = "XPG Spectrix S40G";
        new_controller->vendor  = "XPG";
        new_controller->type    = DEVICE_TYPE_STORAGE;
        
        rgb_controllers.push_back(new_controller);

    }
}   /* DetectSpectrixS40GControllers() */

REGISTER_DETECTOR("XPG Spectrix S40G NVMe", DetectSpectrixS40GControllers);