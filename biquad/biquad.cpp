#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Biquad     flt;
static Oscillator osc, lfo, pan;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float saw, freq, output, pan_out;
    for(size_t i = 0; i < size; i += 2)
    {
        freq = 1000 + (lfo.Process() * 1000);
        saw  = osc.Process();

        pan_out = (1.0f + pan.Process()) / 2.0f;

        flt.SetCutoff(freq);
        output = flt.Process(saw);

        //lfo.Process();
        // output = osc.Process();

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

    // initialize Biquad and set parameters
    flt.Init(sample_rate);
    flt.SetRes(0.7);

    // set parameters for low-frequency oscillator
    lfo.Init(sample_rate);
    lfo.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    lfo.SetAmp(0.2);
    lfo.SetFreq(0.1);

    // set parameters for sine oscillator object
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc.SetFreq(100);
    osc.SetAmp(0.05);

    // set parameters for ping-pong panner as even-lower frequency oscillator
    lfo.Init(sample_rate);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetAmp(1);
    lfo.SetFreq(0.08);

    // == Setup for hardware component: knob
    AdcChannelConfig knob[3];
    knob[0].InitSingle(hw.GetPin(25));
    knob[1].InitSingle(hw.GetPin(24));
    knob[2].InitSingle(hw.GetPin(18));
    //Initialize the adc with the config
    hw.adc.Init(knob, 3);
    //Start reading values
    hw.adc.Start();


    // start callback
    hw.StartAudio(AudioCallback);


    while(1) {
        lfo.SetFreq(5.0 + hw.adc.GetFloat(0)*50.0f);
        osc.SetFreq(50.0 + hw.adc.GetFloat(1)*100.0f);
        pan.SetFreq(0.08 + hw.adc.GetFloat(2)*0.5f);
        if (lfo.IsRising()) hw.SetLed(true);
        else hw.SetLed(false);
        System::Delay(1);
    }
}
