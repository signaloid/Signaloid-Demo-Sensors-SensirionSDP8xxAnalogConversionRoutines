/*
 *	Copyright (c) 2024, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <float.h>
#include <uxhw.h>
#include "utilities.h"


/**
 *	@brief  Implementation of the sign function for distributional values.
 *
 *	@param  arg	: Sign function argument.
 *	@return double	: The result of the sign function.
 */
static double
sign(double arg)
{
	if (arg == 0.0)
	{
		return 0.0;
	}

	return arg / fabs(arg);
}

/**
 *	@brief  Sets the Input Distributions via calls to UxHw API functions.
 *
 *	@param  inputDistributions	: An array of double values, where the function writes the distributional data.
 */
static void
setInputDistributionsViaUxHwCall(double *  inputDistributions)
{
	inputDistributions[kInputDistributionIndexAout] = UxHwDoubleUniformDist(
								kDefaultInputDistributionAoutUniformDistLow,
								kDefaultInputDistributionAoutUniformDistHigh);

	inputDistributions[kInputDistributionIndexVdd] = UxHwDoubleUniformDist(
								kDefaultInputDistributionVddUniformDistLow,
								kDefaultInputDistributionVddUniformDistHigh);

	return;
}

/**
 *	@brief  Sensor calibration routines for different modes taken from
 *		SDP8xx Analog Datasheet, 2024-07-03.
 *
 *	@param  arguments		: Pointer to command-line arguments struct.
 *	@param  inputDistributions	: The array of input distributions used in the calculation.
 * 	@param  outputDistributions	: An array of of output distributions.
 * 						Writes the result to `outputDistributions[outputSelectValue]`.
 *
 *	@return	double			: Returns the distributional value calculated.
 */
static double
calculateSensorOutput(CommandLineArguments *  arguments, double *  inputDistributions, double *  outputDistributions)
{
	double	Vdd = inputDistributions[kInputDistributionIndexVdd];
	double	Aout = inputDistributions[kInputDistributionIndexAout];
	double	calibratedValue;

	bool	calculateAllOutputs = (arguments->common.outputSelect == kOutputDistributionIndexCalibratedSensorOutputMax);

	if (calculateAllOutputs ||
		(arguments->common.outputSelect == kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear500Pa))
	{
		calibratedValue = kSensorCalibrationConstantSDP8x6Linear500Pa1 * Aout / Vdd - kSensorCalibrationConstantSDP8x6Linear500Pa2;
		outputDistributions[kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear500Pa] = calibratedValue;
	}

	if (calculateAllOutputs ||
		(arguments->common.outputSelect == kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear125Pa))
	{
		calibratedValue = kSensorCalibrationConstantSDP8x6Linear125Pa1 * Aout / Vdd - kSensorCalibrationConstantSDP8x6Linear125Pa2;
		outputDistributions[kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear125Pa] = calibratedValue;
	}

	if (calculateAllOutputs ||
		(arguments->common.outputSelect == kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt500Pa))
	{
		calibratedValue = 	sign((Aout / Vdd) - kSensorCalibrationConstantSDP8x6Sqrt500Pa1) *
					pow((Aout / (Vdd * kSensorCalibrationConstantSDP8x6Sqrt500Pa2)) -
					kSensorCalibrationConstantSDP8x6Sqrt500Pa3, 2) *
					kSensorCalibrationConstantSDP8x6Sqrt500Pa4;
		outputDistributions[kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt500Pa] = calibratedValue;
	}

	if (calculateAllOutputs ||
		(arguments->common.outputSelect == kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt125Pa))
	{
		calibratedValue = 	sign((Aout / Vdd) - kSensorCalibrationConstantSDP8x6Sqrt125Pa1) *
					pow((Aout / (Vdd * kSensorCalibrationConstantSDP8x6Sqrt125Pa2)) -
					kSensorCalibrationConstantSDP8x6Sqrt125Pa3, 2) *
					kSensorCalibrationConstantSDP8x6Sqrt125Pa4;
		outputDistributions[kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt125Pa] = calibratedValue;
	}

	return	calibratedValue;
}


int
main(int argc, char *  argv[])
{
	CommandLineArguments	arguments = {0};

	double			calibratedSensorOutput;
	double *		monteCarloOutputSamples = NULL;
	clock_t			start;
	clock_t			end;
	double			cpuTimeUsedSeconds;
	double			inputDistributions[kInputDistributionIndexMax];
	double			outputDistributions[kOutputDistributionIndexCalibratedSensorOutputMax];
	const char *		outputVariableNames[kOutputDistributionIndexCalibratedSensorOutputMax] =
				{
					"Calibrated Sensor Output SDP8x6 Linear 500Pa",
					"Calibrated Sensor Output SDP8x6 Linear 125Pa",
					"Calibrated Sensor Output SDP8x6 Square 500Pa",
					"Calibrated Sensor Output SDP8x6 Square 125Pa",
				};
	MeanAndVariance		meanAndVariance;

	/*
	 *	Get command line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments))
	{
		return kCommonConstantReturnTypeError;
	}

	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples = (double *) checkedMalloc(
							arguments.common.numberOfMonteCarloIterations * sizeof(double),
							__FILE__,
							__LINE__);
	}

	/*
	 *	Start timing.
	 */
	if (arguments.common.isTimingEnabled || arguments.common.isBenchmarkingMode)
	{
		start = clock();
	}

	for (size_t i = 0; i < arguments.common.numberOfMonteCarloIterations; i++)
	{
		/*
		 *	Set input distribution values, inside the main computation
		 *	loop, so that it can also generate samples in the native
		 *	Monte Carlo Execution Mode.
		 */
		setInputDistributionsViaUxHwCall(inputDistributions);

		calibratedSensorOutput = calculateSensorOutput(&arguments, inputDistributions, outputDistributions);

		/*
		 *	For this application, `calibratedSensorOutput` is the item we track.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[i] = calibratedSensorOutput;
		}
	}

	/*
	 *	If not doing Laplace version, then approximate the cost of the third phase of
	 *	Monte Carlo (post-processing), by calculating the mean and variance.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		meanAndVariance = calculateMeanAndVarianceOfDoubleSamples(
					monteCarloOutputSamples,
					arguments.common.numberOfMonteCarloIterations);
		calibratedSensorOutput = meanAndVariance.mean;
	}

	/*
	 *	Stop timing.
	 */
	if (arguments.common.isTimingEnabled || arguments.common.isBenchmarkingMode)
	{
		end = clock();
		cpuTimeUsedSeconds = ((double)(end - start)) / CLOCKS_PER_SEC;
	}

	if (arguments.common.isBenchmarkingMode)
	{
		/*
		 *	In benchmarking mode, we print:
		 *		(1) single result (for calculating Wasserstein distance to reference)
		 *		(2) time in microseconds (benchmarking setup expects cpu time in microseconds)
		 */
		printf("%lf %" PRIu64 "\n", calibratedSensorOutput, (uint64_t)(cpuTimeUsedSeconds*1000000));
	}
	else
	{
		/*
		 *	Print the results (either in JSON or standard output format).
		 */
		if (!arguments.common.isOutputJSONMode)
		{
			if (arguments.common.outputSelect == kOutputDistributionIndexCalibratedSensorOutputMax)
			{
				for (size_t i = 0; i < kOutputDistributionIndexCalibratedSensorOutputMax; i++)
				{
					printCalibratedValueAndProbabilities(outputDistributions[i], outputVariableNames[i]);
				}
			}
			else
			{
				printCalibratedValueAndProbabilities(
					calibratedSensorOutput,
					outputVariableNames[arguments.common.outputSelect]);
			}
		}
		else
		{
			printJSONFormattedOutput(
				&arguments,
				monteCarloOutputSamples,
				outputDistributions,
				outputVariableNames);
		}

		/*
		 *	Print timing result.
		 */
		if (arguments.common.isTimingEnabled)
		{
			printf("\nCPU time used: %lf seconds\n", cpuTimeUsedSeconds);
		}

		/*
		 *	Write output data.
		 */
		if (arguments.common.isWriteToFileEnabled)
		{
			if (writeOutputDoubleDistributionsToCSV(
				arguments.common.outputFilePath,
				outputDistributions,
				outputVariableNames,
				kOutputDistributionIndexCalibratedSensorOutputMax))
			{
				return kCommonConstantReturnTypeError;
			}
		}
	}

	/*
	 *	Save Monte carlo outputs in an output file.
	 *	Free dynamically-allocated memory.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(monteCarloOutputSamples, (uint64_t)(cpuTimeUsedSeconds*1000000), arguments.common.numberOfMonteCarloIterations);
		
		free(monteCarloOutputSamples);
	}

	return 0;
}
