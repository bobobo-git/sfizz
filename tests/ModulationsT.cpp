// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/modulations/ModId.h"
#include "sfizz/modulations/ModKey.h"
#include "sfizz/Synth.h"
#include "TestHelpers.h"
#include "catch2/catch.hpp"

TEST_CASE("[Modulations] Identifiers")
{
    // check that modulations are well defined as either source and target
    // and all targets have their default value defined

    sfz::ModIds::forEachSourceId([](sfz::ModId id)
    {
        REQUIRE(sfz::ModIds::isSource(id));
        REQUIRE(!sfz::ModIds::isTarget(id));
    });

    sfz::ModIds::forEachTargetId([](sfz::ModId id)
    {
        REQUIRE(sfz::ModIds::isTarget(id));
        REQUIRE(!sfz::ModIds::isSource(id));
    });
}

TEST_CASE("[Modulations] Flags")
{
    // check validity of modulation flags

    static auto* checkBasicFlags = +[](int flags)
    {
        REQUIRE(flags != sfz::kModFlagsInvalid);
        REQUIRE((bool(flags & sfz::kModIsPerCycle) +
                 bool(flags & sfz::kModIsPerVoice)) == 1);
    };
    static auto* checkSourceFlags = +[](int flags)
    {
        checkBasicFlags(flags);
        REQUIRE((bool(flags & sfz::kModIsAdditive) +
                 bool(flags & sfz::kModIsMultiplicative) +
                 bool(flags & sfz::kModIsPercentMultiplicative)) == 0);
    };
    static auto* checkTargetFlags = +[](int flags)
    {
        checkBasicFlags(flags);
        REQUIRE((bool(flags & sfz::kModIsAdditive) +
                 bool(flags & sfz::kModIsMultiplicative) +
                 bool(flags & sfz::kModIsPercentMultiplicative)) == 1);
    };

    sfz::ModIds::forEachSourceId([](sfz::ModId id)
    {
        checkSourceFlags(sfz::ModIds::flags(id));
    });

    sfz::ModIds::forEachTargetId([](sfz::ModId id)
    {
        checkTargetFlags(sfz::ModIds::flags(id));
    });
}

TEST_CASE("[Modulations] Display names")
{
    // check all modulations are implemented in `toString`

    sfz::ModIds::forEachSourceId([](sfz::ModId id)
    {
        REQUIRE(!sfz::ModKey(id).toString().empty());
    });

    sfz::ModIds::forEachTargetId([](sfz::ModId id)
    {
        REQUIRE(!sfz::ModKey(id).toString().empty());
    });
}

TEST_CASE("[Modulations] Connection graph from SFZ")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
<region>
sample=*sine
amplitude_oncc20=59 amplitude_curvecc20=3
pitch_oncc42=71 pitch_smoothcc42=32
pan_oncc36=12.5 pan_stepcc36=0.5
width_oncc425=29
)");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("Controller 20 {curve=3, smooth=0, step=0}" -> "Amplitude {0}")",
        R"("Controller 42 {curve=0, smooth=32, step=0}" -> "Pitch {0}")",
        R"("Controller 36 {curve=0, smooth=0, step=0.04}" -> "Pan {0}")",
        R"("Controller 425 {curve=0, smooth=0, step=0}" -> "Width {0}")",
    }));
}

TEST_CASE("[Modulations] Filter CC connections")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
        <region> sample=*sine
        cutoff=100 fil1_gain_oncc3=5 fil1_gain_stepcc3=0.5
        cutoff2=300 cutoff2_cc2=100 cutoff2_curvecc2=2
        resonance3=-1 resonance3_oncc1=2 resonance3_smoothcc1=10
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("Controller 1 {curve=0, smooth=10, step=0}" -> "FilterResonance {0, N=3}")",
        R"("Controller 2 {curve=2, smooth=0, step=0}" -> "FilterCutoff {0, N=2}")",
        R"("Controller 3 {curve=0, smooth=0, step=0.1}" -> "FilterGain {0, N=1}")",
    }));
}

TEST_CASE("[Modulations] EQ CC connections")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
        <region> sample=*sine
        eq1_gain_oncc2=5 eq1_gain_stepcc2=0.5
        eq2_freq_oncc3=300 eq2_freq_curvecc3=3
        eq3_bw_oncc1=2 eq3_bw_smoothcc1=10
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("Controller 1 {curve=0, smooth=10, step=0}" -> "EqBandwidth {0, N=3}")",
        R"("Controller 2 {curve=0, smooth=0, step=0.1}" -> "EqGain {0, N=1}")",
        R"("Controller 3 {curve=3, smooth=0, step=0}" -> "EqFrequency {0, N=2}")",
    }));
}

TEST_CASE("[Modulations] LFO Filter connections")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
        <region> sample=*sine
        lfo1_freq=0.1 lfo1_cutoff1=1
        lfo2_freq=1 lfo2_cutoff=2
        lfo3_freq=2 lfo3_resonance=3
        lfo4_freq=0.5 lfo4_resonance1=4
        lfo5_freq=0.5 lfo5_resonance2=5
        lfo6_freq=3 lfo6_fil1gain=-1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("LFO 1 {0}" -> "FilterCutoff {0, N=1}")",
        R"("LFO 2 {0}" -> "FilterCutoff {0, N=1}")",
        R"("LFO 3 {0}" -> "FilterResonance {0, N=1}")",
        R"("LFO 4 {0}" -> "FilterResonance {0, N=1}")",
        R"("LFO 5 {0}" -> "FilterResonance {0, N=2}")",
        R"("LFO 6 {0}" -> "FilterGain {0, N=1}")",
    }));
}

TEST_CASE("[Modulations] EG Filter connections")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
        <region> sample=*sine
        eg1_time1=0.1 eg1_cutoff1=1
        eg2_time1=1 eg2_cutoff=2
        eg3_time1=2 eg3_resonance=3
        eg4_time1=0.5 eg4_resonance1=4
        eg5_time1=0.5 eg5_resonance2=5
        eg6_time1=3 eg6_fil1gain=-1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("EG 1 {0}" -> "FilterCutoff {0, N=1}")",
        R"("EG 2 {0}" -> "FilterCutoff {0, N=1}")",
        R"("EG 3 {0}" -> "FilterResonance {0, N=1}")",
        R"("EG 4 {0}" -> "FilterResonance {0, N=1}")",
        R"("EG 5 {0}" -> "FilterResonance {0, N=2}")",
        R"("EG 6 {0}" -> "FilterGain {0, N=1}")",
    }));
}

TEST_CASE("[Modulations] LFO EQ connections")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
        <region> sample=*sine
        lfo1_freq=0.1 lfo1_eq1bw=1
        lfo2_freq=1 lfo2_eq2freq=2
        lfo3_freq=2 lfo3_eq3gain=3
        lfo4_freq=0.5 lfo4_eq3bw=4
        lfo5_freq=0.5 lfo5_eq2gain=5
        lfo6_freq=3 lfo6_eq1freq=-1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("LFO 1 {0}" -> "EqBandwidth {0, N=1}")",
        R"("LFO 2 {0}" -> "EqFrequency {0, N=2}")",
        R"("LFO 3 {0}" -> "EqGain {0, N=3}")",
        R"("LFO 4 {0}" -> "EqBandwidth {0, N=3}")",
        R"("LFO 5 {0}" -> "EqGain {0, N=2}")",
        R"("LFO 6 {0}" -> "EqFrequency {0, N=1}")",
    }));
}

TEST_CASE("[Modulations] EG EQ connections")
{
    sfz::Synth synth;
    synth.loadSfzString("/modulation.sfz", R"(
        <region> sample=*sine
        eg1_freq=0.1 eg1_eq1bw=1
        eg2_freq=1 eg2_eq2freq=2
        eg3_freq=2 eg3_eq3gain=3
        eg4_freq=0.5 eg4_eq3bw=4
        eg5_freq=0.5 eg5_eq2gain=5
        eg6_freq=3 eg6_eq1freq=-1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createDefaultGraph({
        R"("EG 1 {0}" -> "EqBandwidth {0, N=1}")",
        R"("EG 2 {0}" -> "EqFrequency {0, N=2}")",
        R"("EG 3 {0}" -> "EqGain {0, N=3}")",
        R"("EG 4 {0}" -> "EqBandwidth {0, N=3}")",
        R"("EG 5 {0}" -> "EqGain {0, N=2}")",
        R"("EG 6 {0}" -> "EqFrequency {0, N=1}")",
    }));
}


TEST_CASE("[Modulations] FlexEG Ampeg target")
{
    sfz::Synth synth;

    synth.loadSfzString(fs::current_path(), R"(
        <region> sample=*sine
        eg1_time1=0  eg1_level1=1
        eg1_time2=1  eg1_level2=0
        eg1_time3=1  eg1_level3=.5 eg1_sustain=3
        eg1_time4=1  eg1_level4=1
        eg1_ampeg=1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createModulationDotGraph({
        R"("Controller 10 {curve=1, smooth=10, step=0}" -> "Pan {0}")",
        R"("Controller 11 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("Controller 7 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("EG 1 {0}" -> "MasterAmplitude {0}")",
    }));
}

TEST_CASE("[Modulations] FlexEG Ampeg target with 2 FlexEGs")
{
    sfz::Synth synth;

    synth.loadSfzString(fs::current_path(), R"(
        <region> sample=*sine
        eg1_time1=0  eg1_level1=1
        eg1_time2=1  eg1_level2=0
        eg1_time3=1  eg1_level3=.5 eg1_sustain=3
        eg1_time4=1  eg1_level4=1
        eg2_time1=0  eg2_level1=1
        eg2_time2=1  eg2_level2=0
        eg2_time3=1  eg2_level3=.5 eg1_sustain=3
        eg2_ampeg=1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createModulationDotGraph({
        R"("Controller 10 {curve=1, smooth=10, step=0}" -> "Pan {0}")",
        R"("Controller 11 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("Controller 7 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("EG 2 {0}" -> "MasterAmplitude {0}")",
    }));
}


TEST_CASE("[Modulations] FlexEG Ampeg target with multiple EGs targeting ampeg")
{
    sfz::Synth synth;

    synth.loadSfzString(fs::current_path(), R"(
        <region> sample=*sine
        eg1_time1=0  eg1_level1=1
        eg1_time2=1  eg1_level2=0
        eg1_time3=1  eg1_level3=.5 eg1_sustain=3
        eg1_time4=1  eg1_level4=1
        eg1_ampeg=1
        eg2_time1=0  eg2_level1=1
        eg2_time2=1  eg2_level2=0
        eg2_time3=1  eg2_level3=.5 eg1_sustain=3
        eg2_ampeg=1
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createModulationDotGraph({
        R"("Controller 10 {curve=1, smooth=10, step=0}" -> "Pan {0}")",
        R"("Controller 11 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("Controller 7 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("EG 1 {0}" -> "MasterAmplitude {0}")",
    }));
}

TEST_CASE("[Modulations] Override the default volume controller")
{
    sfz::Synth synth;

    synth.loadSfzString(fs::current_path(), R"(
        <region> sample=*sine tune_oncc7=1200
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createModulationDotGraph({
        R"("AmplitudeEG {0}" -> "MasterAmplitude {0}")",
        R"("Controller 10 {curve=1, smooth=10, step=0}" -> "Pan {0}")",
        R"("Controller 11 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("Controller 7 {curve=0, smooth=0, step=0}" -> "Pitch {0}")",
    }));
}

TEST_CASE("[Modulations] Override the default pan controller")
{
    sfz::Synth synth;

    synth.loadSfzString(fs::current_path(), R"(
        <region> sample=*sine on_locc10=127 on_hicc10=127
    )");

    const std::string graph = synth.getResources().modMatrix.toDotGraph();
    REQUIRE(graph == createModulationDotGraph({
        R"("AmplitudeEG {0}" -> "MasterAmplitude {0}")",
        R"("Controller 11 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
        R"("Controller 7 {curve=4, smooth=10, step=0}" -> "Amplitude {0}")",
    }));
}
