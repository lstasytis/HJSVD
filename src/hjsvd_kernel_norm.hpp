
template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void norm_computation_splitter(
                      hls::stream<fixed_vec>& A_pre_norm_i_float,
                      hls::stream<fixed_vec>& A_pre_norm_j_float,

                      hls::stream<fixed_vec> A_pre_norm_i_float_split[SPLITS],
                      hls::stream<fixed_vec> A_pre_norm_j_float_split[SPLITS],


                      hls::stream<fixed_vec>& A_pre_rotate_i_fixed,
                      hls::stream<fixed_vec>& A_pre_rotate_j_fixed,

                      int pair_count,
                      int n) {
    #pragma HLS inline off

    int accum = n / CU;

    LOOP_PAIRS_NORM:
    for (int rj = 0; rj < pair_count; rj++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS

        for (int k = 0; k < accum; k++) {
            #pragma HLS PIPELINE II = 1 rewind
            fixed_vec tmp_i = A_pre_norm_i_float.read();
            fixed_vec tmp_j = A_pre_norm_j_float.read();

            A_pre_norm_i_float_split[rj % SPLITS].write(tmp_i);
            A_pre_norm_j_float_split[rj % SPLITS].write(tmp_j);

            A_pre_rotate_i_fixed.write(tmp_i);
            A_pre_rotate_j_fixed.write(tmp_j);
        }

    }   
}


template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void norm_computation_local_merger(

                      hls::stream<T_outer> alpha_strm_local[IS],
                      hls::stream<T_outer> beta_strm_local[IS],
                      hls::stream<T_outer> gamma_strm_local[IS],


                      hls::stream<T_outer>& alpha_strm,
                      hls::stream<T_outer>& beta_strm,
                      hls::stream<T_outer>& gamma_strm,
                      int pair_count,
                      int rj) {
    #pragma HLS inline off

        alpha_strm.write(alpha_strm_local[rj % IS].read());
        beta_strm.write(beta_strm_local[rj % IS].read());
        gamma_strm.write(gamma_strm_local[rj % IS].read());

        alpha_strm.write(alpha_strm_local[rj+1 % IS].read());
        beta_strm.write(beta_strm_local[rj+1 % IS].read());
        gamma_strm.write(gamma_strm_local[rj+1 % IS].read());
}

template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void norm_computation_merger(

                      hls::stream<T_outer> alpha_strm_split[SPLITS],
                      hls::stream<T_outer> beta_strm_split[SPLITS],
                      hls::stream<T_outer> gamma_strm_split[SPLITS],


                      hls::stream<T_outer>& alpha_strm,
                      hls::stream<T_outer>& beta_strm,
                      hls::stream<T_outer>& gamma_strm,
                      int pair_count) {
    #pragma HLS inline off

    LOOP_PAIRS_NORM:
    for (int rj = 0; rj < pair_count; rj++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS

        alpha_strm.write(alpha_strm_split[rj % SPLITS].read());
        beta_strm.write(beta_strm_split[rj % SPLITS].read());
        gamma_strm.write(gamma_strm_split[rj % SPLITS].read());
    }  
}


template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void norm_computation_multiply(
                     norm_data_type alpha_acc[IS][CU][DEP],
                     norm_data_type beta_acc[IS][CU][DEP],
                     norm_data_type gamma_acc[IS][CU][DEP],

                      hls::stream<fixed_vec>& A_pre_norm_i_float,
                      hls::stream<fixed_vec>& A_pre_norm_j_float,

                      int m, // column size

                      int pair_count,
                      int rj_outer)
                      {


            int accum = m/CU;
            
            // start MAC operations
            CALC_TWO_ELE_SETS:
            for (int i =0; i < IS;i++){

                
                CALC_ELEMENTS:
                for (int k = 0; k < accum; k++) {
                    #pragma HLS loop_flatten
                    #pragma HLS PIPELINE II = SPLITS style=frp
                    #pragma HLS LOOP_TRIPCOUNT min = ACUM max = ACUM
                    #pragma HLS dependence variable = alpha_acc inter false
                    #pragma HLS dependence variable = beta_acc inter false
                    #pragma HLS dependence variable = gamma_acc inter false

                    fixed_vec tmp_i = A_pre_norm_i_float.read();
                    fixed_vec tmp_j = A_pre_norm_j_float.read();

                    fixed_vec tmp_i_fixed, tmp_j_fixed;

                    #pragma HLS DEPENDENCE variable=tmp_i type=inter dependent=false
                    #pragma HLS DEPENDENCE variable=tmp_j type=inter dependent=false


                    for (int itm = 0; itm < CU; itm++) {
                        #pragma HLS UNROLL

                        T Aki = tmp_i[itm];
                        T Akj = tmp_j[itm];       

                        T alpha_tmp = Aki*Aki;
                        T beta_tmp = Akj*Akj;
                        T gamma_tmp = Aki*Akj;

                        norm_data_type alpha_acc_tmp;
                        norm_data_type beta_acc_tmp;
                        norm_data_type gamma_acc_tmp;


                        norm_data_type alpha_tmp2 = (norm_data_type) alpha_tmp;
                        norm_data_type beta_tmp2 = (norm_data_type) beta_tmp;
                        norm_data_type gamma_tmp2 = (norm_data_type) gamma_tmp;

                        if (k < DEP){
                            alpha_acc_tmp = 0;
                            beta_acc_tmp = 0;
                            gamma_acc_tmp = 0;

                        }else{
                            alpha_acc_tmp = alpha_acc[i][itm][k % DEP];
                            beta_acc_tmp = beta_acc[i][itm][k % DEP];
                            gamma_acc_tmp = gamma_acc[i][itm][k % DEP];                                
                        }


                        alpha_acc[i][itm][k % DEP] = alpha_acc_tmp + alpha_tmp2;
                        beta_acc[i][itm][k % DEP] = beta_acc_tmp + beta_tmp2;
                        gamma_acc[i][itm][k % DEP] = gamma_acc_tmp + gamma_tmp2;

                    }

                }

            }
    }




template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void norm_computation_accumulate(

                     norm_data_type alpha_acc[IS][CU][DEP],
                     norm_data_type beta_acc[IS][CU][DEP],
                     norm_data_type gamma_acc[IS][CU][DEP],
                      int m, // column size
                      
                      hls::stream<T_outer>& alpha_strm,
                      hls::stream<T_outer>& beta_strm,
                      hls::stream<T_outer>& gamma_strm,
                      int pair_count,
                      int rj,
                      int indx,
                      int max_dep){
            
            

           // if (rj % IS == indx){
        norm_data_type alpha[IS];
        norm_data_type beta[IS];
        norm_data_type gamma[IS];

        T_outer alpha_out[IS];
        T_outer beta_out[IS];
        T_outer gamma_out[IS];


        norm_data_type alpha_sum[IS][16];
        norm_data_type beta_sum[IS][16];
        norm_data_type gamma_sum[IS][16];

        #pragma HLS ARRAY_PARTITION variable = alpha_sum complete dim=0
        #pragma HLS ARRAY_PARTITION variable = beta_sum complete dim=0
        #pragma HLS ARRAY_PARTITION variable = gamma_sum complete dim=0

           #pragma HLS ARRAY_PARTITION variable = alpha_sum complete dim=1
        #pragma HLS ARRAY_PARTITION variable = beta_sum complete dim=1
        #pragma HLS ARRAY_PARTITION variable = gamma_sum complete dim=1         


        CALC_TWO_ELE_SETS:
        for (int i =0; i < IS;i++){
            #pragma HLS unroll
        
            INIT_ACC:
            for (int t = 0; t < 16; t++) {
                //#pragma HLS PIPELINE II = 1 rewind
                #pragma HLS UNROLL
                #pragma HLS LOOP_TRIPCOUNT min = 8 max = 8

                alpha_sum[i][t] = 0;
                beta_sum[i][t] = 0;
                gamma_sum[i][t] = 0;
            }



            DEP_SUMMATION_OUTER:
            for (int k = 0; k < max_dep; k+=2) {
                #pragma HLS LOOP_TRIPCOUNT min = DEP/2 max = DEP/2
                #pragma HLS PIPELINE II = 2
                //#pragma HLS loop_flatten
                //#pragma HLS loop_merge force

                 DEP_SUMMATION_INNER_PARALLEL_1:
                for (int itm = 0; itm < CU; itm++) {
                    
                    #pragma HLS LOOP_TRIPCOUNT min = CU max = CU
                    
                    #pragma HLS UNROLL
                    alpha_sum[i][itm] += alpha_acc[i][itm][k % DEP];
                    beta_sum[i][itm] += beta_acc[i][itm][k % DEP];
                    gamma_sum[i][itm] += gamma_acc[i][itm][k % DEP];
                }


                DEP_SUMMATION_INNER_PARALLEL_2:
                for (int itm = 0; itm < CU; itm++) {
                    #pragma HLS LOOP_TRIPCOUNT min = CU max = CU
    
                    #pragma HLS UNROLL

                    alpha_sum[i][itm+CU] += alpha_acc[i][itm][k+1 % DEP];
                    beta_sum[i][itm+CU] += beta_acc[i][itm][k+1 % DEP];
                    gamma_sum[i][itm+CU] += gamma_acc[i][itm][k+1 % DEP];
                }
            }

            //pidx2++;

            // sum 16 data to 8
            
            norm_data_type alpha_sum_tmp0[IS][8];
            norm_data_type beta_sum_tmp0[IS][8];
            norm_data_type gamma_sum_tmp0[IS][8];

            #pragma HLS ARRAY_PARTITION variable = alpha_sum_tmp0 complete dim=0
            #pragma HLS ARRAY_PARTITION variable = beta_sum_tmp0 complete dim=0
            #pragma HLS ARRAY_PARTITION variable = gamma_sum_tmp0 complete dim=0
            DEP_TO_8_SUM:
            for (int k = 0; k < 8; k++) {
                //#pragma HLS PIPELINE rewind
                #pragma HLS UNROLL
                alpha_sum_tmp0[i][k] = alpha_sum[i][2 * k] + alpha_sum[i][2 * k + 1];
                beta_sum_tmp0[i][k] = beta_sum[i][2 * k] + beta_sum[i][2 * k + 1];
                gamma_sum_tmp0[i][k] = gamma_sum[i][2 * k] + gamma_sum[i][2 * k + 1];
            }

            // sum 8 data to 4
            norm_data_type alpha_sum_tmp1[IS][4];
            norm_data_type beta_sum_tmp1[IS][4];
            norm_data_type gamma_sum_tmp1[IS][4];

            #pragma HLS ARRAY_PARTITION variable = alpha_sum_tmp1 complete dim=0
            #pragma HLS ARRAY_PARTITION variable = beta_sum_tmp1 complete dim=0
            #pragma HLS ARRAY_PARTITION variable = gamma_sum_tmp1 complete dim=0
            

            for (int k = 0; k < 4; k++) {
                //#pragma HLS PIPELINE rewind
                #pragma HLS UNROLL
                alpha_sum_tmp1[i][k] = alpha_sum_tmp0[i][2 * k] + alpha_sum_tmp0[i][2 * k + 1];
                beta_sum_tmp1[i][k] = beta_sum_tmp0[i][2 * k] + beta_sum_tmp0[i][2 * k + 1];
                gamma_sum_tmp1[i][k] = gamma_sum_tmp0[i][2 * k] + gamma_sum_tmp0[i][2 * k + 1];
            }
            // sum 4 data to 2
            norm_data_type alpha_sum_tmp2[IS][2];
            norm_data_type beta_sum_tmp2[IS][2];
            norm_data_type gamma_sum_tmp2[IS][2];

            #pragma HLS ARRAY_PARTITION variable = alpha_sum_tmp2 complete dim=0
            #pragma HLS ARRAY_PARTITION variable = beta_sum_tmp2 complete dim=0
            #pragma HLS ARRAY_PARTITION variable = gamma_sum_tmp2 complete dim=0
            

            for (int k = 0; k < 2; k++) {
                //#pragma HLS PIPELINE rewind
                #pragma HLS UNROLL
                alpha_sum_tmp2[i][k] = alpha_sum_tmp1[i][2 * k] + alpha_sum_tmp1[i][2 * k + 1];
                beta_sum_tmp2[i][k] = beta_sum_tmp1[i][2 * k] + beta_sum_tmp1[i][2 * k + 1];
                gamma_sum_tmp2[i][k] = gamma_sum_tmp1[i][2 * k] + gamma_sum_tmp1[i][2 * k + 1];
            }
            // sum 2 data to 1

            alpha[i] = alpha_sum_tmp2[i][0]+ alpha_sum_tmp2[i][1];
            beta[i] = beta_sum_tmp2[i][0] + beta_sum_tmp2[i][1];
            gamma[i] = gamma_sum_tmp2[i][0]+ gamma_sum_tmp2[i][1];


            alpha_out[i] = (T_outer) alpha[i];
            beta_out[i] = (T_outer) beta[i];
            gamma_out[i] = (T_outer) gamma[i];

            alpha_strm.write(alpha_out[i]);
            beta_strm.write(beta_out[i]);
            gamma_strm.write(gamma_out[i]);

        }

    }



                      
//! Read two columns of A into two seperate Bram
template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void norm_computation(
                      hls::stream<fixed_vec>& A_pre_norm_i_float,
                      hls::stream<fixed_vec>& A_pre_norm_j_float,

                      int m, // column size
                      
                      hls::stream<T_outer>& alpha_strm,
                      hls::stream<T_outer>& beta_strm,
                      hls::stream<T_outer>& gamma_strm,
                      int pair_count,
                      int max_dep) {
    #pragma HLS inline off


    int half_pair = pair_count >> IS/2;


    LOOP_PAIRS_NORM:
    for (int rj = 0; rj < half_pair; rj++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS


        #pragma HLS dataflow


        
        hls::stream<T_outer> alpha_strm_local[IS];
        hls::stream<T_outer> beta_strm_local[IS];
        hls::stream<T_outer> gamma_strm_local[IS];

        #pragma HLS stream variable = alpha_strm_local depth=16
        #pragma HLS stream variable = beta_strm_local depth = 16
        #pragma HLS stream variable = gamma_strm_local depth = 16


        norm_data_type alpha_acc[IS][CU][DEP];
        #pragma HLS BIND_STORAGE variable = alpha_acc type = ram_2p impl = bram
        //#pragma HLS ARRAY_PARTITION variable = alpha_acc complete dim=1
        #pragma HLS ARRAY_PARTITION variable = alpha_acc complete dim=2
        norm_data_type beta_acc[IS][CU][DEP];
        #pragma HLS BIND_STORAGE variable = beta_acc type = ram_2p impl = bram
        //#pragma HLS ARRAY_PARTITION variable = beta_acc complete dim=1
        #pragma HLS ARRAY_PARTITION variable = beta_acc complete dim=2
        norm_data_type gamma_acc[IS][CU][DEP];
        #pragma HLS BIND_STORAGE variable = gamma_acc type = ram_2p impl = bram
        //#pragma HLS ARRAY_PARTITION variable = gamma_acc complete dim=1
        #pragma HLS ARRAY_PARTITION variable = gamma_acc complete dim=2
        #pragma HLS stream variable= alpha_acc type=pipo depth=2
        #pragma HLS stream variable= beta_acc type=pipo depth=2
        #pragma HLS stream variable= gamma_acc type=pipo depth=2



            norm_computation_multiply<T, T_outer, NRMAX, NCMAX, CU, ACUM>(
                                alpha_acc,beta_acc,gamma_acc,
                                A_pre_norm_i_float,
                                A_pre_norm_j_float,
                                m, // column size
                                pair_count,
                                rj);
            
            norm_computation_accumulate<T, T_outer, NRMAX, NCMAX, CU, ACUM>(
                                alpha_acc,beta_acc,gamma_acc,
                                m, // column size
                                alpha_strm,
                                beta_strm,
                                gamma_strm,
                                pair_count,
                                rj,
                                0,
                                max_dep);
        
    }

}
