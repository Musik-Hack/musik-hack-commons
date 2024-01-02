/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <unordered_map>

//==============================================================================
LockfreeExampleEditor::LockfreeExampleEditor(LockfreeExampleProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {

  // Prepare for loading AIFF/WAV
  formatManager.registerBasicFormats();

  // Store all sample file paths in a big array
  const auto sampleDir = juce::File(MUSIKHACK_SAMPLES_DIR);
  sampleFiles =
      sampleDir.findChildFiles(juce::File::findFiles, true, "*.wav;*.aif");

  // Configure the slider to queue loading up a sound when the value changes
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

  title.setText("Example using non-blocking FIFOs",
                juce::NotificationType::dontSendNotification);

  addAndMakeVisible(fileSelector);
  addAndMakeVisible(title);

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 200);
  startTimerHz(30);
}

LockfreeExampleEditor::~LockfreeExampleEditor() { stopTimer(); }

void LockfreeExampleEditor::timerCallback() { repaint(); }

//==============================================================================
void LockfreeExampleEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(28, 28, 28));

  const juce::Colour pink(215, 145, 188);
  const juce::Colour offWhite(247, 244, 242);
  const auto maxSamples = getWidth() * 2;

  auto x = xPos;
  auto draw = true;
  audioProcessor.getVizQueue().forEach([&](float sample) {
    sample = juce::jlimit(-1.f, 1.f, sample);
    if (draw) {
      g.setColour(pink);
      g.drawRect((int)((double)x * 0.5), 100 + static_cast<int>(sample * 90), 1,
                 2);
      x++;

      if (x > maxSamples) {
        x = 0;
        draw = false;
      }
    }
  });

  if (x > 0) {
    g.setColour(pink);
    while (x < maxSamples) {
      g.drawRect((int)((double)x * 0.5), 100, 1, 2);
      x++;
    }
    x = 0;
  }

  const auto indicatorWidth = 15.;
  std::unordered_map<MeterType, bool> done = {{MeterType::peak, false},
                                              {MeterType::rms, false}};

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
  title.setBounds(10, 10, 300, 20);
}
