//----------------------------------------------------------------------------//
// Midi Triggered Commands - Spare time development for fun                   //
// (c) 2021 Laurent Lardinois https://be.linkedin.com/in/laurentlardinois     //
//                                                                            //
// This software is provided 'as-is', without any express or implied          //
// warranty.In no event will the authors be held liable for any damages       //
// arising from the use of this software.                                     //
//                                                                            //
// Permission is granted to anyone to use this software for any purpose,      //
// including commercial applications, and to alter itand redistribute it      //
// freely, subject to the following restrictions :                            //
//                                                                            //
// 1. The origin of this software must not be misrepresented; you must not    //
// claim that you wrote the original software.If you use this software        //
// in a product, an acknowledgment in the product documentation would be      //
// appreciated but is not required.                                           //
// 2. Altered source versions must be plainly marked as such, and must not be //
// misrepresented as being the original software.                             //
// 3. This notice may not be removed or altered from any source distribution. //
//----------------------------------------------------------------------------//

#include "jute/jute.h"
#include "rtmidi/RtMidi.h"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#define SLEEP(x) Sleep(x)
#elif defined(__linux__)
#include <signal.h>
#include <unistd.h>
#define SLEEP(x) usleep(x * 1000)
#else
#define SLEEP(x)
#endif

// http://www.onicos.com/staff/iz/formats/midi-event.html
// http://www.onicos.com/staff/iz/formats/midi-cntl.html

enum class MidiType
{
    MIDI_NOTE_OFF = 0x01,
    MIDI_NOTE_ON = 0x02,
    MIDI_POLYPHONIC_AFTERTOUCH = 0x04,
    MIDI_CHANNEL_AFTERTOUCH = 0x08,
    MIDI_PITCH_WHEEL_RANGE = 0x10,
    MIDI_CONTROL_MODE_CHANGE = 0x20
};

enum class MidiControl
{
    MIDICTRL_CONTINUOUS_CONTROLLER0_MSB = 0x00,
    MIDICTRL_MODULATION_WHEEL_MSB = 0x01,
    MIDICTRL_BREATH_CONTROL_MSB = 0x02,
    MIDICTRL_CONTINUOUS_CONTROLLER3_MSB = 0x03,
    MIDICTRL_FOOT_CONTROLLER_MSB = 0x04,
    MIDICTRL_PORTAMENTO_TIME_MSB = 0x05,
    MIDICTRL_DATA_ENTRY_MSB = 0x06,
    MIDICTRL_MAIN_VOLUME_MSB = 0x07,
    MIDICTRL_CONTINUOUS_CONTROLLER8_MSB = 0x08,
    MIDICTRL_CONTINUOUS_CONTROLLER9_MSB = 0x09,
    MIDICTRL_CONTINUOUS_CONTROLLER10_MSB = 0x0A,
    MIDICTRL_CONTINUOUS_CONTROLLER11_MSB = 0x0B,
    MIDICTRL_CONTINUOUS_CONTROLLER12_MSB = 0x0C,
    MIDICTRL_CONTINUOUS_CONTROLLER13_MSB = 0x0D,
    MIDICTRL_CONTINUOUS_CONTROLLER14_MSB = 0x0E,
    MIDICTRL_CONTINUOUS_CONTROLLER15_MSB = 0x0F,
    MIDICTRL_CONTINUOUS_CONTROLLER16_MSB = 0x10,
    MIDICTRL_CONTINUOUS_CONTROLLER17_MSB = 0x11,
    MIDICTRL_CONTINUOUS_CONTROLLER18_MSB = 0x12,
    MIDICTRL_CONTINUOUS_CONTROLLER19_MSB = 0x13,
    MIDICTRL_CONTINUOUS_CONTROLLER20_MSB = 0x14,
    MIDICTRL_CONTINUOUS_CONTROLLER21_MSB = 0x15,
    MIDICTRL_CONTINUOUS_CONTROLLER22_MSB = 0x16,
    MIDICTRL_CONTINUOUS_CONTROLLER23_MSB = 0x17,
    MIDICTRL_CONTINUOUS_CONTROLLER24_MSB = 0x18,
    MIDICTRL_CONTINUOUS_CONTROLLER25_MSB = 0x19,
    MIDICTRL_CONTINUOUS_CONTROLLER26_MSB = 0x1A,
    MIDICTRL_CONTINUOUS_CONTROLLER27_MSB = 0x1B,
    MIDICTRL_CONTINUOUS_CONTROLLER28_MSB = 0x1C,
    MIDICTRL_CONTINUOUS_CONTROLLER29_MSB = 0x1D,
    MIDICTRL_CONTINUOUS_CONTROLLER30_MSB = 0x1E,
    MIDICTRL_CONTINUOUS_CONTROLLER31_MSB = 0x1F,
    MIDICTRL_CONTINUOUS_CONTROLLER0_LSB = 0x20,
    MIDICTRL_MODULATION_WHEEL_LSB = 0x21,
    MIDICTRL_BREATH_CONTROL_LSB = 0x22,
    MIDICTRL_CONTINUOUS_CONTROLLER3_LSB = 0x23,
    MIDICTRL_FOOT_CONTROLLER_LSB = 0x24,
    MIDICTRL_PORTAMENTO_TIME_LSB = 0x25,
    MIDICTRL_DATA_ENTRY_LSB = 0x26,
    MIDICTRL_MAIN_VOLUME_LSB = 0x27,
    MIDICTRL_CONTINUOUS_CONTROLLER8_LSB = 0x28,
    MIDICTRL_CONTINUOUS_CONTROLLER9_LSB = 0x29,
    MIDICTRL_CONTINUOUS_CONTROLLER10_LSB = 0x2A,
    MIDICTRL_CONTINUOUS_CONTROLLER11_LSB = 0x2B,
    MIDICTRL_CONTINUOUS_CONTROLLER12_LSB = 0x2C,
    MIDICTRL_CONTINUOUS_CONTROLLER13_LSB = 0x2D,
    MIDICTRL_CONTINUOUS_CONTROLLER14_LSB = 0x2E,
    MIDICTRL_CONTINUOUS_CONTROLLER15_LSB = 0x2F,
    MIDICTRL_CONTINUOUS_CONTROLLER16_LSB = 0x30,
    MIDICTRL_CONTINUOUS_CONTROLLER17_LSB = 0x31,
    MIDICTRL_CONTINUOUS_CONTROLLER18_LSB = 0x32,
    MIDICTRL_CONTINUOUS_CONTROLLER19_LSB = 0x33,
    MIDICTRL_CONTINUOUS_CONTROLLER20_LSB = 0x34,
    MIDICTRL_CONTINUOUS_CONTROLLER21_LSB = 0x35,
    MIDICTRL_CONTINUOUS_CONTROLLER22_LSB = 0x36,
    MIDICTRL_CONTINUOUS_CONTROLLER23_LSB = 0x37,
    MIDICTRL_CONTINUOUS_CONTROLLER24_LSB = 0x38,
    MIDICTRL_CONTINUOUS_CONTROLLER25_LSB = 0x39,
    MIDICTRL_CONTINUOUS_CONTROLLER26_LSB = 0x3A,
    MIDICTRL_CONTINUOUS_CONTROLLER27_LSB = 0x3B,
    MIDICTRL_CONTINUOUS_CONTROLLER28_LSB = 0x3C,
    MIDICTRL_CONTINUOUS_CONTROLLER29_LSB = 0x3D,
    MIDICTRL_CONTINUOUS_CONTROLLER30_LSB = 0x3E,
    MIDICTRL_CONTINUOUS_CONTROLLER31_LSB = 0x3F,
    MIDICTRL_DAMPER_PEDAL_ON_OFF = 0x40,
    MIDICTRL_PORTAMENTO_ON_OFF = 0x41,
    MIDICTRL_SUSTENUTO_ON_OFF = 0x42,
    MIDICTRL_SOFT_PEDAL_ON_OFF = 0x43,
    MIDICTRL_LOCAL_CONTROL_ON_OFF = 0x7A,
    MIDICTRL_ALL_NOTES_OFF = 0x7B,
    MIDICTRL_OMNI_MODE_OFF = 0x7C,
    MIDICTRL_OMNI_MODE_ON = 0x7D,
    MIDICTRL_POLY_MODE_ON_OFF = 0x7E,
    MIDICTRL_POLY_MODE_ON = 0x7F,
    MIDICTRL_CRTL_UNDEFINED = 0xFF,
};

namespace
{
    const std::map<std::string, MidiControl> token_to_ctrl = {
        { "MIDICTRL_CONTINUOUS_CONTROLLER0_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER0_MSB },
        { "MIDICTRL_MODULATION_WHEEL_MSB", MidiControl::MIDICTRL_MODULATION_WHEEL_MSB },
        { "MIDICTRL_BREATH_CONTROL_MSB", MidiControl::MIDICTRL_BREATH_CONTROL_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER3_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER3_MSB },
        { "MIDICTRL_FOOT_CONTROLLER_MSB", MidiControl::MIDICTRL_FOOT_CONTROLLER_MSB },
        { "MIDICTRL_PORTAMENTO_TIME_MSB", MidiControl::MIDICTRL_PORTAMENTO_TIME_MSB },
        { "MIDICTRL_DATA_ENTRY_MSB", MidiControl::MIDICTRL_DATA_ENTRY_MSB },
        { "MIDICTRL_MAIN_VOLUME_MSB", MidiControl::MIDICTRL_MAIN_VOLUME_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER8_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER8_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER9_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER9_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER10_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER10_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER11_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER11_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER12_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER12_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER13_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER13_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER14_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER14_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER15_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER15_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER16_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER16_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER17_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER17_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER18_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER18_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER19_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER19_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER20_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER20_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER21_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER21_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER22_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER22_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER23_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER23_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER24_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER24_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER25_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER25_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER26_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER26_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER27_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER27_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER28_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER28_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER29_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER29_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER30_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER30_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER31_MSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER31_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER0_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER0_LSB },
        { "MIDICTRL_MODULATION_WHEEL_LSB", MidiControl::MIDICTRL_MODULATION_WHEEL_LSB },
        { "MIDICTRL_BREATH_CONTROL_LSB", MidiControl::MIDICTRL_BREATH_CONTROL_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER3_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER3_LSB },
        { "MIDICTRL_FOOT_CONTROLLER_LSB", MidiControl::MIDICTRL_FOOT_CONTROLLER_LSB },
        { "MIDICTRL_PORTAMENTO_TIME_LSB", MidiControl::MIDICTRL_PORTAMENTO_TIME_LSB },
        { "MIDICTRL_DATA_ENTRY_LSB", MidiControl::MIDICTRL_DATA_ENTRY_LSB },
        { "MIDICTRL_MAIN_VOLUME_LSB", MidiControl::MIDICTRL_MAIN_VOLUME_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER8_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER8_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER9_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER9_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER10_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER10_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER11_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER11_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER12_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER12_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER13_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER13_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER14_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER14_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER15_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER15_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER16_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER16_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER17_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER17_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER18_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER18_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER19_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER19_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER20_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER20_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER21_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER21_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER22_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER22_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER23_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER23_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER24_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER24_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER25_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER25_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER26_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER26_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER27_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER27_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER28_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER28_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER29_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER29_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER30_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER30_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER31_LSB", MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER31_LSB },
        { "MIDICTRL_DAMPER_PEDAL_ON_OFF", MidiControl::MIDICTRL_DAMPER_PEDAL_ON_OFF },
        { "MIDICTRL_PORTAMENTO_ON_OFF", MidiControl::MIDICTRL_PORTAMENTO_ON_OFF },
        { "MIDICTRL_SUSTENUTO_ON_OFF", MidiControl::MIDICTRL_SUSTENUTO_ON_OFF },
        { "MIDICTRL_SOFT_PEDAL_ON_OFF", MidiControl::MIDICTRL_SOFT_PEDAL_ON_OFF },
        { "MIDICTRL_LOCAL_CONTROL_ON_OFF", MidiControl::MIDICTRL_LOCAL_CONTROL_ON_OFF },
        { "MIDICTRL_ALL_NOTES_OFF", MidiControl::MIDICTRL_ALL_NOTES_OFF },
        { "MIDICTRL_OMNI_MODE_OFF", MidiControl::MIDICTRL_OMNI_MODE_OFF }, { "MIDICTRL_OMNI_MODE_ON", MidiControl::MIDICTRL_OMNI_MODE_ON },
        { "MIDICTRL_POLY_MODE_ON_OFF", MidiControl::MIDICTRL_POLY_MODE_ON_OFF },
        { "MIDICTRL_POLY_MODE_ON", MidiControl::MIDICTRL_POLY_MODE_ON }, { "MIDICTRL_CRTL_UNDEFINED", MidiControl::MIDICTRL_CRTL_UNDEFINED }
    };

    const std::map<MidiControl, std::string> ctrl_to_token = {
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER0_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER0_MSB" },
        { MidiControl::MIDICTRL_MODULATION_WHEEL_MSB, "MIDICTRL_MODULATION_WHEEL_MSB" },
        { MidiControl::MIDICTRL_BREATH_CONTROL_MSB, "MIDICTRL_BREATH_CONTROL_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER3_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER3_MSB" },
        { MidiControl::MIDICTRL_FOOT_CONTROLLER_MSB, "MIDICTRL_FOOT_CONTROLLER_MSB" },
        { MidiControl::MIDICTRL_PORTAMENTO_TIME_MSB, "MIDICTRL_PORTAMENTO_TIME_MSB" },
        { MidiControl::MIDICTRL_DATA_ENTRY_MSB, "MIDICTRL_DATA_ENTRY_MSB" },
        { MidiControl::MIDICTRL_MAIN_VOLUME_MSB, "MIDICTRL_MAIN_VOLUME_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER8_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER8_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER9_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER9_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER10_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER10_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER11_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER11_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER12_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER12_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER13_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER13_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER14_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER14_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER15_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER15_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER16_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER16_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER17_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER17_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER18_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER18_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER19_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER19_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER20_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER20_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER21_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER21_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER22_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER22_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER23_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER23_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER24_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER24_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER25_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER25_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER26_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER26_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER27_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER27_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER28_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER28_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER29_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER29_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER30_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER30_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER31_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER31_MSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER0_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER0_LSB" },
        { MidiControl::MIDICTRL_MODULATION_WHEEL_LSB, "MIDICTRL_MODULATION_WHEEL_LSB" },
        { MidiControl::MIDICTRL_BREATH_CONTROL_LSB, "MIDICTRL_BREATH_CONTROL_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER3_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER3_LSB" },
        { MidiControl::MIDICTRL_FOOT_CONTROLLER_LSB, "MIDICTRL_FOOT_CONTROLLER_LSB" },
        { MidiControl::MIDICTRL_PORTAMENTO_TIME_LSB, "MIDICTRL_PORTAMENTO_TIME_LSB" },
        { MidiControl::MIDICTRL_DATA_ENTRY_LSB, "MIDICTRL_DATA_ENTRY_LSB" },
        { MidiControl::MIDICTRL_MAIN_VOLUME_LSB, "MIDICTRL_MAIN_VOLUME_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER8_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER8_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER9_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER9_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER10_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER10_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER11_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER11_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER12_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER12_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER13_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER13_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER14_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER14_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER15_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER15_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER16_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER16_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER17_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER17_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER18_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER18_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER19_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER19_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER20_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER20_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER21_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER21_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER22_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER22_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER23_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER23_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER24_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER24_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER25_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER25_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER26_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER26_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER27_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER27_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER28_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER28_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER29_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER29_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER30_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER30_LSB" },
        { MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER31_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER31_LSB" },
        { MidiControl::MIDICTRL_DAMPER_PEDAL_ON_OFF, "MIDICTRL_DAMPER_PEDAL_ON_OFF" },
        { MidiControl::MIDICTRL_PORTAMENTO_ON_OFF, "MIDICTRL_PORTAMENTO_ON_OFF" },
        { MidiControl::MIDICTRL_SUSTENUTO_ON_OFF, "MIDICTRL_SUSTENUTO_ON_OFF" },
        { MidiControl::MIDICTRL_SOFT_PEDAL_ON_OFF, "MIDICTRL_SOFT_PEDAL_ON_OFF" },
        { MidiControl::MIDICTRL_LOCAL_CONTROL_ON_OFF, "MIDICTRL_LOCAL_CONTROL_ON_OFF" },
        { MidiControl::MIDICTRL_ALL_NOTES_OFF, "MIDICTRL_ALL_NOTES_OFF" },
        { MidiControl::MIDICTRL_OMNI_MODE_OFF, "MIDICTRL_OMNI_MODE_OFF" }, { MidiControl::MIDICTRL_OMNI_MODE_ON, "MIDICTRL_OMNI_MODE_ON" },
        { MidiControl::MIDICTRL_POLY_MODE_ON_OFF, "MIDICTRL_POLY_MODE_ON_OFF" },
        { MidiControl::MIDICTRL_POLY_MODE_ON, "MIDICTRL_POLY_MODE_ON" }, { MidiControl::MIDICTRL_CRTL_UNDEFINED, "MIDICTRL_CRTL_UNDEFINED" }
    };

}

// TODO: Isolate this code in separate class files, keeping the main.cpp minimalistic
// Note: as it was developed during a dingle sunday afternoon I was a bit lazy to organize it better

class ParseAndTrigger
{
public:
    ParseAndTrigger() = delete;
    ParseAndTrigger(const ParseAndTrigger&) = delete;
    ParseAndTrigger(ParseAndTrigger&&) = delete;
    ParseAndTrigger& operator=(const ParseAndTrigger&) = delete;
    ParseAndTrigger& operator=(ParseAndTrigger&&) = delete;

    explicit ParseAndTrigger(const std::string& p_name)
        : m_stop(false)
    {
        m_parsed = m_parser.parse_file(p_name);

#if defined(_WIN32)
        m_midi.emplace_back(std::make_unique<RtMidiIn>(RtMidi::WINDOWS_MM));
#elif defined(__linux__)
        m_midi.emplace_back(std::make_unique<RtMidiIn>(RtMidi::LINUX_ALSA));
#else
        m_midi.emplace_back(std::make_unique<RtMidiIn>());
#endif

        m_nb_ports = m_midi[0]->getPortCount();
        std::cout << "Nb of input ports available " << m_nb_ports << std::endl;

        std::string port_name;
        for (unsigned int i = 0; i < m_nb_ports; ++i)
        {
            port_name = m_midi[0]->getPortName(i);
            std::cout << " Input port #" << i << " " << port_name << std::endl;
            m_midi_names.emplace_back(port_name);
        }

        for (unsigned int i = 1; i < m_nb_ports; ++i)
        {
#if defined(_WIN32)
            m_midi.emplace_back(std::make_unique<RtMidiIn>(RtMidi::WINDOWS_MM));
#elif defined(__linux__)
            m_midi.emplace_back(std::make_unique<RtMidiIn>(RtMidi::LINUX_ALSA));
#else
            m_midi.emplace_back(std::make_unique<RtMidiIn>());
#endif
        }

        setup();
    }

    ~ParseAndTrigger() { teardown(); }

    void setup()
    {
        for (size_t i = 0; i < static_cast<size_t>(m_parsed["Triggers"].size()); ++i)
        {
            auto token = m_parsed["Triggers"][i]["Input"].as_string();
            auto ctrl = token_to_ctrl.at(token);

            m_index_table.emplace(ctrl, i);
            m_flipflop.emplace(ctrl, false);
            m_cache.emplace(ctrl, 0);
        }

        if (m_nb_ports > 0)
        {
            for (unsigned int i = 0; i < m_nb_ports; ++i)
            {
                std::cout << " Opening port #" << i << " " << m_midi_names[i] << std::endl;
                m_midi[i]->openPort(i);

                // Set our callback function.  This should be done immediately after
                // opening the port to avoid having incoming messages written to the
                // queue instead of sent to the callback function.
                m_midi[i]->setCallback(midi_cb, this);

                // Don't ignore sysex, timing, or active sensing messages.
                m_midi[i]->ignoreTypes(false, false, false);
            }
        }
    }


    void teardown()
    {
        for (unsigned int i = 0; i < m_nb_ports; ++i)
        {
            m_midi[i]->closePort();
        }
    }

    void process_message(double /*delta_time*/, const std::vector<unsigned char>& message)
    {
        std::string label;

        try
        {
            // http://www.midimountain.com/midi/midi_note_numbers.html
            // https://www.midi.org/specifications/item/table-1-summary-of-midi-message

            if ((message[0] >= 0xB0) && (message[0] <= 0xBF)) // Channel aftertouch on channel 1 - 16
            {
                // MIDI_CONTROL_MODE_CHANGE
                // std::uint8_t channel = message[0] & 15;
                std::uint8_t data0 = message[1];
                std::uint8_t data1 = message[2];

                MidiControl function = static_cast<MidiControl>(data0);

                /* Korg Nano Kontrol 2*/
                if ((function >= MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER0_MSB)
                    && (function <= MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER31_MSB))
                {
                    // MIDICTRL_CONTINUOUS_CONTROLLER16_MSB to MIDICTRL_CONTINUOUS_CONTROLLER23_MSB : volume buttons
                    // MIDICTRL_CONTINUOUS_CONTROLLER0_MSB to MIDICTRL_CONTINUOUS_CONTROLLER7_MSB : sliders

                    label = ctrl_to_token.at(function);
                }
                else if ((function >= MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER0_LSB)
                    && (function <= MidiControl::MIDICTRL_CONTINUOUS_CONTROLLER31_LSB))
                {
                    // MIDICTRL_CONTINUOUS_CONTROLLER0_LSB to MIDICTRL_CONTINUOUS_CONTROLLER7_LSB : buttons "S"
                    // MIDICTRL_CONTINUOUS_CONTROLLER16_LSB to MIDICTRL_CONTINUOUS_CONTROLLER23_LSB : buttons "M"
                    // MIDICTRL_CONTINUOUS_CONTROLLER26_LSB : track -
                    // MIDICTRL_CONTINUOUS_CONTROLLER27_LSB : track +
                    // MIDICTRL_CONTINUOUS_CONTROLLER10_LSB : stop
                    // MIDICTRL_CONTINUOUS_CONTROLLER9_LSB : play

                    label = ctrl_to_token.at(function);
                }
                else if (((function >= MidiControl::MIDICTRL_DAMPER_PEDAL_ON_OFF) && (function <= MidiControl::MIDICTRL_SOFT_PEDAL_ON_OFF))
                    || (function == MidiControl::MIDICTRL_LOCAL_CONTROL_ON_OFF))
                {
                    // MIDICTRL_DAMPER_PEDAL_ON_OFF : 1st button "R"

                    label = ctrl_to_token.at(function);
                }

                if (!label.empty())
                {
                    const auto entry = m_index_table.at(function);
                    auto command = m_parsed["Triggers"][entry]["Command"].as_string();
                    auto argument = m_parsed["Triggers"][entry]["Argument"].as_string();
                    auto inject = m_parsed["Triggers"][entry]["Inject"].as_string();
                    auto range_min = m_parsed["Triggers"][entry]["RangeMin"].as_string();
                    auto range_max = m_parsed["Triggers"][entry]["RangeMax"].as_string();
                    auto flip_flop = m_parsed["Triggers"][entry]["FlipFlop"].as_string();
                    auto up_only = m_parsed["Triggers"][entry]["UpOnly"].as_string();

                    int rmin = std::stoi(range_min);
                    int rmax = std::stoi(range_max);
                    int out_range = rmax - rmin;
                    int val = (static_cast<int>(data1) * out_range) / 127;

                    if (val != m_cache.at(function))
                    {
                        m_cache[function] = val;

                        if (flip_flop == "true")
                        {
                            if (val > 0) // and cached value is <> and so is 0
                            {
                                bool bit = !m_flipflop.at(function);
                                m_flipflop[function] = bit;
                                val = bit ? 1 : 0;
                            }
                            else
                            {
                                return;
                            }
                        }
                        else if (up_only == "true")
                        {
                            if (val == 0)
                            {
                                return;
                            }
                        }

                        auto replace = std::to_string(val);
                        argument = std::regex_replace(argument, std::regex(inject), replace);

                        std::string cmd(command + " " + argument);
                        // std::cout << cmd << std::endl;

                        const auto retval = system(cmd.c_str());
                        if (retval < 0)
                        {
                            std::cerr << "system call to " << cmd.c_str() << " failed" << std::endl;
                            return;
                        }
                    }
                }
            }
            // else if ((message[0] >= 0x80) && (message[0] <= 0x8F)) // Note Off on channel 1 - 16
            //{
            // MIDI_NOTE_OFF
            // std::uint8_t channel = message[0] & 15;
            // std::uint8_t data0 = message[1];
            // std::uint8_t data1 = message[2];

            // TODO : process
            //}
            // else if ((message[0] >= 0x90) && (message[0] <= 0x9F)) // Note On on channel 1 - 16
            //{
            // MIDI_NOTE_ON
            //    std::uint8_t channel = message[0] & 15;
            //    std::uint8_t data0 = message[1];
            //    std::uint8_t data1 = message[2];

            // TODO : process
            //}
            // else if ((message[0] >= 0xA0) && (message[0] <= 0xAF)) // Polyphonic aftertouch on channel 1 - 16
            //{
            // MIDI_POLYPHONIC_AFTERTOUCH
            //    std::uint8_t channel = message[0] & 15;
            //    std::uint8_t data0 = message[1];
            //    std::uint8_t data1 = message[2];

            // TODO : process
            //}
            // else if ((message[0] >= 0xD0) && (message[0] <= 0xDF)) // Channel aftertouch on channel 1 - 16
            //{
            // MIDI_CHANNEL_AFTERTOUCH
            //    std::uint8_t channel = message[0] & 15;
            //    std::uint8_t data0 = message[1];
            //    std::uint8_t data1 = message[2];

            // TODO : process
            //}
            // else if ((message[0] >= 0xE0) && (message[0] <= 0xEF)) // Pitch wheel range on channel 1 - 16
            //{
            // MIDI_PITCH_WHEEL_RANGE
            //    std::uint8_t channel = message[0] & 15;
            //    std::uint8_t data0 = message[1];
            //    std::uint8_t data1 = message[2];

            // TODO : process
            //}
        }

        catch (std::exception& e)
        {
            std::cerr << "unknown mapping (" << label << ") - exception " << e.what() << std::endl;
        }
    }

    void run()
    {
        while (!m_stop.load())
        {
            // Sleep for 10 milliseconds.
            SLEEP(10);
        }
    }

    void stop() { m_stop.store(true); }

private:
    jute::parser m_parser;
    jute::jValue m_parsed;
    std::map<MidiControl, size_t> m_index_table;
    std::map<MidiControl, bool> m_flipflop;
    std::map<MidiControl, int> m_cache;
    std::vector<std::unique_ptr<RtMidiIn> > m_midi;
    std::vector<std::string> m_midi_names;
    unsigned int m_nb_ports;
    std::atomic_bool m_stop;

    static void midi_cb(double delta_time, std::vector<unsigned char>* message, void* user_data)
    {
        if (user_data && message)
        {
            auto* instance = reinterpret_cast<ParseAndTrigger*>(user_data);
            if (message->size() > 0)
            {
                instance->process_message(delta_time, *message);
            }
        }
    }
};


#if defined(__linux__)
static ParseAndTrigger* g_instance = nullptr;

static void finish(int /*ignore*/)
{
    if (g_instance)
    {
        g_instance->stop();
    }
}
#endif

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        auto mng = std::make_unique<ParseAndTrigger>(argv[1]);

#if defined(__linux__)
        g_instance = mng.get();
        (void)signal(SIGINT, finish);
#endif

        mng->run();
    }

    return 0;
}