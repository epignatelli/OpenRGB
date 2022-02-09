/*---------------------------------------------------------*\
|  Processing Code for NZXT Hue 1 (Smart Device V1)         |
|                                                           |
|  Adam Honse (calcprogrammer1@gmail.com), 2/8/2022         |
\*---------------------------------------------------------*/

#include "NZXTHue1Controller.h"
#include "LogManager.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>

NZXTHue1Controller::NZXTHue1Controller(hid_device* dev_handle, unsigned int rgb_channels, unsigned int fan_channels, const char* path)
{
    dev         = dev_handle;
    location    = path;

    num_fan_channels = fan_channels;
    num_rgb_channels = rgb_channels;

    SendFirmwareRequest();
    //UpdateDeviceList();

    //fan_cmd.resize(num_fan_channels);
    //fan_rpm.resize(num_fan_channels);
    //UpdateStatus();
}

NZXTHue1Controller::~NZXTHue1Controller()
{
    hid_close(dev);
}

unsigned char NZXTHue1Controller::GetFanCommand
    (
    unsigned char   fan_channel
    )
{
    return(fan_cmd[fan_channel]);
}

unsigned short NZXTHue1Controller::GetFanRPM
    (
    unsigned char   fan_channel
    )
{
    return(fan_rpm[fan_channel]);
}

std::string NZXTHue1Controller::GetLocation()
{
    return("HID: " + location);
}

unsigned int NZXTHue1Controller::GetNumFanChannels()
{
    return(num_fan_channels);
}

unsigned int NZXTHue1Controller::GetNumRGBChannels()
{
    return(num_rgb_channels);
}

std::string NZXTHue1Controller::GetFirmwareVersion()
{
    return(firmware_version);
}

std::string NZXTHue1Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    std::wstring return_wstring = serial_string;
    std::string return_string(return_wstring.begin(), return_wstring.end());

    return(return_string);
}

void NZXTHue1Controller::SendFan
    (
        unsigned char       port,
        unsigned char       /*mode*/,
        unsigned char       speed
    )
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up RGB packet                                     |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x62;
    usb_buf[0x01]   = 0x01;
    usb_buf[0x02]   = 1 << port;
    usb_buf[port + 3] = speed;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 64);
    hid_read(dev, usb_buf, 64);
}

void NZXTHue1Controller::UpdateDeviceList()
{
    unsigned char   usb_buf[65];
    unsigned int    ret_val = 0;

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Device Information Request packet              |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x02;
    usb_buf[0x01]   = 0x5C;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 64);

    /*-----------------------------------------------------*\
    | Receive packets until 0x21 0x03 is received           |
    \*-----------------------------------------------------*/
    do
    {
        ret_val = hid_read(dev, usb_buf, sizeof(usb_buf));
    } while( (ret_val != 65) || (usb_buf[0] != 0x21) || (usb_buf[1] != 0x03) );

    for(unsigned int chan = 0; chan < num_rgb_channels; chan++)
    {
        unsigned int start = 0x0F + (6 * chan);
        unsigned int num_leds_on_channel = 0;

        for(int dev = 0; dev < 6; dev++)
        {
            unsigned int num_leds_in_device = 0;

            switch(usb_buf[start + dev])
            {
            case 0x01: //Hue 1 strip
                num_leds_in_device = 10;
                break;

            case 0x02: //Aer 1 fan
                num_leds_in_device = 8;
                break;

            case 0x04: //Hue 2 strip (10 LEDs)
                num_leds_in_device = 10;
                break;

            case 0x05: //Hue 2 strip (8 LEDs)
                num_leds_in_device = 8;
                break;
            
            case 0x06: //Hue 2 strip (6 LEDs)
                num_leds_in_device = 6;
                break;
           
            case 0x09: //Hue 2 Underglow (300mm) (15 LEDs)
                num_leds_in_device = 15;
                break;
 
            case 0x0A: //Hue 2 Underglow (200mm) (10 LEDs)
                num_leds_in_device = 10;
                break;

            case 0x0B: //Aer 2 fan (120mm)
                num_leds_in_device = 8;
                break;

            case 0x0C: //Aer 2 fan (140mm)
                num_leds_in_device = 8;
                break;

            case 0x10: //Kraken X3 ring
                num_leds_in_device = 8;
                break;
            
            case 0x11: //Kraken X3 logo
                num_leds_in_device = 1;
                break;
            
            case 0x08: //Hue 2 Cable Comb (14 LEDs)
                num_leds_in_device = 14;
                break;

            default:
                break;
            }

            LOG_DEBUG("[NZXT Hue 2] %d: Device ID: %02X LEDs: %d", dev, usb_buf[start + dev], num_leds_in_device);

            num_leds_on_channel += num_leds_in_device;
        }

        channel_leds[chan] = num_leds_on_channel;
    }
}

void NZXTHue1Controller::UpdateStatus()
{
    unsigned char usb_buf[64];
    unsigned int  ret_val = 0;

    if(false)//num_fan_channels > 0)
    {
        /*-----------------------------------------------------*\
        | Zero out buffer                                       |
        \*-----------------------------------------------------*/
        memset(usb_buf, 0, sizeof(usb_buf));

        /*-----------------------------------------------------*\
        | Read packet                                           |
        \*-----------------------------------------------------*/
        do
        {
            ret_val = hid_read(dev, usb_buf, sizeof(usb_buf));
        } while( (ret_val != 64) || (usb_buf[0] != 0x67) || (usb_buf[1] != 0x02) );

        /*-----------------------------------------------------*\
        | Extract fan information                               |
        \*-----------------------------------------------------*/
        for(unsigned int fan_idx = 0; fan_idx < num_fan_channels; fan_idx++)
        {
            unsigned char  cmd;
            unsigned short rpm;

            cmd = usb_buf[40 + fan_idx];
            rpm = ( usb_buf[25 + (2 * fan_idx)] << 8 ) | usb_buf[24 + (2 * fan_idx)];

            fan_cmd[fan_idx] = cmd;
            fan_rpm[fan_idx] = rpm;
        }
    }
}

void NZXTHue1Controller::SetChannelEffect
    (
    unsigned char   channel,
    unsigned char   mode,
    unsigned char   speed,
    bool            direction,
    RGBColor *      colors,
    unsigned int    num_colors
    )
{
    unsigned char color_data[120];

    /*-----------------------------------------------------*\
    | If mode requires no colors, send packet               |
    \*-----------------------------------------------------*/
    if(num_colors == 0)
    {
        /*-----------------------------------------------------*\
        | Send mode without color data                          |
        \*-----------------------------------------------------*/
        SendPacket(channel, mode, direction, 0, speed, 0, NULL);
    }
    /*-----------------------------------------------------*\
    | If mode requires indexed colors, send color index     |
    | packets for each mode color                           |
    \*-----------------------------------------------------*/
    else if(num_colors <= 8)
    {
        for(std::size_t color_idx = 0; color_idx < num_colors; color_idx++)
        {
            /*-----------------------------------------------------*\
            | Fill in color data (40 entries per color)             |
            \*-----------------------------------------------------*/
            for (std::size_t idx = 0; idx < 40; idx++)
            {
                int pixel_idx = idx * 3;
                RGBColor color = colors[color_idx];
                color_data[pixel_idx + 0x00] = RGBGetGValue(color);
                color_data[pixel_idx + 0x01] = RGBGetRValue(color);
                color_data[pixel_idx + 0x02] = RGBGetBValue(color);
            }

            /*-----------------------------------------------------*\
            | Send mode and color data                              |
            \*-----------------------------------------------------*/
            SendPacket(channel, mode, direction, color_idx, speed, 40, &color_data[0]);
        }
    }
    /*-----------------------------------------------------*\
    | If mode requires per-LED colors, fill colors array    |
    \*-----------------------------------------------------*/
    else
    {
        /*-----------------------------------------------------*\
        | Fill in color data (up to 40 colors)                  |
        \*-----------------------------------------------------*/
        for (std::size_t idx = 0; idx < num_colors; idx++)
        {
            int pixel_idx = idx * 3;
            RGBColor color = colors[idx];
            color_data[pixel_idx + 0x00] = RGBGetGValue(color);
            color_data[pixel_idx + 0x01] = RGBGetRValue(color);
            color_data[pixel_idx + 0x02] = RGBGetBValue(color);
        }

        /*-----------------------------------------------------*\
        | Send mode and color data                              |
        \*-----------------------------------------------------*/
        SendPacket(channel, mode, direction, 0, speed, num_colors, &color_data[0]);
    }
}

void NZXTHue1Controller::SetChannelLEDs
    (
    unsigned char   channel,
    RGBColor *      colors,
    unsigned int    num_colors
    )
{
    unsigned char color_data[120];

    /*-----------------------------------------------------*\
    | Fill in color data (up to 40 colors)                  |
    \*-----------------------------------------------------*/
    for (std::size_t idx = 0; idx < num_colors; idx++)
    {
        int pixel_idx = idx * 3;
        RGBColor color = colors[idx];
        color_data[pixel_idx + 0x00] = RGBGetGValue(color);
        color_data[pixel_idx + 0x01] = RGBGetRValue(color);
        color_data[pixel_idx + 0x02] = RGBGetBValue(color);
    }

    /*-----------------------------------------------------*\
    | Send color data                                       |
    \*-----------------------------------------------------*/
    SendPacket(channel, HUE_1_MODE_FIXED, false, 0, 0, num_colors, &color_data[0]);
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void NZXTHue1Controller::SendPacket
    (
    unsigned char   channel,
    unsigned char   mode,
    bool            direction,
    unsigned char   color_idx,
    unsigned char   speed,
    unsigned char   color_count,
    unsigned char*  color_data
    )
{
    unsigned char   usb_buf[65];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up packet                                         |
    \*-----------------------------------------------------*/
    usb_buf[0x00]       = 0x02;
    usb_buf[0x01]       = 0x4C;

    /*-----------------------------------------------------*\
    | Set channel in serial packet                          |
    \*-----------------------------------------------------*/
    usb_buf[0x02]       = channel + 1;

    /*-----------------------------------------------------*\
    | Set mode in serial packet                             |
    \*-----------------------------------------------------*/
    usb_buf[0x03]       = mode;

    /*-----------------------------------------------------*\
    | Set options bitfield in serial packet                 |
    \*-----------------------------------------------------*/
    usb_buf[0x04]       = 0;
    usb_buf[0x04]      |= direction ? ( 1 << 4 ) : 0;

    /*-----------------------------------------------------*\
    | Set color index and speed in serial packet            |
    \*-----------------------------------------------------*/
    //usb_buf[0x05]       = ( color_idx << 5 ) | speed;

    /*-----------------------------------------------------*\
    | Copy in color data bytes                              |
    \*-----------------------------------------------------*/
    memcpy(&usb_buf[0x05], color_data, color_count * 3);

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 65);
}

void NZXTHue1Controller::SendFirmwareRequest()
{
    unsigned char   usb_buf[17];
    unsigned int    ret_val = 0;

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Firmware Request packet                        |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x02;
    usb_buf[0x01]   = 0x5C;

    hid_write(dev, usb_buf, 65);

    /*-----------------------------------------------------*\
    | Receive packets until 0x11 0x01 is received           |
    \*-----------------------------------------------------*/
    do
    {
        ret_val = hid_read(dev, usb_buf, sizeof(usb_buf));
    } while( (ret_val != 17) );

    snprintf(firmware_version, 16, "%u.%u", usb_buf[0x0D], usb_buf[0x0E]);
}
