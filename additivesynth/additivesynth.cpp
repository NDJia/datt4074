#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator oscHigh, oscLow;
static WhiteNoise noise;


float pan;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float output, output_left, output_right, osc1_out, osc2_out;
    for(size_t i = 0; i < size; i += 2)
    {
        osc1_out = oscLow.Process();
        osc2_out = oscHigh.Process();

        output = (osc1_out + osc2_out) / 2.0f;
        // output = osc1_out * osc2_out;

        // left out
        out[i] = output;
        // right out
        out[i + 1] = output;
    }
}


int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();

    // initialize the lower-frequency oscillator
    oscLow.Init(sample_rate);
    oscLow.SetWaveform(Oscillator::WAVE_SIN);
    oscLow.SetFreq(200);
    oscLow.SetAmp(0.1);

    // initialize the higher-frequency oscillator
    oscHigh.Init(sample_rate);
    oscHigh.SetWaveform(Oscillator::WAVE_SIN);
    oscHigh.SetFreq(400);
    oscHigh.SetAmp(0.1);

    // == Setup for two hardware knobs
    AdcChannelConfig knobs[2];
    knobs[0].InitSingle(hw.GetPin(25));
    knobs[1].InitSingle(hw.GetPin(24));

    //Initialize the adc with the config
    hw.adc.Init(knobs, 2);
    //Start reading values
    hw.adc.Start();

    // limit switch setup
    Switch waveformTrigger;
    waveformTrigger.Init(hw.GetPin(26));


    // start audio callback
    hw.StartAudio(AudioCallback);


    // hardware processing should go in this loop
    while(1) {
        waveformTrigger.Debounce();

        if (waveformTrigger.Pressed())
        {
            oscLow.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
            oscLow.SetAmp(0.05);
        }
        else
        {
            oscLow.SetWaveform(Oscillator::WAVE_SIN);
            oscLow.SetAmp(0.1);
        }

        oscLow.SetFreq(220.0f + hw.adc.GetFloat(0)*220.0f);
        oscHigh.SetFreq(222.0f + hw.adc.GetFloat(1)*222.0f);

        // if (hw.adc.GetFloat(1) > 0.5)
        // {
        //     hw.SetLed(true);
        // }
        // else
        // {
        //     hw.SetLed(false);
        // }
        System::Delay(1);
    }
}
