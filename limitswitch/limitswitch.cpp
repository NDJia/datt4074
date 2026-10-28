#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Biquad     flt;
static Oscillator osc, lfo;
static AdEnv      env;
static WhiteNoise noise;


bool buttonIsPressed;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float output, env_out, filter_freq, osc_out, noise_out;
    for(size_t i = 0; i < size; i += 2)
    {
        // trigger envelope to start when button is down
        if(buttonIsPressed && !env.IsRunning())
        {
            env.Trigger();
        }
        // envelope processing
        env_out = env.Process();

        // noise
        noise.SetAmp(env_out);
        noise_out = noise.Process();

        // Biquad filter
        filter_freq = 1000 + (lfo.Process() * 1000);
        flt.SetCutoff(filter_freq);
        // flt.SetRes(env_out);

        // oscillator process     
        osc_out = osc.Process();

        // add together outputs
        output = flt.Process(noise_out) + osc_out;

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

    // init envelope and set parameters
    env.Init(sample_rate);
    env.SetTime(ADENV_SEG_ATTACK, 0.5);
    env.SetTime(ADENV_SEG_DECAY, 5.0);
    env.SetMin(0.0);
    env.SetMax(0.15);
    env.SetCurve(0); // linear

    // Set up noise
    noise.Init();
    noise.SetAmp(0.01);

    // set parameters for sine oscillator object
    lfo.Init(sample_rate);
    lfo.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI);
    lfo.SetAmp(0.8);
    lfo.SetFreq(0.1);

    // set parameters for sine oscillator object
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetFreq(200);
    osc.SetAmp(0.1);

    // == Setup for hardware component: knobs
    AdcChannelConfig knobs[2];
    knobs[0].InitSingle(hw.GetPin(25));
    knobs[1].InitSingle(hw.GetPin(24));
    //Initialize the adc with the config we just made
    hw.adc.Init(knobs, 2);

    // == Setup for hardware component: knobs
    // AdcChannelConfig freqKnob;
    // freqKnob.InitSingle(hw.GetPin(24));
    // //Initialize the adc on the second channel
    // hw.adc.Init(&freqKnob, 2);


    //Start reading values
    hw.adc.Start();


    // set up limit switch as a button
    Switch button;
    button.Init(hw.GetPin(26));


    // start callback
    hw.StartAudio(AudioCallback);


    // hardware processing should go in the main loop
    while(1) {
        button.Debounce();
        if (button.Pressed())
        {
            hw.SetLed(true);
            //osc.SetFreq(100);
            // osc.SetAmp(0.8);
            buttonIsPressed = true;
        }
        else
        {
            hw.SetLed(false);
            //osc.SetFreq(200);
            // osc.SetAmp(0.0);
            buttonIsPressed = false;
        }

        lfo.SetFreq(5.0 + hw.adc.GetFloat(0)*50.0f);

        osc.SetFreq(110 + hw.adc.GetFloat(1)*330.f);

        // if (lfo.IsRising()) hw.SetLed(true);
        // else hw.SetLed(false);
        System::Delay(1);
    }
}
