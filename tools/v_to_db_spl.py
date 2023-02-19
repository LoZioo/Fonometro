import sys, math

# Usage: python samples_generator.py IN_VOLTAGE
# https://www.dsprelated.com/showthread/audiodsp/1153-1.php

# Used mic sensitivity: -50db(V/Pa)

MIC_SENSITIVITY_DB = -50	# db(V/Pa) (reference: 1V/PA).
MIN_SPL_THR = 2e-5				# Reference: 20uPa
AMP_GAIN = 9.11						# 310mV / 34mV

def main() -> None:
	try:
		IN_VOLTAGE = float(sys.argv[2])

	except:
		sys.exit("Parameter errors.")

	MIC_SENSITIVITY = 10 ** (MIC_SENSITIVITY_DB / 20)		# V/Pa
	MIC_VOLTAGE = IN_VOLTAGE / AMP_GAIN

	db_spl = 20 * math.log10(MIC_VOLTAGE / (MIC_SENSITIVITY * MIN_SPL_THR))

	print(db_spl)

if __name__ == "__main__":
	main()
