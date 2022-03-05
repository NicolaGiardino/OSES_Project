Changes:
========

| Name | Date | Commit no. | Description |
|:----:|:----:|:----------:|:-----------:|
| Nicola di Gruttola Giardino | Dec 9 '21 through Feb 4 '22 | Commit fbbe87 to 6ce866e | Added everything needed for the project, began modifying the benchmark, uploaded completed DefServ |
| Fulvio Castello | Feb 8 '22 | Commit 1 | Replaced pointers with arrays [deprecated] & added top comment for verified line [unused] |
| Nicola di Gruttola Giardino | Feb 10 '22 | Commit 2 | Added xeno functions into the header and into the main |
| Nicola di Gruttola Giardino | Feb 10 '22 | Commit 3 | Tested LED and Pushbutton, implemented reading accelerometer (To test) |
| Fulvio Castello | Feb 10 '22 | Commit 4 | Heavily modified several methods inside benchmark and added TODO list (for future reference) |
| Fulvio Castello | Feb 15 '22 | Commit 5 | Resolved issue regarding Xenomai waiting for next periodic release by using a similar RT-Thread function & fixed implicit pointer to integer conversion inside deferrable server source code |
| Nicola di Gruttola Giardino | Feb 16 '22 | Commit 6 | Changed functions of periodic tasks into the equivalent for RT-Thread - Implemented time elapsed functions |
| Fulvio castello | Feb 16 '22 | Commit 7 | Made linking XenoJetBench.h possible thanks to improved SCons implementation - work on OpenMP library still needed to compile correctly |
| Fulvio Castello | Feb 17 '22 | Commit 8 | Apparently eliminated parallel threads support (needs testing for definitive commented code removal); adjusted a lot of console outputs formatting and timing variables types; fixed warning inside deferrable server source code & improved engine selection via a switch statement |
| Nicola di Gruttola Giardino | Feb 17-18 '22 | Commit 9c16c57 to 9521a89 | Switched the project to the STM32H7 |
| Fulvio Castello | Feb 19 '22 | Commit 9| Tidied-up benchmark code (needs final testing for functionality) & rendered engine selection automatic + (pseudo)random |
| Nicola di Gruttola Giardino | Feb 19 '22 | Commit 10 | Implemented all libraries for hardware, first body of the final program implemented |
| Fulvio Castello | Feb 20 '22 | Commit 11 | Added very primitive implementation of interrupts: not linking correctly because of a lack of the right CSP (the required methods are defined but not actually implemented for the Cortex M0) |
| Nicola di Gruttola Giardino | Feb 20 '22 | Commit 12 | Implemented working ISR |
| Fulvio Castello | Feb 22 '22 | Commit 13 | Adapted XenoJetBench for reading from filestructure instead of external file, then implemented its thread inside the main function with mutex controlling capabilities; firther code cleanup |
| Fulvio Castello | Feb 22 '22 | Commit 14 | Performed final code cleanup and implementation for XenoJetBench-related thread to test on a non-parallel architecture; also removed unused files |
| Fulvio CAstello | Feb 22 '22 | Commit 15 | Created a separate method for reading from memory after writing - completely reworked the custom IRQ for the the pushbutton to control the mass storage |
| Nicolo' Carpentieri | Feb 24 '22 | Commit 16 | Major code adjustments, completed multithreading for first tests without DefServ |
| Nicolo' Carpentieri | Feb 25 '22 | Commit 17 | Implemented second IRQ |
| Nicola di Gruttola Giardino | Feb 25 '22 | Commit 18 | Added source code of Deferrable Server |
| Nicolo' Carpentieri | Feb 25 '22 | Commit 19 | Implemented DefServ via define, adjustments on priorities and bug fixes, ready for final tests |
| Nicola di Gruttola Giardino | Feb 26 '22 | Commit 20 | Code adjustments on position of Mutexes, commented code |
| Nicola di Gruttola Giardino | Feb 26 '22 | Commit 21 | Completed tests, everything done until now working. WCETs are: Benchmark approx 186s, Sampling approx 2s, IRQ with DefServ approx 20s |
| Nicolo' Carpentieri | Feb 27 '22 | Commit 22 | Completed implementation of multithreading, first implementation of barometer's library |
| Nicolo' Carpentieri | Mar 03 '22 | Commit 23 | Project fully working, last thing to implement are the proper delays for the threads |
| Nicolo' Carpentieri | Mar 04 '22 | Commit 24 | Implemented delays, project completed, final tests to be done, improved readability |
| Fulvio Castello | Mar 05 '22 | Commit 25 | Updated README.md & added repository info |
| Fulvio Castello | Mar 05 '22 | Commit 26 | Relocated main working files inside a different directory tree - project is ready for final testing |
| Nicola di Gruttola Giardino | Mar 05 '22 | Commit 27 | Completed tests, project finished |
| Fulvio Castello | Mar 05 '22 | Commit 28 | Code beautify with clang-format for atom (v2.0.8) |
