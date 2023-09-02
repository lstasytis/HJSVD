# rebase so that all files we will now manipulate are relative in case this script is called from somewhere else
set script_path [ file dirname [ file normalize [ info script ] ] ]
cd $script_path

# execute some tests

# --need_gmp_override yes
# might be necessary if you have gmp.h installed in your system


#exec tclsh run_toolchain.tcl test --run_type csim >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl test --run_type synth >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl test --run_type sw_emu >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl test --run_type cosim >@ stdout 2>@ stderr 


#exec tclsh run_toolchain.tcl --run_type csim --kernel_name baseline_gesvj.hpp --kernel_data_type double --interface_data_type double >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl --run_type csim --kernel_name pipelined_gesvj.hpp --kernel_data_type double --interface_data_type double >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl --run_type csim --kernel_name pipelined_gesvj.hpp --kernel_data_type float --interface_data_type float >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl --run_type csim --kernel_name pipelined_gesvj.hpp --kernel_type cordic >@ stdout 2>@ stderr 
#exec tclsh run_toolchain.tcl --run_type csim --kernel_name pipelined_gesvj.hpp --kernel_type hybrid_cordic >@ stdout 2>@ stderr 





exec tclsh run_toolchain.tcl --proj_name reproduction_baseline_dsp_double --run_type synth --kernel_name baseline_gesvj.hpp --interface_data_type double --kernel_data_type double >@ stdout 2>@ stderr 

exec tclsh run_toolchain.tcl --proj_name reproduction_pipelined_float_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type float --kernel_data_type float --word_width 32 --kernel_type dsp --dep 4 >@ stdout 2>@ stderr 
exec tclsh run_toolchain.tcl --proj_name reproduction_pipelined_double_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type double --word_width 61 --kernel_type dsp --dep 16 >@ stdout 2>@ stderr 

exec tclsh run_toolchain.tcl --proj_name reproduction_pipelined_fixed32_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type float --kernel_data_type fixed --word_width 32 --kernel_type dsp --dep 4 >@ stdout 2>@ stderr 
exec tclsh run_toolchain.tcl --proj_name reproduction_pipelined_fixed61_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --word_width 61 --kernel_type dsp --dep 8 >@ stdout 2>@ stderr 

exec tclsh run_toolchain.tcl --proj_name reproduction_fixed32_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type float --kernel_data_type fixed --word_width 32 --kernel_type cordic --dep 4 >@ stdout 2>@ stderr 
exec tclsh run_toolchain.tcl --proj_name reproduction_fixed61_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --word_width 61 --kernel_type cordic --dep 8 >@ stdout 2>@ stderr 

exec tclsh run_toolchain.tcl --proj_name reproduction_fixed32_hybrid_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type float --kernel_data_type fixed --word_width 32 --kernel_type hybrid_cordic --dep 4 >@ stdout 2>@ stderr 
exec tclsh run_toolchain.tcl --proj_name reproduction_fixed61_hybrid_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --word_width 61 --kernel_type hybrid_cordic --dep 8 >@ stdout 2>@ stderr 