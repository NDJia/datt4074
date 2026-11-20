#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;


int main(void)
{
    // initialize seed hardware and daisysp modules
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);

    Switch button;
    button.Init(hw.GetPin(25));

    // hardware processing should go in the main loop
    while(1) {
        button.Debounce();
        if (button.Pressed())
        {
            hw.SetLed(true);
            
        }
        else
        {
            hw.SetLed(false);
        }
        System::Delay(1);
    }
}
