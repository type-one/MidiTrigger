//-----------------------------------------------------------------------------//
// Midi Triggered Commands - Spare time development for fun                    //
// (c) 2021-2023 Laurent Lardinois https://be.linkedin.com/in/laurentlardinois //
//                                                                             //
// https://github.com/type-one/MidiTrigger                                     //
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

#include "jute/jute.h"
#include "rtmidi/RtMidi.h"

#include "midi/parse_and_trigger.hpp"

#include <memory>
#include <utility>

#if defined(__linux__)
#include <signal.h>
#endif

#if defined(__linux__)
static parse_and_trigger* g_instance = nullptr;

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
        auto mng = std::make_unique<parse_and_trigger>(argv[1]);

#if defined(__linux__)
        g_instance = mng.get();
        (void)signal(SIGINT, finish);
#endif

        mng->run();
    }

    return 0;
}
