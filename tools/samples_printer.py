import matplotlib.pyplot as plt

# Usage: python samples_printer.py 10,20,30,40,50,
def main() -> None:
	SAMPLES = ""

	samples = SAMPLES.split(",")
	samples = samples[:-1] if samples[-1] == "" else samples	# Handling last ",".
	samples = [int(x) for x in samples]

	AMP_MIN = min(samples)
	AMP_MAX = max(samples)
	AMP_PP = AMP_MAX - AMP_MIN

	print("Len: %d" % len(samples))
	print("Min: %d" % AMP_MIN)
	print("Max: %d" % AMP_MAX)
	print("Amp-PP: %d" % AMP_PP)

	plt.title("Samples")
	plt.xlabel("x")
	plt.ylabel("y")

	plt.plot(samples)
	plt.show()

if __name__ == "__main__":
	main()
