
#include "kernels.hpp"
#include "kernel_hls.hpp"


extern "C" 
void kernel_hls(
    int size_n, 
    int size_m, 
    interface_data_type* A, 
    interface_data_type* U,
    interface_data_type* S,
    interface_data_type* V
    ) {

    /*
    This is the header function for the kernels to maintain
    a consistent name for the top top kernel and simplify the makefiles

    Edit the function parameters as fit, but also edit them in kernel_hls.cpp

    We also perform any axi interface configurations between the kernel and IO here
    */


    #pragma HLS INTERFACE m_axi port = A bundle = gmem0  \
        max_widen_bitwidth = INTERFACE_BITWIDTH  num_read_outstanding = OUTSTANDING_READ_COUNT depth=MAX_M*MAX_N



    #pragma HLS INTERFACE m_axi port = U bundle = gmem0  \
        max_widen_bitwidth = INTERFACE_BITWIDTH num_write_outstanding = OUTSTANDING_WRITE_COUNT depth=MAX_M*MAX_M

    #pragma HLS INTERFACE m_axi port = S bundle = gmem0  \
        max_widen_bitwidth = INTERFACE_BITWIDTH num_write_outstanding = OUTSTANDING_WRITE_COUNT depth=MAX_N

    #pragma HLS INTERFACE m_axi port = V bundle = gmem0  \
        max_widen_bitwidth = INTERFACE_BITWIDTH num_write_outstanding = OUTSTANDING_WRITE_COUNT depth=MAX_N*MAX_N

    #pragma HLS INTERFACE s_axilite port = size_n bundle = control
    #pragma HLS INTERFACE s_axilite port = size_m bundle = control

    #pragma HLS INTERFACE s_axilite port = A bundle = control
    #pragma HLS INTERFACE s_axilite port = U bundle = control
    #pragma HLS INTERFACE s_axilite port = S bundle = control
    #pragma HLS INTERFACE s_axilite port = V bundle = control
    #pragma HLS INTERFACE s_axilite port = return bundle = control


    // kernel to call. Make sure to only important one such function inside kernels.hpp
    //kernel_top<kernel_data_type,interface_data_type, MAX_N, MAX_M>(m, n, X, Y);


    kernel_top<interface_data_type, kernel_data_type, MAXM, MAXN, CU>(size_m, size_n, A,U,S,V);

}
