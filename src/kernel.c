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
#include <stdbool.h>
#include <uxhw.h>
#include "kernel.h"
#include "sensirion-sdp8xx-uxhw.h"
#include "sensirion-sdp8xx-monte-carlo.h"


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

double
SensirionSDP8xx_calculateOutput(uint8_t outputSelect, const double *  inputVariables, double *  outputVariables)
{
	double  Vdd             = inputVariables[kSensirionSDP8xxInputVariableIndexVdd];
	double  Aout            = inputVariables[kSensirionSDP8xxInputVariableIndexAout];
	double  calibratedValue = 0.0;

	bool calculateAllOutputs = (outputSelect == kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax);

	if (calculateAllOutputs ||
	    (outputSelect == kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Linear500Pa))
	{
		calibratedValue = kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear500Pa1 * Aout / Vdd -
		                  kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear500Pa2;
		outputVariables[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Linear500Pa] = calibratedValue;
	}

	if (calculateAllOutputs ||
	    (outputSelect == kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Linear125Pa))
	{
		calibratedValue = kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear125Pa1 * Aout / Vdd -
		                  kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear125Pa2;
		outputVariables[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Linear125Pa] = calibratedValue;
	}

	if (calculateAllOutputs ||
	    (outputSelect == kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt500Pa))
	{
		calibratedValue = sign((Aout / Vdd) - kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa1) *
		                  pow(
			(Aout / (Vdd * kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa2)) -
			kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa3, 2
		                  ) *
		                  kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa4;
		outputVariables[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt500Pa] = calibratedValue;
	}

	if (calculateAllOutputs ||
	    (outputSelect == kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt125Pa))
	{
		calibratedValue = sign((Aout / Vdd) - kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa1) *
		                  pow(
			(Aout / (Vdd * kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa2)) -
			kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa3, 2
		                  ) *
		                  kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa4;
		outputVariables[kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt125Pa] = calibratedValue;
	}

	return calibratedValue;
}

/**
 *	@brief  Sets the Input Variables via call to UxHw Parametric function.
 *
 *	@param  inputVariables	: An array of double values, where the function writes the distributional data.
 */
void
SensirionSDP8xxSetInputVariablesViaUxHwCall(double * inputVariables)
{
	inputVariables[kSensirionSDP8xxInputVariableIndexAout] = UxHwDoubleUniformDist(
		kSensirionSDP8xxDefaultInputDistributionAoutUniformDistLow,
		kSensirionSDP8xxDefaultInputDistributionAoutUniformDistHigh
	);

	inputVariables[kSensirionSDP8xxInputVariableIndexVdd] = UxHwDoubleUniformDist(
		kSensirionSDP8xxDefaultInputDistributionVddUniformDistLow,
		kSensirionSDP8xxDefaultInputDistributionVddUniformDistHigh
	);

	return;
}

double
SensirionSDP8xxCalculateOutputUxHw(
	size_t      outputSelect,
	double *    outputVariables,
	double *    monteCarloOutputSamples)
{
	double  inputVariables[kSensirionSDP8xxInputVariableIndexMax];
	double  result;

	/*
	 *	Sample the input distributions once here, in `kernel.c`, so the
	 *	computational file receives an already-populated `inputVariables`
	 *	array and never needs to know about `UxHwDoubleUniformDist()` or
	 *	the distributional input constants.
	 */
	SensirionSDP8xxSetInputVariablesViaUxHwCall(inputVariables);

	result = sensirionSDP8xxUxHw((uint8_t) outputSelect, inputVariables, outputVariables);

	/*
	 *	UxHw mode performs a single distributional evaluation; that one
	 *	result is also the sole entry of the sample buffer.
	 */
	monteCarloOutputSamples[0] = result;

	return result;
}

double
SensirionSDP8xxCalculateOutputMonteCarlo(
	size_t      outputSelect,
	size_t      numberOfMonteCarloIterations,
	double *    outputVariables,
	double *    monteCarloOutputSamples)
{
	double inputVariables[kSensirionSDP8xxInputVariableIndexMax];

	/*
	 *	Size the `inputVariables` scratch array here, in `kernel.c`, and
	 *	hand the computational file a pointer to it. The Monte Carlo loop
	 *	(inside `sensirionSDP8xxMonteCarlo()`) re-populates
	 *	it via `SensirionSDP8xxSetInputVariablesViaUxHwCall()` once per iteration, so the
	 *	per-iteration re-sampling semantics are preserved exactly while the
	 *	computational file no longer references
	 *	`kSensirionSDP8xxInputVariableIndexMax` itself.
	 */
	return sensirionSDP8xxMonteCarlo(
		(uint8_t) outputSelect,
		inputVariables,
		outputVariables,
		monteCarloOutputSamples,
		numberOfMonteCarloIterations
	);
}
