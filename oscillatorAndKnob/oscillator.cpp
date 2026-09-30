#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process();

        // left out
        out[i] = sig;

        // right out
        out[i + 1] = sig;
    }
}

int main(void)
{
    // initialize seed hardware and oscillator daisysp module
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);

    //This is our ADC configuration
    AdcChannelConfig adcConfig;
    //Configure pin as an ADC input. This is where we'll read the knob.
    adcConfig.InitSingle(hw.GetPin(23));

    //Initialize the adc with the config we just made
    hw.adc.Init(&adcConfig, 1);
    //Start reading values
    hw.adc.Start();

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.1);



    // start callback
    hw.StartAudio(AudioCallback);


    while(1) {
        osc.SetAmp(hw.adc.GetFloat(0));
        hw.ChangeAudioCallback(AudioCallback);
        System::Delay(1);
    }
}
