//#ifdef _IMPLEMENTATION_
//    #ifndef __SYNTHESIS__
 //       #define __SYNTHESIS__
 //   #endif
//#endif

#include <iostream>
#include <iomanip>



#include <hls_math.h>
using namespace std;

#include "kernel_hls.hpp"

#include "ap_fixed.h"
#include "hls_stream.h"
#include "hw/math_helper.hpp"


#include "hjsvd_kernel_norm.hpp"
#include "hjsvd_kernel_angle.hpp"
#include "hjsvd_kernel_rotate.hpp"
#include "hjsvd_kernel_aux.hpp"
#include "hjsvd_kernel_io.hpp"


template <typename T_outer, typename T, int NRMAX, int NCMAX, int CU,int ACUM,int ACUN>
void gesvj_single_round(
    T_outer* A, 
    T_outer* V,
    T matA_in[NCMAX][CU*2][ACUM/2],
    T matV_in[NCMAX][CU*2][ACUN/2],
    T matA_out[NCMAX][CU*2][ACUM/2],
    T matV_out[NCMAX][CU*2][ACUN/2],
    int m,
    int n,
    int rj,
    int pair_count,
    int round_count,
    T_outer &converge,
    int accum,
    int max_dep
){

    cout << "called single round\n";


    #pragma HLS stable variable=matA_out
    #pragma HLS stable variable=matV_out

    #pragma HLS dataflow



    hls::stream<index_pair> ij_load_A_strm;
#pragma HLS STREAM variable = ij_load_A_strm depth = PARAM_FIFO_DEPTH

    hls::stream<index_pair> ij_load_V_strm;
#pragma HLS STREAM variable = ij_load_V_strm depth = PARAM_FIFO_DEPTH


    hls::stream<int> ij_norm_strm;
#pragma HLS STREAM variable = ij_norm_strm depth = PARAM_FIFO_DEPTH

    hls::stream<int> ij_angle_strm;
#pragma HLS STREAM variable = ij_angle_strm depth = PARAM_FIFO_DEPTH

    hls::stream<int> ij_update_strm;
#pragma HLS STREAM variable = ij_update_strm depth = PARAM_FIFO_DEPTH

    hls::stream<index_pair> ij_write_A_strm;
#pragma HLS STREAM variable = ij_write_A_strm depth = PARAM_FIFO_DEPTH

    hls::stream<index_pair> ij_write_V_strm;
#pragma HLS STREAM variable = ij_write_V_strm depth = PARAM_FIFO_DEPTH


    hls::stream<T_outer> alpha_strm;
#pragma HLS STREAM variable = alpha_strm depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> beta_strm;
#pragma HLS STREAM variable = beta_strm depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> gamma_strm;
#pragma HLS STREAM variable = gamma_strm depth = PARAM_FIFO_DEPTH

    hls::stream<T_outer> alpha_strm_split[SPLITS];
#pragma HLS STREAM variable = alpha_strm_split depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> beta_strm_split[SPLITS];
#pragma HLS STREAM variable = beta_strm_split depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> gamma_strm_split[SPLITS];
#pragma HLS STREAM variable = gamma_strm_split depth = PARAM_FIFO_DEPTH

    hls::stream<T_outer> alpha_strm_split2;
#pragma HLS STREAM variable = alpha_strm_split2 depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> beta_strm_split2;
#pragma HLS STREAM variable = beta_strm_split2 depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> gamma_strm_split2;
#pragma HLS STREAM variable = gamma_strm_split2 depth = PARAM_FIFO_DEPTH


    hls::stream<T_outer> alpha_strm_split3;
#pragma HLS STREAM variable = alpha_strm_split3 depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> beta_strm_split3;
#pragma HLS STREAM variable = beta_strm_split3 depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> gamma_strm_split3;
#pragma HLS STREAM variable = gamma_strm_split3 depth = PARAM_FIFO_DEPTH

    hls::stream<T_outer> alpha_strm_split4;
#pragma HLS STREAM variable = alpha_strm_split4 depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> beta_strm_split4;
#pragma HLS STREAM variable = beta_strm_split4 depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> gamma_strm_split4;
#pragma HLS STREAM variable = gamma_strm_split4 depth = PARAM_FIFO_DEPTH



    hls::stream<T> s_strm_a[1];
#pragma HLS STREAM variable = s_strm_a depth = PARAM_FIFO_DEPTH
    hls::stream<T> c_strm_a[1];
#pragma HLS STREAM variable = c_strm_a depth = PARAM_FIFO_DEPTH


    hls::stream<T> s_strm_v[1];
#pragma HLS STREAM variable = s_strm_v depth = PARAM_FIFO_DEPTH
    hls::stream<T> c_strm_v[1];
#pragma HLS STREAM variable = c_strm_v depth = PARAM_FIFO_DEPTH


    hls::stream<T_outer> conv_strm[1];
#pragma HLS STREAM variable = conv_strm depth = PARAM_FIFO_DEPTH


    hls::stream<T_outer> conv_strm_final[1];
#pragma HLS STREAM variable = conv_strm_final depth = PARAM_FIFO_DEPTH

    hls::stream<T_outer> alpha_strm_float[1];
#pragma HLS STREAM variable = alpha_strm_float depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> beta_strm_float[1];
#pragma HLS STREAM variable = beta_strm_float depth = PARAM_FIFO_DEPTH
    hls::stream<T_outer> gamma_strm_float[1];
#pragma HLS STREAM variable = gamma_strm_float depth = PARAM_FIFO_DEPTH



#pragma HLS ARRAY_PARTITION variable = c_strm_a complete
#pragma HLS ARRAY_PARTITION variable = s_strm_a complete

#pragma HLS ARRAY_PARTITION variable = c_strm_v complete
#pragma HLS ARRAY_PARTITION variable = s_strm_v complete


    hls::stream<fixed_vec> A_pre_norm_i_fixed;
#pragma HLS STREAM variable = A_pre_norm_i_fixed depth = DATA_FIFO_DEPTH

    hls::stream<fixed_vec> A_pre_norm_j_fixed;
#pragma HLS STREAM variable = A_pre_norm_j_fixed depth = DATA_FIFO_DEPTH
    
    
    hls::stream<fixed_vec> A_pre_norm_i_float;
#pragma HLS STREAM variable = A_pre_norm_i_float depth = DATA_FIFO_DEPTH
    hls::stream<fixed_vec> A_pre_norm_j_float;
#pragma HLS STREAM variable = A_pre_norm_j_float depth = DATA_FIFO_DEPTH





    hls::stream<fixed_vec> A_pre_norm_i_float_split[SPLITS];
#pragma HLS STREAM variable = A_pre_norm_i_float_split depth = DATA_FIFO_DEPTH
    hls::stream<fixed_vec> A_pre_norm_j_float_split[SPLITS];
#pragma HLS STREAM variable = A_pre_norm_j_float_split depth = DATA_FIFO_DEPTH



hls::stream<fixed_vec> A_pre_rotate_i_fixed;
#pragma HLS STREAM variable = A_pre_rotate_i_fixed depth = DATA_FIFO_DEPTH
hls::stream<fixed_vec> A_pre_rotate_j_fixed;
#pragma HLS STREAM variable = A_pre_rotate_j_fixed depth = DATA_FIFO_DEPTH


hls::stream<fixed_vec> A_post_rotate_i_float;
#pragma HLS STREAM variable = A_post_rotate_i_float depth = DATA_FIFO_DEPTH
hls::stream<fixed_vec> A_post_rotate_j_float;
#pragma HLS STREAM variable = A_post_rotate_j_float depth = DATA_FIFO_DEPTH




hls::stream<fixed_vec> V_pre_rotate_i_float;
#pragma HLS STREAM variable = V_pre_rotate_i_float depth = DATA_FIFO_DEPTH
hls::stream<fixed_vec> V_pre_rotate_j_float;
#pragma HLS STREAM variable = V_pre_rotate_j_float depth = DATA_FIFO_DEPTH

hls::stream<fixed_vec> V_pre_rotate_i_fixed;
#pragma HLS STREAM variable = V_pre_rotate_i_fixed depth = DATA_FIFO_DEPTH
hls::stream<fixed_vec> V_pre_rotate_j_fixed;
#pragma HLS STREAM variable = V_pre_rotate_j_fixed depth = DATA_FIFO_DEPTH



hls::stream<fixed_vec> V_post_rotate_i_float;
#pragma HLS STREAM variable = V_post_rotate_i_float depth = DATA_FIFO_DEPTH
hls::stream<fixed_vec> V_post_rotate_j_float;
#pragma HLS STREAM variable = V_post_rotate_j_float depth = DATA_FIFO_DEPTH


    int half_pair_count = pair_count / SPLITS;

    T alpha[1];
    #pragma HLS ARRAY_PARTITION variable = alpha complete
    T beta[1];
    #pragma HLS ARRAY_PARTITION variable = beta complete
    T gamma[1];
    #pragma HLS ARRAY_PARTITION variable = gamma complete

    T s[1];
    #pragma HLS ARRAY_PARTITION variable = s  complete
    T c[1];
    #pragma HLS ARRAY_PARTITION variable = c  complete



    #pragma HLS DEPENDENCE variable=A type=inter dependent=false
    #pragma HLS DEPENDENCE variable=A type=intra dependent=false

    #pragma HLS DEPENDENCE variable=V type=inter dependent=false
    #pragma HLS DEPENDENCE variable=V type=intra dependent=false

    int i,j;

    generate_rr_indexes<T, T_outer,NRMAX, NCMAX, CU, ACUM, ACUN>(
        m,
        n,
        ij_load_A_strm,
        ij_load_V_strm,
        ij_write_A_strm,
        ij_write_V_strm,
        round_count,
        pair_count,
        rj);
    
      read_2cols_local_double_size<T,T_outer,NRMAX,CU,ACUM>(
        A, 
        matA_in,
        A_pre_norm_i_float,
        A_pre_norm_j_float,
        n,
        ij_load_A_strm,
        round_count,
        pair_count,
        accum);

    read_2cols_local_double_size<T,T_outer,NCMAX,CU,ACUN>(
        V, 
        matV_in,
        V_pre_rotate_i_float,
        V_pre_rotate_j_float,
        n,
        ij_load_V_strm,
        round_count,
        pair_count,
        accum);  
    
    norm_computation_splitter<T, T_outer, NRMAX, NCMAX, CU, ACUM>( 

        A_pre_norm_i_float,
        A_pre_norm_j_float,

        A_pre_norm_i_float_split,
        A_pre_norm_j_float_split,


        A_pre_rotate_i_fixed,
        A_pre_rotate_j_fixed,

        pair_count,
        n);



    norm_computation<T, T_outer, NRMAX, NCMAX, CU, ACUM>( 
        A_pre_norm_i_float_split[0],
        A_pre_norm_j_float_split[0],
        m, 
        alpha_strm_split[0], 
        beta_strm_split[0], 
        gamma_strm_split[0],
        half_pair_count,
        max_dep);


    norm_computation_merger<T, T_outer, NRMAX, NCMAX, CU, ACUM>( 
        alpha_strm_split,
        beta_strm_split,
        gamma_strm_split,

        alpha_strm,
        beta_strm,
        gamma_strm,
        pair_count);


    angle_computation<T,T_outer>(
        round_count,
        pair_count, 
        alpha_strm, 
        beta_strm, 
        gamma_strm, 
        conv_strm[0], 
        s_strm_a[0], 
        c_strm_a[0],                
        s_strm_v[0], 
        c_strm_v[0]);

    rotate_AV<T,T_outer, NRMAX, NCMAX, CU, ACUM, ACUN>(
        A_pre_rotate_i_fixed,
        A_pre_rotate_j_fixed,

        A_post_rotate_i_float,
        A_post_rotate_j_float,

        V_pre_rotate_i_float,
        V_pre_rotate_j_float,

        V_post_rotate_i_float,
        V_post_rotate_j_float,

        n,
        round_count,
        pair_count, 
        s_strm_a[0], 
        c_strm_a[0],
        s_strm_v[0], 
        c_strm_v[0]);


    write_2cols_local_double_size<T,T_outer,NRMAX,CU,ACUM>(
        A,
        matA_out, 
        A_post_rotate_i_float,
        A_post_rotate_j_float,
        n,
        ij_write_A_strm,
        round_count,
        pair_count,
        accum);

    write_2cols_local_double_size<T,T_outer,NRMAX,CU,ACUM>(
        V, 
        matV_out,
        V_post_rotate_i_float,
        V_post_rotate_j_float,
        n,
        ij_write_V_strm,
        round_count,
        pair_count,
        accum);

    
    conv_tester<T, T_outer>(
        round_count, 
        pair_count,
        conv_strm[0],
        converge);

}

template <typename T_outer,typename T, int NRMAX, int NCMAX, int CU>
void kernel_top(
    int n, 
    int m, 
    T_outer* A, 
    T_outer* U, 
    T_outer* S, 
    T_outer* V
    ){




    int accum = n/CU;

    int max_dep;
    if ((m/CU) < DEP) {
        max_dep = m/CU;
    } else {
        max_dep = DEP;
    }


    const int IO_EXP = 2;




    // constants
    const T epsilon_adjustment = 1.e-7;
    const T one = 1.0;
    const T one_minus_ep = 1.0;
    const T one_minus_ep_half = 1;
    T_outer converge = 1.0;
    

    const T two = 2.0;
    const T zero = 0.0;
    const T sign = one;



    std::cout << "create arrays"<< std::endl;

    T_outer sigma[NCMAX];
    #pragma HLS BIND_STORAGE variable = sigma type = RAM_1P impl = bram



    T_outer conv_tmp[NCMAX];
#pragma HLS BIND_STORAGE variable = sigma type = RAM_1P impl = bram

    T matA_in[NCMAX][CU*IO_EXP][ACUM/IO_EXP];
#pragma HLS BIND_STORAGE variable = matA_in type = RAM_1P impl = uram
#pragma HLS ARRAY_PARTITION variable = matA_in complete dim=2


    T matV_in[NCMAX][CU*IO_EXP][ACUM/IO_EXP];
#pragma HLS BIND_STORAGE variable = matV_in type = RAM_1P impl = uram
#pragma HLS ARRAY_PARTITION variable = matV_in complete dim=2


    T matA_out[NCMAX][CU*IO_EXP][ACUM/IO_EXP];
#pragma HLS BIND_STORAGE variable = matA_out type = RAM_1P impl = uram
#pragma HLS ARRAY_PARTITION variable = matA_out complete dim=2



    T matV_out[NCMAX][CU*IO_EXP][ACUM/IO_EXP];
#pragma HLS BIND_STORAGE variable = matV_out type = RAM_1P impl = uram
#pragma HLS ARRAY_PARTITION variable = matV_out complete dim=2


    T_outer matU[NRMAX][NRMAX];
#pragma HLS BIND_STORAGE variable = matU type = RAM_1P impl = uram


std::cout << "init arrays"<< std::endl;


INIT_S:
    for (int j = 0; j < n*16/16; j++) {
#pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
#pragma HLS PIPELINE II = 1
        sigma[j] = 0.0;
    }

// initialize V matrix to diagonal
INIT_V:
    for (int i = 0; i < n*16/16; i++) {
#pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
        for (int j = 0; j < n*16/16; j++) {
#pragma HLS PIPELINE II = 2
//#pragma HLS unroll factor=CU
#pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
            if (i == j) {
                matV_in[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] = one_minus_ep_half;
            } else {
                matV_in[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] = 0.0;
            }
        }
    }

// read A from DDR

READ_A:
    for (int i = 0; i < m; i++) {
#pragma HLS LOOP_TRIPCOUNT min = NRMAX max = NRMAX

READ_A_INNER:
        for (int j = 0; j < n; j++) {
            #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
            #pragma HLS PIPELINE II = 1
            T_outer tmp1 =  A[i * n + j];
            T tmp2 = (T) tmp1;
            matA_in[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] = tmp2;
#ifndef __SYNTHESIS__
#ifdef __SOLVER_DEBUG__
            std::cout << (float) matA[j+z][i % CU][i / CU] << " ";
#endif
#endif
            
        }
#ifndef __SYNTHESIS__
#ifdef __SOLVER_DEBUG__
        std::cout << std::endl;
#endif
#endif
    }




    #pragma HLS DEPENDENCE variable=A type=inter dependent=false
    #pragma HLS DEPENDENCE variable=A type=intra dependent=false

    #pragma HLS DEPENDENCE variable=V type=inter dependent=false
    #pragma HLS DEPENDENCE variable=V type=intra dependent=false


    int pair_count = int(n/2);
    int round_count = n-1;
    int sweep_limit = SWEEP_LIMIT;

    int i = 0;
    int j = 1;
    int tmp;
    bool tick = true;


    int sweep_loop = 0;
    
    T_outer new_converge;

    // e-25 for experiments, in reality ~7 for float, 8-12 for double
    T_outer epsilon = 1.e-25;
    if (sizeof(T_outer) <= sizeof(float)) {
        epsilon = 1.e-25;
    }


    CONV_WHILE:
    while (converge > epsilon && sweep_loop < sweep_limit) {
    #pragma HLS LOOP_TRIPCOUNT min = 10 max = 10
            
        converge = 0;
        //#pragma HLS DATAFLOW
        LOOP_ROUNDS:
        for (int rj = 0; rj < round_count; rj++) {
            #pragma HLS LOOP_TRIPCOUNT min = MAX_ROUNDS max = MAX_ROUNDS

            if (tick){
            gesvj_single_round<T_outer,T,NRMAX, NCMAX, CU, ACUM,ACUN>(
                A,
                V,
                matA_in, 
                matV_in,
                matA_out, 
                matV_out,
                m,
                n,
                rj,
                pair_count,
                round_count,
                new_converge,
                accum,
                max_dep
            );
            tick = false;
            } else {
             gesvj_single_round<T_outer, T,NRMAX, NCMAX, CU, ACUM,ACUN>(
                A,
                V,
                matA_out, 
                matV_out,
                matA_in, 
                matV_in,
                m,
                n,
                rj,
                pair_count,
                round_count,
                new_converge,
                accum,
                max_dep
            );   
            tick = true;            
            }


            // if tick is true, output is matA_in

            converge = hls::max(converge,new_converge);

            cout << "max convergence after "<< rj << " rounds: " << new_converge << "\n";
        }
        sweep_loop++;
        cout << "sweeps finished: " << sweep_loop << "\n";
    }




    // depending on which URAM partition we ended up converging, write back from that one

    if (tick){
    MOVE_OUTPUT:
        for (int i = 0; i < n; i++) {
            #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
            for (int j = 0; j < n; j++) {
                #pragma HLS pipeline II=1
                #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX

                matA_out[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] = matA_in[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] ;
                matV_out[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] = matV_in[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)] ;
            }
        }
    }

    // calculate the sigma diagonal matrix
    const int DEP_SIGMA = 16;
    T_outer accu_s = 0;
    T_outer AUS_accu[DEP_SIGMA];
    int jj = 0;
    CALC_US:
    for (int j = 0; j < n; j++) {
        #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
        accu_s = 0;
        for (int t = 0; t < DEP_SIGMA; t++) {
            #pragma HLS UNROLL
            AUS_accu[t] = 0;
        }
        for (int i = 0; i < m; i++) {
            #pragma HLS PIPELINE II = 1
            #pragma HLS LOOP_TRIPCOUNT min = NRMAX max = NRMAX
            T tmpaa = matA_out[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)];
            T_outer Aij = (T_outer)  tmpaa;

            AUS_accu[i % DEP_SIGMA] += Aij * Aij;
        }

        T_outer AUS_accu_tmp0[8];
        for (int t = 0; t < 8; t++) {
            #pragma HLS PIPELINE
            AUS_accu_tmp0[t] = AUS_accu[2 * t] + AUS_accu[2 * t + 1];
        }
        // add 8 -> 4
        T_outer AUS_accu_tmp1[4];
        for (int t = 0; t < 4; t++) {
            #pragma HLS PIPELINE
            AUS_accu_tmp1[t] = AUS_accu_tmp0[2 * t] + AUS_accu_tmp0[2 * t + 1];
        }
        // add 4 -> 2
        T_outer AUS_accu_tmp2[2];
        for (int t = 0; t < 2; t++) {
            #pragma HLS PIPELINE
            AUS_accu_tmp2[t] = AUS_accu_tmp1[2 * t] + AUS_accu_tmp1[2 * t + 1];
        }
        // add 2 -> 1
        accu_s = (T_outer) (AUS_accu_tmp2[0] + AUS_accu_tmp2[1]);
        #ifdef __SOLVER_DEBUG__
            std::cout << "accu_s before sqrt = " << std::setprecision(15) << (float) accu_s << std::endl;
        #endif
        accu_s = xf::solver::internal::m::sqrt(accu_s);
        #ifdef __SOLVER_DEBUG__
            std::cout << "accu_s = " << std::setprecision(15) << (float) accu_s << std::endl;
        #endif

        sigma[j] = accu_s;

        // when accu_s == 0, skip this column with initialized U_i
        if (accu_s > epsilon) {
            #ifdef __SOLVER_DEBUG__
            std::cout << "accu_s > epsilon " << std::endl;
            #endif
            for (int i = 0; i < m; i++) {

                #pragma HLS PIPELINE II = 1
                #pragma HLS LOOP_TRIPCOUNT min = NRMAX max = NRMAX
                T tmpa = matA_out[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)];
                T_outer tmpa2 = (T_outer) tmpa;
                T_outer tmpa3 = tmpa2 / accu_s;
                matU[i][jj] =  tmpa3;
            }
            jj++;
        }
    }

//================output================
// output S, U, V and store


    OUT_U:
        for (int i = 0; i < m*16/16; i++) {
            #pragma HLS LOOP_TRIPCOUNT min = NRMAX max = NRMAX
        OUT_U_INNER:
            for (int j = 0; j < m*16/16; j+=16) {
                #pragma HLS LOOP_TRIPCOUNT min = NRMAX max = NRMAX
                OUT_U_16_SET:
                for (int z = 0; z < 16 ; z++){
                    #pragma HLS PIPELINE II = 1
                    U[i * m + j+z] = matU[i][j+z];
                }
            }
        }
    
    OUT_S:
    for (int i = 0; i < n*16/16; i++) {
        #pragma HLS PIPELINE II = 1
        #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
        S[i] = sigma[i];
    }


    OUT_V:
        for (int i = 0; i < n*16/16; i++) {
    #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
    OUT_V_INNER:
            for (int j = 0; j < n*16/16; j++) {
                #pragma HLS PIPELINE II = 1
                #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX
                T_outer out_v = (T_outer) matV_out[j][i % (CU*IO_EXP)][i / (CU*IO_EXP)];
                V[i * n + j] = out_v;
                    
            }
        }

#ifndef __SYNTHESIS__
#ifdef __SOLVER_DEBUG__
    std::cout << "sweep iterations = " << sweep_loop << "--------" << std::endl;
#endif
#endif
}
