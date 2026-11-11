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

/**
 *	@brief	Monte Carlo mode sample loop. Runs `numberOfMonteCarloIterations`
 *		independent evaluations of `SensirionSDP8xx_calculateOutput()`,
 *		each with a freshly sampled input pair drawn via
 *		`SensirionSDP8xxSetInputVariablesViaUxHwCall()` into the caller-supplied
 *		`inputVariables` scratch buffer, and fills `monteCarloOutputSamples`
 *		with one sample per iteration.
 *
 *	@param	outputSelect			: Index of the output variable to compute
 *						  (or `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax`
 *						  to compute every output).
 *	@param	inputVariables			: Scratch buffer (sized and owned by the caller)
 *						  that this function re-populates via
 *						  `SensirionSDP8xxSetInputVariablesViaUxHwCall()` on every iteration.
 *	@param	outputVariables			: An array of output variables, overwritten on
 *						  every iteration; holds the last iteration's
 *						  result(s) on return.
 *	@param	monteCarloOutputSamples		: Array of `numberOfMonteCarloIterations` doubles,
 *						  filled with one sample per iteration.
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *	@return	double				: Returns the value of the selected output for the
 *						  last Monte Carlo iteration.
 */
double
sensirionSDP8xxMonteCarlo(
	uint8_t     outputSelect,
	double *    inputVariables,
	double *    outputVariables,
	double *    monteCarloOutputSamples,
	size_t      numberOfMonteCarloIterations);
