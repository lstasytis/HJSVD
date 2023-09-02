
import sys
import getopt
import json
import struct
import uuid

class Options(object):
    def __init__(self):
        self.DATA_SIZE = 1024
        self.sharedLibrary = None
        self.bitstreamFile = None
        self.halLogFile = None
        self.alignment = 4096
        self.option_index = 0
        self.index = 0
        self.cu_index = 0
        self.verbose = False
        self.handle = None
        self.xcl_handle = None
        self.first_mem = -1
        self.cu_base_addr = -1
        self.xuuid = uuid.uuid4()
        self.kernels = []
        self.platformfile = None

    def getOptions(self, argv):
        try:
            opts, args = getopt.getopt(argv[1:], "s:k:p:l:a:c:d:b:f:r:t:o:vhe", ["data_size=","max_data_size=","bitstream=", "platform_json=", "hal_logfile=", "alignment=",
                                                                   "cu_index=", "device=","use_software_hjsvd=","full_test_cap=","repeats=", "datatype=","verbose","kernel_count=","help", "ert"])
        except getopt.GetoptError:
            print(self.printHelp())
            sys.exit(2)

        for o, arg in opts:
            if o in ("--bitstream", "-k"):
                self.bitstreamFile = arg
            elif o in ("--platform_json", "-p"):
                self.platformfile = arg
            elif o in ("--data_size", "-s"):
                self.DATA_SIZE = int(arg)
            elif o in ("--max_data_size", "-ms"):
                self.MAX_DATA_SIZE = int(arg)
            elif o in ("--hal_logfile", "-l"):
                self.halLogFile = arg
            elif o in ("--alignment", "-a"):
                print("-a/--alignment switch is not supported")
            elif o in ("--cu_index", "-c"):
                self.cu_index = int(arg)
            elif o in ("--device", "-d"):
                self.index = int(arg)
            elif o in ("--full_test_cap","-f"):
                self.FULL_TEST_CAP = int(arg)
            elif o in ("--use_software_hjsvd","-b"):
                self.USE_SW_HJSVD = int(arg) #1 for yes, 0 for no
            elif o in ("--repeats","-r"):
                self.REPEATS = int(arg)
            elif o in ("--offset", "-o"):
                self.OFFSET = int(arg)
            elif o in ("--help", "-h"):
                print(self.printHelp())
            elif o == "-t":
                self.DTYPE = arg
            elif o == "-v":
                self.verbose = True
            elif o == "--kernel_count":
                self.KERNEL_COUNT = int(arg)
            elif o in ("-e", "--ert"):
                print("-e/--ert switch is not supported")
            else:
                assert False, "unhandled option"

        if self.bitstreamFile is None:
            raise RuntimeError("No bitstream specified")

        if self.halLogFile:
            print("Log files are not supported on command line, Please use xrt.ini to specify logging configuration")
        print("Host buffer alignment " + str(self.alignment) + " bytes")
        print("Compiled kernel = " + self.bitstreamFile)

    def printHelp(self):
        print("usage: %s [options]")
        print("  -k <bitstream>")
        print("  -p <platform_json>")
        print("  -d <device_index>")
        print("  -c <cu_index>")
        print("  -v <verbose or not>")
        print("  -dt <data type (float/double)>")
        print("  -h")
        print("")
        print("* Bitstream is required")

def parsePlatform(opt):
    desc = open(opt.platformfile, 'r')
    data = json.load(desc)
    desc.close()

    count = 0
    if "hardwarePlatform" in data:
        if "busInterfaces" in data["hardwarePlatform"]:
            for busInterfaces in data["hardwarePlatform"]["busInterfaces"]:
                if "spTag" in busInterfaces:
                    if busInterfaces["spTag"] == "DDR" or busInterfaces["spTag"] == "HOST":
                        if "addressSegments" in busInterfaces:
                            for addressSegments in busInterfaces["addressSegments"]:
                                count += 1
                    elif busInterfaces["spTag"] == "HBM":
                        if "addressSegments" in busInterfaces:
                            count += 1
    return count



import os
import sys
import uuid
import re
import numpy as np
import ctypes
import time
from scipy import linalg
from sklearn.preprocessing import StandardScaler, MinMaxScaler


from hjsvd_sw import *

# Following found in PYTHONPATH setup by XRT
import pyxrt


def peak_error(A,B):
   r = A-B
   return np.log2((np.max(A)-np.min(A))/(2*np.max(abs(r))))


def mean_squared_error(A,B):
   return ((A - B)**2).mean()


def transform_sigma(S,n,m):
    sigma = np.zeros((n, m))
    for i in range(S.shape[0]):
        sigma[i,i] = S[i]
    return sigma

def generate_matrix(size,width,center):
    s = size
    np.random.seed(130912)
    orders = np.floor(np.random.uniform(center-width,center+width, size=(s,s)))
    vals = np.random.uniform(low=1.0,high=9.9999999999999999,size=(s,s))

    A = np.zeros((s,s),dtype=np.float64)

    for i in range(s):
        for j in range(s):
            A[i,j] = np.float64(f'{vals[i,j]}e{int(orders[i,j])}')

    return A


def reconstruct_matrix(U,S,V):
    sigma = transform_sigma(S,len(S),len(S))
    return np.dot(U, np.dot(sigma, V))

current_micro_time = lambda: int(round(time.time() * 1000000))





def runFullKernelTest(opt):
    print(f"Performing a full kernel test up to {opt.FULL_TEST_CAP} array sizes")


    setup_start = current_micro_time()
    result = 0
    d = pyxrt.device(opt.index)
    uuid = d.load_xclbin(opt.bitstreamFile)
    # Instantiate vectorvadd
    vadd = pyxrt.kernel(d, uuid, "kernel_hls")
    run_kernel = True

    
    print("run?")
    if opt.DTYPE == "float":
        dtype = np.float32
        dsize = 4
    elif opt.DTYPE == "double":
        dtype = np.float64
        dsize = 8
    else:
        print(f"DTYPE {opt.DTYPE} not supported!")


    SIZES = [16,32,64,128,256,512]

    errors = []
    sizes = []
    throughputs = []
    sdurations = []
    optimums = []
    optimum_throughputs = []

    MAX_SIZE = 512

    for DATA_SIZE in SIZES:
        if DATA_SIZE <= opt.FULL_TEST_CAP:
            sizes.append(DATA_SIZE)
            param_a = 1
            param_b = 1

            if dtype == np.float64:
                
                BYTE_SIZE = ctypes.sizeof(ctypes.c_int)*2 * DATA_SIZE
                ARRAY_BYTE_SIZE = ctypes.sizeof(ctypes.c_int)*2 * DATA_SIZE**2
            elif dtype == np.float32: # currently broken
                BYTE_SIZE = ctypes.sizeof(ctypes.c_int) * DATA_SIZE
                ARRAY_BYTE_SIZE = ctypes.sizeof(ctypes.c_int) * DATA_SIZE**2


            BUFFER_BYTE_SIZE = BYTE_SIZE
            ARRAY_BUFFER_BYTE_SIZE = ARRAY_BYTE_SIZE


            print("starting buffers")
            


            in1_obj = pyxrt.bo(d, ARRAY_BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(2))
            out1_obj = pyxrt.bo(d, BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(3))
            out2_obj = pyxrt.bo(d, ARRAY_BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(4))
            out3_obj = pyxrt.bo(d, ARRAY_BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(5))

            

            in1_buf = np.asarray(in1_obj.map())
            out1_buf = np.asarray(out1_obj.map())
            out2_buf = np.asarray(out2_obj.map())
            out3_buf = np.asarray(out3_obj.map())


            in_float = np.zeros((DATA_SIZE,DATA_SIZE),dtype=dtype)
            out_float = np.zeros((DATA_SIZE),dtype=dtype)
            out_float2 = np.zeros((DATA_SIZE*DATA_SIZE),dtype=dtype)
            out_float3 = np.zeros((DATA_SIZE*DATA_SIZE),dtype=dtype)

            # Compute golden values
            reference = np.zeros((DATA_SIZE,DATA_SIZE),dtype=dtype)

            print("made buffers")

            A = generate_matrix(DATA_SIZE,3,0)
            scaler = MinMaxScaler(feature_range=(0,1),copy=True)
            A = scaler.fit_transform(A)
            norm = linalg.norm(A) * opt.OFFSET

            norm = 10.0
            A /= norm
            

            np.copyto(in_float,A.astype(dtype))
            

            in_float = in_float

            in_float = in_float.flatten()

            np.copyto(in1_buf,in_float.view(np.uint8))

            setup_end = current_micro_time()

            time_to_setup_s = (setup_end - setup_start) / 1000000.0

            run = pyxrt.run(vadd)

            in1_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_TO_DEVICE, ARRAY_BYTE_SIZE, 0)

            start = current_micro_time()


            BUFFER_SIZE = DATA_SIZE



            group_count = int(DATA_SIZE / BUFFER_SIZE)



            repeats = opt.REPEATS
            print("starting the kernel")
            # Create a run object without starting kernel
            start2 = time.time()
            if run_kernel:
                
                for id in range(group_count):
                    #print("group: ",id)
                    in1_sub_obj = pyxrt.bo(in1_obj, ARRAY_BUFFER_BYTE_SIZE, ARRAY_BUFFER_BYTE_SIZE * id)
                    out1_sub_obj = pyxrt.bo(out1_obj, BUFFER_BYTE_SIZE, BUFFER_BYTE_SIZE * id)
                    out2_sub_obj = pyxrt.bo(out2_obj, ARRAY_BUFFER_BYTE_SIZE, ARRAY_BUFFER_BYTE_SIZE * id)
                    out3_sub_obj = pyxrt.bo(out3_obj, ARRAY_BUFFER_BYTE_SIZE, ARRAY_BUFFER_BYTE_SIZE * id)


                    for i in range(repeats):
                        run.set_arg(0,DATA_SIZE)
                        run.set_arg(1,DATA_SIZE)
                        run.set_arg(2,in1_sub_obj)
                        run.set_arg(3,out2_sub_obj)
                        run.set_arg(4,out1_sub_obj)
                        run.set_arg(5,out3_sub_obj)
                        run.start()
                        state = run.state()
                        state = run.wait()
                    

            end = current_micro_time()
            end2 = time.time()
            print(end2-start2)
            
            # sync device memory to output buffer and assign to np array
            out1_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_FROM_DEVICE, BYTE_SIZE, 0)
            np.copyto(out_float,out1_buf.view(dtype))

            out2_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_FROM_DEVICE, ARRAY_BUFFER_BYTE_SIZE, 0)
            np.copyto(out_float2,out2_buf.view(dtype))

            out3_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_FROM_DEVICE, ARRAY_BUFFER_BYTE_SIZE, 0)
            np.copyto(out_float3,out3_buf.view(dtype))

   
            out_float2 = out_float2.reshape(DATA_SIZE,DATA_SIZE)
            out_float3 = out_float3.reshape(DATA_SIZE,DATA_SIZE).T

            B = reconstruct_matrix(out_float2,out_float,out_float3)*norm


            A_float = A.astype(np.float32)


            if opt.USE_SW_HJSVD == 1:
                U_hsw,S_hsw,V_hsw = HJSVD(A,mode=0,tol=1.e-10) # mode=0 is multiplier rr, m=1 rr cordic, m=2, regular hjsvd
                B_hsw = reconstruct_matrix(U_hsw,S_hsw,V_hsw)*norm

                U_hsw_float,S_hsw_float,V_hsw_float = HJSVD(A_float,mode=0,tol=1.e-5) # mode=0 is multiplier rr, m=1 rr cordic, m=2, regular hjsvd
                B_hsw_float = reconstruct_matrix(U_hsw_float,S_hsw_float,V_hsw_float)*norm


            U,S,V = linalg.svd(A)
            B_linalg = reconstruct_matrix(U,S,V)*norm
            A *= norm

            if opt.verbose:

                print("peak correct bits HW_HJSVD: ",peak_error(A,B))
            
            usduration = (end-start) / repeats
            sduration = usduration/1000000.0
            dbytes = BYTE_SIZE
            buffer_mb_size = BUFFER_BYTE_SIZE / (1024*1024)
            dmbytes = dbytes / (1024*1024)
            bps = dbytes / sduration
            mbps = bps / (1024*1024)
            gbps = mbps / (1024)


            errors.append(peak_error(A,B))
            #throughput = 10/2*DATA_SIZE**3*dsize*2 / sduration / (1024*1024)

            # sweeps * A and V * pair of elements * readANDwrite * cols * pairs * rounds * element size
            bytes_read = 10 * 2 * 2 * 2 * DATA_SIZE * DATA_SIZE/2 * (DATA_SIZE-1) * dsize

            throughput = bytes_read / sduration / (1024*1024)

            # rotation count / cu count / clock speed
            optimum_for_200mhz_8cu = 10 * DATA_SIZE * DATA_SIZE/2 * (DATA_SIZE-1) / 8 / 200000000

            optimum_throughput = bytes_read / optimum_for_200mhz_8cu / (1024*1024)

            throughputs.append(throughput)
            sdurations.append(sduration)
            optimums.append(optimum_for_200mhz_8cu)
            optimum_throughputs.append(optimum_throughput)


    print("matrix size\t throughput \t correct bits \t elapsed time \t expected time")
    for i,size in enumerate(sizes):
        print(f"{size}\t{throughputs[i]} MB \t{errors[i]}\t {sdurations[i]} s\t {optimums[i]} s \t {optimum_throughputs[i]}")


    return 0


def runKernel(opt):

    setup_start = current_micro_time()
    result = 0
    d = pyxrt.device(opt.index)
    uuid = d.load_xclbin(opt.bitstreamFile)
    # Instantiate vectorvadd
    vadd = pyxrt.kernel(d, uuid, "kernel_hls")
    run_kernel = True

    
    print("run?")
    if opt.DTYPE == "float":
        dtype = np.float32
    elif opt.DTYPE == "double":
        dtype = np.float64
    else:
        print(f"DTYPE {opt.DTYPE} not supported!")

    DATA_SIZE = opt.DATA_SIZE
    param_a = 1
    param_b = 1

    if dtype == np.float64:
        BYTE_SIZE = ctypes.sizeof(ctypes.c_int)*2 * DATA_SIZE
        ARRAY_BYTE_SIZE = ctypes.sizeof(ctypes.c_int)*2 * DATA_SIZE**2
    elif dtype == np.float32: 
        BYTE_SIZE = ctypes.sizeof(ctypes.c_int) * DATA_SIZE
        ARRAY_BYTE_SIZE = ctypes.sizeof(ctypes.c_int) * DATA_SIZE**2

    print(vadd.group_id(4))
    for i in range(6):
        print(i,vadd.group_id(i))

    in1_obj = pyxrt.bo(d, ARRAY_BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(2))
    out1_obj = pyxrt.bo(d, BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(3))
    out2_obj = pyxrt.bo(d, ARRAY_BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(4))
    out3_obj = pyxrt.bo(d, ARRAY_BYTE_SIZE, pyxrt.bo.normal, vadd.group_id(5))

    in1_buf = np.asarray(in1_obj.map())
    out1_buf = np.asarray(out1_obj.map())
    out2_buf = np.asarray(out2_obj.map())
    out3_buf = np.asarray(out3_obj.map())


    in_float = np.zeros((DATA_SIZE,DATA_SIZE),dtype=dtype)
    out_float = np.zeros((DATA_SIZE),dtype=dtype)
    out_float2 = np.zeros((DATA_SIZE*DATA_SIZE),dtype=dtype)
    out_float3 = np.zeros((DATA_SIZE*DATA_SIZE),dtype=dtype)

    # Compute golden values
    reference = np.zeros((DATA_SIZE,DATA_SIZE),dtype=dtype)



    A = generate_matrix(opt.DATA_SIZE,3,0)
    scaler = MinMaxScaler(feature_range=(0,1),copy=True)
    A = scaler.fit_transform(A)

    
    norm = linalg.norm(A) * opt.OFFSET
    norm = 10.0

    A /= norm

    np.copyto(in_float,A.astype(dtype))
    

    in_float = in_float

    in_float = in_float.flatten()


    np.copyto(in1_buf,in_float.view(np.uint8))

    setup_end = current_micro_time()

    time_to_setup_s = (setup_end - setup_start) / 1000000.0

    run = pyxrt.run(vadd)

    in1_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_TO_DEVICE, ARRAY_BYTE_SIZE, 0)

    start = current_micro_time()

    BUFFER_SIZE = DATA_SIZE


    if dtype == np.float64:
        BUFFER_BYTE_SIZE = ctypes.sizeof(ctypes.c_int)*2 * BUFFER_SIZE
        ARRAY_BUFFER_BYTE_SIZE = ctypes.sizeof(ctypes.c_int)*2 * BUFFER_SIZE**2
    elif dtype == np.float32: # currently broken
        BUFFER_BYTE_SIZE = ctypes.sizeof(ctypes.c_int) * BUFFER_SIZE
        ARRAY_BUFFER_BYTE_SIZE = ctypes.sizeof(ctypes.c_int) * BUFFER_SIZE**2
    group_count = int(DATA_SIZE / BUFFER_SIZE)



    repeats = opt.REPEATS

    # Create a run object without starting kernel
    start2 = time.time()
    if run_kernel:
        
        for id in range(group_count):
            #print("group: ",id)
            in1_sub_obj = pyxrt.bo(in1_obj, ARRAY_BUFFER_BYTE_SIZE, ARRAY_BUFFER_BYTE_SIZE * id)
            out1_sub_obj = pyxrt.bo(out1_obj, BUFFER_BYTE_SIZE, BUFFER_BYTE_SIZE * id)
            out2_sub_obj = pyxrt.bo(out2_obj, ARRAY_BUFFER_BYTE_SIZE, ARRAY_BUFFER_BYTE_SIZE * id)
            out3_sub_obj = pyxrt.bo(out3_obj, ARRAY_BUFFER_BYTE_SIZE, ARRAY_BUFFER_BYTE_SIZE * id)


            for i in range(repeats):
                run.set_arg(0,opt.DATA_SIZE)
                run.set_arg(1,opt.DATA_SIZE)
                run.set_arg(2,in1_sub_obj)
                run.set_arg(3,out2_sub_obj)
                run.set_arg(4,out1_sub_obj)
                run.set_arg(5,out3_sub_obj)
                run.start()
                state = run.state()
                state = run.wait()
            

    end = current_micro_time()
    end2 = time.time()
    print(end2-start2)
    
    # sync device memory to output buffer and assign to np array
    out1_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_FROM_DEVICE, BYTE_SIZE, 0)
    np.copyto(out_float,out1_buf.view(dtype))

    out2_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_FROM_DEVICE, ARRAY_BUFFER_BYTE_SIZE, 0)
    np.copyto(out_float2,out2_buf.view(dtype))

    out3_obj.sync(pyxrt.xclBOSyncDirection.XCL_BO_SYNC_BO_FROM_DEVICE, ARRAY_BUFFER_BYTE_SIZE, 0)
    np.copyto(out_float3,out3_buf.view(dtype))

    #out_float = out_float.reshape(opt.DATA_SIZE,opt.DATA_SIZE)
    out_float2 = out_float2.reshape(opt.DATA_SIZE,opt.DATA_SIZE)
    out_float3 = out_float3.reshape(opt.DATA_SIZE,opt.DATA_SIZE).T

    B = reconstruct_matrix(out_float2,out_float,out_float3)*norm


    A_float = A.astype(np.float32)




    if opt.USE_SW_HJSVD == 1:
        U_hsw,S_hsw,V_hsw = HJSVD(A,mode=0,tol=1.e-10) # mode=0 is multiplier rr, m=1 rr cordic, m=2, regular hjsvd
        B_hsw = reconstruct_matrix(U_hsw,S_hsw,V_hsw)*norm

        U_hsw_float,S_hsw_float,V_hsw_float = HJSVD(A_float,mode=0,tol=1.e-5) # mode=0 is multiplier rr, m=1 rr cordic, m=2, regular hjsvd
        B_hsw_float = reconstruct_matrix(U_hsw_float,S_hsw_float,V_hsw_float)*norm


    U,S,V = linalg.svd(A)
    B_linalg = reconstruct_matrix(U,S,V)*norm
    A *= norm


    if opt.verbose:
        print("Compare the FPGA results with golden data")

        print("U_hw")
        print(out_float2)

        print("S_hw")
        print(out_float)

        print("V_hw")
        print(out_float3)

        print("norm: ",norm)
        print("peak correct bits HW_HJSVD: ",peak_error(A,B))
        print("peak correct bits linalg64 (52 frac bits): ",peak_error(A,B_linalg))

        if opt.USE_SW_HJSVD == 1:
            print("peak correct bits SW_HJSVD_double: ",peak_error(A,B_hsw))
            print("peak correct bits SW_HJSVD_float: ",peak_error(A,B_hsw_float))
            


        print("mean squared error HW_HJSVD: ",np.abs(np.log2(mean_squared_error(A,B))))
        print("mean squared error linalg64 (52 frac bits): ",np.abs(np.log2(mean_squared_error(A,B_linalg))))


        if opt.USE_SW_HJSVD == 1:
            print("U peak ",peak_error(U_hsw_float,out_float2))
            print("S peak ",peak_error(S_hsw_float,out_float))
            print("V peak",peak_error(V_hsw_float,out_float3))

            print("U double peak ",peak_error(U_hsw,out_float2))
            print("S double peak ",peak_error(S_hsw,out_float))
            print("V double peak",peak_error(V_hsw,out_float3))


            print("U mse ",np.abs(np.log2(mean_squared_error(U_hsw_float,out_float2))))
            print("S mse ",np.abs(np.log2(mean_squared_error(S_hsw_float,out_float))))
            print("V mse",np.abs(np.log2(mean_squared_error(V_hsw_float,out_float3))))

            print("U double mse ",np.abs(np.log2(mean_squared_error(U_hsw,out_float2))))
            print("S double mse ",np.abs(np.log2(mean_squared_error(S_hsw,out_float))))
            print("V double mse",np.abs(np.log2(mean_squared_error(V_hsw,out_float3))))


            print("mean squared error SW_HJSVD_double: ",np.abs(np.log2(mean_squared_error(A,B_hsw))))
            print("mean squared error SW_HJSVD_float: ",np.abs(np.log2(mean_squared_error(A,B_hsw_float))))
            

    usduration = (end-start) / repeats
    sduration = usduration/1000000.0
    dbytes = BYTE_SIZE
    buffer_mb_size = BUFFER_BYTE_SIZE / (1024*1024)
    dmbytes = dbytes / (1024*1024)
    bps = dbytes / sduration
    mbps = bps / (1024*1024)
    gbps = mbps / (1024)

    print("s_to_set_up: ",time_to_setup_s)
    print("us_duration: ",usduration)
    print("s_duration: ",sduration)
    print("data_Bytes: ", round(dbytes,2))
    print("data_MBytes: ", round(dmbytes,2))
    print("Bps: ",round(bps,2))
    print("MBps: ",round(mbps,2))

    print("local buffer size (MB): ",round(buffer_mb_size,2))
    print("global data  size (MB): ", round(dmbytes,2))
    print("GBps: ",round(gbps,2))


    return 0


def main(args):
    opt = Options()
    print(args)
    Options.getOptions(opt, args)

    try:
        if opt.FULL_TEST_CAP == 0:
            runKernel(opt)
        else :
            runFullKernelTest(opt)
        print("TEST PASSED")
        return 0

    except OSError as o:
        print(o)
        print("TEST FAILED")
        return -o.errno

    except AssertionError as a:
        print(a)
        print("TEST FAILED")
        return -1
    except Exception as e:
        print(e)
        print("TEST FAILED")
        return -1

if __name__ == "__main__":
    os.environ["Runtime.xrt_bo"] = "false"
    result = main(sys.argv)
    print("Done, exiting.")
    sys.exit(result)
