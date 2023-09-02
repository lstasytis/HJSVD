# HJSVD_dev

Requirements to run everything in this repo:


-- Linux environment, tested with Ubuntu 20.04 and 20.06

-- tclsh interpreter installed

-- Vitis, Vitis_HLS, Vivado, tested on 2022.1 and 2023.1 - this is enough to run csim, cosim, synth experiments to see resource utilization numbers and functional correctness

-- Xilinx XRT, tested on 2022.1 and 2023.1 - this is needed if testing HJSVD precision and throughput with an actual FPGA device

-- Python 3 with numpy, scikit and scipy installed



Setup:


-- source your Xilinx toolchain settings64.sh

-- adjust /scripts/default_configs.tcl to match your device / part that your toolchain supports

-- adjust /aux/conn.cfg if you dont have hbm / have different ports you want to hook to via axi master interfaces

-- adjust aux/Makefile and aux/utils.mk if anything else is failing. This repo is a *very* heavily adapted Vitis_Accel_Examples & Vitis_Libraries project so the xrt front is very similar to example projects

-- set stack size to 32k with "ulimit -s 32768"


To reproduce the results from the paper, run the commands in /scripts/paper_reproduction.sh

The repository features an HLS project template I recently wrote and adapted the HJSVD kernel to so bugs are expected, please let me know if something breaks.
