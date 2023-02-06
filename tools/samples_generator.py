import sys, math
import matplotlib.pyplot as plt

# Usage: python samples_generator.py SAMPLES_NUM AMP
def main() -> None:
	assert len(sys.argv) == 3

	try:
		SAMPLES_NUM = int(sys.argv[1])
		AMP = int(sys.argv[2])
	
	except:
		sys.exit("Parameter errors.")

	samples = generate_samples(SAMPLES_NUM, AMP)

	AMP_MIN = min(samples)
	AMP_MAX = max(samples)
	AMP_PP = AMP_MAX - AMP_MIN

	print("Min: %d" % AMP_MIN)
	print("Max: %d" % AMP_MAX)
	print("Amp-PP: %d" % AMP_PP)

	print()
	print("const uint16_t SAMPLES_LEN = %d;" % len(samples))
	print("const uint8_t SAMPLES[] = %s;" % str(samples).replace("[", "{ ").replace("]", " }"))

	# plt.title("Samples")
	# plt.xlabel("n")
	# plt.ylabel("y")

	# plt.plot(samples)
	# plt.show()

# Sample at the given sampling frequency, one period from a sinewave with the given amp.
def generate_samples(samples_num: int, amp: int) -> list[int]:
	samples: list[int] = []

	PERIOD_US = 1000000
	SAMPLING_PERIOD_US = int(PERIOD_US / samples_num)

	DC_SHIFT = 127
	
	for x in range(0, PERIOD_US, SAMPLING_PERIOD_US):
		samples.append(int(DC_SHIFT + amp/2 * math.sin(2 * math.pi * x/PERIOD_US)))

	return samples

if __name__ == "__main__":
	main()
