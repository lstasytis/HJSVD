# Vitis-only flow TCL file for RTL and C sim, synth, cosim, .xo export etc
# launch with:
# tclsh run_toolchain.tcl --toolchain vitis --run_type csim
# for a default vitis run of c simulation of a passthrough kernel that just adds 1 to an array
# Author: Lukas Stasytis, lukas.stasytis@tu-darmstadt.de


set proj_name [lindex $argv 2]
set clock_freq [lindex $argv 3] 
set run_type [lindex $argv 4] 
set updated_sources [lindex $argv 5] 

set output_type [lindex $argv 6]
set proj_dir [lindex $argv 7]
set device_part [lindex $argv 8]
set run_or_build [lindex $argv 9]
set interface_data_type [lindex $argv 10]
set clock_uncertainty [lindex $argv 11]
set host_args_vitis [lindex $argv 12]



puts "proj_name: $proj_name" 
puts "clock_freq: $clock_freq"
puts "clock uncertainty: $clock_uncertainty"
puts "run_type: $run_type"
puts "updated_sources: $updated_sources"
puts "output_type: $output_type"
puts "proj_dir: $proj_dir"
puts "device_part: $device_part"
puts "run_or_build: $run_or_build"
puts "interface_data_type: $interface_data_type"
puts "host args: $host_args_vitis"




cd ../generated/$proj_name/project


# create or open the project and set a top kernel function
open_project kernel_project
open_solution kernel_solution -flow_target vitis
set_top kernel_hls

# add sources
add_files ../src/kernel_hls.cpp
add_files -tb ../src/kernel_hls_tb.cpp

# set device
set_part  $device_part

#set clock
set clock_freq_mhz "${clock_freq}MHz"
create_clock -period $clock_freq_mhz
set_clock_uncertainty $clock_uncertainty


# dataflow to warning mode for sanity
config_dataflow -strict_mode warning


# prepare sim folder path for later xsim hijacking to get out vcd dumps
set sim_folder ./kernel_project/kernel_solution/sim/verilog

# standard flags for c++ testbench
set LDFLAGS "--verbose --std=c++0x"

# add own host arguments. \" char had to be passed so vitis_hls doesn't scream but now we remove it, since the host won't like it
set tb_args $host_args_vitis
set tb_args [string map {\" ""} $tb_args]

# a flag to notify at the end of the script if to fetch back a synthesis report
set generated_hardware 0

# run csim, csynth, export_design depending on script arguments


if {$run_or_build != "run"} {

    set start_time [clock seconds]

    if {$run_type == "csim"} {
        csim_design -argv $tb_args
    } elseif {$run_type == "synth" || $run_type == "cosim"} {
        csynth_design
        set generated_hardware 1

    } elseif {$run_type == "export"} {
        csynth_design
        set generated_hardware 1
        export_design -flow impl
    } elseif {$run_type == "xo"} {
        config_export -format xo -ipname kernel_hls_0
        csynth_design
        set generated_hardware 1
        export_design
    } else {
        puts "Not running csim/csynth or export, assuming file replacements with updated_sources"
    }

    # time the build runtime
    set final_build_time [clock seconds]

    set total_build_time_seconds [expr $final_build_time - $start_time]
    set total_build_time_minutes [expr $total_build_time_seconds / 60]
    set total_build_time_seconds_remaining [expr $total_build_time_seconds - $total_build_time_minutes*60]

    puts "build time: $total_build_time_minutes minutes and $total_build_time_seconds_remaining seconds"

    # store this build time for any potential analysis
    exec echo -e "$total_build_time_seconds" > $proj_dir/outputs/build_time.txt 
    exec echo -e "$total_build_time_seconds" > $proj_dir/../../latest_outputs/build_time.txt 
}




if {$run_or_build != "build"} {

    set start_time [clock seconds]

    # run xelab compilation in preparation for any modifications before running actual xsim
    if {$run_type == "cosim"} {
        cosim_design -ldflags $LDFLAGS -O -tool xsim -rtl verilog -coverage -trace_level all -argv $tb_args -setup
    }


    # if replacing any verilog or c files, notify here
    if {$updated_sources == "host"} {
        puts "Replacing host files before launching xsim (PLACEHOLDER)"

    } elseif {$updated_sources == "kernel"} {
        puts "Replacing kernel files before launching xsim (PLACEHOLDER)"
    } elseif {$updated_sources == "verilog"} {
        puts "Replacing verilog files before launching xsim (PLACEHOLDER)"
    } else {
        puts "Undefined replacement of src files inbetween xsim compilation and execution"
    }

    if {$run_type == "cosim"} {

        puts "starting safe or updated cosim"

        cd $sim_folder
        puts "executing xelab compilation"

        # extract the xsim and xelab commands out of run_xsim.sh
        set fp [open "run_xsim.sh" r]
        set file_data [read $fp]
        set data [split $file_data "\n"]
        close $fp

        # we are splitting the xsim and xelab steps to separate sh files so we can do work inbetween
        set xelab_script [lindex $data  1]
        set filename "own_xelab.sh"
        set fileId [open $filename "w"]
        puts -nonewline $fileId $xelab_script
        close $fileId        

        set xsim_script [lindex $data  2]
        set filename "own_xsim.sh"
        set fileId [open $filename "w"]
        puts -nonewline $fileId $xsim_script
        close $fileId     

        # launch xelab
        set xelab_log [exec sh own_xelab.sh | tee xelab_log.txt >@stdout]


        # =============================================
        # OUR WORK TO INJECT CHANGES WITHOUT RECOMPILING
        # =============================================


        # replacing with our vcd dump request and setting custom simulation time
        exec sed -i "s/.*quit.*//" "kernel_hls.tcl"
        exec sed -i "s/.*run all.*//" "kernel_hls.tcl"
        exec cat $proj_dir/src/kernel_hls_vcd.tcl >> kernel_hls.tcl

        # =============================================
        # OUR WORK TO INJECT CHANGES WITHOUT RECOMPILING
        # =============================================

        # launch xsim
        set xsim_log [exec sh own_xsim.sh | tee xsim_log.txt >@stdout]
        
        # we're done, return to project root
        cd $proj_dir

        # if we are generating a vcd dump, should fetch it back to a more shallow folder out of the simulation folder
        if {$output_type == "vcd"} {
            puts "fetching back vcd from the following project:"
            puts [exec pwd]
            exec cp project/$sim_folder/dump.vcd  outputs
            exec cp project/$sim_folder/dump.vcd ../../latest_outputs
        }
    }


    # time the final host runtime
    set final_run_time [clock seconds]

    set total_run_time_seconds [expr $final_run_time - $start_time]
    set total_run_time_minutes [expr $total_run_time_seconds / 60]
    set total_run_time_seconds_remaining [expr $total_run_time_seconds - $total_run_time_minutes*60]

    puts "run time: $total_run_time_minutes minutes and $total_run_time_seconds_remaining seconds"

    # store this build time for any potential analysis
    exec echo -e "$total_run_time_seconds" > $proj_dir/outputs/run_time.txt 
    exec echo -e "$total_run_time_seconds" > $proj_dir/../../latest_outputs/run_time.txt 

}



cd $proj_dir
# if we ran a csynth at some point, should fetch back the csynth report to a more shallow folder

if {$generated_hardware == 1} {
    cd $proj_dir
    puts "fetching back csynth.rpt"
    exec cp project/kernel_project/kernel_solution/syn/report/csynth.rpt  outputs
    exec cp project/kernel_project/kernel_solution/syn/report/csynth.rpt ../../latest_outputs

    # lets also capture the kernel total resource cost post-synthesis to output to cmd
    set kernel_line_start "PS: '+' for module; 'o' for loop; '*' for dataflow"

    #get the file lines
    set fp [open "outputs/csynth.rpt" r]
    set file_data [read $fp]
    set data [split $file_data "\n"]
    close $fp

    #get index of line where the resource report starts

    set count 0
    set true_line_index 0
    foreach line $data {
        # find the line where resource consumptions are shown
        if {[string first $kernel_line_start $line] != -1} {
            set true_line_index [expr $count +1]
        }
        set count [expr $count +1]
    }
    
    #output it all to terminal
    puts [lindex $data  $true_line_index]
    puts [lindex $data  [expr $true_line_index +1]]
    puts [lindex $data  [expr $true_line_index +2]]
    puts [lindex $data  [expr $true_line_index +3]]
    puts [lindex $data  [expr $true_line_index +4]]
    puts [lindex $data  $true_line_index]

} else {
    puts "no csynth.rpt to fetch back"
}

# vitis run generates a log file in /scripts/ which we should move out as well
if {$run_or_build != "run"} {
    exec cp ../../scripts/vitis_hls.log outputs/vitis_hls_BUILD.log
    exec mv ../../scripts/vitis_hls.log ../../latest_outputs/vitis_hls_BUILD.log
} else {
    exec cp ../../scripts/vitis_hls.log outputs/vitis_hls_RUN.log
    exec mv ../../scripts/vitis_hls.log ../../latest_outputs/vitis_hls_RUN.log    
}
puts "successfully finished run_vitis_toolchain.tcl script"

exit