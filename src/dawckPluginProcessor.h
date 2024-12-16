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
// name: dawckPluginProcesser.h
// desc: audio plugin processor for DAWck: ChucK in DAW
//
// authors: Summer Krinsky (https://summerkrinsky.com/)
//          Ge Wang (https://ccrma.stanford.edu/~ge/)
//    date: Fall 2024
//-----------------------------------------------------------------------------
#pragma once

#include <JuceHeader.h>
#include "chuck.h"


//=============================================================================
// name: class DAWckAudioProcesser
// desc: primary audio processing mechanism for DAWck
//=============================================================================
class DAWckAudioProcesser  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DAWckAudioProcesser();
    ~DAWckAudioProcesser() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState& getTreeState() { return treeState; }
    //new
    float getGlobalFloat(const std::string& variableName) const;


public: // dawck-specific functions
    // get pointer to the ChucK system associated with this processor
    ChucK * chuck();

public: // ui functions
    void updateFloats( float v, float v1 );
    double getCurrentBPM() const { return currentBPM; } // Getter for BPM
    double getCurrentPPQPosition() const { return currentPPQPosition; }
    double getCurrentTimeInSeconds() const { return currentTimeInSeconds; }
    int getTimeSignatureNumerator() const { return timeSignatureNumerator; }
    int getTimeSignatureDenominator() const { return timeSignatureDenominator; }
    bool isTransportPlaying() const { return m_isTransportPlaying; }
    

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState treeState;

    // our ChucK instance
    ChucK * m_chuck = NULL;
    // audio input buffer
    SAMPLE * m_inputBuffer = NULL;
    // audio output buffer
    SAMPLE * m_outputBuffer = NULL;
    // our audio buffer size
    t_CKINT m_bufferSize = 0;
    
    void updateBPM();
    double currentBPM = 120.0; //default BPM
    double currentPPQPosition = 0.0;          // Playhead position in PPQ
    double currentTimeInSeconds = 0.0;        // Playhead position in seconds
    int timeSignatureNumerator = 4;           // Default time signature numerator
    int timeSignatureDenominator = 4;         // Default time signature denominator
    bool m_isTransportPlaying = false;          // Transport state

    //==============================================================================
    // last frequency
    t_CKFLOAT g_last_frequency = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWckAudioProcesser)
};
