#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc, pan, osc2;
static Line       line;

#define PI 3.14

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float osc_out, osc2_out, pan_out;
    for(size_t i = 0; i < size; i += 2)
    {
        osc_out = osc.Process();
        osc2_out = osc2.Process();

        pan_out = (1 + pan.Process()) / 2.0f;

        // left out
        out[i] = ((osc_out * pan_out) + (osc2_out * (1 - pan_out))) / 2.0f;
        // right out
        out[i + 1] = ((osc2_out * pan_out) + (osc_out * (1 - pan_out))) / 2.0f;
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

    // initialize the sine oscillator
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.1);

    // initialize the other sine oscillator
    osc2.Init(sample_rate);
    osc2.SetWaveform(Oscillator::WAVE_SIN);
    osc2.SetFreq(480);
    osc2.SetAmp(0.1);

    // initialize the low frequency oscillator
    pan.Init(sample_rate);
    pan.SetWaveform(Oscillator::WAVE_SIN);
    pan.SetFreq(0.08);
    pan.SetAmp(1);

    // intialize line
    line.Init(sample_rate);

    // == Setup for two hardware knobs
    AdcChannelConfig knobs[3];
    knobs[0].InitSingle(hw.GetPin(25));
    knobs[1].InitSingle(hw.GetPin(24));
    knobs[2].InitSingle(hw.GetPin(18));

    //Initialize the adc with the config
    hw.adc.Init(knobs, 3);
    //Start reading values
    hw.adc.Start();


    // start audio callback
    hw.StartAudio(AudioCallback);


    // hardware processing should go in this loop
    while(1) {
        float knob1Pos = hw.adc.GetFloat(0);
        if (knob1Pos < 0.01)
        {
            pan.SetFreq(0);
        }
        else
        {
            pan.SetFreq(0.5 + hw.adc.GetFloat(0)*5.0f);
        }

        System::Delay(1);
    }
}
