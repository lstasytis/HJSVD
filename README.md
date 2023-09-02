# Code Repository for reproducing the Optimization Techniques for Hestenes-Jacobi SVD on FPGAs by Lukas Stasytis and Zsolt István presented @ FPL2023

Requirements to run everything in this repo:

- Linux environment, tested with Ubuntu 20.04 and 20.06

- `tclsh` interpreter installed separately from the Vitis toolchain

- Vitis Unified Software Platform, tested on 2022.1 and 2023.1 (Paper results use 2022.1)

- Xilinx XRT, tested on 2022.1 and 2023.1

- Python 3 with `numpy`, `scikit` and `scipy` installed


Setup:

- source your Xilinx toolchain `settings64.sh`

- adjust `/scripts/default_configs.tcl` to match your device & part which your toolchain supports

- adjust `/aux/conn.cfg` if your device does not support HBM / you have different ports you want to connect to via axi master interfaces

- adjust `scripts/default_configs.tcl`, `scripts/run_xrt_toolchain.tcl`, `scripts/run_vitis_toolchain.tcl` `aux/Makefile` or `aux/utils.mk` if anything else is failing

- set stack size to 32k with `ulimit -s 32768` (only for sw_emu with xrt)

To reproduce the results from the paper, run the commands in `/scripts/paper_reproduction.sh`

Mind the clock frequencies, the repository does not split synth and place & route clock requests yet which makes 200 MHz requests not route to a full 200 MHz. The paper results were done with heuristically increased clock requests on synthesis till reaching 200 MHz, but this repository does not automate this yet.

The repository features an HLS project template I recently wrote and adapted the HJSVD kernel to so bugs are expected, please let me know if something breaks.
