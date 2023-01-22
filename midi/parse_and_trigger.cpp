//-----------------------------------------------------------------------------//
// Midi Triggered Commands - Spare time development for fun                    //
// (c) 2021-2023 Laurent Lardinois https://be.linkedin.com/in/laurentlardinois //
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

#include "parse_and_trigger.hpp"
#include "jute/jute.h"
#include "midi_events.hpp"
#include "rtmidi/RtMidi.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
    const std::map<std::string, midi_control> token_to_ctrl = {
        { "MIDICTRL_CONTINUOUS_CONTROLLER0_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER0_MSB },
        { "MIDICTRL_MODULATION_WHEEL_MSB", midi_control::MIDICTRL_MODULATION_WHEEL_MSB },
        { "MIDICTRL_BREATH_CONTROL_MSB", midi_control::MIDICTRL_BREATH_CONTROL_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER3_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER3_MSB },
        { "MIDICTRL_FOOT_CONTROLLER_MSB", midi_control::MIDICTRL_FOOT_CONTROLLER_MSB },
        { "MIDICTRL_PORTAMENTO_TIME_MSB", midi_control::MIDICTRL_PORTAMENTO_TIME_MSB },
        { "MIDICTRL_DATA_ENTRY_MSB", midi_control::MIDICTRL_DATA_ENTRY_MSB },
        { "MIDICTRL_MAIN_VOLUME_MSB", midi_control::MIDICTRL_MAIN_VOLUME_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER8_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER8_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER9_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER9_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER10_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER10_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER11_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER11_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER12_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER12_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER13_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER13_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER14_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER14_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER15_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER15_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER16_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER16_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER17_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER17_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER18_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER18_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER19_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER19_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER20_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER20_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER21_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER21_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER22_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER22_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER23_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER23_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER24_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER24_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER25_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER25_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER26_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER26_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER27_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER27_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER28_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER28_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER29_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER29_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER30_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER30_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER31_MSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER31_MSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER0_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER0_LSB },
        { "MIDICTRL_MODULATION_WHEEL_LSB", midi_control::MIDICTRL_MODULATION_WHEEL_LSB },
        { "MIDICTRL_BREATH_CONTROL_LSB", midi_control::MIDICTRL_BREATH_CONTROL_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER3_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER3_LSB },
        { "MIDICTRL_FOOT_CONTROLLER_LSB", midi_control::MIDICTRL_FOOT_CONTROLLER_LSB },
        { "MIDICTRL_PORTAMENTO_TIME_LSB", midi_control::MIDICTRL_PORTAMENTO_TIME_LSB },
        { "MIDICTRL_DATA_ENTRY_LSB", midi_control::MIDICTRL_DATA_ENTRY_LSB },
        { "MIDICTRL_MAIN_VOLUME_LSB", midi_control::MIDICTRL_MAIN_VOLUME_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER8_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER8_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER9_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER9_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER10_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER10_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER11_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER11_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER12_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER12_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER13_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER13_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER14_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER14_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER15_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER15_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER16_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER16_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER17_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER17_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER18_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER18_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER19_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER19_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER20_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER20_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER21_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER21_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER22_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER22_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER23_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER23_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER24_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER24_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER25_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER25_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER26_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER26_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER27_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER27_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER28_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER28_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER29_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER29_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER30_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER30_LSB },
        { "MIDICTRL_CONTINUOUS_CONTROLLER31_LSB", midi_control::MIDICTRL_CONTINUOUS_CONTROLLER31_LSB },
        { "MIDICTRL_DAMPER_PEDAL_ON_OFF", midi_control::MIDICTRL_DAMPER_PEDAL_ON_OFF },
        { "MIDICTRL_PORTAMENTO_ON_OFF", midi_control::MIDICTRL_PORTAMENTO_ON_OFF },
        { "MIDICTRL_SUSTENUTO_ON_OFF", midi_control::MIDICTRL_SUSTENUTO_ON_OFF },
        { "MIDICTRL_SOFT_PEDAL_ON_OFF", midi_control::MIDICTRL_SOFT_PEDAL_ON_OFF },
        { "MIDICTRL_LOCAL_CONTROL_ON_OFF", midi_control::MIDICTRL_LOCAL_CONTROL_ON_OFF },
        { "MIDICTRL_ALL_NOTES_OFF", midi_control::MIDICTRL_ALL_NOTES_OFF },
        { "MIDICTRL_OMNI_MODE_OFF", midi_control::MIDICTRL_OMNI_MODE_OFF }, { "MIDICTRL_OMNI_MODE_ON", midi_control::MIDICTRL_OMNI_MODE_ON },
        { "MIDICTRL_POLY_MODE_ON_OFF", midi_control::MIDICTRL_POLY_MODE_ON_OFF },
        { "MIDICTRL_POLY_MODE_ON", midi_control::MIDICTRL_POLY_MODE_ON }, { "MIDICTRL_CRTL_UNDEFINED", midi_control::MIDICTRL_CRTL_UNDEFINED }
    };

    const std::map<midi_control, std::string> ctrl_to_token = {
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER0_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER0_MSB" },
        { midi_control::MIDICTRL_MODULATION_WHEEL_MSB, "MIDICTRL_MODULATION_WHEEL_MSB" },
        { midi_control::MIDICTRL_BREATH_CONTROL_MSB, "MIDICTRL_BREATH_CONTROL_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER3_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER3_MSB" },
        { midi_control::MIDICTRL_FOOT_CONTROLLER_MSB, "MIDICTRL_FOOT_CONTROLLER_MSB" },
        { midi_control::MIDICTRL_PORTAMENTO_TIME_MSB, "MIDICTRL_PORTAMENTO_TIME_MSB" },
        { midi_control::MIDICTRL_DATA_ENTRY_MSB, "MIDICTRL_DATA_ENTRY_MSB" },
        { midi_control::MIDICTRL_MAIN_VOLUME_MSB, "MIDICTRL_MAIN_VOLUME_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER8_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER8_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER9_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER9_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER10_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER10_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER11_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER11_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER12_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER12_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER13_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER13_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER14_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER14_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER15_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER15_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER16_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER16_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER17_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER17_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER18_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER18_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER19_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER19_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER20_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER20_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER21_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER21_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER22_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER22_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER23_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER23_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER24_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER24_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER25_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER25_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER26_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER26_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER27_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER27_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER28_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER28_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER29_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER29_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER30_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER30_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER31_MSB, "MIDICTRL_CONTINUOUS_CONTROLLER31_MSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER0_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER0_LSB" },
        { midi_control::MIDICTRL_MODULATION_WHEEL_LSB, "MIDICTRL_MODULATION_WHEEL_LSB" },
        { midi_control::MIDICTRL_BREATH_CONTROL_LSB, "MIDICTRL_BREATH_CONTROL_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER3_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER3_LSB" },
        { midi_control::MIDICTRL_FOOT_CONTROLLER_LSB, "MIDICTRL_FOOT_CONTROLLER_LSB" },
        { midi_control::MIDICTRL_PORTAMENTO_TIME_LSB, "MIDICTRL_PORTAMENTO_TIME_LSB" },
        { midi_control::MIDICTRL_DATA_ENTRY_LSB, "MIDICTRL_DATA_ENTRY_LSB" },
        { midi_control::MIDICTRL_MAIN_VOLUME_LSB, "MIDICTRL_MAIN_VOLUME_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER8_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER8_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER9_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER9_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER10_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER10_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER11_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER11_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER12_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER12_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER13_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER13_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER14_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER14_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER15_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER15_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER16_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER16_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER17_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER17_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER18_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER18_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER19_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER19_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER20_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER20_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER21_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER21_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER22_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER22_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER23_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER23_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER24_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER24_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER25_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER25_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER26_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER26_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER27_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER27_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER28_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER28_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER29_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER29_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER30_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER30_LSB" },
        { midi_control::MIDICTRL_CONTINUOUS_CONTROLLER31_LSB, "MIDICTRL_CONTINUOUS_CONTROLLER31_LSB" },
        { midi_control::MIDICTRL_DAMPER_PEDAL_ON_OFF, "MIDICTRL_DAMPER_PEDAL_ON_OFF" },
        { midi_control::MIDICTRL_PORTAMENTO_ON_OFF, "MIDICTRL_PORTAMENTO_ON_OFF" },
        { midi_control::MIDICTRL_SUSTENUTO_ON_OFF, "MIDICTRL_SUSTENUTO_ON_OFF" },
        { midi_control::MIDICTRL_SOFT_PEDAL_ON_OFF, "MIDICTRL_SOFT_PEDAL_ON_OFF" },
        { midi_control::MIDICTRL_LOCAL_CONTROL_ON_OFF, "MIDICTRL_LOCAL_CONTROL_ON_OFF" },
        { midi_control::MIDICTRL_ALL_NOTES_OFF, "MIDICTRL_ALL_NOTES_OFF" },
        { midi_control::MIDICTRL_OMNI_MODE_OFF, "MIDICTRL_OMNI_MODE_OFF" }, { midi_control::MIDICTRL_OMNI_MODE_ON, "MIDICTRL_OMNI_MODE_ON" },
        { midi_control::MIDICTRL_POLY_MODE_ON_OFF, "MIDICTRL_POLY_MODE_ON_OFF" },
        { midi_control::MIDICTRL_POLY_MODE_ON, "MIDICTRL_POLY_MODE_ON" }, { midi_control::MIDICTRL_CRTL_UNDEFINED, "MIDICTRL_CRTL_UNDEFINED" }
    };

}


parse_and_trigger::parse_and_trigger(const std::string& p_name)
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

parse_and_trigger::~parse_and_trigger() { teardown(); }

void parse_and_trigger::setup()
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

void parse_and_trigger::teardown()
{
    for (unsigned int i = 0; i < m_nb_ports; ++i)
    {
        m_midi[i]->closePort();
    }
}

void parse_and_trigger::process_message(double /*delta_time*/, const std::vector<unsigned char>& message)
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

            midi_control function = static_cast<midi_control>(data0);

            /* Korg Nano Kontrol 2*/
            if ((function >= midi_control::MIDICTRL_CONTINUOUS_CONTROLLER0_MSB)
                && (function <= midi_control::MIDICTRL_CONTINUOUS_CONTROLLER31_MSB))
            {
                // MIDICTRL_CONTINUOUS_CONTROLLER16_MSB to MIDICTRL_CONTINUOUS_CONTROLLER23_MSB : volume buttons
                // MIDICTRL_CONTINUOUS_CONTROLLER0_MSB to MIDICTRL_CONTINUOUS_CONTROLLER7_MSB : sliders

                label = ctrl_to_token.at(function);
            }
            else if ((function >= midi_control::MIDICTRL_CONTINUOUS_CONTROLLER0_LSB)
                && (function <= midi_control::MIDICTRL_CONTINUOUS_CONTROLLER31_LSB))
            {
                // MIDICTRL_CONTINUOUS_CONTROLLER0_LSB to MIDICTRL_CONTINUOUS_CONTROLLER7_LSB : buttons "S"
                // MIDICTRL_CONTINUOUS_CONTROLLER16_LSB to MIDICTRL_CONTINUOUS_CONTROLLER23_LSB : buttons "M"
                // MIDICTRL_CONTINUOUS_CONTROLLER26_LSB : track -
                // MIDICTRL_CONTINUOUS_CONTROLLER27_LSB : track +
                // MIDICTRL_CONTINUOUS_CONTROLLER10_LSB : stop
                // MIDICTRL_CONTINUOUS_CONTROLLER9_LSB : play

                label = ctrl_to_token.at(function);
            }
            else if (((function >= midi_control::MIDICTRL_DAMPER_PEDAL_ON_OFF) && (function <= midi_control::MIDICTRL_SOFT_PEDAL_ON_OFF))
                || (function == midi_control::MIDICTRL_LOCAL_CONTROL_ON_OFF))
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

void parse_and_trigger::run()
{
    while (!m_stop.load())
    {
        std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
    }
}

void parse_and_trigger::stop() { m_stop.store(true); }

void parse_and_trigger::midi_cb(double delta_time, std::vector<unsigned char>* message, void* user_data)
{
    if (user_data && message)
    {
        auto* instance = reinterpret_cast<parse_and_trigger*>(user_data);
        if (message->size() > 0)
        {
            instance->process_message(delta_time, *message);
        }
    }
}
