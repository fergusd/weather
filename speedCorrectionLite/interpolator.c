/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    C/C++ code to implement wind speed correction for Davis Vantage Pro 2 Anemometers
    Implements linear speed and angle interpolation between defined speed and angle
    sites as defined in the document linked below :-

    http://www.davis-tr.com/Downloads/Davis_Rzgr_Kepceleri_Karakteristikleri.pdf

    I have noticed that a number of the results appear to be inconsistent with the
    supplied speed correction figues in this document, these are marked in the test
    data tables below.

    This code should be compiled using g++ and should run without asserting.
    It has been tested on a linux and mac platforms.

    Fergus Duncan (github : @fergusd)
*/

#include "interpolator.h"

#define correctionTableSize  29
//define speedIndex 0
#define zeroDegreeIndex 0
#define ninetyDegreeIndex 1
#define oneeightyDegreeIndex 2

// from http://www.davis-tr.com/Downloads/Davis_Rzgr_Kepceleri_Karakteristikleri.pdf

/*
float correctionTable[correctionTableSize][4] = {{0,0.0,0.0,0.0}, // simplifies algorithm - causes zero correction at zero speed
                                                 {20,3.3,-2.3,-3.6},
                                                 {25,3.5,-2.7,-4.6},
                                                 {30,3.8,-2.9,-4.8},
                                                 {35,4.2,-3.4,-5.3},
                                                 {40,4.5,-4.1,-5.7},
                                                 {45,4.7,-3.8,-4.5},
                                                 {50,5.0,-4.5,-4.9},
                                                 {55,5.3,-4.8,-5.2},
                                                 {60,5.7,-5.3,-5.9},
                                                 {65,5.8,-6.0,-6.0},
                                                 {70,6.2,-5.6,-6.1},
                                                 {75,6.4,-6.0,-6.8},
                                                 {80,6.8,-6.4,-6.9},
                                                 {85,7.1,-7.4,-6.8},
                                                 {90,7.4,-8.0,-6.8},
                                                 {95,7.5,-8.1,-7.5},
                                                 {100,7.7,-7.9,-7.2},
                                                 {105,8.2,-8.1,-7.7},
                                                 {110,8.5,-8.5,-7.7},
                                                 {115,8.9,-8.8,-8.5},
                                                 {120,9.5,-9.4,-9.0},
                                                 {125,10.0,-9.6,-9.8},
                                                 {130,9.8,-9.8,-10.3},
                                                 {135,9.8,-10.0,-11.0},
                                                 {140,9.3,-10.2,-11.3},
                                                 {145,9.5,-10.9,-10.5},
                                                 {150,9.8,-12.1,-12.0},
                                                 {999,9.8,-12.1,-12.0}}; // simplifies algorithm - maintains same correction above 150
*/

uint8_t speedTable[correctionTableSize] = {0, // simplifies algorithm - causes zero correction at zero speed
                                          20,
                                          25,
                                          30,
                                          35,
                                          40,
                                          45,
                                          50,
                                          55,
                                          60,
                                          65,
                                          70,
                                          75,
                                          80,
                                          85,
                                          90,
                                          95,
                                          100,
                                          105,
                                          110,
                                          115,
                                          120,
                                          125,
                                          130,
                                          135,
                                          140,
                                          145,
                                          150,
                                          255}; // max supported speed

int8_t correctionTable[correctionTableSize][3] = {{0,0,0}, // zero correction at zero speed
                                       /*  20 */  {33,-23,-36},
                                       /*  25 */  {35,-27,-46},
                                       /*  30 */  {38,-29,-48},
                                       /*  35 */  {42,-34,-53},
                                       /*  40 */  {45,-41,-57},
                                       /*  45 */  {47,-38,-45},
                                       /*  50 */  {50,-45,-49},
                                       /*  55 */  {53,-48,-52},
                                       /*  60 */  {57,-53,-59},
                                       /*  65 */  {58,-60,-60},
                                       /*  70 */  {62,-56,-61},
                                       /*  75 */  {64,-60,-68},
                                       /*  80 */  {68,-64,-69},
                                       /*  85 */  {71,-74,-68},
                                       /*  90 */  {74,-80,-68},
                                       /*  95 */  {75,-81,-75},
                                       /* 100 */  {77,-79,-72},
                                       /* 105 */  {82,-81,-77},
                                       /* 110 */  {85,-85,-77},
                                       /* 115 */  {89,-88,-85},
                                       /* 120 */  {95,-94,-90},
                                       /* 125 */  {100,-96,-98},
                                       /* 130 */  {98,-98,-103},
                                       /* 135 */  {98,-100,-110},
                                       /* 140 */  {93,-102,-113},
                                       /* 145 */  {95,-109,-105},
                                       /* 150 */  {98,-121,-120},
                                       /* 255 */  {98,-121,-120}}; // max correction up to max speed

float correctSpeed(uint8_t rawSpeed, uint8_t angle)
{
    float calculatedSpeed = 0.0;

    // find the closest speed site in the correction table
    uint8_t speedIndexHigh = 0;
    uint8_t speedIndexLow = 0;
    uint8_t correctionAngle = 0;

    // calculate the angle to be used in the calculation
    if(angle>180)
    {
        correctionAngle = 180-(angle-180);
    }
    else
    {
        correctionAngle = angle;
    }

    // calculate the speed sites to be used in the calculation
    // speedIndexHigh will be the speed site above the input speed
    // speedIndexLow will be the speed site below the input speed
    for(uint8_t index=0;index<correctionTableSize;index++)
    {
        if((rawSpeed-speedTable[index]<=0))
        {
            // overshot
            speedIndexHigh = index;
            break;
        }

        speedIndexHigh = index;
    }

    speedIndexLow = speedIndexHigh-1;

    // calculate the scaling factor based on the input speeds position relative to the
    // high and low speed sites
    float speedDelta = (speedTable[speedIndexHigh]-speedTable[speedIndexLow]);
    float speedOffset = (rawSpeed-speedTable[speedIndexLow]);
    float speedFactor = (speedOffset/speedDelta);

    // depending on the correctionAngle we select either the 0->90 or 90->180 degree angle site
    if(correctionAngle<=90)
    {
        // calculate speed minimum and maximum sites
        // then apply linear interpolation between angle sites for input angle
        float angleFactor = (correctionAngle/90.0);
        float speedCorrectionLow = (correctionTable[speedIndexLow][ninetyDegreeIndex]-correctionTable[speedIndexLow][zeroDegreeIndex]);
        speedCorrectionLow *= angleFactor;
        speedCorrectionLow += correctionTable[speedIndexLow][zeroDegreeIndex];
        float speedCorrectionHigh = (correctionTable[speedIndexHigh][ninetyDegreeIndex]-correctionTable[speedIndexHigh][zeroDegreeIndex]);
        speedCorrectionHigh *= angleFactor;
        speedCorrectionHigh += correctionTable[speedIndexHigh][zeroDegreeIndex];

        // apply linear interpolation to output of previous calc
        calculatedSpeed = ((speedCorrectionHigh-speedCorrectionLow)*speedFactor)+speedCorrectionLow;
        calculatedSpeed /=10;
        calculatedSpeed += rawSpeed;
    }
    else
    {
        // calculate speed minimum and maximum sites
        // then apply linear interpolation between angle sites for input angle
        float angleFactor = ((correctionAngle-90.0)/90.0);
        float speedCorrectionLow = (correctionTable[speedIndexLow][oneeightyDegreeIndex]-correctionTable[speedIndexLow][ninetyDegreeIndex]);
        speedCorrectionLow *= angleFactor;
        speedCorrectionLow += correctionTable[speedIndexLow][ninetyDegreeIndex];
        float speedCorrectionHigh = (correctionTable[speedIndexHigh][oneeightyDegreeIndex]-correctionTable[speedIndexHigh][ninetyDegreeIndex]);
        speedCorrectionHigh *= angleFactor;
        speedCorrectionHigh += correctionTable[speedIndexHigh][ninetyDegreeIndex];

        // apply linear interpolation to output of previous calc
        calculatedSpeed = ((speedCorrectionHigh-speedCorrectionLow)*speedFactor)+speedCorrectionLow;
        calculatedSpeed /=10;
        calculatedSpeed += rawSpeed;
    }

    return calculatedSpeed;
}

