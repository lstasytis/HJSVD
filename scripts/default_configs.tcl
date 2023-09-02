
# this script features the default experiment configuration
# for each arg you wish to change, the tcl call should have a -- prefix, ex: --run_type cosim
# for host arguments, prefix with == instead, ex: ==data_size 16
# Author: Lukas Stasytis, lukas.stasytis@tu-darmstadt.de


# ==============================================================================================
# csim|synth|cosim|export|sw_emu|hw_emu|hw   (csim,synth, cosim,safe_cosim, export all use )
set run_type "csim"
# ==============================================================================================
# if to build, execute or both for the kernel (build,run,both)
set run_or_build "both"
# ==============================================================================================
# name of the project
set proj_name "template_project"
# ==============================================================================================
# name of top kernel function source file to control what is imported to the wrapped kernel
set kernel_name "kernel_passthrough.hpp"
# ==============================================================================================
# yes|no  (if yes, resets the entire proj directory, otherwise only copies over /src and /aux)
set proj_reset "yes"
# ==============================================================================================
# fpga part to use ( xcu55c-fsvh2892-2L-e for U55C)
set device_part "xcu55c-fsvh2892-2L-e"  
# ==============================================================================================
# fpga platform name to use ( xilinx_u55c_gen3x16_xdma_3 for U55C)
set platform "xilinx_u55c_gen3x16_xdma_3"
# ==============================================================================================
# this argument can be used to enable some dynamic /src replacement inbetween xsim compilations
# and launches if wishing to avoid full resynthesis in some scenarios
# has to be implemented by the user in run_vitis_toolchain.tcl
# host|kernel_hls|verilog
set updated_sources "none"
# ==============================================================================================
# clock_freq in MHz
set clock_freq "200"
# ==============================================================================================
# clock uncertainty to more aggressively synthesize for the target clock
#(at least half of the clock_freq period)
set clock_uncertainty "2.5"
# ==============================================================================================
# array_size N                      (4-512)
set array_size_n "512"
# ==============================================================================================
# array_size M                      (4-512)
set array_size_m "512"
# ==============================================================================================
# number of compute units. 1 CU = 1 Norm and 2 Rotate element-pairs computed / cycle
set cu_count "8";
# ==============================================================================================
# interface_data_type             (ap_fixed<32,2>, float, double, etc)
set interface_data_type "double"
# ==============================================================================================
# kernel_data_type                (ap_fixed<32,2>, float, double, etc)
set kernel_data_type "double"
# ==============================================================================================
# word width for fixed-point, must be defined separately for fixed point kernel data types
# 61 for double precision equivalent due to DSPs not fitting properly, 32 for single precision
set word_width "61";
# ==============================================================================================
# data type for the norm step, if using fixed, define the pre-padded version if using fixed,
# additional padding is handled at compile-time
set norm_data_type "double";
# ==============================================================================================
# type of Rotate kernel between dps, cordic and hybrid_cordic
set kernel_type "dsp";
# ==============================================================================================
# do we fetch vcd type waveforms from xsim?
# vcd|none
set output_type "vcd"
# ==============================================================================================
# these arguments are passed as is at the end of the host file invocation
# be very careful with whitespaces, they split names and values
# https://stackoverflow.com/questions/28468367/tcl-eval-and-exec-confusing-point has 
# explanations to change it to allow whitespaces if needed

# arguments can be modified for both arg lists from the master tcl call with a 
# == prefix. For example ==data_size 8 will update both --data_size and -data_size values to 8

# xrt args are prefixed with --
set host_args_xrt "--max_data_size $array_size_n --data_size $array_size_n --kernel_count 1 -t $interface_data_type -v -o 1 -r 1 --full_test_cap 0 --use_software_hjsvd 1"

# vitis-only host arguments are prefixed with -
set host_args_vitis "-seed 42 -data_size $array_size_n -t $interface_data_type"
# ==============================================================================================

# this is a very specific switch for solving a bug in systems where gmp has been installed,
# overriding vitis' version
# you basically need to #include <gmp.h> in all your sources. This flag does precisely that
# with a #DEFINE switch
# yes | no
set import_gmp "no"
# ==============================================================================================
# runtime in nanoseconds for the hardware simulations using xsim. either integer
# or "all" without quotations
set sim_runtime "all"
# ==============================================================================================
# depth of the accumulator array, this is a function of the latency of the multiplication
# we dont run this math dynamically
# double = 16, float = 6, fixed61 = 8, fixed32=4
set dep "16";
# ==============================================================================================
# II for the reduction first loop, we go with 6 and let the tools lower or
# increase it depending on the other configs
set accum_ii "6";
# ==============================================================================================
# how many words we fit into a single axi stream between the kernels. This should be a function
# of cu_count, but we go with 8 for a 512bit axi stream packet when using double precision and
# adjust acordingly. AKA: DON'T TOUCH
set num_words "8";
# ==============================================================================================
# how many bits to pad the norm. This should be log2(m)+3 for very consistent results
set norm_padding "11";
# ==============================================================================================
# optimization level when performing place and route and such, 3 for high effort, 0 = default
set vpp_optimization_level "0"
# ==============================================================================================