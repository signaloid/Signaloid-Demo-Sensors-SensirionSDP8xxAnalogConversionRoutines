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

#pragma once

#include <stddef.h>
#include <stdint.h>


/*
 *	These constant values are taken from page 4 of
 *	SDP8xx Analog Datasheet, 2024-07-03.
 */
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear500Pa1 (750)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear500Pa2 (150)

#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear125Pa1 (190)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Linear125Pa2 (38)

#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa1   (0.5)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa2   (0.4)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa3   (1.25)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt500Pa4   (525)

#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa1   (0.5)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa2   (0.4)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa3   (1.25)
#define kSensirionSDP8xxSensorCalibrationConstantSDP8x6Sqrt125Pa4   (133)

#define kSensirionSDP8xxDefaultInputDistributionAoutUniformDistLow  (1.3)
#define kSensirionSDP8xxDefaultInputDistributionAoutUniformDistHigh (1.7)
#define kSensirionSDP8xxDefaultInputDistributionVddUniformDistLow   (3.5)
#define kSensirionSDP8xxDefaultInputDistributionVddUniformDistHigh  (3.9)

/*
 *	Input Variables:
 *		kInputVariableIndexAout	: Ratiometric Analog Voltage Value (in Volts)
 *		kInputVariableIndexVdd	: Supply Voltage (in Volts)
 */
typedef enum
{
	kSensirionSDP8xxInputVariableIndexAout  = 0,
	kSensirionSDP8xxInputVariableIndexVdd   = 1,
	kSensirionSDP8xxInputVariableIndexMax,
} SensirionSDP8xxInputVariableIndex;

/*
 *	Output Variables:
 *		kOutputVariableIndexCalibratedSensorOutputSDP8x6Linear500Pa:
 *			Differential Pressure Output (in Pascal) for the Linear Configuration for 500Pa
 *		kOutputVariableIndexCalibratedSensorOutputSDP8x6Linear125Pa:
 *			Differential Pressure Output (in Pascal) for the Linear Configuration for 125Pa
 *		kOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt500Pa:
 *			Differential Pressure Output (in Pascal) for the Square Root Configuration for 500Pa
 *		kOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt125Pa:
 *			Differential Pressure Output (in Pascal) for the Square Root Configuration for 125Pa
 */
typedef enum
{
	kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Linear500Pa  = 0,
	kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Linear125Pa  = 1,
	kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt500Pa    = 2,
	kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputSDP8x6Sqrt125Pa    = 3,
	kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax,
} SensirionSDP8xxOutputVariableIndex;

/**
 *	@brief  Sensor calibration routines for different modes taken from
 *		SDP8xx Analog Datasheet, 2024-07-03.
 *
 *	@param  outputSelect		: Index of the output variable to compute
 *					  (or `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax`
 *					  to compute every output).
 *	@param  inputVariables		: The array of input variables used in the calculation.
 *	@param  outputVariables		: An array of output variables.
 *					  Writes the result to `outputVariables[outputSelectValue]`.
 *	@return	double			: Returns the distributional value calculated.
 */
double
SensirionSDP8xx_calculateOutput(
	uint8_t         outputSelect,
	const double *  inputVariables,
	double *        outputVariables);

/**
 *	@brief	Set the sensor's input variables (`Aout` and `Vdd`) via calls to
 *		`UxHwDoubleUniformDist()`. Shared by both the UxHw and Monte Carlo
 *		kernels: in UxHw mode a single call yields a fully distributional
 *		input pair; in Monte Carlo mode each call (one per iteration) yields
 *		a fresh concrete sample pair.
 *
 *	@param	inputVariables	: An array of size `kSensirionSDP8xxInputVariableIndexMax`,
 *				  where the function writes the distributional data.
 */
void
SensirionSDP8xxSetInputVariablesViaUxHwCall(double * inputVariables);

/**
 *	@brief	UxHw-mode calculation kernel. Samples the input distributions once
 *		via `SensirionSDP8xxSetInputVariablesViaUxHwCall()`, then dispatches the resulting
 *		already-populated `inputVariables` array to
 *		`sensirionSDP8xxUxHw()` for a single distributional
 *		evaluation. Writes the selected (or every, if all outputs are
 *		selected) output into `outputVariables` and the single
 *		distributional result into `monteCarloOutputSamples[0]`.
 *
 *	@param	outputSelect		: Index of the output variable to compute
 *					  (or `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax`
 *					  to compute every output). A `size_t`, so that
 *					  the caller passes
 *					  `CommonCommandLineArguments.outputSelect`
 *					  through unnarrowed.
 *	@param	outputVariables		: Array of size `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax` to fill.
 *	@param	monteCarloOutputSamples	: Single-element array for the distributional result.
 *	@return	double			: Returns the value of the selected output (or the last-computed output when all outputs are selected).
 */
double
SensirionSDP8xxCalculateOutputUxHw(
	size_t      outputSelect,
	double *    outputVariables,
	double *    monteCarloOutputSamples);

/**
 *	@brief	Monte Carlo calculation kernel. Sizes the `inputVariables` scratch
 *		array (dimension `kSensirionSDP8xxInputVariableIndexMax`) and hands
 *		it, along with `numberOfMonteCarloIterations`, to
 *		`sensirionSDP8xxMonteCarlo()`, which runs the
 *		independent evaluations (each with a freshly sampled input pair,
 *		drawn via `SensirionSDP8xxSetInputVariablesViaUxHwCall()` inside its own loop)
 *		into `monteCarloOutputSamples`. Writes the selected output into
 *		`outputVariables`.
 *
 *	@param	outputSelect		: Index of the output variable to compute
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *					  (or `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax`
 *					  to compute every output). A `size_t`, so that
 *					  the caller passes
 *					  `CommonCommandLineArguments.outputSelect`
 *					  through unnarrowed.
 *	@param	outputVariables		: Array of size `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax` to fill.
 *	@param	monteCarloOutputSamples	: Array of `numberOfMonteCarloIterations` doubles, filled with samples.
 *	@return	double			: Returns the value of the selected output for the last Monte Carlo iteration.
 */
double
SensirionSDP8xxCalculateOutputMonteCarlo(
	size_t      outputSelect,
	size_t      numberOfMonteCarloIterations,
	double *    outputVariables,
	double *    monteCarloOutputSamples);
