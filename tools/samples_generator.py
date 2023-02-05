import sys, math
import matplotlib.pyplot as plt

# Usage: python samples_generator.py AMP SAMPLES_NUM
def main() -> None:
	assert len(sys.argv) == 3

	try:
		AMP = int(sys.argv[1])
		SAMPLES_NUM = int(sys.argv[2])
	
	except:
		sys.exit("Parameter errors.")

	samples = generate_samples(AMP, SAMPLES_NUM)

	print("Min: %d" % min(samples))
	print("Max: %d" % max(samples))

	print()
	print("const uint16_t SAMPLES_LEN = %d;" % len(samples))
	print("const uint8_t SAMPLES[] = %s;" % str(samples).replace("[", "{ ").replace("]", " }"))

	# plt.title("Samples")
	# plt.xlabel("n")
	# plt.ylabel("y")

	# plt.plot(samples)
	# plt.show()

# Sample at the given sampling frequency, one period from a sinewave with the given amp.
def generate_samples(amp: int, samples_num: int) -> list[int]:
	samples: list[int] = []

	PERIOD_US = 1000000
	SAMPLING_PERIOD_US = int(PERIOD_US / samples_num)

	DC_SHIFT = amp/2
	
	for x in range(0, PERIOD_US, SAMPLING_PERIOD_US):
		samples.append(int(DC_SHIFT + DC_SHIFT * math.sin(2 * math.pi * x/PERIOD_US)))

	return samples

if __name__ == "__main__":
	main()
