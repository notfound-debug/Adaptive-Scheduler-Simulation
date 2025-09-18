import pandas as pd
import numpy as np

def analyze_results(mlfq_file, rr_file):
    try:
        # --- STEP 1: Load Data ---
        print("Step 1: Loading CSV files...")
        mlfq_df = pd.read_csv(mlfq_file)
        rr_df = pd.read_csv(rr_file)
        print(" -> Success: Data loaded into DataFrames.\n")

        # --- STEP 2: Validate Data ---
        print("Step 2: Validating loaded data...")
        if mlfq_df.empty or rr_df.empty:
            print(" -> ERROR: One or both CSV files contain no data rows (only headers).")
            print(" -> This means the C++ simulation ran but completed ZERO tasks.")
            print(" -> Please check the C++ program's console output for any errors or infinite loops.")
            return # Stop execution
        
        required_cols = ['Status', 'WaitTime', 'CPURequired', 'CompletionTime', 'ArrivalTime']
        if not all(col in mlfq_df.columns for col in required_cols) or \
           not all(col in rr_df.columns for col in required_cols):
            print(" -> ERROR: One of the CSV files is missing required columns.")
            print(f" -> MLFQ Columns: {mlfq_df.columns.tolist()}")
            print(f" -> RR Columns:   {rr_df.columns.tolist()}")
            return # Stop execution
            
        print(" -> Success: Data appears valid and contains task results.\n")
        
        # --- STEP 3: Perform Analysis ---
        print("Step 3: Analyzing scheduler performance...")

        # Calculate turnaround time
        mlfq_df['TurnaroundTime'] = mlfq_df['CompletionTime'] - mlfq_df['ArrivalTime']
        rr_df['TurnaroundTime'] = rr_df['CompletionTime'] - rr_df['ArrivalTime']
        
        # Calculate basic metrics
        mlfq_completed = len(mlfq_df[mlfq_df['Status'] == 'COMPLETED'])
        mlfq_missed = len(mlfq_df[mlfq_df['Status'] == 'MISSED_DEADLINE'])
        mlfq_sla_compliance = mlfq_completed / len(mlfq_df) * 100
        
        rr_completed = len(rr_df[rr_df['Status'] == 'COMPLETED'])
        rr_missed = len(rr_df[rr_df['Status'] == 'MISSED_DEADLINE'])
        rr_sla_compliance = rr_completed / len(rr_df) * 100
        
        # Calculate wait and turnaround statistics
        mlfq_avg_wait = mlfq_df['WaitTime'].mean()
        rr_avg_wait = rr_df['WaitTime'].mean()
        mlfq_avg_turnaround = mlfq_df[mlfq_df['Status'] == 'COMPLETED']['TurnaroundTime'].mean()
        rr_avg_turnaround = rr_df[rr_df['Status'] == 'COMPLETED']['TurnaroundTime'].mean()
        
        # Starvation calculation
        mlfq_starved = len(mlfq_df[mlfq_df['WaitTime'] > mlfq_df['CPURequired'] * 3])
        rr_starved = len(rr_df[rr_df['WaitTime'] > rr_df['CPURequired'] * 3])
        
        # --- STEP 4: Print Results ---
        print(" -> Success: Analysis complete.\n")
        print("=" * 60)
        print("MLFQ Scheduler Results:")
        print("=" * 60)
        print(f"  Total tasks processed: {len(mlfq_df)}")
        print(f"  Completed tasks: {mlfq_completed}")
        print(f"  Missed deadlines: {mlfq_missed}")
        print(f"  Starved Tasks: {mlfq_starved}")
        print(f"  SLA Compliance: {mlfq_sla_compliance:.2f}%")
        print(f"  Average wait time: {mlfq_avg_wait:.2f}")
        print(f"  Average turnaround time: {mlfq_avg_turnaround:.2f}")
        
        print("\n" + "=" * 60)
        print("Round Robin Scheduler Results:")
        print("=" * 60)
        print(f"  Total tasks processed: {len(rr_df)}")
        print(f"  Completed tasks: {rr_completed}")
        print(f"  Missed deadlines: {rr_missed}")
        print(f"  Starved Tasks: {rr_starved}")
        print(f"  SLA Compliance: {rr_sla_compliance:.2f}%")
        print(f"  Average wait time: {rr_avg_wait:.2f}")
        print(f"  Average turnaround time: {rr_avg_turnaround:.2f}")
        
        # Improvements Calculation
        starvation_reduction = (1.0 - (mlfq_starved / rr_starved)) * 100 if rr_starved > 0 else float('inf')
        sla_improvement = mlfq_sla_compliance - rr_sla_compliance
        wait_time_improvement = (1.0 - (mlfq_avg_wait / rr_avg_wait)) * 100 if rr_avg_wait > 0 else float('inf')
        turnaround_improvement = (1.0 - (mlfq_avg_turnaround / rr_avg_turnaround)) * 100 if rr_avg_turnaround > 0 else float('inf')
        
        print("\n" + "=" * 60)
        print("Performance Comparison (MLFQ vs Round Robin):")
        print("=" * 60)
        print(f"Starvation Reduction: {starvation_reduction:.2f}%")
        print(f"SLA Compliance Improvement: {sla_improvement:.2f}% points")
        print(f"Wait Time Improvement: {wait_time_improvement:.2f}%")
        print(f"Turnaround Time Improvement: {turnaround_improvement:.2f}%")

    except Exception as e:
        print("\n" + "="*60)
        print("AN UNEXPECTED ERROR OCCURRED!")
        print("="*60)
        print(f"The Python script failed with the following error:\n -> {e}")
        print("\nThis might be due to a malformed CSV file or an issue with your Python environment.")

if __name__ == "__main__":
    # Using the path from your latest output
    mlfq_csv_path = '../cpp_scheduler/build/data/mlfq_results.csv'
    rr_csv_path = '../cpp_scheduler/build/data/rr_results.csv'
    
    print(f"Attempting to run analysis on:\n  {mlfq_csv_path}\n  {rr_csv_path}\n")
    
    analyze_results(mlfq_csv_path, rr_csv_path)
    # Add this to the end of your analyse_results.py file

import matplotlib.pyplot as plt
import seaborn as sns

def plot_wait_time_distribution(mlfq_df, rr_df):
    """Generates a plot to visualize wait time distributions."""
    print("\nGenerating Wait Time Distribution plot...")
    
    plt.style.use('seaborn-v0_8-whitegrid')
    fig, ax = plt.subplots(figsize=(12, 7))

    sns.histplot(data=rr_df, x='WaitTime', ax=ax, color='salmon', label='Round Robin', kde=True, bins=50)
    sns.histplot(data=mlfq_df, x='WaitTime', ax=ax, color='skyblue', label='MLFQ', kde=True, bins=50)
    
    ax.set_title('Distribution of Task Wait Times (MLFQ vs. Round Robin)', fontsize=16, fontweight='bold')
    ax.set_xlabel('Wait Time (units)', fontsize=12)
    ax.set_ylabel('Number of Tasks', fontsize=12)
    ax.legend()
    
    # Set a reasonable x-axis limit to see the main distribution, as RR can have extreme outliers
    plt.xlim(-100, rr_df['WaitTime'].quantile(0.95))
    
    plt.savefig('wait_time_distribution.png', dpi=150)
    print(" -> Success: Plot saved to 'wait_time_distribution.png'")

# At the very end of the file, after the analyze_results call, add this:
# (You'll need to read the data again or pass it down, let's just re-read for simplicity)
if __name__ == "__main__":
    mlfq_csv_path = '../cpp_scheduler/build/data/mlfq_results.csv'
    rr_csv_path = '../cpp_scheduler/build/data/rr_results.csv'
    
    print(f"Attempting to run analysis on:\n  {mlfq_csv_path}\n  {rr_csv_path}\n")
    analyze_results(mlfq_csv_path, rr_csv_path)
    
    # Now, let's call our new plotting function
    try:
        mlfq_data = pd.read_csv(mlfq_csv_path)
        rr_data = pd.read_csv(rr_csv_path)
        plot_wait_time_distribution(mlfq_data, rr_data)
    except Exception as e:
        print(f"Could not generate plot. Error: {e}")