

union double_casting {
    double d;
    uint64_t i;
};

union float_casting {
    float d;
    uint32_t i;
};

//! calc the converge of next sweep
template <typename T, typename T_outer>
void calc_converge(T_outer alpha, T_outer beta, T_outer gamma, hls::stream<T_outer>& conv_strm) {

    T_outer converge = xf::solver::internal::m::abs(gamma) / xf::solver::internal::m::sqrt(alpha * beta); // compute convergence
    
    conv_strm.write(converge);
}

template <typename T, typename T_outer>
void angle_computation(

    int round_count,
    int pair_count,
    hls::stream<T_outer>& alpha_strm_float, 
    hls::stream<T_outer>&  beta_strm_float, 
    hls::stream<T_outer>&  gamma_strm_float, 
    hls::stream<T_outer>& conv_strm, 
    hls::stream<T>& s_strm_a, 
    hls::stream<T>& c_strm_a, 
    hls::stream<T>& s_strm_v, 
    hls::stream<T>& c_strm_v
    ) {

    #pragma HLS inline off


    LOOP_PAIRS_ANGLE:
    for (int rj = 0; rj < pair_count; rj++) {

        #pragma HLS LOOP_TRIPCOUNT min = MAX_PAIRS max = MAX_PAIRS
        

        // It's extremely critical that this is set as frp (free running pipeline)
        // HLS 2022.1 doesn't understand to immediately forward results otherwise
        #pragma HLS PIPELINE II=PIPE_II style=frp

        int tmp1,tmp2,tmp3;

        T_outer alpha = alpha_strm_float.read();
        T_outer beta = beta_strm_float.read();
        T_outer gamma = gamma_strm_float.read();


        T_outer c_right = 1.0;
        T_outer s_right = 0.0;

        T s_right_fixed;
        T c_right_fixed;


        const T pi_inv = 0.3183098861837907;


        T_outer alpha_beta;
        T_outer tmp_ratio;


        T_outer epsilon = 1.e-8;
        #ifdef __SOLVER_DEBUG__
            std::cout << " checking gamma > ep" << endl;
        #endif
        alpha_beta = alpha * beta;

        tmp_ratio = xf::solver::internal::m::abs(gamma) / (alpha_beta);
        T_outer converge = xf::solver::internal::m::abs(gamma) / xf::solver::internal::m::sqrt(alpha * beta); // compute convergence

        if (tmp_ratio > epsilon){
            #ifdef __SOLVER_DEBUG__
                        std::cout << " gamma > epsilon " << endl;
            #endif

            double m00, m01, m11;
            m00 = beta;
            m01 = gamma;
            m11 = alpha;
            double d;
            #pragma HLS BIND_OP variable = d op = dsub impl = fabric
            d = m00 - m11; // calculate the off-diagonal value
            ap_uint<11> exp1;
            ap_uint<52> sig1;
            union double_casting dc;
            // calculate deno = 2*m01
            dc.d = m01;
            ap_uint<64> data = dc.i;
            exp1(10, 0) = data(62, 52);
            exp1 = exp1 + ap_uint<11>(1);
            data(62, 52) = exp1(10, 0);
            dc.i = data;
            double deno = dc.d;
            ///////////////////////////
            // calculate KK = 2*abs(m00 - m11)
            dc.d = d;
            data = dc.i;
            exp1(10, 0) = data(62, 52);
            exp1 = exp1 + ap_uint<11>(1);
            data(62, 52) = exp1(10, 0);
            data[63] = 0;
            dc.i = data;
            double KK = dc.d;
            ///////////////////////////

            double deno2, d2;
            #pragma HLS BIND_OP variable = d2 op = dmul impl = maxdsp
            #pragma HLS BIND_OP variable = deno2 op = dmul impl = maxdsp
            d2 = d * d;          // d2 = (m00 - m11)^2
            deno2 = deno * deno; // deno2 = 4*(m01)^2
            double m;
            #pragma HLS BIND_OP variable = m op = dadd impl = fabric
            m = deno2 + d2;                                  // m = (m00 - m11)^2 + 4*(m01)^2
            double sqrtM = xf::solver::internal::m::sqrt(m); // sqrtM = sqrt((m00-m11)^2 + 4*(m01)^2)
            //////////////////
            // calculate M2
            dc.d = m;
            data = dc.i;
            exp1(10, 0) = data(62, 52);
            exp1 = exp1 + ap_uint<11>(1);
            data(62, 52) = exp1(10, 0);
            dc.i = data;
            double M2 = dc.d; // M2 = 2*m
            ////////////////////////////////////
            double tmpMul, tmpSum, tmpSub;
            #pragma HLS BIND_OP variable = tmpMul op = dmul impl = maxdsp
            tmpMul = KK * sqrtM; // tmpMul = 2*abs(m00 - m11) * sqrt((m00-m11)^2 + 4*(m01)^2)
            #pragma HLS BIND_OP variable = tmpSum op = dadd impl = fabric
            tmpSum = tmpMul + M2;
            double tmpDivider = deno2 / tmpSum;
            #pragma HLS BIND_OP variable = tmpSub op = dsub impl = fabric
            tmpSub = 1 - tmpDivider;
            c_right = xf::solver::internal::m::sqrt(tmpSub);
            double tmp = xf::solver::internal::m::sqrt(tmpDivider);
            s_right = (((d > 0) && (deno > 0)) | ((d < 0) && (deno < 0))) ? tmp : -tmp;

            //cout <<"atan,s2,c2: "<< ang_tan << ", " << s_right2 << ", " <<c_right2 << endl;







        }
        s_right_fixed = (T) s_right;
        c_right_fixed = (T) c_right;


    // additional arcsin to extract back theta, since we only need theta and cordic handles sin(t) & cos(t) internally.
    // there is an op overhead here that could be avoided with more clever math, we are reaching sin in a roundabout manner skipping theta, when we could
    // somehow theta directly. Out of scope for the HJSVD paper

        #ifdef _CORDIC_
            s_right_fixed = (T) hls::asin(s_right); // expensive OP!
            s_right_fixed = (T) (s_right_fixed * pi_inv);
        #endif

        s_strm_a.write(s_right_fixed);
        c_strm_a.write(c_right_fixed);

        s_strm_v.write(s_right_fixed);
        c_strm_v.write(c_right_fixed);
    
        conv_strm.write(converge);

    }


}
