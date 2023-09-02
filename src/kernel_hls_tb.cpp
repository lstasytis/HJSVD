// testbench for the kernel_passthrough.hpp kernel which reads and writes a single array with sizes as extra axi-lite parameters
// this has been adapted from AMD-Xilinx's Vitis Libraries repository


// NOTE - this testbench will fail if the input array is smaller the the kernel maximum array. You have to allocate the same size memory region or you will get a double free error in cosim

#include "kernel_hls.hpp"


#include <string.h>
#include <sys/time.h>
#include <algorithm>


#include "ext/MatrixGen/matrixUtility.hpp"






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

    std::string num_str;
    int size_m, size_n, seed, num_runs;


    // parse CMD arguments

    if (!parser.getCmdOption("-seed", num_str)) {
        seed = 42;
        std::cout << "INFO:seed is not set!\n";
    } else {
        seed = std::stoi(num_str);
    }

    if (!parser.getCmdOption("-num_runs", num_str)) {
        num_runs = 1;
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


    int A_size = size_n * size_m;
    int U_size = size_m * size_m;
    int S_size = size_n;
    int V_size = size_n * size_n;

    
    int A_size_max = size_n * size_m;
    int U_size_max = size_m * size_m;
    int S_size_max = size_n;
    int V_size_max = size_n * size_n;


    interface_data_type* A_host;
    interface_data_type* A_device;
    interface_data_type* U_device;
    interface_data_type* U_host;
    interface_data_type* S_device;
    interface_data_type* S_host;
    interface_data_type* V_device;
    interface_data_type* V_host;
    interface_data_type* V_host_transposed;
    interface_data_type* A_reconstructed;

    A_host = aligned_alloc<interface_data_type>(A_size);
    U_host = aligned_alloc<interface_data_type>(U_size);
    S_host = aligned_alloc<interface_data_type>(A_size);
    V_host = aligned_alloc<interface_data_type>(V_size);
    V_host_transposed = aligned_alloc<interface_data_type>(V_size);

    A_device = aligned_alloc<interface_data_type>(A_size_max);
    U_device = aligned_alloc<interface_data_type>(U_size_max);
    S_device = aligned_alloc<interface_data_type>(S_size_max);
    V_device = aligned_alloc<interface_data_type>(V_size_max);

    A_reconstructed = aligned_alloc<interface_data_type>(A_size);
    
    
    
    int norm_factor = 10000;

    // Generate general matrix dataAM x dataAN

    std::cout << "generating matrix" << std::endl;
    matGen<interface_data_type>(size_n, size_m, seed, A_host);

    // lower down to normalized digits
    for (int i = 0; i < size_n; ++i) {
        for (int j = 0; j < size_m; j++) {
            A_host[i * size_n + j] = A_host[i * size_n + j] / norm_factor;
        }
    }

    // move A from host to device
    for (int i = 0; i < size_n; ++i) {
        for (int j = 0; j < size_m; j++) {
            A_device[i * size_n + j] = A_host[i * size_n + j];
        }
    }

    // Variables to measure time
    struct timeval tstart, tend;


    // Launch kernel and compute kernel execution time
    gettimeofday(&tstart, 0);

    //kernel_hls(dataAM,dataAN,dataA_svd,sigma_svd,dataU_svd,dataV_svd, rr_round_count, rr_pair_count, rr_table_i,rr_table_j);
    kernel_hls(size_n,size_m,A_device,U_device,S_device,V_device);


    gettimeofday(&tend, 0);
    std::cout << "INFO: Finish kernel execution" << std::endl;
    int exec_time = diff(&tend, &tstart);
    std::cout << "INFO: FPGA execution time of " << num_runs << " runs:" << exec_time << " us\n"
              << "INFO: Average executiom per run: " << exec_time / num_runs << " us\n";


    // Calculate A_out = U*sigma*VT and compare with original A matrix


    // move S from device to host
    int s_idx = 0;
    for (int i = 0; i < size_n; ++i) {
        if (S_device[i] < 1.e-8) {
            for (int j = 0; j < size_m; j++) {
                S_device[j * size_n + i] = 0;
            }
        } else {
            for (int j = 0; j < size_m; j++) {
                if (j == s_idx) {
                    S_host[j * size_n + i] = S_device[i];
                } else {
                    S_host[j * size_n + i] = 0;
                }
            }
            s_idx++;
        }
    }

    // move U from device to host
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_m; j++) {
            U_host[i * size_m + j] = U_device[i * size_m + j];
        }
    }

    // move V from device to host
    for (int i = 0; i < size_n; i++) {
        for (int j = 0; j < size_n; j++) {
            V_host[i * size_n + j] = V_device[i * size_n + j];
        }
    }

    transposeMat<interface_data_type>(size_n, V_host, V_host_transposed);




    std::cout  << "U" << std::endl;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            std::cout << U_host[i * size_n + j] << " ";
            //std::cout << dataV_svd[i * dataAN +j] << "\n";
            
        }
        std::cout << std::endl;
    }

    std::cout << "S" << std::endl;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            std::cout << S_host[i * size_n + j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "V" << std::endl;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            std::cout << V_host[i * size_n + j] << " ";      
        }
        std::cout << std::endl;
    }




    std::cout << std::endl;
    
    MulMat(size_m, size_m, size_n, size_n, U_host, S_host, V_host_transposed, A_reconstructed);

    
    std::cout  << "Original" << std::endl;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            std::cout << A_host[i * size_n + j] << " ";
            //std::cout << dataV_svd[i * dataAN +j] << "\n";
            
        }
        std::cout << std::endl;
    }

    std::cout << "A reconstructed" << std::endl;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            std::cout << A_reconstructed[i * size_n + j] << " ";
            //std::cout << dataV_svd[i * dataAN +j] << "\n";
            
        }
        std::cout << std::endl;
    }

    // Calculate err between dataA_svd and dataA_out
    interface_data_type errA = 0;
    for (int i = 0; i < size_m; i++) {
        for (int j = 0; j < size_n; j++) {
            errA += (A_host[i * size_n + j] - A_reconstructed[i * size_n + j]) *
                    (A_host[i * size_n + j] - A_reconstructed[i * size_n + j]);
        }
    }
    errA = std::sqrt(errA);



    // Delete created buffers

    delete [] A_host;
    delete [] A_device;
    delete [] U_device;
    delete [] U_host;
    delete [] S_device;
    delete [] S_host;
    delete [] V_device;
    delete [] V_host;
    delete [] A_reconstructed;

    std::cout << "error A: " << errA << std::endl;
    

}
