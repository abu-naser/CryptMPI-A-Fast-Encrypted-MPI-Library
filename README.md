# CryptMPI: A Fast Encrypted MPI Library
CrytMPI provides secure inter-node communication.
We implemented two prototypes in MPICH-3.2.1 (for Ethernet) and MVAPICH2-2.3.1 (for Infiniband), both using AES-
GCM from the [BoringSSL library](https://boringssl.googlesource.com/boringssl/).


## Installation
To install cryptMPI for the Infiniband and Ethernet network please follow following steps:
### Package requirement
 autoconf version... >= 2.67
 automake version... >= 1.15
 libtool version... >= 2.4.4

To get the above package you could use get-lib.sh


After installing, set the path for the above packages.

```bash
export PATH=/HOME_DIR/automake/bin:$PATH
export LD_LIBRARY_PATH=/HOME_DIR/automake/lib:$LD_LIBRARY_PATH
```

### Infiniband
Steps:

```bash
tar -xvzf cryptMPI-mvapich2-2.3.2.tar.gz
cd mvapich2-2.3.2
./autogen.sh
./configure --prefix=/MVAPICH_INSTALL_DIR/install  --with-boringssl-include="/YOUR_PATH_TO_MVAPICH/mvapich2-2.3.2/boringssl-master/include/ -fopenmp"

```
In the *MAKEFILE* add -L/YOUR_PATH_TO_MVAPICH/mvapich2-2.3.2/boringssl-master/build/crypto -lcrypto in *LIBS*


(e.g. LIBS =-L/YOUR_PATH_TO_MVAPICH/mvapich2-2.3.2/boringssl-master/build/crypto -lcrypto -libmad -lrdmacm -libumad -libverbs -ldl -lrt -lm -lpthread)


```bash
export LD_LIBRARY_PATH=/YOUR_PATH_TO_MVAPICH/mvapich2-2.3.2/boringssl-master/build/crypto
make clean
make -j
make install
```


### Ethernet
Steps:

```bash
tar -xvzf cryptMPI-mpich-3.2.1.tar.gz
cd mpich-3.2.1
./autogen.sh
./configure --prefix=/MPICH_INSTALL_DIR/install  --with-boringssl-include="/YOUR_PATH_TO_MPICH/mpich-3.2.1/boringssl-master/include/ -fopenmp"

```
In the *MAKEFILE* add -L/YOUR_PATH_TO_MPICH/mpich-3.2.1/boringssl-master/build/crypto -lcrypto in *LIBS*


(e.g. LIBS = -L/YOUR_PATH_TO_MPICH/mpich-3.2.1/boringssl-master/build/crypto -lcrypto -lpthread )


And also *-fopenmp* as LDFLAGS = -fopenmp

```bash
export LD_LIBRARY_PATH=/YOUR_PATH_TO_MPICH/mpich-3.2.1/boringssl-master/build/crypto
make clean
make -j
make install
```


## Usage
To run MPI applications using CryptMPI please follow following steps:
### Infiniband
```bash
export LD_LIBRARY_PATH=/MVAPICH_INSTALL_DIR/install/lib:/YOUR_PATH_TO_MVAPICH/mvapich2-2.3.2/boringssl-master/build/crypto
export MV2_ENABLE_AFFINITY=1
export MV2_CPU_BINDING_POLICY=hybrid
export MV2_HYBRID_BINDING_POLICY=spread 
/MVAPICH_INSTALL_DIR/install/mpiexec -n 64 -f host ./mpi_app
```

### Ethernet
```bash
export LD_LIBRARY_PATH=/MPICH_INSTALL_DIR/install/lib:/YOUR_PATH_TO_MPICH/mpich-3.2.1/boringssl-master/build/crypto
/MPICH_INSTALL_DIR/install/mpiexec -n 64 -f host ./mpi_app
```




## License
[MIT](https://choosealicense.com/licenses/mit/)





