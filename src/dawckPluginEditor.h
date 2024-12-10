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
// name: dawckPluginEditor.h
// desc: audio plugin frontend for DAWck: ChucK in DAW
//
// authors: Summer Krinsky (https://summerkrinsky.com/)
//          Ge Wang (https://ccrma.stanford.edu/~ge/)
//    date: Fall 2024
//-----------------------------------------------------------------------------
#pragma once

#include <JuceHeader.h>
#include "dawckPluginProcessor.h"


//==============================================================================
class DAWckAudioProcesserEditor  : public juce::AudioProcessorEditor
{
public:
    DAWckAudioProcesserEditor (DAWckAudioProcesser&);
    ~DAWckAudioProcesserEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    //void sliderValueChanged(juce::Slider *slider);
    //void sliderValueChanged(juce::Slider *rotary);
    
    //ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> sliderAttach;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DAWckAudioProcesser& audioProcessor;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rotary1Attachment;
    void openButtonClicked();
    juce::TextButton openButton;
    juce::Slider gainSlider;
    juce::Slider rotary1;
    juce::Label pitchLabel;
    juce::Label mixLabel;
    juce::Image backgroundImage; // Background image
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWckAudioProcesserEditor)
};
