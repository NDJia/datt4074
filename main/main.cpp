#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;

// Oscillators:
//    - osc and osc2: used for additive synthesis
//    - ringmod: LFO used for ring modulation
static Oscillator osc, ringmod, osc2, panOsc;
static Line       line;
static Adsr       fmEnv, ringmodEnv;
static Biquad     flt;
static ReverbSc   reverb;
static Metro      tick1;
static Limiter    limit;

#define PI 3.14
bool fmGate = false;
bool ringmodGate = false;
bool lineStarted = false;
bool panEnabled = false;
float mainVolume = 1.0;
float phaseShift = 0.1;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float osc_out, osc2_out, ringmod_out;
    float osc_sum, output_dry, output_wet;
    float panLeft = 0.5;
    for(size_t i = 0; i < size; i += 2)
    {
        // flt.SetRes(fmEnv.Process(fmGate)*100.0 + 15.0);
        
        // osc.PhaseAdd(-1 * phaseShift * fmEnv.Process(fmGate));

        // if (panEnabled)
        // {
        //     panLeft = panOsc.Process();
        // }

        osc_out = osc.Process();
        osc2_out = osc2.Process();

        osc_sum = (osc_out + osc2_out) / 2.0;

        // flt.SetCutoff(flt_freq);

        // ringmod.PhaseAdd(0.01 * ringmodEnv.Process(ringmodGate));
        // ringmod.SetAmp(0.4 + 0.4 * ringmodEnv.Process(ringmodGate));
        // ringmod_out = ringmod.Process();

        output_dry = osc_sum;

        // reverb.Process(output_dry, output_dry, &output_wet, &output_wet);

        // left out
        // float a = osc.Process();
        // out[i] = (output_dry + output_wet) * panLeft * mainVolume;
        out[i] = osc_out;
        // right out
        // out[i + 1] = (output_dry + output_wet) * (1.0 - panLeft) * mainVolume;
        out[i + 1] = osc_out;
    }

    // Run full output through limiter for volume control.
    limit.ProcessBlock(out, size, 0.6);
}


int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(48);
    sample_rate = hw.AudioSampleRate();

    // initialize the sine oscillator
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.1);

    // initialize the other sine oscillator
    osc2.Init(sample_rate);
    osc2.SetWaveform(Oscillator::WAVE_SIN);
    osc2.SetFreq(440.1);
    osc2.SetAmp(0.2);

    // initialize the other sine oscillator
    // panOsc.Init(sample_rate);
    // panOsc.SetWaveform(Oscillator::WAVE_SIN);
    // panOsc.SetFreq(1.2);
    // panOsc.SetAmp(1);

    // initialize the low frequency oscillator
    // ringmod.Init(sample_rate);
    // ringmod.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    // ringmod.SetFreq(1);
    // ringmod.SetAmp(0.3);

    // initialize biquad
    // flt.Init(sample_rate);
    // flt.SetCutoff(500);

    //setup metro
    // tick1.Init(2.0f, sample_rate);

    // intialize line
    // line.Init(sample_rate);

    //setup reverb
    // reverb.Init(sample_rate);
    // reverb.SetFeedback(0.9f);
    // reverb.SetLpFreq(19000.0f);

    // initialize fmEnv
    // fmEnv.Init(sample_rate);
    // fmEnv.SetTime(ADSR_SEG_ATTACK, 0.8);
    // fmEnv.SetTime(ADSR_SEG_DECAY, 1.0);
    // fmEnv.SetTime(ADSR_SEG_RELEASE, 0.5);

    // fmEnv.SetSustainLevel(0.5);

    // initialize fmEnv
    // ringmodEnv.Init(sample_rate);
    // ringmodEnv.SetTime(ADSR_SEG_ATTACK, 0.8);
    // ringmodEnv.SetTime(ADSR_SEG_DECAY, 1.0);
    // ringmodEnv.SetTime(ADSR_SEG_RELEASE, 0.5);

    // ringmodEnv.SetSustainLevel(0.5);

    // Limiter setup
    limit.Init();

    // == Setup for five hardware knobs
    AdcChannelConfig knobs[6];
    
    knobs[0].InitSingle(hw.GetPin(15));
    knobs[1].InitSingle(hw.GetPin(16)); 
    knobs[2].InitSingle(hw.GetPin(17));
    knobs[3].InitSingle(hw.GetPin(18));
    knobs[4].InitSingle(hw.GetPin(19));
    knobs[5].InitSingle(hw.GetPin(20));


    // //Initialize the adc with the config
    hw.adc.Init(knobs, 6);

    // === TEST KNOB
    // AdcChannelConfig knob;
    // knob.InitSingle(hw.GetPin(24));
    // hw.adc.Init(&knob, 1);

    //Start reading values
    hw.adc.Start();

    // button setup
    Switch button1, button2, button3, button4;
    button1.Init(hw.GetPin(0));
    button2.Init(hw.GetPin(1));
    button3.Init(hw.GetPin(2));
    button4.Init(hw.GetPin(3));

    // start audio callback
    hw.StartAudio(AudioCallback);


    // hardware processing should go in this loop
    while(1) {
        button1.Debounce();
        button2.Debounce();
        button3.Debounce();
        button4.Debounce();

        // float whiteKnobPos = hw.adc.GetFloat(0);
        // float blueKnobPos = hw.adc.GetFloat(1);
        // float highGreenKnobPos = hw.adc.GetFloat(2);
        // float highOrangeKnobPos = hw.adc.GetFloat(3);
        // float lowGreenKnobPos = hw.adc.GetFloat(4);
        // float lowOrangeKnobPos = hw.adc.GetFloat(5);


        // Upper-left button
        if (button1.Pressed())
        {
            ringmodGate = true;
            // hw.SetLed(true);
        }
        else
        {
            ringmodGate = false;
            // hw.SetLed(false);
        }

        // Upper-right button
        if (button2.Pressed())
        {
            fmGate = true;
        }
        else
        {
            fmGate = false;
        }

        // Lower-right button
        if (button3.Pressed())
        {
            panEnabled = true;
        }
        else
        {
            panEnabled = false;
        }

        // Lower-left button
        if (button4.Pressed())
        {
            hw.SetLed(true);
        }
        else
        {
            hw.SetLed(false);
        }

        // osc.SetFreq(440.0f + orangeKnobPos*440.0f);
        // osc2.SetFreq(220.0f + blueKnobPos*661.0f);
        // ringmod.SetAmp(blueKnobPos*0.9f);


        // mainVolume = blueKnobPos;
        // reverb.SetFeedback(blueKnobPos*0.98f);
        // phaseShift = knob4Pos / 10.0f;

        System::Delay(10);
    }
}
