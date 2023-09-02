
#include <bitset>

string print_fixed_vector(fp_vector in){
    std::bitset<32> bit_representation;
    ap_uint<32> in_uint;
    in_uint.range(32-1,0) = in.range(32-1,0);
    bit_representation =(uint) in_uint;
    return bit_representation.to_string();
}


string print_fixed_angle(fp_angle in){
    std::bitset<32> bit_representation;
    ap_uint<32> in_uint;
    in_uint.range(32-1,0) = in.range(32-1,0);
    bit_representation =(uint) in_uint;
    return bit_representation.to_string();
}



void init_tables_hybrid(fp_angle  ai[STAGES]){



    double pi = 3.1415926535897932384626;
    double ang;
    double fa;


     for (int j = 0; j<STAGES;j++){
        double ang = atan(pow(2,-j));

        // no normalization version
        //ai[j] = (fp_angle ) (ang);

        // normalized version
        ai[j] = (fp_angle ) (ang/pi);

        fa = (double) ang * 180.0 / pi;
    }   
}


void init_tables_pure(fp_angle  ai[WORD_SIZE]){



    double pi = 3.1415926535897932384626;
    double ang;
    double fa;


     for (int j = 0; j<WORD_SIZE;j++){
        double ang = atan(pow(2,-j));

        // no normalization version
        //ai[j] = (fp_angle ) (ang);

        // normalized version
        ai[j] = (fp_angle ) (ang/pi);

        fa = (double) ang * 180.0 / pi;

    }   
}


void hybrid_cordic(fp_vector xi, fp_vector yi,fp_angle zi, fp_vector &xo, fp_vector &yo )
{
    //#pragma HLS inline off

    double pi = 3.1415926535897932384626;
    fp_reduced_pi pi_fixed = (fp_reduced_pi) pi;
   

    bool sign[STAGES];

    fp_vector_cordic x[STAGES];
    fp_vector_cordic y[STAGES];
    fp_angle_cordic z[STAGES];




    // no normalization version
    //fp_angle deg45 = (fp_angle) (pi/4);
    //fp_angle deg90 = (fp_angle) (2*pi/4);
    //fp_angle deg135 = (fp_angle) (3*pi/4);
    
    // normalized version
    fp_angle deg45 = 0.25;
    fp_angle deg90 = 0.5;
    fp_angle deg135 = 0.75;



    fp_angle ai[STAGES];
    //fp_vector deg90[3];
    

    ap_uint<3> quad;



    init_tables_hybrid(ai);


    quad.range(2,0) = zi.range(WORD_SIZE-1,WORD_SIZE-3);
   
    switch(quad){

        case 0: // 0-0.25 CORRECT

            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic)  zi;
            break;

        case 1: // 0.25 - 0.5

            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic) zi;
            break;

        case 2: // 0.5 - 0.75 CORRECT

            x[0] = (fp_vector_cordic) -yi;
            y[0] = (fp_vector_cordic) xi;
            z[0] = (fp_angle_cordic) (zi - deg90);
            break;

        case 3: // 0.75 - 1

            x[0] = (fp_vector_cordic) -yi;
            y[0] = (fp_vector_cordic) xi;
            z[0] = (fp_angle_cordic)  (zi - deg90);
            break;

        case 4: // -0.75 - -1

            x[0] = (fp_vector_cordic) yi;
            y[0] = (fp_vector_cordic) -xi;
            z[0] = (fp_angle_cordic) (zi + deg90);
            break;

        case 5: // -0.5 - -0.75 CORRECT

            x[0] = (fp_vector_cordic) yi;
            y[0] = (fp_vector_cordic) -xi;
            z[0] = (fp_angle_cordic) (zi + deg90);
            break;

        case 6: // -0.25 - -0.5

            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic)  zi;
            break;

        case 7: // 0 - -0.25 CORRECT

            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic) zi;
            break;

        default:

            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic) zi;      
            break;          

    }

    ap_int<2> sgn = 1;

    cordic_loop:                                                                                                                                                            
    for (int i = 0; i < STAGES-1; i++){
        #pragma HLS LOOP_TRIPCOUNT min = STAGES max = STAGES
        #pragma HLS pipeline ii = 1 rewind
                

        if (z[i] >= 0)
        {
            sgn = 1;
        } else {
            sgn = -1;
        }
        

        x[i+1] = x[i] - sgn*(y[i] >> i);
        y[i+1] = y[i] + sgn*(x[i] >> i);
        z[i+1] = z[i] - sgn*ai[i];
    }





    //regular cordic out
    //xo = (fp_vector) x[STAGES-1];
    //yo = (fp_vector) y[STAGES-1];

    //reduced iteration cordic out

    fp_reduced_vector xor1 = (fp_reduced_vector) x[STAGES-1];
    fp_reduced_vector yor = (fp_reduced_vector) y[STAGES-1];
    fp_angle zor_pi = (fp_angle) (z[STAGES-1] *  pi_fixed);
    xo = (fp_vector) (x[STAGES-1] - (yor * zor_pi));
    yo = (fp_vector) (y[STAGES-1] + (xor1 * zor_pi));


}

void cordic(fp_vector xi, fp_vector yi,fp_angle zi, fp_vector &xo, fp_vector &yo )
{
    //#pragma HLS inline off


    double pi = 3.1415926535897932384626;
    fp_reduced_pi pi_fixed = (fp_reduced_pi) pi;
   

    bool sign[WORD_SIZE];

    fp_vector_cordic x[WORD_SIZE];
    fp_vector_cordic y[WORD_SIZE];
    fp_angle_cordic z[WORD_SIZE];




    // no normalization version
    //fp_angle deg45 = (fp_angle) (pi/4);
    //fp_angle deg90 = (fp_angle) (2*pi/4);
    //fp_angle deg135 = (fp_angle) (3*pi/4);
    
    // normalized version
    fp_angle deg45 = 0.25;
    fp_angle deg90 = 0.5;
    fp_angle deg135 = 0.75;


    fp_angle ai[WORD_SIZE];

    ap_uint<3> quad;


    init_tables_pure(ai);

    quad.range(2,0) = zi.range(WORD_SIZE-1,WORD_SIZE-3);

    switch(quad){

        case 0: // 0-0.25 CORRECT
            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic)  zi;
            break;

        case 1: // 0.25 - 0.5

            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            
            z[0] = (fp_angle_cordic) zi;
            
            break;

        case 2: // 0.5 - 0.75 CORRECT            
            x[0] = (fp_vector_cordic) -yi;
            y[0] = (fp_vector_cordic) xi;
            z[0] = (fp_angle_cordic) (zi - deg90);
            break;

        case 3: // 0.75 - 1
            x[0] = (fp_vector_cordic) -yi;
            y[0] = (fp_vector_cordic) xi;
            z[0] =(fp_angle_cordic)  (zi - deg90);
            break;

        case 4: // -0.75 - -1
            x[0] = (fp_vector_cordic) yi;
            y[0] = (fp_vector_cordic) -xi;
            z[0] = (fp_angle_cordic) (zi + deg90);

            break;

        case 5: // -0.5 - -0.75 CORRECT
            x[0] = (fp_vector_cordic) yi;
            y[0] = (fp_vector_cordic) -xi;
            z[0] = (fp_angle_cordic) (zi + deg90);
            break;

        case 6: // -0.25 - -0.5
            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] =(fp_angle_cordic)  zi;
            break;

        case 7: // 0 - -0.25 CORRECT
            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic) zi;
            break;

        default:
            x[0] = (fp_vector_cordic) xi;
            y[0] = (fp_vector_cordic) yi;
            z[0] = (fp_angle_cordic) zi;      
            break;          

    }


    ap_int<2> sgn = 1;


    cordic_loop:                                                                                                                                                            
    for (int i = 0; i < WORD_SIZE-1; i++){
        #pragma HLS LOOP_TRIPCOUNT min = WORD_SIZE max = WORD_SIZE
        #pragma HLS pipeline ii = 1 rewind
                

        if (z[i] >= 0)
        {
            sgn = 1;
        } else {
            sgn = -1;
        }

        x[i+1] = x[i] - sgn*(y[i] >> i);
        y[i+1] = y[i] + sgn*(x[i] >> i);
        z[i+1] = z[i] - sgn*ai[i];
    }


    xo = (fp_vector) (x[WORD_SIZE-1]);
    yo = (fp_vector) (y[WORD_SIZE-1]);
    

}



template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM>
void rotate_A(
    int pair_count,
    int n,
    int i, 
    int j, 
    hls::stream<fixed_vec>& A_post_norm_i,
    hls::stream<fixed_vec>& A_post_norm_j,
    hls::stream<fixed_vec>& A_post_rotate_i,
    hls::stream<fixed_vec>& A_post_rotate_j,
    hls::stream<T>& s_strm_a,
    hls::stream<T>& c_strm_a,
    int indx
    ) {


    //#pragma HLS inline off

    #pragma HLS DATAFLOW

    fp_vector xi_buf[CU];
    fp_vector yi_buf[CU];

    fp_vector xo_buf[CU];
    fp_vector yo_buf[CU];

    fp_angle zi_buf[CU];

    double gain = 0.6072529350088812561694;
    fp_vector gain_fixed = (fp_vector) gain;

    int accum = n / CU;



    LOOP_PAIRS_ROTATE_A:
    for (int rj = 0; rj < pair_count; rj++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS

        T s = s_strm_a.read();
        T c = c_strm_a.read();

        ROTATE_A:
        for (int k = 0; k < accum; k++) {
            #pragma HLS loop_flatten
            #pragma HLS PIPELINE II = 1 style=frp
            #pragma HLS LOOP_TRIPCOUNT min = ACUM max = ACUM





            fixed_vec tmp_i = A_post_norm_i.read();
            fixed_vec tmp_j = A_post_norm_j.read();

            fixed_vec tmp_i_rotated, tmp_j_rotated;

            for (int itm = 0; itm < CU; itm++) {
                #pragma HLS UNROLL

                    // IF DSP:
                    #ifdef _DSP_
                        T tki = tmp_i[itm];
                        T tkj = tmp_j[itm];
                        tmp_i_rotated[itm] = (T) (c * tki - s * tkj); // matA[k][i] = c*tki - s*matA[k][j]
                        tmp_j_rotated[itm] = (T) (s * tki + c * tkj);
                    #endif

                    // IF CORDIC:
                    #ifdef _CORDIC_
                        xi_buf[itm] = (fp_vector) tmp_i[itm];
                        yi_buf[itm] = (fp_vector) tmp_j[itm];
                        zi_buf[itm] = (fp_angle) s;
                        #ifdef _HYBRID_
                            hybrid_cordic(xi_buf[itm],yi_buf[itm], zi_buf[itm],xo_buf[itm],yo_buf[itm]);
                        #endif

                        #ifndef _HYBRID_
                            cordic(xi_buf[itm],yi_buf[itm], zi_buf[itm],xo_buf[itm],yo_buf[itm]);
                        #endif

                        tmp_i_rotated[itm] = (T) (xo_buf[itm]*gain_fixed);
                        tmp_j_rotated[itm] = (T) (yo_buf[itm]*gain_fixed);
                    #endif

            }

            A_post_rotate_i.write(tmp_i_rotated);
            A_post_rotate_j.write(tmp_j_rotated);
        }
    }
}

template <typename T, typename T_outer, int NCMAX, int CU, int ACUN>
void rotate_V(
    int pair_count,
    int n,
    int i, 
    int j, 
    hls::stream<fixed_vec>& V_pre_rotate_i,
    hls::stream<fixed_vec>& V_pre_rotate_j,
    hls::stream<fixed_vec>& V_post_rotate_i,
    hls::stream<fixed_vec>& V_post_rotate_j,
    hls::stream<T>& s_strm_v,
    hls::stream<T>& c_strm_v,
    int indx
    ) {



    #pragma HLS DATAFLOW
    //#pragma HLS inline off


    fp_vector xi_buf[CU];
    fp_vector yi_buf[CU];

    fp_vector xo_buf[CU];
    fp_vector yo_buf[CU];

    fp_angle zi_buf[CU];




    double gain = 0.6072529350088812561694;
    fp_vector gain_fixed = (fp_vector) gain;

    int accum = n/CU;

    LOOP_PAIRS_ROTATE_V:
    for (int rj = 0; rj < pair_count; rj++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS
        T s = s_strm_v.read();
        T c = c_strm_v.read();

        ROTATE_V:
        for (int k = 0; k < accum; k++) {
            #pragma HLS loop_flatten
            #pragma HLS PIPELINE II = 1 style=frp
            #pragma HLS LOOP_TRIPCOUNT min = ACUN max = ACUN



            fixed_vec tmp_i = V_pre_rotate_i.read();
            fixed_vec tmp_j = V_pre_rotate_j.read();


            fixed_vec tmp_i_rotated, tmp_j_rotated;


            for (int itn = 0; itn < CU; itn++) {
                #pragma HLS UNROLL
                    
                    #ifdef _DSP_
                        T tki = tmp_i[itn];
                        T tkj =  tmp_j[itn];
                        tmp_i_rotated[itn] =  (c * tki - s * tkj); 
                        tmp_j_rotated[itn] =  (s * tki + c * tkj); 
                    #endif

                    // IF CORDIC:
                    #ifdef _CORDIC_
                        xi_buf[itn] = (fp_vector) tmp_i[itn];
                        yi_buf[itn] = (fp_vector) tmp_j[itn];
                        zi_buf[itn] = (fp_angle) s;
                        //cout << (float) zi_buf[itn] << "\n";
                        #ifdef _HYBRID_
                            hybrid_cordic(xi_buf[itn],yi_buf[itn], zi_buf[itn],xo_buf[itn],yo_buf[itn]);
                        #endif

                        #ifndef _HYBRID_
                            cordic(xi_buf[itn],yi_buf[itn], zi_buf[itn],xo_buf[itn],yo_buf[itn]);
                        #endif
                        tmp_i_rotated[itn] = (T) (xo_buf[itn]*gain_fixed);
                        tmp_j_rotated[itn] = (T) (yo_buf[itn]*gain_fixed);
                    #endif
            }
            V_post_rotate_i.write(tmp_i_rotated);
            V_post_rotate_j.write(tmp_j_rotated);
        }
    }
}


template <typename T, typename T_outer, int NRMAX, int NCMAX, int CU, int ACUM, int ACUN>
void rotate_AV(
                hls::stream<fixed_vec>& A_pre_rotate_i_fixed,
                hls::stream<fixed_vec>& A_pre_rotate_j_fixed,

                hls::stream<fixed_vec>& A_post_rotate_i_float,
                hls::stream<fixed_vec>& A_post_rotate_j_float,

                hls::stream<fixed_vec>& V_pre_rotate_i_float,
                hls::stream<fixed_vec>& V_pre_rotate_j_float,

                hls::stream<fixed_vec>& V_post_rotate_i_float,
                hls::stream<fixed_vec>& V_post_rotate_j_float,                          
               int n,
               int round_count,
               int pair_count,
               hls::stream<T>& s_strm_a,
               hls::stream<T>& c_strm_a,
               hls::stream<T>& s_strm_v,
               hls::stream<T>& c_strm_v
               ) {


    #pragma HLS inline off
    #pragma HLS DATAFLOW

    int i,j,indx;

    rotate_A<T, T_outer, NRMAX, NCMAX, CU, ACUM>(pair_count,  n,i,j,A_pre_rotate_i_fixed,
    A_pre_rotate_j_fixed,A_post_rotate_i_float,A_post_rotate_j_float, s_strm_a, c_strm_a,indx);

    rotate_V<T, T_outer, NCMAX, CU, ACUN>(pair_count,         n,i,j,V_pre_rotate_i_float,
    V_pre_rotate_j_float,V_post_rotate_i_float,V_post_rotate_j_float, s_strm_v, c_strm_v,indx);  

}
