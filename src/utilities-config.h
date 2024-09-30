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

/*
 *	These constant values are taken from page 4 of
 *	SDP8xx Analog Datasheet, 2024-07-03.
 */
#define kSensorCalibrationConstantSDP8x6Linear500Pa1			(750)
#define kSensorCalibrationConstantSDP8x6Linear500Pa2			(150)

#define kSensorCalibrationConstantSDP8x6Linear125Pa1			(190)
#define kSensorCalibrationConstantSDP8x6Linear125Pa2			(38)

#define kSensorCalibrationConstantSDP8x6Sqrt500Pa1			(0.5)
#define kSensorCalibrationConstantSDP8x6Sqrt500Pa2			(0.4)
#define kSensorCalibrationConstantSDP8x6Sqrt500Pa3			(1.25)
#define kSensorCalibrationConstantSDP8x6Sqrt500Pa4			(525)

#define kSensorCalibrationConstantSDP8x6Sqrt125Pa1			(0.5)
#define kSensorCalibrationConstantSDP8x6Sqrt125Pa2			(0.4)
#define kSensorCalibrationConstantSDP8x6Sqrt125Pa3			(1.25)
#define kSensorCalibrationConstantSDP8x6Sqrt125Pa4			(133)

/*
 *	Input Distributions:
 *		kInputDistributionIndexAout	: Ratiometric Analog Voltage Value (in Volts)
 *		kInputDistributionIndexVdd	: Supply Voltage (in Volts)
 */
typedef enum
{
	kInputDistributionIndexAout	= 0,
	kInputDistributionIndexVdd	= 1,
	kInputDistributionIndexMax,
} InputDistributionIndex;

/*
 *	Output Distributions:
 *		kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear500Pa:
 *			Differential Pressure Output (in Pascal) for the Linear Configuration for 500Pa
 *		kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear125Pa:
 *			Differential Pressure Output (in Pascal) for the Linear Configuration for 125Pa
 *		kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt500Pa:
 *			Differential Pressure Output (in Pascal) for the Square Root Configuration for 500Pa
 *		kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt125Pa:
 *			Differential Pressure Output (in Pascal) for the Square Root Configuration for 125Pa
 */
typedef enum
{
	kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear500Pa	= 0,
	kOutputDistributionIndexCalibratedSensorOutputSDP8x6Linear125Pa	= 1,
	kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt500Pa	= 2,
	kOutputDistributionIndexCalibratedSensorOutputSDP8x6Sqrt125Pa	= 3,
	kOutputDistributionIndexCalibratedSensorOutputMax,
} OutputDistributionIndex;

#define kDefaultInputDistributionAoutUniformDistLow			(1.3)
#define kDefaultInputDistributionAoutUniformDistHigh			(1.7)
#define kDefaultInputDistributionVddUniformDistLow			(3.5)
#define kDefaultInputDistributionVddUniformDistHigh			(3.9)
