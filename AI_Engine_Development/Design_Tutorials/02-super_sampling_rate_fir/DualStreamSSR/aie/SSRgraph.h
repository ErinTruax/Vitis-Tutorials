/*
* (c) Copyright 2021-2022 Xilinx, Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#pragma once

#include <adf.h>
#include "system_settings.h"
#include "aie_kernels.h"


std::vector<cint16> taps = std::vector<cint16>({
    {   -82,  -253},{     0,  -204},{    11,   -35},{  -198,   273},
    {  -642,   467},{ -1026,   333},{  -927,     0},{  -226,   -73},
    {   643,   467},{   984,  1355},{   550,  1691},{     0,   647},
    {   538, -1656},{  2860, -3936},{  6313, -4587},{  9113, -2961},
    {  9582,     0},{  7421,  2411},{  3936,  2860},{  1023,  1409},
    {  -200,  -615},{     0, -1778},{   517, -1592},{   467,  -643},
    {  -192,   140},{  -882,   287},{ -1079,     0},{  -755,  -245},
    {  -273,  -198},{    22,    30},{    63,   194},{     0,   266}
});

std::vector<cint16> taps_aie(taps.rbegin(),taps.rend());

#define PHASE(N,NP) {taps_aie[N],taps_aie[N+NP],taps_aie[N+2*NP],taps_aie[N+3*NP]}

std::vector<cint16> taps8p[8] = { PHASE(0,8), PHASE(1,8), PHASE(2,8), PHASE(3,8),
    PHASE(4,8), PHASE(5,8), PHASE(6,8), PHASE(7,8) };

    const int SHIFT = 0; // to analyze the output generated by impulses at the input
    //const int SHIFT = 15; // for realistic input samples


    using namespace adf;

    #define FirstCol 23
    #define LastCol (FirstCol+7)

    class FIRGraph_SSR8: public adf::graph
    {
    private:
        kernel k[8][8];

    public:
        input_port in[16]; // 8 columns, 2 streams per kernel
        output_port out[16]; // 8 columns, 2 streams per kernel

        FIRGraph_SSR8()
        {
            // k[N][0] is always the first in the cascade stream
            // Topology of the TopGraph
            //
            //      7,7   ...   ...   7,0 <--
            //  --> 6,0   ...   ...   6,7
            //       .     .     .     .
            //       .     .     .     .
            //       .     .     .     .
            //      1,7   ...   ...   1,0 <--
            //  --> 0,0   ...   ...   0,7

            const int NPhases = 8;

            for(int row = 0;row<NPhases; row++)
            for(int col=0;col<NPhases; col++)
            {
                int col1 = (row%2?NPhases-col-1:col); // Revert col order on odd rows

                // Which Phase in (row,col) ?
                int PhaseSelect = col-row;
                if(PhaseSelect<0) PhaseSelect += NPhases;
                int DiscardSample = (row>col?1:0); // Must discard 1 sample on some aie_kernels

                if(DiscardSample == 1) // Don't know how to make DiscardSample a constant suitable for a template parameter
                {
                    // kernel instanciation
                    if(col1==0)
                    {
                        k[row][col1] = kernel::create_object<DoubleStream::FIR_MultiKernel_cout<NUM_SAMPLES,SHIFT,true,false>>(taps8p[PhaseSelect]);
                    }
                    else
                    {
                        if(col1==NPhases-1)
                        {
                            k[row][col1] = kernel::create_object<DoubleStream::FIR_MultiKernel_cin<NUM_SAMPLES,SHIFT,true,false>>(taps8p[PhaseSelect]);
                        }
                        else
                        {
                            k[row][col1] = kernel::create_object<DoubleStream::FIR_MultiKernel_cincout<NUM_SAMPLES,SHIFT,true,false>>(taps8p[PhaseSelect]);
                        }
                    }
                }
                else
                {
                    // kernel instanciation
                    if(col1==0)
                    {
                        k[row][col1] = kernel::create_object<DoubleStream::FIR_MultiKernel_cout<NUM_SAMPLES,SHIFT,false,false>>(taps8p[PhaseSelect]);
                    }
                    else
                    {
                        if(col1==NPhases-1)
                        {
                            k[row][col1] = kernel::create_object<DoubleStream::FIR_MultiKernel_cin<NUM_SAMPLES,SHIFT,false,false>>(taps8p[PhaseSelect]);
                        }
                        else
                        {
                            k[row][col1] = kernel::create_object<DoubleStream::FIR_MultiKernel_cincout<NUM_SAMPLES,SHIFT,false,false>>(taps8p[PhaseSelect]);
                        }
                    }
                }
            }

            for(int i=0;i<NPhases;i++)
            for(int j=0;j<NPhases;j++)
            {
                runtime<ratio>(k[i][j]) = 0.9;
                source(k[i][j]) = "aie_kernels/FirDoubleStream.cpp";
                headers(k[i][j]) = {"aie_kernels/FirDoubleStream.h"};
            }

            // Constraints: location of the first kernel in the cascade
            for(int i=0;i<NPhases;i++)
            {
                int j = (i%2?LastCol:FirstCol); // 23 on even rows and 230on odd rows
                location<kernel>(k[i][0]) = tile(j,i);
            }


            // Cascade Connections
            for(int row=0;row<NPhases;row++)
            {
                for(int i=0;i<NPhases-1;i++) connect<cascade> (k[row][i].out[0],k[row][i+1].in[2]);
                connect<stream> (k[row][NPhases-1].out[0],out[2*row]);
                connect<stream> (k[row][NPhases-1].out[1],out[2*row+1]);
            }

            // Input Streams connections
            for(int row = 0;row<NPhases;row++)
            for(int col=0;col<NPhases;col++)
            {
                int col1 = (row%2?NPhases-col-1:col); // kernel col is inverted on odd rows
                int fiforow = row%2;

                connect<stream> n0(in[2*col],k[row][col1].in[0]);
                connect<stream> n1(in[2*col+1],k[row][col1].in[1]);
                fifo_depth(n0) = 512;
                fifo_depth(n1) = 512;

                location<fifo>(n0) = dma_fifo(aie_tile, FirstCol+col, fiforow, 0x0000, 512);
                location<fifo>(n1) = dma_fifo(aie_tile, FirstCol+col, fiforow, 0x2000, 512);
            }
        };
    };


    class TopGraph: public adf::graph
    {
    public:
			FIRGraph_SSR8 G1;

			input_plio plin[16]; // 8 columns, 2 streams per kernel
			output_plio plout[16]; // 8 columns, 2 streams per kernel

			TopGraph()
			{
				for(int i=0;i<8;i++)
				{
          plin[2*i]   = input_plio::create("PhaseIn_"+std::to_string(i)+"_0",plio_64_bits,"data/PhaseIn_"+std::to_string(i)+"_0.txt",500);
          plin[2*i+1] = input_plio::create("PhaseIn_"+std::to_string(i)+"_1",plio_64_bits,"data/PhaseIn_"+std::to_string(i)+"_1.txt",500);

          plout[2*i]   = output_plio::create("PhaseOut_"+std::to_string(i)+"_0",plio_64_bits,"data/PhaseOut_"+std::to_string(i)+"_0.txt",500);
          plout[2*i+1] = output_plio::create("PhaseOut_"+std::to_string(i)+"_1",plio_64_bits,"data/PhaseOut_"+std::to_string(i)+"_1.txt",500);

          connect<> (plin[2*i].out[0],G1.in[2*i]);
          connect<> (plin[2*i+1].out[0],G1.in[2*i+1]);
          connect<> (G1.out[2*i],plout[2*i].in[0]);
          connect<> (G1.out[2*i+1],plout[2*i+1].in[0]);

				}
			}
    };
