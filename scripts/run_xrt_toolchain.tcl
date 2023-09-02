# XRT-targetting flow TCL file for RTL and C sw_emu, hw_emu and hw generation runs
# launch with:
# tclsh run_toolchain.tcl --toolchain xrt --run_type sw_emu
# for a default vitis run of c simulation of a passthrough kernel that just adds 1 to an array
# Author: Lukas Stasytis, lukas.stasytis@tu-darmstadt.de

#!/usr/bin/tclsh

set proj_name [lindex $argv 0]
set clock_freq [lindex $argv 1] 
set run_type [lindex $argv 2] 
set updated_sources [lindex $argv 3] 
set output_type [lindex $argv 4]
set proj_dir [lindex $argv 5]
set platform [lindex $argv 6]
set run_or_build [lindex $argv 7]
set interface_data_type [lindex $argv 8]
set host_args_xrt [lindex $argv 9]
set vpp_optimization_level [lindex $argv 10]

puts "proj_name: $proj_name" 
puts "clock_freq: $clock_freq"
puts "run_type: $run_type"
puts "updated_sources: $updated_sources"
puts "output_type: $output_type"
puts "proj_dir: $proj_dir"
puts "platform: $platform"
puts "run_or_build: $run_or_build"
puts "interface_data_type: $interface_data_type"
puts "host args: $host_args_xrt"
puts "vpp_optimization_level: $vpp_optimization_level"


cd ../generated/$proj_name/src

#set clock
# (it likely does not need to be set in 5 different places, but for safety sake :) )
set clock_hz "${clock_freq}000000"
set clock_hz_kernel "${clock_hz}:kernel_hls"
exec sed -i "s/.*VPP_FLAGS_kernel_hls += --hls.clock.*/VPP_FLAGS_kernel_hls += --hls.clock $clock_hz_kernel/" "$proj_dir/src/Makefile"
exec sed -i "s/.*VPP_LDFLAGS_kernel_hls += --clock.defaultFreqHz.*/VPP_LDFLAGS_kernel_hls += --clock.defaultFreqHz $clock_hz/" "$proj_dir/src/Makefile"
exec sed -i "s/.*VPP_LDFLAGS_kernel_hls += --kernel_frequency.*/VPP_LDFLAGS_kernel_hls += --kernel_frequency $clock_freq/" "$proj_dir/src/Makefile"

# set optimization level
exec sed -i "s/.*VPP_FLAGS_kernel_hls += --link --optimize.*/VPP_FLAGS_kernel_hls += --link --optimize $vpp_optimization_level/" "$proj_dir/src/Makefile"
exec sed -i "s/.*VPP_LDFLAGS_kernel_hls += --link --optimize.*/VPP_LDFLAGS_kernel_hls += --link --optimize $vpp_optimization_level/" "$proj_dir/src/Makefile"
# set blackboxes
exec sed -i "s/.*ADD_BB := .*/ADD_BB := no/" "$proj_dir/src/Makefile"

# edit sw/hw_emu/hw
puts "setting target"
exec sed -i "s/.*TARGET ?=.*/TARGET ?= $run_type/" "$proj_dir/src/Makefile"

# edit platform
exec sed -i "s/.*PLATFORM := xilinx_u55c_gen3x16_xdma_3_202210_1.*/PLATFORM := $platform/" "$proj_dir/src/Makefile"

#make all



if {$run_or_build != "run"} {

    set start_time [clock seconds]

    # building the kernel either for sw_emu, hw_emu or hw. In all cases an .xclbin is generated

    puts "starting make all for selected target between sw_emu hw_emu and hw"

    catch {exec make all | tee xrt_makefile_log.txt >@stdout} make_result

    puts "caught build errors: $make_result"
    # now depending on the run_type should fetch back the .xclbin file from the build_dir.sw_emu etc folder to outputs
        
    cd $proj_dir
    exec cp project/build_dir.$run_type.$platform/kernel_hls.xclbin  outputs
    exec cp project/build_dir.$run_type.$platform/kernel_hls.xclbin ../../latest_outputs

    # time the hw build time
    set final_build_time [clock seconds]

    set total_build_time_seconds [expr $final_build_time - $start_time]
    set total_build_time_minutes [expr $total_build_time_seconds / 60]
    set total_build_time_seconds_remaining [expr $total_build_time_seconds - $total_build_time_minutes*60]

    puts "build time: $total_build_time_minutes minutes and $total_build_time_seconds_remaining seconds"

    # store this build time for any potential analysis
    exec echo -e "$total_build_time_seconds" > $proj_dir/outputs/build_time.txt 
    exec echo -e "$total_build_time_seconds" > $proj_dir/../../latest_outputs/build_time.txt 



    # store resource report if hw run
    if {$run_type == "hw"} {
        exec cp $proj_dir/project/_x_temp.hw.xilinx_u55c_gen3x16_xdma_3/link/vivado/vpl/prj/prj.runs/impl_1/kernel_util_routed.rpt $proj_dir/outputs/hw_util.rpt 
        exec cp $proj_dir/project/_x_temp.hw.xilinx_u55c_gen3x16_xdma_3/link/vivado/vpl/prj/prj.runs/impl_1/_new_clk_freq $proj_dir/outputs/hw_timing.rpt 

        exec cp $proj_dir/project/_x_temp.hw.xilinx_u55c_gen3x16_xdma_3/link/vivado/vpl/prj/prj.runs/impl_1/kernel_util_routed.rpt $proj_dir/../../latest_outputs/hw_util.rpt 
        exec cp $proj_dir/project/_x_temp.hw.xilinx_u55c_gen3x16_xdma_3/link/vivado/vpl/prj/prj.runs/impl_1/_new_clk_freq $proj_dir/../../latest_outputs/hw_timing.rpt 

        exec cat $proj_dir/outputs/hw_util.rpt
        exec cat $proj_dir/outputs/hw_timing.rpt 
    }
}


if {$run_or_build != "build"} {

    set start_time [clock seconds]
    # run the host file versus the previously built kernel. If there is no kernel, inform

    cd $proj_dir

    if {[file exist $proj_dir/outputs/kernel_hls.xclbin]} {

        puts "Running host versus the built xclbin kernel file"
        # remember to set right xcl environment for the kernel type

        if {$run_type != "hw"} {
            # set the env variable for what type of emulation it is, if it's not a hardware run
            #puts [exec export XCL_EMULATION_MODE=$run_type]
            set env(XCL_EMULATION_MODE) $run_type
        }

        # execute the python host!
        # host have to be prepared in such a manner that simply linking the kernel xclbin should be enough
        # for any non-default configurations, additional scripting should be employed or just run the host.py from terminal
        puts "starting the python host script"
        catch {exec python3 $proj_dir/src/kernel_hls_tb.py -k $proj_dir/outputs/kernel_hls.xclbin -t $interface_data_type {*}$host_args_xrt  | tee xrt_host_run_log.txt >@stdout} run_result
        puts "caught run errors: $run_result"

        # time the host run
        set final_run_time [clock seconds]

        set total_run_time_seconds [expr $final_run_time - $start_time]
        set total_run_time_minutes [expr $total_run_time_seconds / 60]
        set total_run_time_seconds_remaining [expr $total_run_time_seconds - $total_run_time_minutes*60]

        puts "run time: $total_run_time_minutes minutes and $total_run_time_seconds_remaining seconds"

        # store this build time for any potential analysis
        exec echo -e "$total_run_time_seconds" > $proj_dir/outputs/run_time.txt 
        exec echo -e "$total_run_time_seconds" > $proj_dir/../../latest_outputs/run_time.txt 
        

    } else {
        puts "!!! NO KERNEL BUILT, CANNOT RUN HOST !!!"
    }

}

puts "successfully finished run_xrt_toolchain.tcl script"