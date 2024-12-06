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
// name: dawckPluginEditor.cpp
// desc: audio plugin frontend for DAWck: ChucK in DAW
//
// authors: Summer Krinsky (https://summerkrinsky.com/)
//    date: Fall 2024
//-----------------------------------------------------------------------------
#include "dawckPluginProcessor.h"
#include "dawckPluginEditor.h"
#include "BinaryData.h"




//==============================================================================
DAWckAudioProcesserEditor::DAWckAudioProcesserEditor (DAWckAudioProcesser& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    //sliderAttach - new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.treeState, GAIN_ID, gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setRange(-12.0, 12.0);
    gainSlider.setValue(0.5);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    gainSlider.onValueChange = [this]{ audioProcessor.updateFloats( gainSlider.getValue(), rotary1.getValue() ); };
    addAndMakeVisible(gainSlider);
    gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    
    //new rotary
    rotary1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    rotary1.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    rotary1.setMouseDragSensitivity(75);
    rotary1.setValue(juce::MathConstants<double>::pi);
    rotary1.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    rotary1.setRange(0.0, 1.0, 0.01); // Range from 0 to 1 with a step size of 0.01
    rotary1.onValueChange = [this]{ audioProcessor.updateFloats( gainSlider.getValue(), rotary1.getValue() ); };
    rotary1.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    /* rotaryKnob.setSliderStyle(juce::Slider::Rotary); // Circular knob
    rotaryKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20); // Optional textbox
    rotaryKnob.setRange(0.0, 2.0 * juce::MathConstants<double>::pi, 0.01); // Set range (0 to 2π radians)
    rotaryKnob.setValue(juce::MathConstants<double>::pi); // Set initial value (optional)
    rotaryKnob.setMouseDragSensitivity(150); // Adjust sensitivity*/

    // Add the rotary knob to the component
    addAndMakeVisible(rotary1);
    // Configure the label
    pitchLabel.setText("Pitch Slider", juce::dontSendNotification);
    pitchLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    pitchLabel.setJustificationType(juce::Justification::centred);
    pitchLabel.attachToComponent(&gainSlider, false); // Attach label to the slider (below)
    addAndMakeVisible(pitchLabel);
    
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.attachToComponent(&rotary1, false); // Attach label to the slider (below)
    addAndMakeVisible(mixLabel);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::black);
}

DAWckAudioProcesserEditor::~DAWckAudioProcesserEditor()
{
}

//==============================================================================
void DAWckAudioProcesserEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
    {
        // Draw the background image
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        // Fallback if image fails to load
        g.fillAll(juce::Colours::darkgrey);
        // g.setColour(juce::Colours::white);
        // g.drawText("Background Image Not Found", getLocalBounds(), juce::Justification::centred);
    }
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    // g.setColour (juce::Colours::white);
    // g.setFont (juce::FontOptions (15.0f));
    // g.drawFittedText ("=>DAW", getLocalBounds(), juce::Justification::bottomRight, 1);
}

void DAWckAudioProcesserEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    int knobSize = 100; // Smaller size for the knob
    int margin = 20;
    //gainSlider.setBounds(getLocalBounds());
    rotary1.setBounds(margin, (getHeight() - knobSize) / 2, knobSize, knobSize);
    
    // Define the size and position of the gain slider
    int sliderWidth = 80;  // Width of the slider
    int sliderHeight = 200; // Height of the slider
    int xPosition = (getWidth() - sliderWidth) / 2; // Center horizontally
    int yPosition = 50; // Position 20 pixels from the top

    gainSlider.setBounds(static_cast<int>(xPosition), static_cast<int>(yPosition), static_cast<int>(sliderWidth), static_cast<int>(sliderHeight));
}

//void DAWckAudioProcesserEditor::sliderValueChanged(juce::Slider *slider)
//{
//    // if( slider == &gainSlider )
//    {
//        audioProcessor.updateFloat( gainSlider.getValue() );
//    }
//}
