import argparse
import random
import struct
import wave


def generate_wav(filename, length, **kwargs):
    sample_rate = kwargs.get('sample_rate', 16000)
    amplitude = kwargs.get('amplitude', 32767.0)

    num_samples = int(length * sample_rate)
    samples = bytearray()
    for _ in range(num_samples):
        random_sample = random.uniform(-amplitude, amplitude)
        samples.extend(struct.pack('<e', random_sample))

    with wave.open(filename, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(sample_rate)

        out.writeframes(samples)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Create WAV file with random content in it.")
    parser.add_argument('output',
                        help="Name of the output file.")
    parser.add_argument('length', type=float,
                        help="Length of the file in seconds.")

    args = parser.parse_args()

    generate_wav(args.output, args.length)
