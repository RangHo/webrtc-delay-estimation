import argparse
import random
import wave


def delay_wav(filename: str, delay_samples: int, output_filename: str = None, randomize: bool = False):
    """
    Creates a delayed copy of the given .wav file.
    """
    input_wav = wave.open(filename, 'rb')

    # Information required to create delayed waves
    sample_rate = input_wav.getframerate()
    sample_width = input_wav.getsampwidth()

    # Setup the output file
    output_wav = wave.open(
        output_filename if output_filename else "delayed.wav",
        'wb'
    )
    output_wav.setframerate(sample_rate)
    output_wav.setsampwidth(sample_width)
    output_wav.setnchannels(input_wav.getnchannels())

    # Create delay buffer
    print("Delaying samples: ", delay_samples)
    delay_buffer = bytearray()
    if randomize:
        for _ in range(delay_samples * sample_width):
            delay_buffer.append(random.randrange(0, 256))
    else:
        delay_buffer.extend(b'\x00' * delay_samples * sample_width)

    # Write delay buffer to the output file
    output_wav.writeframes(delay_buffer)

    input_bytes = input_wav.readframes(input_wav.getnframes())
    output_wav.writeframes(input_bytes)

    input_wav.close()
    output_wav.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Create delayed WAV files.")
    parser.add_argument('input',
                        help="Name of the input file.")
    parser.add_argument('delay', type=int,
                        help="Amount of delay samples.")
    parser.add_argument('output', nargs='?', default='delay.wav',
                        help="Name of the output file.")
    parser.add_argument('--random', action='store_true',
                        help="Prepend random bytes instead of 0.")

    args = parser.parse_args()

    delay_wav(
        args.input,
        args.delay,
        args.output,
        args.random
    )
