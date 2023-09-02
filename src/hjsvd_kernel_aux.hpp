
template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM,int ACUN>
void generate_rr_indexes(
    int m,
    int n,
    hls::stream<index_pair>& ij_load_A_strm,
    hls::stream<index_pair>& ij_load_V_strm,

    hls::stream<index_pair>& ij_write_A_strm,
    hls::stream<index_pair>& ij_write_V_strm,

    int round_count,
    int pair_count,
    int rj_out)
{


    #pragma HLS DATAFLOW
    #pragma HLS inline off

    int i_indx, j_indx,i,j;
    int i_dividend, j_dividend,incrm;
    int m2 = m-1;
    incrm = 0;
    int rj = rj_out+1;

    index_pair load_a,write_a,load_v,write_v;

        LOOP_PAIRS:
        for (int ri = 0; ri < pair_count; ri++) {
            #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS

            // dynamic generation approach

            if (ri == 0) {
                i = 0;
            } else {
                i_dividend = ri-rj;
                if ((i_dividend >= 0) == 1){
                    i = ((i_dividend) % (m2)) + 1;
                } else{
                    i = ((i_dividend) % (m2)) + 1 + m2;
                }
                
            }


            j_dividend = ri+rj-1;
            if ((j_dividend >= 0) == 1){
                j = (m - 1 - (j_dividend) % (m2));
            } else {
                j = (m - 1 - (j_dividend) % (m2))+m2;
            }

            load_a[0] = i;
            load_a[1] = j;

            load_v[0] = i;
            load_v[1] = j;

            write_a[0] = i;
            write_a[1] = j;

            write_v[0] = i;
            write_v[1] = j;
            
            ij_load_A_strm.write(load_a);
            ij_load_V_strm.write(load_v);
            
            ij_write_A_strm.write(write_a);
            ij_write_V_strm.write(write_v);
        }
}




template <typename T, typename T_outer>
void conv_tester(int round_count, int pair_count, hls::stream<T_outer>& conv_strm,T_outer &conv_out)
{

    T_outer conv_buffer = 0.0;
    T_outer conv_temp;

    LOOP_PAIRS_TEST_COVARIANCE:
    for (int ri = 0; ri < pair_count; ri++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS
        #pragma HLS PIPELINE II=PIPE_II

         conv_temp = conv_strm.read();

        conv_buffer = hls::max(conv_buffer,conv_temp);

    }
    conv_out = conv_buffer;

}



template <typename T, typename T_outer,int ACUM,int CU>
void fixed_to_float(
    int round_count,
    int pair_count, 
    hls::stream<T>& alpha_strm, 
    hls::stream<T>& beta_strm, 
    hls::stream<T>& gamma_strm,

    hls::stream<T_outer>& alpha_strm_float, 
    hls::stream<T_outer>& beta_strm_float, 
    hls::stream<T_outer>& gamma_strm_float)
    {

    
    int incr = 0;

    T alpha_tmp,beta_tmp,gamma_tmp;
    T_outer alpha_tmp1,beta_tmp1,gamma_tmp1;

    LOOP_PAIRS:
    for (int ri = 0; ri < pair_count; ri++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS
        

        alpha_tmp = alpha_strm.read();
        beta_tmp = beta_strm.read();
        gamma_tmp = gamma_strm.read();


        alpha_tmp1 = (T_outer) alpha_tmp;
        beta_tmp1 = (T_outer) beta_tmp;
        gamma_tmp1 = (T_outer) gamma_tmp;


        alpha_strm_float.write(alpha_tmp1);
        beta_strm_float.write(beta_tmp1);
        gamma_strm_float.write(gamma_tmp1);
    }
}

template <typename T, typename T_outer, int NCMAX>
void convert_array_float_to_fixed(
    T_outer* A,
    int n,
    int m
    )
    {

        #pragma HLS DATAFLOW

        T_outer col_buf[NCMAX];
        T col_buf_fixed[NCMAX];


        for (int i = 0; i < n; i++){

            // read a col
            for (int j = 0; j < m; j++){
                col_buf[j] = A[i*m+j];
            }
            for (int j = 0; j < m; j++){
                col_buf_fixed[j] = (T) col_buf[j];
            }

            for (int j = 0; j < m; j++){
                col_buf[j].range(31,0) = (T) col_buf[j].range(31,0);
            }

            for (int j = 0; j < m; j++){
                A[i*m+j] = col_buf[j];
            }

        }
    }

template <typename T, typename T_outer,int NCMAX>
void convert_array_fixed_to_float(
    T_outer* A,
    int n,
    int m
    )
    {

    #pragma HLS DATAFLOW

    T_outer col_buf[NCMAX];
    T col_buf_fixed[NCMAX];


    for (int i = 0; i < n; i++){

        // read a col
        for (int j = 0; j < m; j++){
            col_buf[j] = A[i*m+j];
        }

        for (int j = 0; j < m; j++){
            col_buf_fixed[j].range(31,0) = col_buf[j].range(31,0);
        }

        for (int j = 0; j < m; j++){
            col_buf[j] = (T) col_buf_fixed[j];
        }

        for (int j = 0; j < m; j++){
            A[i*m+j] = col_buf[j];
        }

    }
}


template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU,int ACUM>
void float_to_fixed_A(
    int round_count,
    int pair_count, 
    int n,
    hls::stream<float_vec>& float_vec_strm,
    hls::stream<fixed_vec>& fixed_vec_norm_strm,
    hls::stream<fixed_vec>& fixed_vec_rotate_strm

    )
    {

    #pragma HLS DATAFLOW
    float_vec tmp_float;
    fixed_vec tmp_fixed;
    

    LOOP_PAIRS:
    for (int ri = 0; ri < pair_count; ri++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS
        
        CONVERT_FLOAT_TO_FIXED:
        for (int k = 0; k < ACUM; k++) {
            #pragma HLS PIPELINE II = 1
            #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX

            tmp_float = float_vec_strm.read();

            for (int itm = 0; itm < CU; itm++) {
                #pragma HLS UNROLL

                if (k * CU + itm < n) {
                    tmp_fixed[itm] = (T) tmp_float[itm];
                }
            }

            fixed_vec_norm_strm.write(tmp_fixed);
            fixed_vec_rotate_strm.write(tmp_fixed);
        }
    }
}

template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU,int ACUM>
void float_to_fixed_V(
    int round_count,
    int pair_count, 
    int n,
    hls::stream<float_vec>& float_vec_strm,
    hls::stream<fixed_vec>& fixed_vec_strm

    )
    {

    #pragma HLS DATAFLOW
    float_vec tmp_float;
    fixed_vec tmp_fixed;
    

    LOOP_PAIRS:
    for (int ri = 0; ri < pair_count; ri++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS
        
        CONVERT_FLOAT_TO_FIXED:
        for (int k = 0; k < ACUM; k++) {
            #pragma HLS PIPELINE II = 1
            #pragma HLS LOOP_TRIPCOUNT min = NCMAX max = NCMAX

            tmp_float = float_vec_strm.read();

            for (int itm = 0; itm < CU; itm++) {
                #pragma HLS UNROLL

                if (k * CU + itm < n) {
                    tmp_fixed[itm] = (T) tmp_float[itm];
                }
            }

            fixed_vec_strm.write(tmp_fixed);
        }
    }
}