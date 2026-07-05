#include <m2/audio/AudioManager.h>
#include <m2/common/Error.h>
#include <m2/Game.h>
#include <m2/Options.h>

m2::AudioManager::Playback::Playback(const Song* _song, float _volume, PlayPolicy _play_policy) : song(_song), volume(_volume), playPolicy(_play_policy) {}

m2::AudioManager::AudioManager() : device(thirdparty::audio::Device::Open(AudioCallback, nullptr)) {}

m2::PlaybackId m2::AudioManager::Play(const Song* song, PlayPolicy policy, float volume) {
	if (song->SampleCount() < device.ObtainedBufferSizeInSamples()) {
		throw M2_ERROR("Playing short audio is not supported");
	}

	// Emplace under the lock, but release it before calling into the SDL device. SDL's audio callback thread might
	// invoke AudioManager::AudioCallback (which takes playbacksMutex) immediately while holding SDL's internal lock.
	PlaybackId playbackId;
	{
		std::unique_lock lock{playbacksMutex};
		playbackId = playbacks.Emplace(song, volume, policy).GetId();
	}
	device.Resume();
	return playbackId;
}

void m2::AudioManager::Stop(PlaybackId id) {
	// Decide whether the device should pause under the lock, but call device.Pause() only after releasing
	// it, for the same lock-ordering reason as Play().
	bool shouldPauseDevice;
	{
		std::unique_lock lock{playbacksMutex};
		playbacks.Free(id);
		shouldPauseDevice = playbacks.Size() == 0;
	}
	if (shouldPauseDevice) {
		device.Pause();
	}
}

bool m2::AudioManager::HasPlayback(PlaybackId id) {
	std::unique_lock lock{playbacksMutex};
	return playbacks.Get(id) != nullptr;
}
void m2::AudioManager::SetPlaybackVolume(PlaybackId id, float volume) {
	std::unique_lock lock{playbacksMutex};
	if (auto* playback = playbacks.Get(id); playback) {
		playback->volume = volume;
	}
}
void m2::AudioManager::SetPlaybackLeftVolume(PlaybackId id, float volume) {
	std::unique_lock lock{playbacksMutex};
	if (auto* playback = playbacks.Get(id); playback) {
		playback->leftVolume = volume;
	}
}
void m2::AudioManager::SetPlaybackRightVolume(PlaybackId id, float volume) {
	std::unique_lock lock{playbacksMutex};
	if (auto* playback = playbacks.Get(id); playback) {
		playback->rightVolume = volume;
	}
}

void m2::AudioManager::AudioCallback(MAYBE void* user_data, uint8_t* stream, int length) {
	auto& audio_manager = *M2_GAME.audio_manager;
	auto* out_stream = reinterpret_cast<audio::synthesizer::AudioSample*>(stream);
	auto out_length = static_cast<size_t>(length) / sizeof(audio::synthesizer::AudioSample); // in samples

	// Clear buffer
	std::fill_n(out_stream, out_length, audio::synthesizer::AudioSample{});

	auto copy = [=](Playback* playback, size_t copy_count) {
		const auto* begin = playback->song->Data() + playback->nextSample;
		const auto* end = begin + copy_count;

		for (auto it = begin; it != end; ++it) {
			if (silent) {
				out_stream[it - begin] = {};
			} else {
				const auto& playback_sample = *it;
				auto l_playback_sample = playback_sample.l * playback->volume * playback->leftVolume;
				auto r_playback_sample = playback_sample.r * playback->volume * playback->rightVolume;
				out_stream[it - begin] += audio::synthesizer::AudioSample(l_playback_sample, r_playback_sample);
			}
		}

		playback->nextSample = (playback->nextSample + copy_count) % playback->song->SampleCount();
	};

	std::unique_lock lock{audio_manager.playbacksMutex};
	for (auto it = audio_manager.playbacks.begin(); it != audio_manager.playbacks.end(); ++it) {
		// Copy the samples left in the playback buffer
		auto samples_left_playback_buffer = it->song->SampleCount() - it->nextSample;
		auto min_len = std::min(samples_left_playback_buffer, out_length);
		copy(&*it, min_len);

		if (it->playPolicy == PlayPolicy::ONCE && it->nextSample == 0) {
			// Playback finished
			audio_manager.playbacks.Free(it.GetId());
		} else {
			// Playback wrapped around and there is space in output buffer
			copy(&*it, out_length - min_len);
		}
	}
}
