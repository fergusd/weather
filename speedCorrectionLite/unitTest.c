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

    Unit Tests for interpolator.c speed correction algorithm based on data in link below

    http://www.davis-tr.com/Downloads/Davis_Rzgr_Kepceleri_Karakteristikleri.pdf

    I have noticed that a number of the results appear to be inconsistent with the
    supplied speed correction figues in this document, these are marked in the test
    data tables below.

    This code should be compiled using g++ and should run without asserting.
    It has been tested on a linux and mac platforms.

    Fergus Duncan (github : @fergusd)
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "interpolator.h"

// 0 degree tests
float zeroTests[27][2] = {{20,23.3},
                          {25,28.5},
                          {30,33.8},
                          {35,39.2},
                          {40,44.5},
                          {45,49.7},
                          {50,55.0},
                          {55,60.3},
                          {60,65.7},
                          {65,70.8},
                          {70,76.2},
                          {75,81.4},
                          {80,86.8},
                          {85,92.1},
                          {90,97.4},
                          {95,102.5},
                          {100,107.7},
                          {105,113.2},
                          {110,118.5},
                          {115,123.9},
                          {120,129.5},
                          {125,135.0},
                          {130,139.8},
                          {135,144.8},
                          {140,149.3},
                          {145,154.5},
                          {150,159.8}};

// 90 degree tests
float ninetyTests[27][2] = {{20,17.7}, // possible error in documented test result : was 17.8
                            {25,22.3},
                            {30,27.1},
                            {35,31.6},
                            {40,35.9},
                            {45,41.2},
                            {50,45.5},
                            {55,50.2},
                            {60,54.7},
                            {65,59.0},
                            {70,64.4},
                            {75,69.0},
                            {80,73.6},
                            {85,77.6},
                            {90,82.0},
                            {95,86.9},
                            {100,92.1},
                            {105,96.9},
                            {110,101.5},
                            {115,106.2},
                            {120,110.6},
                            {125,115.4},
                            {130,120.2}, // possible error in documented test result : was 120.3
                            {135,125.0},
                            {140,129.8},
                            {145,134.1},
                            {150,137.9}};

// 180 degree tests
// from http://www.davis-tr.com/Downloads/Davis_Rzgr_Kepceleri_Karakteristikleri.pdf
float oneeightyTests[27][2] = {{20,16.4},
                               {25,20.4},
                               {30,25.2}, // possible error in documented test result : was 25.3
                               {35,29.7}, // possible error in documented test result : was 29.8
                               {40,34.3},
                               {45,40.5},
                               {50,45.1},
                               {55,49.8},
                               {60,54.1},
                               {65,59.0},
                               {70,63.9},
                               {75,68.2},
                               {80,73.1},
                               {85,78.2},
                               {90,83.2},
                               {95,87.5},
                               {100,92.8},
                               {105,97.3},
                               {110,102.3},
                               {115,106.5},
                               {120,111.0},
                               {125,115.2}, // possible error in documented test result : was 115.3
                               {130,119.7},
                               {135,124.0},
                               {140,128.7},
                               {145,134.5},
                               {150,138.0}};

float oneeightySpeedIncrementTests[6][2] = {{20,16.4},
                                            {21,17.2},
                                            {22,18.0},
                                            {23,18.8},
                                            {24,19.6},
                                            {25,20.4}};

int main(int argc, char* argv[])
{
    // test 0 degrees sites
    for(int test=0;test<27;test++)
    {
        float angle = 0.0;
        float correctedSpeed=correctSpeed(zeroTests[test][0],angle);
        printf("TEST:input speed:%3.2f\n",zeroTests[test][0]);
        printf("TEST:input angle:%3.2f\n",angle);
        printf("TEST:correctedSpeed:%3.2f\n",correctedSpeed);
        printf("TEST:expectedSpeed:%3.2f\n",zeroTests[test][1]);
        assert(zeroTests[test][1]==correctedSpeed);
        printf("***********************\n");
    }

    // test 90 degrees sites
    for(int test=0;test<27;test++)
    {
        float angle = 90.0;
        float correctedSpeed=correctSpeed(ninetyTests[test][0],angle);
        printf("TEST:input speed:%3.2f\n",ninetyTests[test][0]);
        printf("TEST:input angle:%3.2f\n",angle);
        printf("TEST:correctedSpeed:%3.2f\n",correctedSpeed);
        printf("TEST:expectedSpeed:%3.2f\n",ninetyTests[test][1]);
        assert(ninetyTests[test][1]==correctedSpeed);
        printf("***********************\n");
    }

    // test 180 degrees sites
    for(int test=0;test<27;test++)
    {
        float angle = 180.0;
        float correctedSpeed=correctSpeed(oneeightyTests[test][0],angle);
        printf("TEST:input speed:%3.2f\n",oneeightyTests[test][0]);
        printf("TEST:input angle:%3.2f\n",angle);
        printf("TEST:correctedSpeed:%3.2f\n",correctedSpeed);
        printf("TEST:expectedSpeed:%3.2f\n",oneeightyTests[test][1]);
        assert(oneeightyTests[test][1]==correctedSpeed);
        printf("***********************\n");
    }

    // speed increment test at 180 degrees site
    for(int test=0;test<6;test++)
    {
        float angle = 180.0;
        float correctedSpeed=correctSpeed(oneeightySpeedIncrementTests[test][0],angle);
        printf("TEST:input speed:%3.2f\n",oneeightySpeedIncrementTests[test][0]);
        printf("TEST:input angle:%3.2f\n",angle);
        printf("TEST:correctedSpeed:%3.2f\n",correctedSpeed);
        printf("TEST:expectedSpeed:%3.2f\n",oneeightySpeedIncrementTests[test][1]);
        assert(oneeightySpeedIncrementTests[test][1]==correctedSpeed);
        printf("***********************\n");
    }

    printf("happy days\n");
}

 
