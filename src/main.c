/*
 *	Copyright (c) 2024-2026, Signaloid.
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
#include "kernel.h"

#ifdef NO_OS_AVAILABLE

void
returnZeroNoOS(void);
#endif

int
main(int argc, char *  argv[])
{
	CommandLineArguments arguments = { 0 };

	double *                    monteCarloOutputSamples = NULL;
	clock_t                     start               = 0;
	clock_t                     end                 = 0;
	double                      cpuTimeUsedSeconds  = 0.0;
	double                      outputVariables[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax] = { 0 };
	const char *                outputVariableNames[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax] = {
		"Calibrated Sensor Output SDP8x6 Linear 500Pa",
		"Calibrated Sensor Output SDP8x6 Linear 125Pa",
		"Calibrated Sensor Output SDP8x6 Square 500Pa",
		"Calibrated Sensor Output SDP8x6 Square 125Pa"
	};
	const char *                outputVariableDescriptions[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax] = {
		"Calibrated Sensor Output SDP8x6 Linear 500Pa",
		"Calibrated Sensor Output SDP8x6 Linear 125Pa",
		"Calibrated Sensor Output SDP8x6 Square 500Pa",
		"Calibrated Sensor Output SDP8x6 Square 125Pa"
	};
	kOutputVariableTypeIndex    outputVariableTypes[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax] = {
		kOutputVariableTypeDistribution,
		kOutputVariableTypeDistribution,
		kOutputVariableTypeDistribution,
		kOutputVariableTypeDistribution
	};
	const char *                applicationDescription = "Sensirion SDP8xx Analog Conversion Routines";

	/*
	 *	Get command-line arguments. In the no-OS build there is no command
	 *	line, so this instead applies the hard-coded configuration in
	 *	`setNoOSCommandLineArguments()`.
	 */
	if (getCommandLineArguments(argc, argv, &arguments))
	{
		return kCommonConstantReturnTypeError;
	}

	/*
	 *	MonteCarlo output samples are used even in the UxHw use case to
	 *	store the result of the single distributional evaluation.
	 */
	monteCarloOutputSamples = (double *) checkedMalloc(
		(arguments.common.numberOfMonteCarloIterations > 0 ? arguments.common.numberOfMonteCarloIterations : 1) * sizeof(double),
		__FILE__,
		__LINE__
	);

	const bool isScalarOutputSelected = isSelectedOutputScalar(
		&arguments.common,
		outputVariableTypes,
		kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax
	);

	/*
	 *	Start timing.
	 */
	if (arguments.common.isTimingEnabled)
	{
		start = clock();
	}

	/*
	 *	Dispatch to the mode-specific kernel. The two functions called here
	 *	size the `inputVariables` scratch array and forward: the Monte Carlo
	 *	loop (when applicable) lives one level down, inside
	 *	`sensirionSDP8xxMonteCarlo()`, and the single distributional
	 *	evaluation of UxHw mode inside `sensirionSDP8xxUxHw()`.
	 *	Both take the fields they need as scalars, so this is the only place
	 *	that unpacks `CommandLineArguments` for the kernel.
	 *
	 *	Every output of this demo is a `kOutputVariableTypeDistribution`
	 *	(see `outputVariableTypes` above and `signaloid.yaml`), so there is
	 *	no scalar output to special-case here, unlike the GBM demo this
	 *	structure is modelled on.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		SensirionSDP8xxCalculateOutputMonteCarlo(
			arguments.common.outputSelect,
			arguments.common.numberOfMonteCarloIterations,
			outputVariables,
			monteCarloOutputSamples
		);

		/*
		 *	If not doing UxHw version, then approximate the cost of the third phase of
		 *	Monte Carlo (post-processing), by calculating the mean and variance.
		 *	The result is not used. Only the cost matters, for the timing result.
		 */
		if (!isScalarOutputSelected)
		{
			calculateMeanAndVarianceOfDoubleSamples(
				monteCarloOutputSamples,
				arguments.common.numberOfMonteCarloIterations
			);
		}
	}
	else
	{
		SensirionSDP8xxCalculateOutputUxHw(
			arguments.common.outputSelect,
			outputVariables,
			monteCarloOutputSamples
		);
	}

	/*
	 *	Stop timing.
	 */
	if (arguments.common.isTimingEnabled)
	{
		end                 = clock();
		cpuTimeUsedSeconds  = ((double) (end - start)) / CLOCKS_PER_SEC;
	}

	/*
	 *	Print the results (either in JSON or standard output format).
	 */
	if (arguments.common.isOutputJSONMode)
	{
		printJSONFormattedOutput(
			&arguments.common,
			monteCarloOutputSamples,
			outputVariables,
			outputVariableNames,
			kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax,
			applicationDescription,
			outputVariableTypes
		);
	}
	else
	{
		printHumanConsumableOutput(
			&arguments.common,
			kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax,
			outputVariables,
			outputVariableNames,
			outputVariableDescriptions,
			monteCarloOutputSamples,
			outputVariableTypes
		);
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
				outputVariables,
				outputVariableNames,
				kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax
		))
		{
			return kCommonConstantReturnTypeError;
		}
	}

	/*
	 *	Save Monte carlo outputs in an output file.
	 *	Free dynamically-allocated memory.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples, (uint64_t) (cpuTimeUsedSeconds * 1000000),
			isScalarOutputSelected ? 1 : arguments.common.numberOfMonteCarloIterations
		);
	}
	free(monteCarloOutputSamples);

#ifdef NO_OS_AVAILABLE
	returnZeroNoOS();
#else

	return 0;

#endif
}
