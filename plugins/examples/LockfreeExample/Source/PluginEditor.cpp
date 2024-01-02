/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <cmath>
#include <unordered_map>

//==============================================================================
LockfreeExampleEditor::LockfreeExampleEditor(LockfreeExampleProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  formatManager.registerBasicFormats();
  const auto sampleDir = juce::File(MUSIKHACK_SAMPLES_DIR);
  sampleFiles =
      sampleDir.findChildFiles(juce::File::findFiles, true, "*.wav;*.aif");

  fileSelector.setRange(0, sampleFiles.size() - 1, 1);
  fileSelector.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
  fileSelector.onValueChange = [this]() {
    const auto index = static_cast<int>(fileSelector.getValue());
    const auto f = sampleFiles[index];
    audioProcessor.queueSoundLoad(
        {f.getFileNameWithoutExtension(), f, &formatManager});
  };
  fileSelector.textFromValueFunction = [this](double value) {
    const auto index = static_cast<int>(value);
    const auto f = sampleFiles[index];
    return f.getFileNameWithoutExtension();
  };

  addAndMakeVisible(fileSelector);

  setSize(400, 200);
  startTimerHz(30);
}

LockfreeExampleEditor::~LockfreeExampleEditor() { stopTimer(); }

void LockfreeExampleEditor::timerCallback() { repaint(); }

//==============================================================================
void LockfreeExampleEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  const juce::Colour pink(215, 145, 188);
  const juce::Colour offWhite(247, 244, 242);
  g.fillAll(juce::Colour(28, 28, 28));

  auto x = xPos;
  const auto maxWidth = getWidth();
  auto draw = true;
  audioProcessor.getVizQueue().forEach([&](float sample) {
    sample = juce::jlimit(-1.f, 1.f, sample);
    if (draw) {
      g.setColour(pink);
      g.drawRect(x / 2, 90 + static_cast<int>(sample * 80), 1, 2);
      x++;

      if (x > maxWidth * 2) {
        x = 0;
        draw = false;
      }
    }
  });
  xPos = x;

  const auto indicatorWidth = 15.;
  std::unordered_map<MeterType, bool> done;
  done[MeterType::peak] = false;
  done[MeterType::rms] = false;
  audioProcessor.getMeterQueue().forEach([&](MeterPair &pair) {
    auto val = juce::jmin(1.f, pair.second);

    if (pair.first == MeterType::peak && !done[MeterType::peak]) {
      lastMeterVal *= 0.9f;
      lastMeterVal = juce::jmax(val, lastMeterVal);
      g.setColour(offWhite.withAlpha(lastMeterVal));
      g.fillEllipse(static_cast<float>(getWidth() - indicatorWidth - 10), 10,
                    indicatorWidth, indicatorWidth);
      done[MeterType::peak] = true;

    } else if (pair.first == MeterType::rms && !done[MeterType::rms]) {
      g.setColour(offWhite.withAlpha(val));
      g.fillEllipse(static_cast<float>(getWidth() - indicatorWidth - 40), 10,
                    indicatorWidth, indicatorWidth);
      done[MeterType::rms] = true;
    }
  });
}

void LockfreeExampleEditor::resized() {
  fileSelector.setBounds(20, getBottom() - 25, getWidth() - 40, 20);
}
