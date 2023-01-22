//-----------------------------------------------------------------------------//
// Midi Triggered Commands - Spare time development for fun                    //
// (c) 2021-2022 Laurent Lardinois https://be.linkedin.com/in/laurentlardinois //
//                                                                             //
// This software is provided 'as-is', without any express or implied           //
// warranty.In no event will the authors be held liable for any damages        //
// arising from the use of this software.                                      //
//                                                                             //
// Permission is granted to anyone to use this software for any purpose,       //
// including commercial applications, and to alter itand redistribute it       //
// freely, subject to the following restrictions :                             //
//                                                                             //
// 1. The origin of this software must not be misrepresented; you must not     //
// claim that you wrote the original software.If you use this software         //
// in a product, an acknowledgment in the product documentation would be       //
// appreciated but is not required.                                            //
// 2. Altered source versions must be plainly marked as such, and must not be  //
// misrepresented as being the original software.                              //
// 3. This notice may not be removed or altered from any source distribution.  //
//-----------------------------------------------------------------------------//

#pragma once

#if !defined(__MIDI_EVENTS_HPP__)
#define __MIDI_EVENTS_HPP__

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

#endif // __MIDI_EVENTS_HPP__
