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
using namespace juce;



//==============================================================================
DAWckAudioProcesserEditor::DAWckAudioProcesserEditor (DAWckAudioProcesser& p)
    : AudioProcessorEditor (&p), audioProcessor (p), openButton("Load ChucK file(s)")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    openButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(&openButton);
    
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
    // Attach the slider to the treeState parameter
    // Attach the slider to the treeState parameter via the getter
    // TODO: this appears to be causing a crash on cleanup, from within the attachment's destructor
    rotary1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getTreeState(), "rotary1", rotary1);
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
    
    // Configure bpmLabel
    bpmLabel.setText("BPM: --", juce::dontSendNotification); // Default text
    bpmLabel.setFont(juce::Font(16.0f));
    bpmLabel.setJustificationType(juce::Justification::bottomRight);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(bpmLabel); // Add to the editor
    
    // Playhead Position Label
    playheadPositionLabel.setText("Position: --", juce::dontSendNotification);
    playheadPositionLabel.setFont(juce::Font(16.0f));
    playheadPositionLabel.setJustificationType(juce::Justification::bottomRight);
    playheadPositionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(playheadPositionLabel);

    // Time Signature Label
    timeSignatureLabel.setText("Time Signature: --", juce::dontSendNotification);
    timeSignatureLabel.setFont(juce::Font(16.0f));
    timeSignatureLabel.setJustificationType(juce::Justification::bottomRight);
    timeSignatureLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(timeSignatureLabel);

    // Transport State Label
    transportStateLabel.setText("Transport: Stopped", juce::dontSendNotification);
    transportStateLabel.setFont(juce::Font(16.0f));
    transportStateLabel.setJustificationType(juce::Justification::bottomRight);
    transportStateLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(transportStateLabel);
   
    // Start the timer to poll ChucK for updates
    startTimerHz(30); // Poll 30 times per second
}

DAWckAudioProcesserEditor::~DAWckAudioProcesserEditor()
{
    stopTimer();
    rotary1Attachment.reset();
}

//==============================================================================
void DAWckAudioProcesserEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Background color
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

void DAWckAudioProcesserEditor::openButtonClicked()
{
    DBG("clicked");
    // create a file chooser
    FileChooser chooser( "Choose one or more ChucK (.ck) file",  File::getSpecialLocation(File::userDesktopDirectory), "*.ck", true);

    // open file dialog
    if( chooser.browseForFileToOpen() )
    {
        // remove all shreds currently in VM (thread-safe)
        audioProcessor.chuck()->removeAllShreds();

        // this will hold the result
        File results;
        // what did the user choose?
        results = chooser.getResult();
        // the absolute path
        std::string absolutePath = results.getFullPathName().toStdString();
        // tell chuck to compile and run
        audioProcessor.chuck()->compileFile( absolutePath );
    }
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
    openButton.setBounds(10, 10, getWidth()-20, 30);
    bpmLabel.setBounds (getLocalBounds().reduced(20));
    playheadPositionLabel.setBounds(10, 50, getWidth() - 20, 30);
    timeSignatureLabel.setBounds(10, 90, getWidth() - 20, 30);
    transportStateLabel.setBounds(10, 130, getWidth() - 20, 30);
    gainSlider.setBounds(static_cast<int>(xPosition), static_cast<int>(yPosition), static_cast<int>(sliderWidth), static_cast<int>(sliderHeight));
}

//write chuck changes to rotary
void DAWckAudioProcesserEditor::timerCallback()
{
    // Get the updated value from ChucK
    float chuckValue = audioProcessor.getGlobalFloat("INPUT_FREQUENCY1");

    // Update the rotary1 slider if needed
    if (rotary1.getValue() != chuckValue)
    {
        rotary1.setValue(chuckValue, juce::dontSendNotification);
    }
    
    double bpm = audioProcessor.getCurrentBPM();
    bpmLabel.setText("BPM: " + juce::String(bpm), juce::dontSendNotification);
    
    playheadPositionLabel.setText("Position: " + juce::String(audioProcessor.getCurrentPPQPosition(), 2)
        + " PPQ / " + juce::String(audioProcessor.getCurrentTimeInSeconds(), 2) + " sec",
        juce::dontSendNotification);

    timeSignatureLabel.setText("Time Signature: " 
        + juce::String(audioProcessor.getTimeSignatureNumerator()) + "/"
        + juce::String(audioProcessor.getTimeSignatureDenominator()), 
        juce::dontSendNotification);

    transportStateLabel.setText("Transport: " + juce::String(audioProcessor.isTransportPlaying() ? "Playing" : "Stopped"),
        juce::dontSendNotification);
}

//void DAWckAudioProcesserEditor::sliderValueChanged(juce::Slider *slider)
//{
//    // if( slider == &gainSlider )
//    {
//        audioProcessor.updateFloat( gainSlider.getValue() );
//    }
//}
