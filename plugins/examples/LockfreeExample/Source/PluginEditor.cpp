/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 200);
    startTimerHz(60);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    stopTimer();
}

void NewProjectAudioProcessorEditor::timerCallback(){
    repaint();
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    
    auto x = xPos;
    const auto maxWidth = getWidth();
    auto draw = true;
    audioProcessor.getVizQueue().forEach([&](float& sample){
        if(draw){
            g.drawRect(x/2, 100 + static_cast<int>(sample * 90), 2, 2);
            x++;
            
            if(x > maxWidth * 2){
                x = 0;
                draw = false;
            }
        }
    });
    xPos = x;
    
    const auto center = getWidth() / 2;
    const auto indicatorWidth = 40;
    audioProcessor.getMeterQueue().forEach([&](MeterPair & pair){
        if(pair.first == MeterType::peak){
            g.setColour(juce::Colours::red.withAlpha(pair.second));
            g.fillEllipse(center - indicatorWidth / 2, 30, indicatorWidth, indicatorWidth);
            
        } else if (pair.first == MeterType::random){
            g.setColour(juce::Colours::yellow.withAlpha(pair.second));
            g.fillEllipse(center - indicatorWidth / 2, getBottom() - 100, indicatorWidth, indicatorWidth);
        }
    });
}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
