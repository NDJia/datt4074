#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc, lfo;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process() * lfo.Process();

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
    lfo.Init(sample_rate);

    //This is our ADC configuration
    AdcChannelConfig adcConfig;
    //Configure pin as an ADC input. This is where we'll read the knob.
    adcConfig.InitSingle(hw.GetPin(25));

    //Initialize the adc with the config we just made
    hw.adc.Init(&adcConfig, 1);
    //Start reading values
    hw.adc.Start();

    //Configuration for button
    Switch button;
    button.Init(hw.GetPin(26));

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.75);

    lfo.SetWaveform(osc.WAVE_TRI);
    osc.SetFreq(0.1);
    osc.SetAmp(1.0);

    // start callback
    hw.StartAudio(AudioCallback);


    while(1) {
        // button.Debounce();
        // if (button.Pressed())
        // {
        //     osc.SetAmp(lfo.GetFloat(0)*0.75);
        // }
        // else
        // {
        //     osc.SetAmp(0.75);
        // }
        lfo.SetFreq(0.1f + hw.adc.GetFloat(0)*5.0f);
        if (lfo.IsRising()) hw.SetLed(true);
        else hw.SetLed(false);
        System::Delay(1);
    }
}
