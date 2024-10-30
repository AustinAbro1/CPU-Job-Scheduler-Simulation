# CPU-Job-Scheduler-Simulation
This code takes in an input file of CPU Jobs and runs a simulation outputting multiple files showing statistics and a visual of what is happening in the system.

Please look at cpuIntensiveJobs.txt as a reference for valid CPU jobs

cpuIntensiveLog.txt shows what is happneing at each time unit
cpuIntensiveOutput.txt shows the metrics related to the jobs
cpuIntensiveVisual.txt shows what the movement of jobs from different states

While there is error handling, for your CPU Jobs to be valid
they must be formatted like this
Arrival Time, # of CPU Bursts, CPU Burst 1 Value, I/O Burst 1 Value, CPU Burst 2 Value... etc
