/*
  ==============================================================================

    ProcessorWrapper.cpp
    Created: 8 May 2022 9:38:17pm
    Author:  StoneyDSP

  ==============================================================================
*/

#include "PluginWrapper.h"
#include "PluginProcessor.h"

template <typename SampleType>
ProcessWrapper<SampleType>::ProcessWrapper(OrfanidisBiquadAudioProcessor& p, APVTS& apvts, juce::dsp::ProcessSpec& spec) 
    :
    audioProcessor(p),
    state(apvts),
    setup(spec)
{
    setup.sampleRate = audioProcessor.getSampleRate();
    setup.maximumBlockSize = audioProcessor.getBlockSize();
    setup.numChannels = audioProcessor.getTotalNumInputChannels();

    frequencyPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("frequencyID"));
    resonancePtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("bandwidthID"));
    gainPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("gainID"));
    transformPtr = dynamic_cast <juce::AudioParameterChoice*>(state.getParameter("transformID"));
    outputPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("outputID"));
    mixPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("mixID"));
    bypassPtr = dynamic_cast <juce::AudioParameterBool*> (state.getParameter("bypassID"));

    jassert(frequencyPtr != nullptr);
    jassert(resonancePtr != nullptr);
    jassert(gainPtr != nullptr);
    jassert(transformPtr != nullptr);
    jassert(outputPtr != nullptr);
    jassert(mixPtr != nullptr);
    jassert(bypassPtr != nullptr);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::prepare(juce::dsp::ProcessSpec& spec)
{
    spec.sampleRate = audioProcessor.getSampleRate();
    spec.maximumBlockSize = audioProcessor.getBlockSize();
    spec.numChannels = audioProcessor.getTotalNumInputChannels();

    mixer.prepare(spec);
    filter.prepare(spec);
    output.prepare(spec);

    reset();
    update();
};

template <typename SampleType>
void ProcessWrapper<SampleType>::reset() 
{
    mixer.reset();
    filter.reset(static_cast<SampleType>(0.0));
    output.reset();
};

template <typename SampleType>
void ProcessWrapper<SampleType>::process(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    update();

    juce::dsp::AudioBlock<SampleType> block(buffer);

    mixer.pushDrySamples(block);

    juce::dsp::ProcessContextReplacing<SampleType> context(block);

    context.isBypassed = bypassPtr->get();

    filter.process(context);
    output.process(context);
    mixer.mixWetSamples(block);
};

template <typename SampleType>
void ProcessWrapper<SampleType>::update()
{
    setup.sampleRate = audioProcessor.getSampleRate();
    setup.maximumBlockSize = audioProcessor.getBlockSize();
    setup.numChannels = audioProcessor.getTotalNumInputChannels();

    audioProcessor.setBypassParameter(bypassPtr);

    mixer.setWetMixProportion(mixPtr->get() * 0.01f);

    filter.setFrequency(frequencyPtr->get());
    filter.setResonance(resonancePtr->get());
    filter.setGain(gainPtr->get());
    filter.setTransformType(static_cast<TransformationType>(transformPtr->getIndex()));

    output.setGainLinear(juce::Decibels::decibelsToGain(outputPtr->get()));
};

//==============================================================================
template class ProcessWrapper<float>;
template class ProcessWrapper<double>;