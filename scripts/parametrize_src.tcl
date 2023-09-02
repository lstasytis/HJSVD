# this script is used to update the source files in /src of the generated project folders for compile-time parametrization
# the SED routines should be used with the arguments provided to run_toolchain.tcl
# we do all this because it's just less of a mess than trying to properly pass each argument to the src via v++
# Author: Lukas Stasytis, lukas.stasytis@tu-darmstadt.de


# use sed 's/foo/bar/' for parameters
# sed -i 's/FIND_TEXT/REPLACE_WITH_TEXT/g' SOMEFILE



# array size
exec sed -i "s/.*const int SIZE_N.*/const int SIZE_N = $array_size_n;/" "$proj_dir/src/kernel_hls.hpp"
exec sed -i "s/.*const int SIZE_M.*/const int SIZE_M = $array_size_m;/" "$proj_dir/src/kernel_hls.hpp"

# data type of interface and the kernel internals (if interface is float, kernel is fixed, we expect to convert somewhere)
exec sed -i "s/.* interface_data_type;.*/typedef $interface_data_type interface_data_type;/" "$proj_dir/src/kernel_hls.hpp"


if {$kernel_data_type == "fixed"} {
	exec sed -i "s/.* kernel_data_type;.*/typedef ap_fixed<$word_width,2, AP_RND_CONV> kernel_data_type;/" "$proj_dir/src/kernel_hls.hpp"
} else {
	exec sed -i "s/.* kernel_data_type;.*/typedef $kernel_data_type kernel_data_type;/" "$proj_dir/src/kernel_hls.hpp"
}

if {$norm_data_type == "float" || $norm_data_type == "double"} {
	# float, nothing magical happens
    exec sed -i "s/.* norm_data_type;.*/typedef $norm_data_type norm_data_type;/" "$proj_dir/src/kernel_hls.hpp"
} else {
	# fixed point, we'll be padding Norm
	puts "PADDING NORM!"
	puts "s/.* norm_data_type;.*/typedef ap_fixed<$word_width+$norm_padding,$norm_padding, AP_RND_CONV> norm_data_type;/"
	exec sed -i "s/.* norm_data_type;.*/typedef ap_fixed<$word_width+$norm_padding,$norm_padding, AP_RND_CONV> norm_data_type;/" "$proj_dir/src/kernel_hls.hpp"
}



# primary configuration arguments for our HJSVD variant

exec sed -i "s/.*const int CU.*/const int CU = $cu_count;/" "$proj_dir/src/kernel_hls.hpp"

exec sed -i "s/.*const int DEP.*/const int DEP = $dep;/" "$proj_dir/src/kernel_hls.hpp"
exec sed -i "s/.*const int ACCUM_II.*/const int ACCUM_II = $accum_ii;/" "$proj_dir/src/kernel_hls.hpp"
exec sed -i "s/.*const int NUM_WORDS.*/const int NUM_WORDS = $num_words;/" "$proj_dir/src/kernel_hls.hpp"
exec sed -i "s/.*const int WORD_SIZE.*/const int WORD_SIZE = $word_width;/" "$proj_dir/src/kernel_hls.hpp"


# switching between dsp, classic cordic and hybrid cordic variants for the Rotate operation

if {$kernel_type == "cordic" || $kernel_type == "hybrid_cordic"} {
    exec sed -i "s/.*#define _CORDIC_.*/#define _CORDIC_/" "$proj_dir/src/kernel_hls.hpp"
    exec sed -i "s/.*#define _DSP_.*/\\/\\/#define _DSP_/" "$proj_dir/src/kernel_hls.hpp"

	if {$kernel_type == "hybrid_cordic"} {
		puts "Hybrid CORDIC kernel type"
		exec sed -i "s/.*#define _HYBRID_.*/#define _HYBRID_/" "$proj_dir/src/kernel_hls.hpp"
	} else {
		puts "Classic CORDIC kernel type"
		exec sed -i "s/.*#define _HYBRID_.*/\\/\\/#define _HYBRID_/" "$proj_dir/src/kernel_hls.hpp"
	}
} elseif {$kernel_type == "dsp"} {
    exec sed -i "s/.*#define _CORDIC_.*/\\/\\/#define _CORDIC_/" "$proj_dir/src/kernel_hls.hpp"
    exec sed -i "s/.*#define _DSP_.*/#define _DSP_/" "$proj_dir/src/kernel_hls.hpp"
} else {
	# default to dsps
    exec sed -i "s/.*#define _CORDIC_.*/\\/\\/#define _CORDIC_/" "$proj_dir/src/kernel_hls.hpp"
    exec sed -i "s/.*#define _DSP_.*/#define _DSP_/" "$proj_dir/src/kernel_hls.hpp"
}