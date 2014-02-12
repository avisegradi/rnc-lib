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

/** \file

    \brief This application measures the efficiency of sparse coding
 */

#include <glib.h>
#include <rnc>
#include <rnc-lib/rlc.h>
#include <iostream>
#include <time.h>
#include <exception>
#include <stdlib.h>
#include <stdio.h>
#include <mkstr>
#include <auto_arr_ptr>
#include <memory>
#include <string.h>
#include <sys/time.h>
#include <test.h>

using namespace std;
using namespace rnc;
using namespace rnc::coding;
using namespace rnc::matrix;
using namespace rnc::test;

rnc::random::mt_state rnd_state;

void chkSystem(int retval)
{
        if (retval < 0)
                throw string("error: fork failed");
        else if (retval)
                throw string("error: command execution failed");

}

int getint(string str)
{
        stringstream ss(str);
        int i;
        ss >> i;
        return i;
}

double getdouble(string str)
{
        stringstream ss(str);
        double i;
        ss >> i;
        return i;
}

double tdiff(const struct timeval& t1,
             const struct timeval& t2)
{
        double retval = 0;
        retval = t2.tv_sec - t1.tv_sec;
        if (t1.tv_usec < t2.tv_usec)
        {
                retval += 1-(t1.tv_usec - t2.tv_usec)*1e-6;
                retval -= 1;
        }
        else
        {
                retval += (t2.tv_usec - t1.tv_usec)*1e-6;
        }
        return retval;
}

char* throughput(off_t size,
                 const struct timeval& t1,
                 const struct timeval& t2)
{
        double s = size;
        static char buf[32];
        sprintf(buf, "%.4fMB/s", s/tdiff(t1, t2)/(1<<20));
        return buf;
}

char* timediff(const struct timeval& t1,
                const struct timeval& t2)
{
        static char buf[32];
        sprintf(buf, "%f", tdiff(t1, t2));
        return buf;
}

void p(const BlockList &blocks)
{
        Matrix *C, *D;
        blocks.to_matrices(&C, &D);

        auto_ptr<Matrix> apC(C), apD(D);

        p(*C, *D, cout);
}

struct WSGatherResult
{
        bool success;
        int wasted_capacity;
        WSGatherResult(bool success, int wasted_capacity)
                : success(success), wasted_capacity(wasted_capacity)
        {}

        void p(const char *prefix = "")
        {
                printf("%s%s %d\n",
                       prefix, success ? "1" : "0", wasted_capacity);
        }
};

WSGatherResult gather_working_set(BlockList &src, BlockList &working_set)
{
        const size_t N = (*src.blocks())->coeff_count;

        WSGatherResult result(false, 0);

        if (src.count() < N)
                return result;

        BlockList source_set = src.shallow_copy();

        for (size_t i = 0; i < N; ++i)
                working_set.add(source_set.random_drop(&rnd_state));

        for (int i=0; ; ++i) {
                auto_ptr<Matrix> m(working_set.to_matrix(BlockList::Coefficients));
                Matrix inverse(N, N);
                if (matrix::invert(*m, inverse))
                {
//                        m.release();
                        break;
                }

                if (source_set.count() == 0)
                        return result;

                working_set.random_drop(&rnd_state);
                working_set.add(source_set.random_drop(&rnd_state));
                result.wasted_capacity += 1;
        }

        result.success = true;
        return result;
}

void replenish(BlockList &src, BlockList &dst, int threshold, int target,
                         double A)
{
        const size_t N = (*src.blocks())->coeff_count;
        const size_t M = (*src.blocks())->block_length;
        const int cnt = target - dst.count();

        if (src.count() > (unsigned int)threshold)
                return;

        BlockList N_set = src.random_sample(N, &rnd_state);

        const size_t S = int(A * N) + 1;

        //printf("### Working set:\n");
        //p(working_set);

        Matrix *coeffP, *dataP;
        working_set.to_matrices(&coeffP, &dataP);
        auto_ptr<Matrix> coeffap(coeffP), dataap(dataP);
        Matrix &coeff = *coeffP;
        Matrix &data = *dataP;

        for (int i=0; i<cnt; ++i)
        {
                Matrix rnd_coeff(1, N);
                Matrix result_coeff(1, S, false);
                Matrix result_data(1, M, false);
                rand_matr(rnd_coeff, A, &rnd_state);
                mul(rnd_coeff, coeff, result_coeff);
                mul(rnd_coeff, data, result_data);

                Block *blk = new Block();
                blk->coefficients = result_coeff.rows[0];
                blk->data = result_data.rows[0];
                blk->coeff_count = N;
                blk->block_length = M;

                dst.add(blk);
        }

        return res;
}

void prInt(int arr[], int length, int nulls_from)
{
        bool first = true;

        for (int i=0; i<nulls_from; ++i)
        {
                if (first) first=false;
                else printf(";");
                printf("%04d", arr[i]);
        }
        for (int i=nulls_from; i<length; ++i)
        {
                if (first) first=false;
                else printf(";");
                printf("NULL");
        }

}

int main(int argc, char **argv)
try
{
        fq::init();

        random::init(&rnd_state, time(NULL));

        if (argc < 8)
                throw string(MKStr() << "usage: " << argv[0] <<
                             " <input filename> "
                             "<N : block count> "
                             "<A : sparse coding factor> "
                             "<T : redundancy threshold> "
                             "<R : redundancy target> "
                             "<F : failure probability> "
                             "<id>");

        const string fname = argv[1];
        const int N = getint(argv[2]);
        const double A = getdouble(argv[3]);
        const int T = getint(argv[4]);
        const int R = getint(argv[5]);
        const double F = getdouble(argv[6]);
        const int id = getint(argv[7]);

        //printf("%02d MEM file=%s q=%d mode=sim N=%d A=%f T=%d R=%d F=%f\n",
        //       id, fname.c_str(), fq_size, N, A, T, R, F);

        File infile(fname, N);

        const int M = infile.ncols();

        Matrix orig(infile.data(), N, M);
        Matrix identity(N, N);
        set_identity(identity);
        BlockList blocks = infile.block_list(identity.rows);

        //printf("block count: %lu\n", blocks.count());

        Matrix *C, *D;
        blocks.to_matrices(&C, &D);

        //printf("C: %lu x %lu\n", C->nrows, C->ncols);
        //printf("D: %lu x %lu\n", D->nrows, D->ncols);
        //p(orig);

        /*
        BlockList sample = blocks.random_sample(15, &rnd_state);
        printf("### Random sample:\n");
        p(sample);
        sample.drop(1);
        printf("### Dropped @[1]:\n");
        p(sample);
        sample.random_drop(0.3, 15, &rnd_state);
        printf("### Random drop:\n");
        p(sample);
        sample.random_drop(&rnd_state);
        printf("### Random drop one:\n");
        p(sample);
        */

        BlockList block_set(N, true);
        replenish(blocks, block_set, T, R, A);

        const int maxsteps = 50;
        int wasted[maxsteps];
        int blockcount[maxsteps];
        int replenished[maxsteps];
        int dead_at = -1;
        for (int i = 0; i < maxsteps; ++i)
        {
                block_set.random_drop(F, block_set.count(), &rnd_state);
                int cnt_after_drop = block_set.count();
                WSGatherResult r = replenish(block_set, block_set, T, R, A);
                blockcount[i] = block_set.count();
                replenished[i] = block_set.count() - cnt_after_drop;

                if (!r.success)
                {
                        dead_at = i;
                        break;
                }
                wasted[i] = r.wasted_capacity;
        }

        printf("RESULT %d %s %d %d %f %d %d %f ",
               id, fname.c_str(), fq_size, N, A, T, R, F);
        if (dead_at < 0)
                printf("NULL ");
        else
                printf("%d ", dead_at);

        if (dead_at < 0)
                dead_at = maxsteps;

        prInt(wasted, maxsteps, dead_at);
        printf(" "); prInt(blockcount, maxsteps, dead_at);
        printf(" "); prInt(replenished, maxsteps, dead_at);
        printf("\n");

        return 0;
}
catch (exception &ex)
{
        cerr << "Error: " << ex.what() << endl;
}
catch (const string &msg)
{
        cerr << "Error: " << msg << endl;
}
