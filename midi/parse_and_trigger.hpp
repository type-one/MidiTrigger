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

#pragma once

#if !defined(__PARSE_AND_TRIGGER_HPP__)
#define __PARSE_AND_TRIGGER_HPP__

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "jute/jute.h"
#include "midi_events.hpp"
#include "rtmidi/RtMidi.h"

class parse_and_trigger
{
public:
    parse_and_trigger() = delete;
    parse_and_trigger(const parse_and_trigger&) = delete;
    parse_and_trigger(parse_and_trigger&&) = delete;
    parse_and_trigger& operator=(const parse_and_trigger&) = delete;
    parse_and_trigger& operator=(parse_and_trigger&&) = delete;

    explicit parse_and_trigger(const std::string& p_name);
    ~parse_and_trigger();

    void setup();
    void teardown();
    void process_message(double /*delta_time*/, const std::vector<unsigned char>& message);
    void run();
    void stop();

private:
    jute::parser m_parser;
    jute::jValue m_parsed;
    std::map<midi_control, size_t> m_index_table;
    std::map<midi_control, bool> m_flipflop;
    std::map<midi_control, int> m_cache;
    std::vector<std::unique_ptr<RtMidiIn> > m_midi;
    std::vector<std::string> m_midi_names;
    unsigned int m_nb_ports;
    std::atomic_bool m_stop;

    static void midi_cb(double delta_time, std::vector<unsigned char>* message, void* user_data);
};


#endif // __PARSE_AND_TRIGGER_HPP__
