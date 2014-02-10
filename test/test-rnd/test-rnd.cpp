/* -*- mode: c++; coding: utf-8-unix -*-
 *
 * Copyright 2013 MTA SZTAKI
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

/**
   \file
   \brief Test matrix operations.
*/

#include <test.h>
#include <time.h>
#include <rnc>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <rnc-lib/mt.h>

using namespace std;
using namespace rnc::test;
using namespace rnc::fq;

#define N 10000

int main(int argc, char **argv)
{
        char const * const OUTFNAME = "random_numbers.txt";

        int n = N;
        if (argc>1)
                n = atoi(argv[1]);
        if (!n) n = N;

        FILE *f = fopen(OUTFNAME, "w");
        if (!f) throw string("Can't open file 'random_numbers.txt' for writing.");

        rnc::random::mt_state state;
        rnc::random::init(&state, time(0));
        for (int i=0; i<n; ++i)
                fprintf(f, "%f\n", rnc::random::generateP(&state));
        fclose(f);

        int nums[50];
        for (int i=0; i<50; ++i) nums[i]=i;
        rnc::random::shuffle(nums, 50, &state);
        for (int i=0; i<50; ++i)
                printf("%3d", nums[i]);
        printf("\n");

        printf("Written %d random numbers to '%s'\n", n, OUTFNAME);

        return 0;
}
