/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

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

void LockfreeExampleEditor::timerCallback() {
  repaint();
  audioProcessor.getLogQueue().forEach(
      [&](Logger::Message const &msg) { DBG(Logger::format(msg)); });
}

//==============================================================================
void LockfreeExampleEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(28, 28, 28));

  const juce::Colour pink(215, 145, 188);
  const juce::Colour offWhite(247, 244, 242);

  g.setColour(pink);
  auto &ring = audioProcessor.getVizRing();
  const auto maxSamples = ring.getSize();
  const auto increment = (double)maxSamples / (double)getWidth();
  auto x = xPos;

  // draw waveform
  audioProcessor.getVizRing().forEach([&](float sample) {
    sample = juce::jlimit(-1.f, 1.f, sample);
    juce::Rectangle<double> pixel(x * increment, (double)sample * 90. + 100.,
                                  increment, 2.);
    g.drawRect(pixel.toFloat(), 1.);
    x++;
  });

  // draw peak and RMS meters
  const auto indicatorWidth = 15.;
  const auto peakVal = audioProcessor.getPeak();
  lastMeterVal *= 0.9f;
  lastMeterVal = juce::jmax(peakVal, lastMeterVal);
  g.setColour(offWhite.withAlpha(lastMeterVal));
  g.fillEllipse(static_cast<float>(getWidth() - indicatorWidth - 10), 10,
                indicatorWidth, indicatorWidth);

  g.setColour(offWhite.withAlpha(audioProcessor.getRMS()));
  g.fillEllipse(static_cast<float>(getWidth() - indicatorWidth - 40), 10,
                indicatorWidth, indicatorWidth);
}

void LockfreeExampleEditor::resized() {
  fileSelector.setBounds(20, getBottom() - 25, getWidth() - 40, 20);
  title.setBounds(10, 10, 300, 20);
}
