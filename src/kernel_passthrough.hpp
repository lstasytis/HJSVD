
#include "kernel_hls.hpp"


#include <iomanip>
#include <hls_math.h>
#include "hw/math_helper.hpp"


using namespace std;


template <typename T,typename T_outer, int MAX_N, int MAX_M>
void read(T_outer* X,hls::stream<T_outer>& in_stream, int m, int n)
{
    READ_OUTER:
    for (int i = 0; i < m; i++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_M max = MAX_M

        READ_INNER:
        for (int j = 0; j < n; j++) {
            #pragma HLS LOOP_TRIPCOUNT min = MAX_N max = MAX_N
            #pragma HLS PIPELINE II = 1
            T_outer tmp =  X[i * n + j];
            in_stream.write(tmp);

            #ifndef __SYNTHESIS__
            #ifdef __DEBUGGING__
                std::cout << (float) tmp << " ";
            #endif
            #endif
            
        }
        #ifndef __SYNTHESIS__
        #ifdef __DEBUGGING__
            std::cout << std::endl;
        #endif
        #endif
    }
}

template <typename T_inner,typename T_outer, int MAX_N, int MAX_M>
void write(T_outer* Y,hls::stream<T_outer>& out_stream, int m, int n)
{
    WRITE_OUTER:
    for (int i = 0; i < m; i++) {
        #pragma HLS LOOP_TRIPCOUNT min = MAX_M max = MAX_M

        WRITE_INNER:
        for (int j = 0; j < n; j++) {
            #pragma HLS LOOP_TRIPCOUNT min = MAX_N max = MAX_N
            #pragma HLS PIPELINE II = 1
            T_outer tmp =  out_stream.read();
            Y[i * n + j] = tmp;
            
            #ifndef __SYNTHESIS__
            #ifdef __DEBUGGING__
                std::cout << (float) tmp << " ";
            #endif
            #endif
            
        }
        #ifndef __SYNTHESIS__
        #ifdef __DEBUGGING__
            std::cout << std::endl;
        #endif
        #endif
    }
}

template <typename T_inner,typename T_outer, int MAX_N, int MAX_M>
void compute(hls::stream<interface_data_type>& in_stream, hls::stream<interface_data_type>& out_stream, int m, int n)
{
    COMPUTE_OUTER:
    for (int i = 0; i < m; i++) {
    #pragma HLS LOOP_TRIPCOUNT min = MAX_M max = MAX_M

        COMPUTE_INNER:
        for (int j = 0; j < n; j++) {
            #pragma HLS LOOP_TRIPCOUNT min = MAX_N max = MAX_N
            #pragma HLS PIPELINE II = 1

            // read stream, convert dtype, do operation, write back

            T_outer tmp_outer_in =  in_stream.read();
            T_inner tmp = (T_inner) tmp_outer_in;
            tmp += 1;
            T_outer tmp_outer_out = (T_outer) tmp;
            out_stream.write(tmp_outer_out);

            
            #ifndef __SYNTHESIS__
            #ifdef __DEBUGGING__
                std::cout << (float) tmp << " ";
            #endif
            #endif
            
        }
        #ifndef __SYNTHESIS__
        #ifdef __DEBUGGING__
            std::cout << std::endl;
        #endif
        #endif
    }
}


template <typename T_inner,typename T_outer, int MAX_N, int MAX_M>
void kernel_top(
    int m, 
    int n, 
    interface_data_type* X, 
    interface_data_type* Y
    ){

    // read X in read(), stream to compute(), add 1, write back to Y in write()

    hls::stream<T_outer> in_stream;
    #pragma HLS STREAM variable = in_stream depth = PARAM_FIFO_DEPTH

    hls::stream<T_outer> out_stream;
    #pragma HLS STREAM variable = out_stream depth = PARAM_FIFO_DEPTH

    #pragma HLS DATAFLOW
    read<T_inner, T_outer,MAX_N, MAX_M>(X,in_stream,m ,n);
    compute<T_inner, T_outer,MAX_N, MAX_M>(in_stream,out_stream,m ,n);
    write<T_inner, T_outer,MAX_N, MAX_M>(Y,out_stream,m ,n);

}