
#include "kernel_hls.hpp"



#include <string.h>
#include <sys/time.h>
#include <algorithm>
#include "ext/MatrixGen/matrixUtility.hpp"

// xrt-specific includes
#include "xcl2.hpp"
#include "xf_utils_sw/logger.hpp"
// xrt-specific includes



// Memory alignment
template <typename T>
T* aligned_alloc(std::size_t num) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 8192, num * sizeof(T))) {
        throw std::bad_alloc();
    }
    return reinterpret_cast<T*>(ptr);
}


// Memory alignment
template <int>
int* aligned_alloc(std::size_t num) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 8192, num * sizeof(int))) {
        throw std::bad_alloc();
    }
    return reinterpret_cast<int*>(ptr);
}

// Compute time difference
unsigned long diff(const struct timeval* newTime, const struct timeval* oldTime) {
    return (newTime->tv_sec - oldTime->tv_sec) * 1000000 + (newTime->tv_usec - oldTime->tv_usec);
}

// Arguments parser
class ArgParser {
   public:
    ArgParser(int& argc, const char** argv) {
        for (int i = 1; i < argc; ++i) mTokens.push_back(std::string(argv[i]));
    }
    bool getCmdOption(const std::string option, std::string& value) const {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
        if (itr != this->mTokens.end() && ++itr != this->mTokens.end()) {
            value = *itr;
            return true;
        }
        return false;
    }

   private:
    std::vector<std::string> mTokens;
};

//! Core function of SVD benchmark
int main(int argc, const char* argv[]) {
    // Initialize parser
    ArgParser parser(argc, argv);

    // Initialize paths addresses
    std::string xclbin_path;
    std::string num_str;
    int size_m, size_n, seed, num_runs;


    // parse CMD arguments

    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "INFO:input path is not set!\n";
    }

    if (!parser.getCmdOption("-seed", num_str)) {
        seed = 42;
        std::cout << "INFO:seed is not set!\n";
    } else {
        seed = std::stoi(num_str);
    }

    if (!parser.getCmdOption("-num_runs", num_str)) {
        num_runs = 42;
        std::cout << "INFO:number of runs are not set!\n";
    } else {
        num_runs = std::stoi(num_str);
    }

    if (!parser.getCmdOption("-data_size_m", num_str)) {
        size_m = MAX_M;
        std::cout << "INFO:row size M is not set!\n";
    } else {
        size_m = std::stoi(num_str);
    }

    if (!parser.getCmdOption("-data_size_n", num_str)) {
        size_n = MAX_N;
        std::cout << "INFO:column size N is not set!\n";
    } else {
        size_n = std::stoi(num_str);
    }

    if (!parser.getCmdOption("-data_size", num_str)) {
    
        std::cout << "INFO:square matrix size is not set!\n";
    } else {
        // override everything with this new value
        size_n = std::stoi(num_str);
        size_m = std::stoi(num_str);
    }

    // Output the inputs information
    std::cout << "INFO: Matrix Row M: " << size_m << std::endl;
    std::cout << "INFO: Matrix Col N: " << size_n << std::endl;
    std::cout << "INFO: seed: " << seed << std::endl;
    std::cout << "INFO: num_runs: " << num_runs << std::endl;


    // initialize arrays
    interface_data_type* X;
    interface_data_type* Y_hw;
    X = aligned_alloc<interface_data_type>(size_m*size_n);
    Y_hw = aligned_alloc<interface_data_type>(size_m*size_n);

    interface_data_type* Y_sw;
    Y_sw = new interface_data_type[size_m * size_n];


    matGen<interface_data_type>(size_m, size_n, seed, X);


    
    // !!!
    // XRT LAUNCH CODE
    // !!!




    // Platform related operations
    xf::common::utils_sw::Logger logger;
    cl_int xrt_err = CL_SUCCESS;

    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Creating Context and Command Queue for selected Device
    cl::Context context(device, NULL, NULL, NULL, &xrt_err);
    logger.logCreateContext(xrt_err);

    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &xrt_err);
    logger.logCreateCommandQueue(xrt_err);

    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    printf("INFO: Found Device=%s\n", devName.c_str());

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);

    cl::Program program(context, devices, xclBins, NULL, &xrt_err);
    logger.logCreateProgram(xrt_err);

    cl::Kernel kernel_hls_0(program, "kernel_hls", &xrt_err);
    logger.logCreateKernel(xrt_err);

    // Output the inputs information
    std::cout << "INFO: Number of kernel runs: " << num_runs << std::endl;
    std::cout << "INFO: Matrix Row M: " << size_m << std::endl;
    std::cout << "INFO: Matrix Col N: " << size_n << std::endl;






    // DDR Settings
    std::vector<cl_mem_ext_ptr_t> mext_i(1);
    std::vector<cl_mem_ext_ptr_t> mext_o(1);
    // mext_i[0].flags = XCL_MEM_DDR_BANK0;
    // mext_o[0].flags = XCL_MEM_DDR_BANK0;
    // mext_o[1].flags = XCL_MEM_DDR_BANK0;
    // mext_o[2].flags = XCL_MEM_DDR_BANK0;
    // mext_i[0].obj = dataA_svd;
    // mext_i[0].param = 0;
    // mext_o[0].obj = sigma_svd;
    // mext_o[0].param = 0;
    // mext_o[1].obj = dataU_svd;
    // mext_o[1].param = 0;
    // mext_o[2].obj = dataV_svd;
    // mext_o[2].param = 0;
    mext_i[0] = {2, X, kernel_hls_0()};
    mext_o[0] = {3, Y_hw, kernel_hls_0()};

    // Create device buffer and map dev buf to host buf
    std::vector<cl::Buffer> input_buffer(1), output_buffer(3);

    input_buffer[0] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                 sizeof(interface_data_type) * size_m*size_n, &mext_i[0]);
    output_buffer[0] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                  sizeof(interface_data_type) * size_m*size_n, &mext_o[0]);





    // Data transfer from host buffer to device buffer
    std::vector<std::vector<cl::Event> > kernel_evt(2);
    kernel_evt[0].resize(1);
    kernel_evt[1].resize(1);

    std::vector<cl::Memory> ob_in, ob_out;
    ob_in.push_back(input_buffer[0]);
    ob_out.push_back(output_buffer[0]);


    q.enqueueMigrateMemObjects(ob_in, 0, nullptr, &kernel_evt[0][0]); // 0 : migrate from host to dev
    q.finish();
    std::cout << "INFO: Finish data transfer from host to device" << std::endl;

    // Setup kernel
    kernel_hls_0.setArg(0, size_m);
    kernel_hls_0.setArg(1, size_n);
    kernel_hls_0.setArg(2, input_buffer[0]);
    kernel_hls_0.setArg(3, output_buffer[0]);
    q.finish();
    std::cout << "INFO: Finish kernel setup" << std::endl;

    // Variables to measure time
    struct timeval tstart, tend;

    // Launch kernel and compute kernel execution time
    gettimeofday(&tstart, 0);
    for (int i = 0; i < num_runs; ++i) {
        q.enqueueTask(kernel_hls_0, nullptr, nullptr);
    }
    q.finish();
    gettimeofday(&tend, 0);
    std::cout << "INFO: Finish kernel execution" << std::endl;
    int exec_time = diff(&tend, &tstart);
    std::cout << "INFO: FPGA execution time of " << num_runs << " runs:" << exec_time << " us\n"
              << "INFO: Average executiom per run: " << exec_time / num_runs << " us\n";

    // Data transfer from device buffer to host buffer
    q.enqueueMigrateMemObjects(ob_out, 1, nullptr, nullptr); // 1 : migrate from dev to host
    q.finish();

    // !!!
    // END OF XRT KERNEL LAUNCH CODE
    // !!!


    for (int i = 0; i < size_n; i++) {
        for (int j = 0; j < size_m; j++) {
            Y_sw[i * size_n + j] = X[i * size_n +j] + 1;

        }
    }

    // Calculate err between X and Y
    interface_data_type err = 0;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            err += (Y_sw[i * size_n + j] - Y_hw[i * size_n + j]) *
                    (Y_sw[i * size_n + j] - Y_hw[i * size_n + j]);
        }
    }
    err = std::sqrt(err);



    // Delete created buffers
    delete[] X;
    delete[] Y_sw;
    delete[] Y_hw;

    std::cout << "error: " << err << std::endl;

}
