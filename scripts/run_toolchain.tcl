# Master TCL file for RTL and C sim, synth, cosim, .xo export etc
# launch with tclsh run_toolchain.tcl
# Author: Lukas Stasytis, lukas.stasytis@tu-darmstadt.de


puts "-------------------------------"
puts "Starting run_toolchain.tcl flow"
puts "In case of argument errors, refer to 'explanations.txt'"
puts "-------------------------------"


if {[lindex $argv 0] == "test"} {
    puts "Using backup configs due to a full script test request"
    source backup_configs.tcl
} else {
    # rebase so that all files we will now manipulate are relative in case this script is called from somewhere else
    set script_path [ file dirname [ file normalize [ info script ] ] ]
    cd $script_path
    source default_configs.tcl
}





set start_time [clock seconds]


# start of compile_time argument setting

# the following script takes input arguments that are prefixed with "--" and sets the following argument to this previous argument
# for example "tclsh run_toolchain.tcl --toolchain vitis" will run the toolchain with the "toolchain" argument overriden from the default to vitis

set count 0
set src_prefix "--"
set host_prefix "=="



set host_args_xrt [split $host_args_xrt " "]
set host_args_vitis [split $host_args_vitis " "]

foreach argValue $argv {
    if {[string match $src_prefix* $argValue]} {
        set arg [string trimleft $argValue $src_prefix]
        set new_arg [lindex $argv [expr $count + 1]]
        puts "default src argument $arg is being overriden to $new_arg"
        set $arg $new_arg
    }
   set count [expr $count + 1]
}

set count 0
foreach argValue $argv {

    if {[string match $host_prefix* $argValue]} {
        set arg [string trimleft $argValue $host_prefix]
        set new_arg [lindex $argv [expr $count + 1]]
        puts "default host argument $arg is being overriden to $new_arg"
        set old_arg_xrt " --$arg"
        set old_arg_vitis " -$arg"

        set host_count 0
        foreach e $host_args_xrt {
            if {[string match *$e* $old_arg_xrt]} {
                puts "updating $old_arg_xrt in xrt arg list with new value $new_arg"
                # now we replace the value
                set i [expr $host_count +1]
                lset host_args_xrt $i $new_arg
                #set $host_args_xrt [lreplace $host_args_xrt $i $i $new_arg]
            }
            set host_count [expr $host_count +1]
        }

        set host_count 0
        foreach e $host_args_vitis {
            if {[string match *$e* $old_arg_vitis]} {
                puts "updating $old_arg_vitis in vitis arg list with new value $new_arg"
                # now we replace the value
                set i [expr $host_count +1]
                lset host_args_vitis $i $new_arg
                #set $host_args_xrt [lreplace $host_args_xrt $i $i $new_arg]
            }
            set host_count [expr $host_count +1]
        }
    }
   set count [expr $count + 1]
}

puts "total argument count: $count"

# end of compile_time argument setting


# fetch main directory
cd ../
set dir [exec pwd]
cd scripts


# create an identifier string for reproducing the project afterwards

set systemTime [clock seconds]
set current_date "Date: [clock format $systemTime -format %D]:"
set current_time "[clock format $systemTime -format %H:%M:%S]"
set arguments [lindex $argv] 

set identifier "Command:\ntclsh run_toolchain.tcl $arguments \nTime launched: $current_date $current_time"


# print out all the inputs for sanity checks by the user

puts "project name: $proj_name"
puts "working directory: $dir"


if {$run_type == "csim" || $run_type == "synth" || $run_type == "export" || $run_type == "xo" || $run_type == "cosim" || $run_type == "safe_cosim"} {
    set toolchain "vitis"
    if {$run_type == "cosim" || $run_type == "safe_cosim"} {
        set sim_type $run_type
    } else {
        set sim_type "none"
    }

} elseif {$run_type == "sw_emu" || $run_type == "hw_emu" || $run_type == "hw"} {
    set toolchain "xrt"
}



puts "toolchain: $toolchain"

if {$proj_reset == "yes"} {
    puts "Resetting the project before compilation"
} else {
    puts "Not resetting the project before compilation"
}


puts "type of run: $run_type"
puts "sources being updated: $updated_sources" 
puts "-------------------------------"
puts "clock frequency: $clock_freq"
puts "-------------------------------"
puts "array M size: $array_size_m"
puts "array N size: $array_size_n"
puts "interface data type: $interface_data_type"
puts "kernel data type: $kernel_data_type"
puts "-------------------------------"
puts "kernel type: $kernel_name"
puts "-------------------------------"
puts "output type to fetch: $output_type"
puts "-------------------------------"
puts "vitis host input args: $host_args_vitis"
puts "-------------------------------"
puts "xrt host input args: $host_args_xrt"
puts "-------------------------------"


# generate project directory if requesting a reset or if it doesnt exist yet

# however, if running the hosts only, by force disallow resets, since that would make no sense
if {$run_or_build == "run"} {
    set proj_reset "no"
}


puts "-------------------------------"
puts "handling project folder creation"
puts "-------------------------------"

set proj_dir "$dir/generated/$proj_name"


set latest_output_dir "$dir/latest_outputs"

if {[file exist $latest_output_dir]} {
    puts "found latest_outputs folder"

} else {
    file mkdir $latest_output_dir
}

if {[file exist $proj_dir]} {
    puts "found proj folder"

    # check that the project folder is not a file

    if {! [file isdirectory $proj_dir]} {
        puts "$proj_dir exists, but it's a file, something is wrong!"
    } else {
        puts "project folder $proj_dir already exists, checking if to reset it"

        if {$proj_reset == "yes"} {
            puts "resetting the project folder entirely"
            # delete previous project folder and create a fresh one
            exec rm -rf $proj_dir
            file mkdir $proj_dir

            # create new folders for the project
            file mkdir "$proj_dir/src"
            file mkdir "$proj_dir/reports"
            file mkdir "$proj_dir/outputs"
            file mkdir "$proj_dir/project"


            # move over sources
            exec cp -r -a "$dir/src/." "$dir/generated/$proj_name/src"
            exec cp -r -a "$dir/aux/." "$dir/generated/$proj_name/src"
            puts "replaced src"


        } else {
            puts "no reset of project folder taking place"
        }
    }
} else {
    file mkdir $proj_dir
    puts "$proj_dir project directory is being created"
    # create new folders for the project
    file mkdir "$proj_dir/src"
    file mkdir "$proj_dir/reports"
    file mkdir "$proj_dir/outputs"
    file mkdir "$proj_dir/project"


    # move over sources
    exec cp -r -a "$dir/src/." "$dir/generated/$proj_name/src"
    exec cp -r -a "$dir/aux/." "$dir/generated/$proj_name/src"
    puts "replaced src"
}

exec echo $identifier > "$proj_dir/identifier.txt"




# sed routines for simulation timings
# vitis-only version which hijacks xsim
exec sed -i "s/.*run.*/run $sim_runtime/" "$proj_dir/src/kernel_hls_vcd.tcl"
# xrt version for xrt.ini pre_sim script
exec sed -i "s/.*run.*/run $sim_runtime/" "$proj_dir/src/vcd_override_pre_sim_script.tcl"


# switch which kernel.cpp file is used as the top
exec sed -i "s/.*#include.*/#include \"$kernel_name\"/" "$proj_dir/src/kernels.hpp"

# switch to enable gmp imports for systems with gmp installed overriding vitis' version
if {$import_gmp == "yes"} {
    puts "INCLUDING GMP"
    exec sed -i "s/.*\\/\\/#define \_INCLUDE\_GMP\_.*/#define \_INCLUDE\_GMP\_/" "$proj_dir/src/kernel_hls.hpp"    
}

puts "-------------------------------"
puts "running SED routines to replace parameters in /src files"
source parametrize_src.tcl



# ===================================================================================
puts "-------------------------------"
puts "Starting vitis toolchain"
puts "-------------------------------"


if {$toolchain == "vitis"} {
    puts "running non-xrt flow"
    exec vitis_hls -f run_vitis_toolchain.tcl $proj_name $clock_freq $run_type $updated_sources $output_type $proj_dir $device_part $run_or_build $interface_data_type $clock_uncertainty "\"$host_args_vitis\"" >@ stdout 2>@ stderr 
} elseif {$toolchain == "xrt"} {
    puts "running xrt flow"

    exec tclsh run_xrt_toolchain.tcl $proj_name $clock_freq $run_type $updated_sources $output_type $proj_dir $platform $run_or_build $interface_data_type $host_args_xrt $vpp_optimization_level >@ stdout 2>@ stderr 
}

# time the toolchain full runtime
set final_time [clock seconds]

set total_time_seconds [expr $final_time - $start_time]
set total_time_minutes [expr $total_time_seconds / 60]
set total_time_seconds [expr $total_time_seconds - $total_time_minutes*60]

puts "Toolchain call finished in $total_time_minutes minutes and $total_time_seconds seconds! Exiting."


exit