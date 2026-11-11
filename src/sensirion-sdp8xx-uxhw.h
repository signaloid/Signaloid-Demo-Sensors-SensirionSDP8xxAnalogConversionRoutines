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

#include <stdint.h>

/**
 *	@brief	UxHw-mode single distributional evaluation. Evaluates
 *		`SensirionSDP8xx_calculateOutput()` once on the already-sampled,
 *		fully distributional `inputVariables` pair supplied by the caller.
 *		No sampling and no sample loop: the caller (`kernel.c`) is
 *		responsible for populating `inputVariables` via
 *		`SensirionSDP8xxSetInputVariablesViaUxHwCall()` beforehand.
 *
 *	@param	outputSelect	: Index of the output variable to compute
 *				  (or `kSensirionSDP8xxOutputVariableIndexCalibratedSensorOutputMax`
 *				  to compute every output).
 *	@param	inputVariables	: The already-populated array of distributional input
 *				  variables (`Aout` and `Vdd`) used in the calculation.
 *	@param	outputVariables	: An array of output variables. Writes the result(s)
 *				  computed by `SensirionSDP8xx_calculateOutput()`.
 *	@return	double		: Returns the distributional value calculated.
 */
double
sensirionSDP8xxUxHw(
	uint8_t         outputSelect,
	const double *  inputVariables,
	double *        outputVariables);
