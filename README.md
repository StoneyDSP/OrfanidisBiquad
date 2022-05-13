# OrfanidisBiquad
Audio EQ Peak Band with no digital "cramping", and selectable transformations (Direct Form I & II, plus Direct Form I & II transposed).

(Shown below; a +30dB bell-shaped boost at 10kHz, performed on a harmonic-rich 20Hz sawtooth-wave in Reaper)

![Orfanidis Biquad](Res/OrfanidisBiquad.png)


(write-up and comparisons to follow - currently under revision)

Reference: Original paper (including MATLAB code) "Digital Parametric Equalizer Design With Prescribed
Nyquist-Frequency Gain", Sophocles J. Orfanidis, 1996; https://www.ece.rutgers.edu/~orfanidi/ece348/peq.pdf


# Transformations
Determining an output transfer function (Y(n)), given an input value (X(n)) and six multiplier coefficients within an audio feedback path (b0, b1, b2, a1, and a2 - all of which are pre-scaled by 1/a0) - please note that each feedback term requires a delay of one audio sample;

+ X(n) = input sample

+ Y(n) = output sample

+ b0, b1, etc.. = coefficient gain multiplier

+ (+) = summing point (addition)

+ z-1 = unit (single sample) delay

+ <-, ->.. etc = signal flow direction 

![DF I](https://github.com/StoneyDSP/OrfanidisBiquad/blob/0a9c1168752616b455d68b52a2b0b841102dfa16/Res/400px-Biquad_filter_DF-I.svg.png)

 Some forms have what may be considered an additional audio feedback path (notated as W(n) in this description, but may vary);

![DF II](https://github.com/StoneyDSP/OrfanidisBiquad/blob/0a9c1168752616b455d68b52a2b0b841102dfa16/Res/Biquad_filter_DF-IIx.svg.png)


# Creating the coefficients;

First, we need to create an input (Xn), and output (Yn), and our 6 coefficients for gain multiplication, the results of which are summed together using linear addition. We can start with an arrangement that simply passes the audio sample from input to output unchanged.

Here's some pseudo-code to get us started with a template;

    {
    
    X = input sample;                                                             // audio input stream...
    
    _b0 = 1;                                                                       // initialize coefficients to safe numbers...
    _b1 = 0;
    _b2 = 0;
    _a0 = 1;
    _a1 = 0;
    _a2 = 0;
    
    a0 = (1 / a0_);                                                                 // calculate new coefficients from parameters...

    a1 = (-1 * (a1_ * a0));
    a2 = (-1 * (a2_ * a0));

    b0 = (b0_ * a0);
    b1 = (b1_ * a0);
    b2 = (b2_ * a0);
    
    Y = ((X * b0) + (X * b1) + (X * b2) + (X * a1) + (X * a2));                     // apply coefficients to input sample...
    
    output sample = Y;                                                              // audio output stream...
    
    }

(please note that extra care is taken where necessary within the code to ensure thread safety between parameter (message thread) and playback (audio thread).)

I have also re-created all of the code from this study in my preferred visual-programming/workbench environment, Reaktor Core, which I have chosen to share here for it's ease of readability. Please see the above C++ code as translated visually into Core, below;

![Workbench](https://github.com/StoneyDSP/OrfanidisBiquad/blob/0a9c1168752616b455d68b52a2b0b841102dfa16/Res/Workbench%20-%20Bypass%20(coded%20by%20StoneyDSP).png)

For any Reaktor Core builders in attendance - you will see some structures in this write-up that are not in accordance with Reaktor Core's processing paradigm, particularly with respect to feedback macros and handling. The writer has assumed that readers have (practically) no interest in Reaktor Core itself, and are primarily here for the theory and possibly the C code. Thus, I have intentionally mis-used Reaktor's feedback handling purely for the sake of the visual demonstrations ahead, which I believe are very clear, even for non-Reaktor users. To create safe versions of these macros, please use the factory "z-1fdbk" non-solid macro for all unit delays, and ideally add an SR bundle distribution.

For any readers unfamiliar with Reaktor Core, please keep in mind that signal flows from left (input) to right (output). In addition to the basic math operators connecting inputs to outputs (grey), we have a few macros (blue) that may raise queries - this symbol legend may help fill in a few blanks;

![legend](https://github.com/StoneyDSP/OrfanidisBiquad/blob/0a9c1168752616b455d68b52a2b0b841102dfa16/Res/Workbench%20-%20Legend%20(coded%20by%20StoneyDSP).png)

* *The blue macros perform a memory allocation as part of their operation, in case you were curious. This is useful for thread safety, which I have also somewhat considered within the pseudo-code (and indeed test plugin) that follows.*  

Now I shall use a combination of visuals and pseudo-code as presented above to re-create and further investigate our various transformations available within the test plugin. Let's begin.

# Direct Form I;

Direct Form I is characterized by having a total four unit delays present within it's architecture, with two coefficients (a1 and a2) arranged as to control negative feedback gain, and the remaining three (b0 to b2) controlling positive feedback gain, with all feedback terms summed (added) together in a simple linear fashion;  

Calculus:

![Direct Form I calc](https://github.com/StoneyDSP/OrfanidisBiquad/blob/79913795b69fea84185d890bc17b1998918f8e5e/Res/DFI.svg)

Flow diagram:

![Direct Form I](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/496px-Digital_Biquad_Direct_Form_1_Untransformed.svg.png)

Implementation:

![Direct Form I core](https://github.com/StoneyDSP/OrfanidisBiquad/blob/e8a995fec9f730532160692e6f69e9800725756e/Res/DFI_core.png)

* Blue cable = audio path

* Red cable = feedback path

* Grey cable = parameter value

Pseudo-code:

    {
    
    X = input sample;
    
    _b0 = 1;
    _b1 = 0;
    _b2 = 0;
    _a0 = 1;
    _a1 = 0;
    _a2 = 0;

    a0 = (1 / a0_);

    a1 = (-1 * (a1_ * a0));
    a2 = (-1 * (a2_ * a0));

    b0 = (b0_ * a0);
    b1 = (b1_ * a0);
    b2 = (b2_ * a0);

    Y = ((X * b0) + (X(z-1) * b1) + (X(z-2) * b2) + (Y(z-1) * a1) + (Y(z-2) * a2));
    
    X(z-2) = X(z-1);
    X(z-1) = X;
    
    Y(z-2) = Y(z-1);
    Y(z-1) = Y;

    output sample = Y;
    
    }



Notes:

DFI utlizes a total of four samples of delay ("z-1"). This (comparatively) higher number of unit delays present in the DFI structure make this arrangement relatively unstable when modulating the parameters while simultaneously passing audio, resulting in loud (and potentially damaging) clicks and pops in the resulting audio. In our test workbench (running as a VST3 effect in Reaper), even just moderate sweeps of the filter frequency control can incur signal overloads significant enough to trigger the in-built "channel auto-mute" safety feature, which avoids sending potentially damaging signals to the audio playback device and speakers.

# Direct Form II;

Direct Form II (also known as the "canonical" form, at least of the two discussed thus far) uses the same arrangement of coefficents, but only two unit delays - it also has what may be viewed as a "second" feedback path, here denoted as W(n);

Calculus:

![Direct Form II calc W](https://github.com/StoneyDSP/OrfanidisBiquad/blob/79913795b69fea84185d890bc17b1998918f8e5e/Res/DFII%20w.svg)

![Direct Form II calc Y](https://github.com/StoneyDSP/OrfanidisBiquad/blob/79913795b69fea84185d890bc17b1998918f8e5e/Res/DFII%20y.svg)

Flow diagram:

![Direct Form II](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/496px-Digital_Biquad_Direct_Form_2_Untransformed.svg.png)

Implementation:

![Direct Form II core](https://github.com/StoneyDSP/OrfanidisBiquad/blob/e8a995fec9f730532160692e6f69e9800725756e/Res/DFII_core.png)

Pseudo-code:

    {
    
    X = input sample;
    
    _b0 = 1;
    _b1 = 0;
    _b2 = 0;
    _a0 = 1;
    _a1 = 0;
    _a2 = 0;

    a0 = (1 / a0_);

    a1 = (-1 * (a1_ * a0));
    a2 = (-1 * (a2_ * a0));

    b0 = (b0_ * a0);
    b1 = (b1_ * a0);
    b2 = (b2_ * a0);
    
    W = (X + ((W(z-1) * a1) + (W(z-2) * a2)));
    Y = ((W * b0) + (W(z-1) * b1) + (W(z-2) * b2));

    W(z-2) = W(z-1);
    W(z-1) = W;

    output sample = Y;
    
    }



Notes:

DFII, using less unit delays in it's architecture, produces much less significant artefacts during parameter modulation; in all but the most extreme cases, the output remains relatively benign. However, this structure is far more prone to "round-off" errors due to a narrowing computational precision in certain parts of the feedback network; this can manifest as a kind of "quantization noise" - much like un-dithered fixed-point audio - creeping well into the audible range, and in some cases enveloping low-amplitudinal parts of the input signal. This can be particularly extenuated by very large boosts of a tight "bell" shape in the lowest bass frequencies, causing strong quantization-error noise to permeate the upper-mid and treble ranges of the signal (image to follow).

# Direct Form I Transposed;

For the "transposed" forms, all terms are inverted (signal flow reversed, summing points become split points, and multpiliers swap positions), creating the same output transfer function for the same number of components but in a somewhat "mirrored" directional flow of our input signal, resulting in our coefficient multiplactions occuring *before* the unit delays;

Flow diagram:

![Direct Form IT](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/496px-Digital_Biquad_Direct_Form_1_Transformed.svg.png)

Implementation:

![Direct Form I transposed core](https://github.com/StoneyDSP/OrfanidisBiquad/blob/e8a995fec9f730532160692e6f69e9800725756e/Res/DFtI_core.png)

Pseudo-code:


    {
    
    X = input sample;
    
    _b0 = 1;
    _b1 = 0;
    _b2 = 0;
    _a0 = 1;
    _a1 = 0;
    _a2 = 0;

    a0 = (1 / a0_);

    a1 = (-1 * (a1_ * a0));
    a2 = (-1 * (a2_ * a0));

    b0 = (b0_ * a0);
    b1 = (b1_ * a0);
    b2 = (b2_ * a0);
    
    W = (X + W(z-2));
    Y = ((W * b0) + X(z-2);
    
    X(z-2) = ((W * b1) + X(z-1));
    X(z-1) = (W * b2);
    
    W(z-2) = ((W * a1) + W(z-1));
    W(z-1) = (W * a2);
    
    output sample = Y;
    
    }
    


Notes:

The transposed Direct Form I ("DFI(t)") utilizes the four unit-delays of it's predecessor, meaning a higher memory footprint, while it *also* incurs the exact same "round-off error" and quantization noise as the original DFII structure. This is a dangerous combination for real-world audio use-cases, such as a mixing scenario.

So far, we've encountered one transformation resulting in zipper noise, one resulting in quantization noise, and one resulting in *both*.

This may seem discouraging, but we still have one final arrangement to try.

# Direct Form II Transposed;

Direct Form II transposed only requires the two unit delays (like it's non-transposed counterpart), as opposed to the four of the Direct Form I (both counterparts), and likewise features it's multiplication coefficients happening before the unit delays occur;

Calculus:

![Direct Form IIT calc y](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/DFIIt%20y.svg)

![Direct Form IIT calc s1](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/DFIIt%20s1.svg)

![Direct Form IIT calc s2](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/DFIIt%20s2.svg)

Flow diagram:

![Direct Form IIT](https://github.com/StoneyDSP/OrfanidisBiquad/blob/532db8ad616b7c0ed7f51e13772adba6b54e5cf3/Res/496px-Digital_Biquad_Direct_Form_2_Transformed.svg.png)

Implementation:
    
![Direct Form II transposed core](https://github.com/StoneyDSP/OrfanidisBiquad/blob/e8a995fec9f730532160692e6f69e9800725756e/Res/DFtII_core.png)

Pseudo-code:

    {
    
    X = input sample;
    
    _b0 = 1;
    _b1 = 0;
    _b2 = 0;
    _a0 = 1;
    _a1 = 0;
    _a2 = 0;

    a0 = (1 / a0_);

    a1 = (-1 * (a1_ * a0));
    a2 = (-1 * (a2_ * a0));

    b0 = (b0_ * a0);
    b1 = (b1_ * a0);
    b2 = (b2_ * a0);
    
    Y = ((X * b0) + (X(z-2));
    
    X(z-2) = ((X * b1) + (X(z-1)) + (Y * a1));
    X(z-1) = ((X * b2) + (Y * a2));
    
    output sample = Y;
    
    }
    


Notes:

The Transposed Direct Form II, similarly to it's predecessor, uses only two unit-delays, making it much more amenable to audio-rate modulation; meanwhile, this form also successfully manages to avoid the higher "round-off" error and quantization noise of it's predecessor (and the DFI(t) structure).

# Observations and comparisons;

As depicted in the above diagrams, the Direct Form I ("DFI") and II ("DFII") utlize a chain of single-sample unit delays in a feedback arrangement, with the coefficients a1 through to b2 controlling the gain at various points in the feedback network (in the case of DFII, actually two feedback networks).

The two "transposed" forms provide us the same output characteristics for the same number of components, but arranged in inverse terms as compared with the non-transposed forms; summing points become split points, gain multpiliers swap positions within the network, and the entire signal flow is reversed (our images are also flipped around to keep the input and output directions visually aligned with the previous structures). 

This transposition places our coefficient multipliers *before* our unit delays, instead of after them.

I have seen some texts which provide several insights into what this change means with regards to our system stability and performance; one particularly intriguing which suggests that the transposed arrangements cause the unit delays (assumedly at their concurrent point of summation) act as a kind of smoother between changed values - perhaps very much like a single-pole low-pass filter? Another consideration might be the precision of our unit delays - let us imagine, for a moment, that our unit delays were operating at a lowly 16-bit fixed point arithmetic, causing truncation of the lowest bit ("least significant bit") of the audio data that we feed it. If we were to pipe our audio into our low-precision unit delay with a very low gain value (for example, we multiply by 0.0625 on the way in), and then compensate with a corresponding gain boost on the way out, we will have lost several bits of the "least significant part" of our audio to the low-precision unit delay. By contrast, if we were instead to pipe our audio into the unit delay at *unity* gain and then simply scale it to the appropriate level afterward, this gain scaling (coming after the unit-delay's truncation happens) does *not* affect our audio precision at all; any loss of "least significant" data going into the unit-delay has already happened before this scaling occurs.

Of course, our circuit is much more complicated than this, especially as the number of poles (and zeros) grows - but these outlined concepts might at least help us gain some bearings on *why* these real-time performance differences arise between supposedly equal circuits.

Now, if we compare our results side by side, the results are interesting; 

+ DFI = four delay units (unstable modulation and higher footprint), higher precision (less quantization noise)
+ DFII =  two delay units (stable modulation and lower footprint), lower precision (more quantization noise)
+ DFI(t) = four delay units (unstable modulation and higher footprint), lower precision (more quantization noise)
+ DFII(t) = two delay units (stable modulation and lower footprint), higher precision (less quantization noise)

(images to follow)
    
# And the winner is...;

As we can observe from the above comparison, our DFII(t) structure is the most favourable in all cases - it has only two delay units, meaning it is more stable under modulation, favourably comparable to the DFII structure; it also produces less quantization noise, comparable to the DFI structure in this regard. The lower unit delay count also produces a lower memory footprint in realtime use.

Our DFI(t) structure performs the most poorly in our highlighted cases - the quantization round-off error compares unfavourably to the DFII structure; while the four delay units contribute to heavy and unpredictable click/pop "zipper" noise underparameter modulation and also a higher memory footprint.

For a plugin designed for real audio mixing use cases, it's good practice to apply an appropriate amount of smoothing to all ranged parameters, in most cases. For now, smoothing has intentionally *not* been applied to the plugin's parameters, as this allows us to observe the differences between the four tranforms with respect to audio-rate modulation, one of the key issues with this entire filter design.

A parameter smoother may be applied in a future revision. However, during testing so far, it has been observed that a smoothed parameter, derived from a clock source that is likely asynchronous to the audio buffer clock, generates an unusual notch-like filter effect on the resulting audio spectrum whilst said parameter is under modulation; the deterministic frequency of this "notch-like" effect is seemingly related in periodicity to the parameter smoother's clock speed. Increasing precision to Doubles also seemingly eradicates the "notch-like periodicity" that our parameter smoother incurrs on the waveform to imperceptible levels.

Likewise, the quantization noise created by the feedback network's computational round-off errors can be seemingly entirely negated by increasing processing precision from Floats to Doubles; the resulting noise floor falls not only well below the audible threshold, but also below the reach of abilities of our testing software.

However, out of sight and out of mind does not mean out the window; we are able to produce several very pronounced audible artefacts in three of the four structures when processing in Floats (commonly deemed to be a beyond acceptable processing precision for audio purposes, to be debated elsewhere). Indeed only the Transposed Direct Form II manages favourably in all cases, and thus appears to be the prime candidate transformation for Biquad-based Equalizers in all audio application contexts at the time of writing. It stands to reason that these differences in quality shall also hold true (to some degree) for processing in Doubles, although we'd be extremely unlikely to encounter these differences when processing at that level of precision, seemingly well beyond the scope of measurement of our analysis tools - especially, and most critically, our ears. 

^ Credit: Native Instruments for the Direct Form I code (taken from Reaktor 5's Core "Static Filter" library - go figure!) as well as the Core library unit delay, audio thread, and math modulation macros used here (I programmed the three other forms myself; both in Core and in C++).

- Nathan Hood (StoneyDSP), May 2022.

^^ Reference: Transformations images taken from: https://en.wikipedia.org/wiki/Digital_biquad_filter
