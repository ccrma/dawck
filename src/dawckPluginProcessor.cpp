//-----------------------------------------------------------------------------
// DAWck: ChucK programming language in digital audio workstations
//   by Summer Krinsky, Ge Wang, and ChucK Team | 2024-present
//-----------------------------------------------------------------------------
// Started in 2024, Project DAWck is a new from-the-ground-up re-design and
//   implementation of ChucK as an audio plugin for DAWs. It is based on
//   the ChucK Racks projects (2016) [1] and benefits greatly from
//   its blueprint and many findings.
//
// [1] Jordan Hochenbaum, Spencer Salazar, Rodrigo Sena. 2017. "ChucK Racks:
//   Text-based Music Programming for the Digital Audio Workshop."
//   /International Conference on Mathematics and Computing/.
//-----------------------------------------------------------------------------
// name: dawckPluginProcesser.cpp
// desc: audio plugin processor for DAWck: ChucK in DAW
//
// authors: Summer Krinsky (https://summerkrinsky.com/)
//          Ge Wang (https://ccrma.stanford.edu/~ge/)
//    date: Fall 2024
//-----------------------------------------------------------------------------
#include "dawckPluginProcessor.h"
#include "dawckPluginEditor.h"
#include "chuck_globals.h" // for global communication with chuck

#include <iostream>
using namespace std;




//=============================================================================
// name: DAWckAudioProcesser()
// desc: constructor
//=============================================================================
DAWckAudioProcesser::DAWckAudioProcesser()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor( BusesProperties()
                       //#if ! JucePlugin_IsMidiEffect
                       //#if ! JucePlugin_IsSynth
                         .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       //#endif
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       //#endif
                     ),
                     treeState(*this, nullptr, "state",
                 {
                     std::make_unique<juce::AudioParameterFloat>(
                         juce::ParameterID{ "rotary1", 1 },   // Parameter ID and version
                         "Rotary 1",                         // Parameter name
                         juce::NormalisableRange<float>(0.0f, 1.0f), // Range
                         0.5f                                // Default value
                     )
                 })
#endif
{
    // Optionally, add a sub-tree to store UI state if needed
    treeState.state.addChild({ "uiState", { { "width", 400 }, { "height", 300 } }, {} }, -1, nullptr);
  
    // create a new chuck instances; this includes compiler, VM, audio engine
    m_chuck = new ChucK();

    // set number of input channels
    m_chuck->setParam( CHUCK_PARAM_INPUT_CHANNELS, 2 );
    // number of output channels
    m_chuck->setParam( CHUCK_PARAM_OUTPUT_CHANNELS, 2 );
    // whether to halt the VM when there is no more shred running
    m_chuck->setParam( CHUCK_PARAM_VM_HALT, FALSE );
    // set hint so internally can advise things like async data writes etc.
    m_chuck->setParam( CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, FALSE );
    // FYI the sample rate is set in prepareToPlay()
    
    // TODO: disable chugin-loading OR set chugin paths

    // turn on logging to see what ChucK is up to; higher == more info
    m_chuck->setLogLevel( CK_LOG_SYSTEM );
}




//-----------------------------------------------------------------------------
// name: DAWckAudioProcesser()
// desc: desctructor
//-----------------------------------------------------------------------------
DAWckAudioProcesser::~DAWckAudioProcesser()
{
    // TODO: probably should clean up chuck
}




//-----------------------------------------------------------------------------
// name: getName()
// desc: get plugin name
//-----------------------------------------------------------------------------
const juce::String DAWckAudioProcesser::getName() const
{
    return JucePlugin_Name;
}



//-----------------------------------------------------------------------------
// name: acceptsMidi()
// desc: does plugin accept MIDI
//-----------------------------------------------------------------------------
bool DAWckAudioProcesser::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}




//-----------------------------------------------------------------------------
// name: producesMidi()
// desc: does plugin produce MIDI
//-----------------------------------------------------------------------------
bool DAWckAudioProcesser::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}




//-----------------------------------------------------------------------------
// name: isMidiEffect()
// desc: is plugin MIDI effect
//-----------------------------------------------------------------------------
bool DAWckAudioProcesser::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}




//-----------------------------------------------------------------------------
// name: getTailLengthSeconds()
// desc: probably should set this to something > 0, maybe for potential reverb tails?
//-----------------------------------------------------------------------------
double DAWckAudioProcesser::getTailLengthSeconds() const
{
    return 0.0;
}




//-----------------------------------------------------------------------------
// name: getNumPrograms()
// desc: we don't know what a program is in this context
//-----------------------------------------------------------------------------
int DAWckAudioProcesser::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}




//-----------------------------------------------------------------------------
// name: getCurrentProgram()
// desc: we don't know what a program is in this context
//-----------------------------------------------------------------------------
int DAWckAudioProcesser::getCurrentProgram()
{
    return 0;
}




//-----------------------------------------------------------------------------
// name: setCurrentProgram()
// desc: we don't know what a program is in this context
//-----------------------------------------------------------------------------
void DAWckAudioProcesser::setCurrentProgram( int index )
{
}




//-----------------------------------------------------------------------------
// name: getProgramName()
// desc: we don't know what a program is in this context
//-----------------------------------------------------------------------------
const juce::String DAWckAudioProcesser::getProgramName( int index )
{
    return {};
}




//-----------------------------------------------------------------------------
// name: changeProgramName()
// desc: we don't know what a program is in this context
//-----------------------------------------------------------------------------
void DAWckAudioProcesser::changeProgramName( int index, const juce::String & newName )
{
}




//=============================================================================
// name: prepareToPlay()
// desc: this is called when the host is ready to begin processing with this plugin
//=============================================================================
void DAWckAudioProcesser::prepareToPlay( double sampleRate, int samplesPerBlock )
{
    // set sample rate; this can be called repeated; setting this parameter
    // will automatically trigger chuck to update its relevant internal state
    m_chuck->setParam( CHUCK_PARAM_SAMPLE_RATE, (t_CKINT)(sampleRate+0.5) );
    
    // allocate buffers, if needed
    if( !m_outputBuffer || m_bufferSize != samplesPerBlock )
    {
        // clean up memory, if allocated
        CK_SAFE_DELETE( m_inputBuffer );
        CK_SAFE_DELETE( m_outputBuffer );
        // remember
        m_bufferSize = samplesPerBlock;
        // TODO: how big to make the buffers at this point needs further investigate
        t_CKUINT numFrames = samplesPerBlock * 2; // the 2 is a fudge factor
        // allocate audio buffers as expected by ChucK's run() function below
        m_inputBuffer = new SAMPLE[numFrames * m_chuck->getParamInt(CHUCK_PARAM_INPUT_CHANNELS)];
        m_outputBuffer = new SAMPLE[numFrames * m_chuck->getParamInt(CHUCK_PARAM_OUTPUT_CHANNELS)];
        // zero out
        memset( m_inputBuffer, 0, numFrames * m_chuck->getParamInt(CHUCK_PARAM_INPUT_CHANNELS) );
        memset( m_outputBuffer, 0, numFrames * m_chuck->getParamInt(CHUCK_PARAM_OUTPUT_CHANNELS) );
    }

    // print language version
    cerr << "[chuck] language version: " << m_chuck->version() << endl;
    // initialize ChucK, after the parameters are set
    if( !m_chuck->isInit() )
    {
        // do it
        m_chuck->init();
        // still not initialized?
        if( !m_chuck->isInit() ) return;
    }

    // start ChucK VM and synthesis engine
    m_chuck->start();
    // clear VM (since we are updating sample rate)
    m_chuck->removeAllShreds();

    // test run some code
    //m_chuck->compileCode( "adc => LPF lpf => dac; 400 => lpf.freq; while( true ) { 1::second => now; }", "", 1 );
    //m_chuck->compileCode( "global float INPUT_FREQUENCY; global float INPUT_FREQUENCY1; adc => LPF lpf => dac; 400 => lpf.freq; while( true ) { 10+(INPUT_FREQUENCY *2000) => lpf.freq; INPUT_FREQUENCY1 => lpf.Q; 1::second => now; }", "", 1 );
    //m_chuck->compileCode( "global float INPUT_FREQUENCY; global float INPUT_FREQUENCY1; adc => BiQuad eq => dac; 0.9 => eq.prad; while(true){ INPUT_FREQUENCY => eq.pfreq; INPUT_FREQUENCY1 => eq.gain; 1::second => now; }", "", 1);
    m_chuck->compileCode( "global float INPUT_FREQUENCY; global float INPUT_FREQUENCY1; adc => PitShift ps => dac; while(true){ INPUT_FREQUENCY => ps.shift; INPUT_FREQUENCY1 => ps.mix; 1::second => now; }", "", 1);
    // m_chuck->compileCode( "SinOsc foo(440) => dac; while( true ) { <<< now/second >>>; second => now; }", "", 1 );
    // m_chuck->compileCode( "global float INPUT_FREQUENCY; SinOsc foo => dac; while( true ) { 200+(INPUT_FREQUENCY*300) => foo.freq; 10::ms => now; }", "", 1 );
}




//-----------------------------------------------------------------------------
// name: releaseResources()
// desc: TODO
//-----------------------------------------------------------------------------
void DAWckAudioProcesser::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}




//-----------------------------------------------------------------------------
// name: isBusesLayoutSupported()
// desc: ...
//-----------------------------------------------------------------------------
#ifndef JucePlugin_PreferredChannelConfigurations
bool DAWckAudioProcesser::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif




//=============================================================================
// name: processBlock()
// desc: this is the audio callback from the host, buffer by buffer,
//       where the chuck and audio computations happen
//=============================================================================
void DAWckAudioProcesser::processBlock( juce::AudioBuffer<float> & buffer, juce::MidiBuffer & midiMessages )
{
    // by default, chuck expects interleaved buffers
    // by default, chuck SAMPLE is a float

    // NOTE: this could be where DAWck processes global and other communications with chuck
    // NOTE: this is on the audio thread, which allows more functions to be called in m_chuck->globals()
    // NOTE: use with care!
    // get a global variable value from chuck
    // t_CKFLOAT f = m_chuck->globals()->get_global_float_value( "g_value1" );
    // set a global variable value in chuck
    // m_chuck->globals()->setGlobalFloat( "g_value2", 5 );

    // get number of sample frames
    t_CKUINT N = buffer.getNumSamples();
    // get number of channels
    t_CKUINT channels = buffer.getNumChannels();
   
    // check # of chanels
    if( channels == 2 )
    {
        // pointer to input buffer
        const float * inputLeft = buffer.getReadPointer(0);
        const float * inputRight = buffer.getReadPointer(1);
        
        // interleave into our own input buffer
        for( t_CKINT i = 0; i < N; i++ )
        {
            // assume stereo
            m_inputBuffer[i*2] = inputLeft[i];
            m_inputBuffer[i*2+1] = inputRight[i];
        }
        
        // tell chuck compute the next block of audio
        m_chuck->run( m_inputBuffer, m_outputBuffer, N );
        
        // pointer to JUCE's output buffers
        float * outLeft = buffer.getWritePointer(0);
        float * outRight = buffer.getWritePointer(1);
        
        // de-interleave into our own output buffer into JUCE output buffer
        for( t_CKINT i = 0; i < N; i++ )
        {
            // assume stereo
            outLeft[i] = m_outputBuffer[i*2];
            outRight[i] = m_outputBuffer[i*2+1];
        }
    }
    else if( channels == 1 )
    {
        // pointer to input buffer
        const float * input = buffer.getReadPointer(0);
        // copy into our own input buffer
        for( t_CKINT i = 0; i < N; i++ )
        {
            // mono; copy to left and right channels for input
            m_inputBuffer[i*2] = m_inputBuffer[i*2+1] = input[i];
        }

        // tell chuck compute the next block of audio
        m_chuck->run( m_inputBuffer, m_outputBuffer, N );
        
        // pointer to JUCE's output buffers
        float * output = buffer.getWritePointer(0);
        
        // copy out to JUCE's output buffer
        for( t_CKINT i = 0; i < N; i++ )
        {
            // assume stereo output from chuck, average to mono
            output[i] = (m_outputBuffer[i*2]+m_outputBuffer[i*2+1])/2;
        }
    }
}




//==============================================================================
bool DAWckAudioProcesser::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DAWckAudioProcesser::createEditor()
{
    return new DAWckAudioProcesserEditor (*this);
}

//==============================================================================
void DAWckAudioProcesser::getStateInformation (juce::MemoryBlock& destData)
{
   // juce::MemoryOutputStream (destData, true).writeFloat (*gain);
}

void DAWckAudioProcesser::setStateInformation (const void* data, int sizeInBytes)
{
   // gain->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
}

ChucK * DAWckAudioProcesser::chuck()
{
    return m_chuck;
}

void DAWckAudioProcesser::updateFloats( float v, float v1 )
{
    m_chuck->globals()->setGlobalFloat( "INPUT_FREQUENCY", v );
    m_chuck->globals()->setGlobalFloat( "INPUT_FREQUENCY1", v1 );
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DAWckAudioProcesser();
}
