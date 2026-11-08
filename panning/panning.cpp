#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc, ringmod, osc2;
static Line       line;
static Adsr       adsr;
static Biquad     flt;

#define PI 3.14
bool gate = false;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float osc_out, osc2_out, ringmod_out, flt_freq;
    float osc_sum, output;
    for(size_t i = 0; i < size; i += 2)
    {
        // flt.SetRes(adsr.Process(gate)*100.0 + 15.0);

        osc_out = osc.Process();
        osc2_out = osc2.Process();

        osc_sum = (osc_out + osc2_out) / 2.0f;

        // flt.SetCutoff(flt_freq);
        output = osc_sum;

        ringmod_out = ringmod.Process();

        // left out
        out[i] = output * ringmod_out;
        // right out
        out[i + 1] = output * ringmod_out;
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
    ringmod.Init(sample_rate);
    ringmod.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    ringmod.SetFreq(0.08);
    ringmod.SetAmp(0.3);

    // initialize biquad
    flt.Init(sample_rate);
    flt.SetCutoff(500);

    // intialize line
    line.Init(sample_rate);

    // initialize ADSR
    adsr.Init(sample_rate);
    adsr.SetTime(ADSR_SEG_ATTACK, 1.6);
    adsr.SetTime(ADSR_SEG_DECAY, 2.2);
    adsr.SetTime(ADSR_SEG_RELEASE, 0.5);

    adsr.SetSustainLevel(0.7);

    // == Setup for two hardware knobs
    AdcChannelConfig knobs[3];
    knobs[0].InitSingle(hw.GetPin(25));
    knobs[1].InitSingle(hw.GetPin(24));
    knobs[2].InitSingle(hw.GetPin(18));

    //Initialize the adc with the config
    hw.adc.Init(knobs, 3);
    //Start reading values
    hw.adc.Start();

    // button setup
    Switch button;
    button.Init(hw.GetPin(26));

    // start audio callback
    hw.StartAudio(AudioCallback);


    // hardware processing should go in this loop
    while(1) {
        button.Debounce();
        float knob1Pos = hw.adc.GetFloat(0);
        float knob2Pos = hw.adc.GetFloat(1);
        float knob3Pos = hw.adc.GetFloat(2);

        if (button.Pressed())
        {
            gate = true;
            hw.SetLed(true);
        }

        else
        {
            gate = false;
            hw.SetLed(false);
        }

        osc.SetFreq(660.0f + knob1Pos*660.0f);
        // osc.SetFreq(440.0f + adsr.Process(gate)*220.0f);
        osc2.SetFreq(441.0f + knob2Pos*441.0f);
        ringmod.SetFreq(2.0f + knob3Pos*3.0f);

        System::Delay(1);
    }
}
