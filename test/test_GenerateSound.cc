#include <gtest/gtest.h>
#include <m2/synth/MixNote.h>

TEST(GenerateSound, sine_simple) {
	m2::pb::SynthSound sound;
	sound.set_shape(m2::pb::SoundWaveShape::SINE);
	sound.set_amplitude(1.0f);

	std::vector<m2::SynthSample> buffer(120000);
	m2::synth::generate_sound(buffer.begin(), buffer.end(), 0, sound, 440.0f);

	EXPECT_FLOAT_EQ(buffer[0], 0.0f);
	EXPECT_FLOAT_EQ(buffer[1], 0.05756402696f);
	EXPECT_FLOAT_EQ(buffer[48000], 0.0f);
	EXPECT_FLOAT_EQ(buffer[48001],  0.05756402696f);
	EXPECT_FLOAT_EQ(buffer[100000], -0.8660254038f);
	EXPECT_FLOAT_EQ(buffer[100001], -0.8933713883f);
}

TEST(GenerateSound, sine_concat) {
	m2::pb::SynthSound sound;
	sound.set_shape(m2::pb::SoundWaveShape::SINE);
	sound.set_amplitude(1.0f);

	std::vector<m2::SynthSample> buffer(120000);
	auto next_phase_1 = m2::synth::generate_sound(buffer.begin(), buffer.begin() + 40000, 0, sound, 440.0f, 1.0f);
	auto next_phase_2 = m2::synth::generate_sound(buffer.begin() + 40000, buffer.begin() + 80000, 0, sound, 440.0f, 1.0f, next_phase_1);
	m2::synth::generate_sound(buffer.begin() + 80000, buffer.end(), 0, sound, 440.0f, 1.0f, next_phase_2);

	EXPECT_FLOAT_EQ(buffer[0], 0.0f);
	EXPECT_FLOAT_EQ(buffer[1], 0.05756402696f);
	EXPECT_FLOAT_EQ(buffer[48000], 0.0f);
	EXPECT_FLOAT_EQ(buffer[48001],  0.05756402696f);
	EXPECT_FLOAT_EQ(buffer[100000], -0.8660254038f);
	EXPECT_FLOAT_EQ(buffer[100001], -0.8933713883f);
}
