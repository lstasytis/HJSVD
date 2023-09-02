//#define _INCLUDE_GMP_


//#define _CORDIC_
//#define _HYBRID_
//#define _DSP_
#define _IMPL_


#ifdef _INCLUDE_GMP_
    #include <gmp.h> 
    #define __gmp_const const
#endif

#pragma once


#include <iostream>
#include <fstream>
#include <vector>

#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include <ap_int.h>
#include "ap_fixed.h"

#include <hls_vector.h>
using namespace std;



// PARAMETERS TO GET COMPILE_TIME_CHANGED WITH SED



// PARAMETERS OUTSIDE OF SED ROUTINES


const int INTERFACE_BITWIDTH = 512;
const int OUTSTANDING_READ_COUNT = 64;
const int OUTSTANDING_WRITE_COUNT = 64;
const int PARAM_FIFO_DEPTH = 128;




// ==========================================================================
// HJSVD PARAMS
// ==========================================================================

const int WORD_SIZE = 32;
const int CU = 1;



typedef double interface_data_type;
typedef double kernel_data_type;
typedef double norm_data_type;

const int SIZE_N = 32;
const int SIZE_M = 32;

const int MAX_M = SIZE_N; // Matrix Row size
const int MAX_N = SIZE_M; // Matrix Col size



const int DEP = 16;
const int ACCUM_II = 6; 
const int SIZE = SIZE_N;
const int FIFO_DEPTH = 16;
const int GMEM_DEPTH = SIZE_N;
const int NUM_WORDS = 8;


//const int OP_LIMIT = int(256 / WORD_SIZE);



const int  DATA_FIFO_DEPTH = 512; // Up to BRAM capacity


//typedef ap_int<32> ap_int32_t;


typedef hls::vector<interface_data_type, NUM_WORDS> float_vec;
typedef hls::vector<kernel_data_type, NUM_WORDS> fixed_vec;

typedef hls::vector<kernel_data_type, NUM_WORDS*2> fixed_vec_double_size;

const int MAXM = SIZE_N; // Matrix Row size
const int MAXN = SIZE_M; // Matrix Col size

const int MAX_ROUNDS = MAXN-1;
const int MAX_PAIRS = MAXN/2;


const int NRMAX = SIZE_N;
const int NCMAX = SIZE_M;

const int SWEEP_LIMIT = 10;

// parameter for how aggressively to pipeline the Angle step
const int PIPE_II = int(MAXM/CU);


// Norm step splits at high level, not used in paper
const int SPLITS = 1;

// partial sum array and reduction tree count for Norm step
const int IS = 2;



// CORDIC RELATED PARAMETERS


const int REDUCED_WORD_SIZE = WORD_SIZE;

const int STAGES = int(WORD_SIZE/2)+1;

//const int WW = 32;
//const int CW = 32; // cordic word width
typedef ap_fixed<WORD_SIZE,1> pt;
typedef ap_fixed<WORD_SIZE,1> fp_angle;
typedef ap_fixed<WORD_SIZE,2> fp_vector;


typedef ap_fixed<REDUCED_WORD_SIZE,1> fp_angle_cordic;
typedef ap_fixed<REDUCED_WORD_SIZE,2> fp_vector_cordic;

// necessary only for hybrid cordic
typedef ap_fixed<REDUCED_WORD_SIZE-STAGES,1> fp_reduced_angle;
typedef ap_fixed<REDUCED_WORD_SIZE-STAGES,2> fp_reduced_vector;
typedef ap_fixed<REDUCED_WORD_SIZE-STAGES,4> fp_reduced_pi;
//typedef ap_fixed<WW,1> fpww;

// CORDIC RELATED PARAMETERS

typedef hls::vector<fp_angle, NUM_WORDS> fixed_angle_vec;
typedef hls::vector<fp_vector, NUM_WORDS> fixed_vector_vec;


typedef hls::vector<int, 2> index_pair;

// num of elements in each MCU
const int ACUM = (MAXM + CU - 1) / CU;
// num of elements in each NCU
const int ACUN = (MAXN + CU - 1) / CU;


// ==========================================================================
// HJSVD PARAMS
// ==========================================================================





// the definition of our outside kernel interface, should be edited manually

extern "C" 

void kernel_hls(
    int size_n, 
    int size_m, 
    interface_data_type* A, 
    interface_data_type* U,
    interface_data_type* S,
    interface_data_type* V
    );

