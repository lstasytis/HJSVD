
//! Read two columns (i and j) of A into two seperate Bram A_i[N] and A_j[N]
template <typename T, typename T_outer, int MCMAX, int CU, int ACUM>
void read_2cols_local_double_size(
    T_outer* A,
    T matA[MCMAX][CU*2][ACUM/2],
    hls::stream<fixed_vec>& A_pre_norm_i,
    hls::stream<fixed_vec>& A_pre_norm_j,
    
    int n,                     
    hls::stream<index_pair>& ij_load_A_strm,
    int round_count,
    int pair_count,
    int accum) 
{

    LOOP_PAIRS_READ_A:
    for (int rj = 0; rj < pair_count; rj++) {
        //#pragma HLS loop_flatten
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS

        index_pair write_a = ij_load_A_strm.read(); 
        int i = write_a[0];
        int j = write_a[1];

        LOOP_PAIRS_read_ACCUM:
        for (int k = 0; k < accum/2; k++) {
            //#pragma HLS loop_flatten
            #pragma HLS PIPELINE II = 2 style=frp
            #pragma HLS LOOP_TRIPCOUNT min = ACUM/2 max = ACUM/2
            fixed_vec tmp_i_1,tmp_i_2, tmp_j_1,tmp_j_2;


            LOOP_PAIRS_read_PARALELL_I:
            for (int itn = 0; itn < CU; itn++) {
                #pragma HLS UNROLL
                tmp_i_1[itn] = matA[i][itn][k];
                tmp_j_1[itn] = matA[j][itn][k];
            }

            A_pre_norm_i.write(tmp_i_1);
            A_pre_norm_j.write(tmp_j_1);


            LOOP_PAIRS_read_PARALELL_J:
            for (int itn = CU; itn < CU*2; itn++) {
                #pragma HLS UNROLL
                tmp_i_2[itn-CU] = matA[i][itn][k];
                tmp_j_2[itn-CU] = matA[j][itn][k];
            }

            A_pre_norm_i.write(tmp_i_2);
            A_pre_norm_j.write(tmp_j_2);

        }
    }
}


//! Write two columns (i and j) of A into two seperate Bram A_i[N] and A_j[N]
template <typename T, typename T_outer, int MCMAX, int CU, int ACUM>
void write_2cols_local_double_size(
    T_outer* A,
    T matA[MCMAX][CU*2][ACUM/2],
    hls::stream<fixed_vec>& A_post_rotate_i,
    hls::stream<fixed_vec>& A_post_rotate_j,
    
    
    int n,                     
    hls::stream<index_pair>& ij_write_A_strm,
    int round_count,
    int pair_count,
    int accum) 
{
    //#pragma HLS inline off
    #pragma HLS DATAFLOW


    LOOP_PAIRS_write:
    for (int rj = 0; rj < pair_count; rj++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS

        index_pair write_a = ij_write_A_strm.read();
        int i = write_a[0];
        int j = write_a[1];

        LOOP_PAIRS_write_ACCUM:
        for (int k = 0; k < accum/2; k++) {
           // #pragma HLS loop_flatten
            #pragma HLS PIPELINE II = 2 style=frp
            
            #pragma HLS LOOP_TRIPCOUNT min = ACUM/2 max = ACUM/2


            fixed_vec tmp_i_1, tmp_i_2, tmp_j_1,tmp_j_2;


            tmp_i_1 = A_post_rotate_i.read();
            tmp_j_1 = A_post_rotate_j.read();

            LOOP_PAIRS_write_PARALELL_I:
            for (int itn = 0; itn < CU; itn++) {
                #pragma HLS UNROLL
                matA[i][itn][k] = tmp_i_1[itn];
                matA[j][itn][k] = tmp_j_1[itn];
                
            }
            
            
            tmp_i_2 = A_post_rotate_i.read();
            tmp_j_2 = A_post_rotate_j.read();

            LOOP_PAIRS_write_PARALELL_J:
            for (int itn = CU; itn < CU*2; itn++) {
                #pragma HLS UNROLL
                matA[i][itn][k] = tmp_i_2[itn-CU];
                matA[j][itn][k] = tmp_j_2[itn-CU];
                
            }
        }
    }
}

