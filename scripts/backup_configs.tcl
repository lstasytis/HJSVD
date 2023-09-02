
# this script features the default experiment configuration to keep as a fallback in case of 
# erronous changes. the test_all.tcl should call this config to exhaust all the main run_types
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


# this argument can be used to enable some dynamic /src replacement inbetween xsim compilations
# and launches if wishing to avoid full resynthesis in some scenarios
# has to be implemented by the user in run_vitis_toolchain.tcl
# host|kernel_hls|verilog
set updated_sources "none"
# ==============================================================================================
# clock_freq in MHz
set clock_freq "100"
# ==============================================================================================
# array_size N                      (4-512)
set array_size_n "16"
# ==============================================================================================
# array_size M                      (4-512)
set array_size_m "16"
# ==============================================================================================
# interface_data_type             (ap_fixed<32,2>, float, double, etc)
set interface_data_type "float"
# ==============================================================================================
# kernel_data_type                (ap_fixed<32,2>, float, double, etc)
set kernel_data_type "float"
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
set host_args_xrt "--cu_count 1 --data_size 16 -t $interface_data_type"

# vitis-only host arguments are prefixed with -
set host_args_vitis "-seed 42 -data_size 16 -t $interface_data_type"

# ==============================================================================================

# this is a very specific switch for solving a bug in systems where gmp has been installed,
# overriding vitis' version
# you basically need to #include <gmp.h> in all your sources. This flag does precisely that
# with a #DEFINE switch
set import_gmp "no"

# ==============================================================================================
# runtime in nanoseconds for the hardware simulations using xsim. either integer
# or "all" without quotations
set sim_runtime "all"
# ==============================================================================================
# fpga part to use ( xcu55c-fsvh2892-2L-e for U55C)
set device_part "xcu55c-fsvh2892-2L-e"  
# ==============================================================================================
# fpga platform name to use ( xilinx_u55c_gen3x16_xdma_3 for U55C)
set platform "xilinx_u55c_gen3x16_xdma_3"
# ==============================================================================================






