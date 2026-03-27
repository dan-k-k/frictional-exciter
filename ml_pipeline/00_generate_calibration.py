# ml_pipeline/00_generate_calibration.py
import numpy as np
from scipy.io import wavfile

def generate_calibration_track(filename="data/inputs/calibration_track.wav", sample_rate=48000):
    # Total duration: 3.5 seconds (gives the final swell room to ring out)
    duration = 3.5
    total_samples = int(sample_rate * duration)
    
    # Create an empty array of zeroes for our audio track
    audio = np.zeros(total_samples)

    # ==========================================
    # 1. THE CLICK (Starts at 0.0s)
    # A 5-millisecond burst of white noise with a fast fade-out
    # ==========================================
    click_duration = 0.010
    click_samples = int(sample_rate * click_duration)
    
    # Generate raw white noise
    noise = np.random.uniform(-1.0, 1.0, click_samples)
    
    # Apply a linear fade-out envelope so it doesn't "pop" at the end
    click_env = np.linspace(1.0, 0.0, click_samples)
    audio[0:click_samples] = noise * click_env

    # ==========================================
    # 2. THE PLUCK (Starts at 1.0s)
    # A dry, 110Hz (A2) synth bass pluck using FM-style harmonics
    # ==========================================
    pluck_start = int(sample_rate * 1.0)
    pluck_duration = 0.5
    pluck_samples = int(sample_rate * pluck_duration)
    t_pluck = np.linspace(0, pluck_duration, pluck_samples, endpoint=False)
    
    # Generate a fundamental frequency + 1 harmonic for a "synth" tone
    freq = 110.0
    pluck_wave = np.sin(2 * np.pi * freq * t_pluck) + 0.5 * np.sin(2 * np.pi * (freq * 2) * t_pluck)
    
    # Apply a sharp exponential decay envelope (the "pluck" shape)
    pluck_env = np.exp(-12.0 * t_pluck) 
    audio[pluck_start : pluck_start + pluck_samples] = pluck_wave * pluck_env

    # ==========================================
    # 3. THE SWELL (Starts at 2.0s)
    # A 220Hz (A3) pure sine wave, 1 second long, fading in and out
    # ==========================================
    swell_start = int(sample_rate * 2.0)
    swell_duration = 1.0
    swell_samples = int(sample_rate * swell_duration)
    t_swell = np.linspace(0, swell_duration, swell_samples, endpoint=False)
    
    # Generate pure sine wave
    swell_wave = np.sin(2 * np.pi * 220.0 * t_swell)
    
    # A "Hann window" creates a perfect, smooth bell-curve envelope (fades in, then out)
    swell_env = np.hanning(swell_samples)
    audio[swell_start : swell_start + swell_samples] = swell_wave * swell_env

    # ==========================================
    # FINAL EXPORT
    # ==========================================
    # Normalize the audio to exactly -3dB to prevent any clipping when fed into your C++ engine
    max_amp = np.max(np.abs(audio))
    audio_normalized = (audio / max_amp) * 0.707 

    # Convert the 32-bit float array to standard 16-bit PCM audio
    audio_16bit = np.int16(audio_normalized * 32767)

    # Save the file
    wavfile.write(filename, sample_rate, audio_16bit)
    print(f"Success! {filename} generated.")

if __name__ == "__main__":
    # Ensure the data/inputs directory exists before running, or change the filepath above!
    import os
    os.makedirs("data/inputs", exist_ok=True)
    generate_calibration_track()

