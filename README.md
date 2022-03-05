# OSES Project

This application was developed using the [RT-Thread](https://github.com/RT-Thread/rt-thread) OS kernel, to be loaded onto an **STM32 Nucleo-144** board with the aim of realizing an H7 CPU scheduler stress-test based on the [XenoJetBench](https://github.com/ArsalanShahid116/XenoJetBench) benchmark.

The workflow can be divided into three steps: analog sampling, FPU benchmarking and data storage. Raw sensor readings get first collected by means of an **MPU-6050** accelerometer (acceleration & gyroscopic data on the three Euclidean axes) and a **BMP280** barometer (ambient pressure readings). A hardware-dependent [external library](https://github.com/LonelyWolf/stm32/tree/master/bmp280) had to be included in order to get the latter peripheral to work properly within the target platform.

These input values are then processed by the synthetic benchmark, whose results can eventually be saved into a **W25Q64FV** 64 Mbit memory. Two pushbuttons are utilized to control such archiving functionality, so that the first is used to write the data deriving from the external peripherals into the flash, while the other one is used to read the latest results of the benchmark from it.

The asynchronous requests were initially implemented by means of additional aperiodic tasks, whereas a deferrable server was later added to the system in order to significantly improve the overall workload of such real-time system, whilst providing a considerably higher level of optimization and a safer operation at the same time.

<img width="600" alt="Platform" src="https://user-images.githubusercontent.com/95169925/156853632-3112ba81-083c-43b7-bfc3-573541343d66.PNG">
