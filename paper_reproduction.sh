# run all these from HJSVD/ root folder
# synthesizes each of the designs mentioned in the paper, synth reports are found in /generated/<proj_name>/outputs

tclsh scripts/run_toolchain.tcl --proj_name reproduction_baseline_dsp_double --run_type synth --kernel_name baseline_gesvj.hpp --interface_data_type double --kernel_data_type double --norm_data_type double

tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_float_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type float --norm_data_type float --word_width 32 --kernel_type dsp --dep 4
tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_double_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type double --norm_data_type double --word_width 61 --kernel_type dsp --dep 16

tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_fixed32_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type dsp --dep 4
tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_fixed61_dsp --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type dsp --dep 8

tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_fixed32_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type cordic --dep 4
tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_fixed61_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type cordic --dep 8

tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_fixed32_hybrid_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type hybrid_cordic --dep 4
tclsh scripts/run_toolchain.tcl --proj_name reproduction_pipelined_fixed61_hybrid_cordic --run_type synth --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type hybrid_cordic --dep 8


# sw_emu each of the designs mentioned in the paper to check correctness with a 16x16 matrix size input

tclsh scripts/run_toolchain.tcl --proj_name sw_emu_baseline_dsp_double --run_type sw_emu ==data_size 16 --kernel_name baseline_gesvj.hpp --interface_data_type double --kernel_data_type double --norm_data_type double

tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_float_dsp --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type float --norm_data_type float --word_width 32 --kernel_type dsp --dep 4
tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_double_dsp --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type double --norm_data_type double --word_width 61 --kernel_type dsp --dep 16

tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_fixed32_dsp --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type dsp --dep 4
tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_fixed61_dsp --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type dsp --dep 8

tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_fixed32_cordic --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type cordic --dep 4
tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_fixed61_cordic --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type cordic --dep 8

tclsh scripts/run_toolchain.tcl --proj_name sw_emu_pipelined_fixed32_hybrid_cordic --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type hybrid_cordic --dep 4
tclsh scripts/run_toolchain.tcl --proj_name sw_em_pipelinedu_fixed61_hybrid_cordic --run_type sw_emu ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type hybrid_cordic --dep 8


# generate the hardware of each of the designs mentioned in the paper to check final resource utilization and throughputs.

tclsh scripts/run_toolchain.tcl --proj_name hw_baseline_dsp_double --run_type hw ==data_size 16 --kernel_name baseline_gesvj.hpp --interface_data_type double --kernel_data_type double --norm_data_type double --run_or_build build

tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_float_dsp --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type float --norm_data_type float --word_width 32 --kernel_type dsp --dep 4 --run_or_build build
tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_double_dsp --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type double --norm_data_type double --word_width 61 --kernel_type dsp --dep 16 --run_or_build build

tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_fixed32_dsp --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type dsp --dep 4 --run_or_build build
tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_fixed61_dsp --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type dsp --dep 8 --run_or_build build

tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_fixed32_cordic --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type cordic --dep 4 --run_or_build build
tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_fixed61_cordic --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type cordic --dep 8 --run_or_build build

tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_fixed32_hybrid_cordic --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 32 --kernel_type hybrid_cordic --dep 4 --run_or_build build
tclsh scripts/run_toolchain.tcl --proj_name hw_pipelined_fixed61_hybrid_cordic --run_type hw ==data_size 16 --kernel_name pipelined_gesvj.hpp --interface_data_type double --kernel_data_type fixed --norm_data_type fixed --word_width 61 --kernel_type hybrid_cordic --dep 8 --run_or_build build


# to test the generated hardware, we run the host with a parameter set to exhaustively test all matrix sizes up to 512, outputting throughputs used in the throughput figure of the paper as well as bit-accuracy to confirm correct performance

python3 src/kernel_hls_tb.py -k generated/hw_baseline_dsp_double/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0

python3 src/kernel_hls_tb.py -k generated/hw_pipelined_float_dsp/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0
python3 src/kernel_hls_tb.py -k generated/hw_pipelined_double_dsp/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0

python3 src/kernel_hls_tb.py -k generated/hw_pipelined_fixed32_dsp/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0
python3 src/kernel_hls_tb.py -k generated/hw_pipelined_fixed61_dsp/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0

python3 src/kernel_hls_tb.py -k generated/hw_pipelined_fixed32_cordic/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0
python3 src/kernel_hls_tb.py -k generated/hw_pipelined_fixed61_cordic/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0

python3 src/kernel_hls_tb.py -k generated/hw_pipelined_fixed32_hybrid_cordic/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0
python3 src/kernel_hls_tb.py -k generated/hw_pipelined_fixed61_hybrid_cordic/outputs/kernel_hls.xclbin  --max_data_size 512 --data_size 512 --kernel_count 1 -t double -v -o 1 -r 1 --full_test_cap 512 --use_software_hjsvd 0
