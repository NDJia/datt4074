#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;

// Oscillators:
//    - osc and osc2: used for additive synthesis
//    - ringmod: LFO used for ring modulation
static Oscillator osc, ringmod, osc2;
static Line       line;
static Adsr       adsr;
static Biquad     flt;
static ReverbSc   reverb;
static Metro      tick1;

#define PI 3.14
bool gate = false;
float mainVolume = 1.0;
float phaseShift = 0.0;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float osc_out, osc2_out, ringmod_out;
    float osc_sum, output_dry, output_wet;
    for(size_t i = 0; i < size; i += 2)
    {
        // flt.SetRes(adsr.Process(gate)*100.0 + 15.0);
        if (tick1.Process())
            gate = !gate;

        osc.PhaseAdd(phaseShift * adsr.Process(gate));

        osc_out = osc.Process();
        osc2_out = osc2.Process();

        osc_sum = (osc_out + osc2_out) / 2.0f;

        // flt.SetCutoff(flt_freq);

        ringmod_out = ringmod.Process();

        output_dry = osc_sum * ringmod_out;

        reverb.Process(output_dry, output_dry, &output_wet, &output_wet);

        // left out
        out[i] = (output_dry + output_wet) * mainVolume;
        // out[i] = (osc_sum * 0.2);
        // right out
        out[i + 1] = (output_dry + output_wet) * mainVolume;
        // out[i + 1] = osc_sum * 0.2;
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
    osc2.SetFreq(440);
    osc2.SetAmp(0.1);

    // initialize the low frequency oscillator
    ringmod.Init(sample_rate);
    ringmod.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    ringmod.SetFreq(0.08);
    ringmod.SetAmp(0.3);

    // initialize biquad
    flt.Init(sample_rate);
    flt.SetCutoff(500);

    //setup metro
    tick1.Init(2.0f, sample_rate);

    // intialize line
    line.Init(sample_rate);

    //setup reverb
    reverb.Init(sample_rate);
    reverb.SetFeedback(0.9f);
    reverb.SetLpFreq(19000.0f);

    // initialize ADSR
    adsr.Init(sample_rate);
    adsr.SetTime(ADSR_SEG_ATTACK, .6);
    adsr.SetTime(ADSR_SEG_DECAY, 2.2);
    adsr.SetTime(ADSR_SEG_RELEASE, 0.5);

    adsr.SetSustainLevel(0.7);

    // == Setup for two hardware knobs
    AdcChannelConfig knobs[4];
    knobs[0].InitSingle(hw.GetPin(15));
    knobs[1].InitSingle(hw.GetPin(25));
    knobs[2].InitSingle(hw.GetPin(16));
    knobs[3].InitSingle(hw.GetPin(17));

    //Initialize the adc with the config
    hw.adc.Init(knobs, 4);
    //Start reading values
    hw.adc.Start();

    // button setup
    Switch button;
    // button.Init(hw.GetPin(26));

    // start audio callback
    hw.StartAudio(AudioCallback);


    // hardware processing should go in this loop
    while(1) {
        // button.Debounce();
        float knob1Pos = hw.adc.GetFloat(0);
        float knob2Pos = hw.adc.GetFloat(1);
        float knob3Pos = hw.adc.GetFloat(2);
        float knob4Pos = hw.adc.GetFloat(3);

        // if (button.Pressed())
        // {
        //     gate = true;
        //     hw.SetLed(true);
        // }

        // else
        // {
        //     gate = false;
        //     hw.SetLed(false);
        // }

        osc.SetFreq(660.0f + knob1Pos*220.0f);
        osc2.SetFreq(440.0f + knob2Pos*440.0f);
        // ringmod.SetAmp(knob3Pos*0.9f);


        // mainVolume = knob3Pos;
        reverb.SetFeedback(knob3Pos*0.98f);
        phaseShift = knob4Pos / 10.0f;

        System::Delay(1);
    }
}
