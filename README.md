# C++ CPU Scheduler Simulation 📊

This project provides a simulation and comparative analysis of two common CPU scheduling algorithms: **Multi-Level Feedback Queue (MLFQ)** and **Round Robin (RR)**. The C++ application runs the simulation to process a generated workload of tasks and exports the raw performance data to CSV files. A companion Python script is included to analyze this data and visualize the results.

***

### Project Overview

The goal of this project is to demonstrate the performance differences between a sophisticated, priority-based scheduler (MLFQ) and a simple, non-preemptive scheduler (Round Robin). The simulation is designed to handle a mixed workload of short (I/O-bound) and long (CPU-bound) tasks, highlighting MLFQ's strengths in providing better responsiveness and preventing starvation.

***

### Features

* **Two Scheduler Implementations**: A robust **MLFQ** scheduler with priority queues, time slicing, and an anti-starvation priority boost mechanism, and a standard **Round Robin** scheduler.
* **Realistic Workload Generation**: The program generates a set of 10,000 tasks with varying arrival times, CPU requirements, and deadlines to simulate a real-world server environment.
* **Performance Metrics**: The simulation calculates key performance indicators for each scheduler:
    * SLA Compliance (Deadline Met %)
    * Average Task Wait Time
    * Average Task Turnaround Time
    * Number of Starved Tasks
* **Data Export**: The raw results of the simulation are saved to `mlfq_results.csv` and `rr_results.csv` for external analysis.
* **Python Analysis & Visualization**: A Python script (`analyse_results.py`) processes the CSV files to generate a summary report and a visual plot comparing the wait time distributions of the two algorithms.

***

### Technologies Used

* **C++17**: For the core simulation logic.
* **Python 3**: For data analysis and visualization.
* **Pandas**: For data manipulation and analysis in Python.
* **Matplotlib & Seaborn**: For generating the performance plots.

***
### Results
<img width="500" height="650" alt="image" src="https://github.com/user-attachments/assets/741a88b1-73f4-4747-ad82-fe8434ebb41b" />

### ## How to Compile and Run

#### ### 1. Compile the C++ Simulation

You'll need a C++ compiler that supports C++17 (like g++).

1.  Navigate to the `cpp_scheduler` directory in your terminal.
    ```bash
    cd cpp_scheduler
    ```
2.  Compile the source files.
    ```bash
    g++ -std=c++17 -o scheduler_sim main.cpp scheduler.cpp
    ```
    This will create an executable file named `scheduler_sim`.

#### ### 2. Run the Simulation

Run the compiled program from the `cpp_scheduler` directory.

```bash
./scheduler_sim
