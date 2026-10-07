#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Biquad     flt;
static Oscillator osc, lfo;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float saw, freq, output;
    for(size_t i = 0; i < size; i += 2)
    {
        freq = 1000 + (lfo.Process() * 1000);
        saw  = osc.Process();

        flt.SetCutoff(freq);
        output = flt.Process(saw);

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

    // set parameters for sine oscillator object
    lfo.Init(sample_rate);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetAmp(1);
    lfo.SetFreq(0.1);

    // set parameters for sine oscillator object
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE);
    osc.SetFreq(100);
    osc.SetAmp(0.25);

    // == Setup for hardware component: knob
    AdcChannelConfig knob;
    knob.InitSingle(hw.GetPin(25));
    //Initialize the adc with the config we just made
    hw.adc.Init(&knob, 1);
    //Start reading values
    hw.adc.Start();


    // start callback
    hw.StartAudio(AudioCallback);


    while(1) {
        lfo.SetFreq(0.1f + hw.adc.GetFloat(0)*3.0f);
        if (lfo.IsRising()) hw.SetLed(true);
        else hw.SetLed(false);
        System::Delay(1);
    }
}
