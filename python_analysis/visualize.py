import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def create_gantt_chart(df, title, filename):
    # Filter only completed tasks
    completed_df = df[df['Status'] == 'COMPLETED'].copy()
    
    # Sort by start time
    completed_df = completed_df.sort_values('StartTime')
    
    # Create figure
    plt.figure(figsize=(12, 8))
    
    # Create horizontal bars
    y_pos = np.arange(len(completed_df))
    plt.barh(y_pos, completed_df['CompletionTime'] - completed_df['StartTime'], 
             left=completed_df['StartTime'], alpha=0.6)
    
    # Add deadlines as vertical lines
    for i, row in completed_df.iterrows():
        plt.axvline(x=row['Deadline'], color='r', linestyle='--', alpha=0.3)
    
    plt.xlabel('Time')
    plt.ylabel('Tasks')
    plt.title(f'Gantt Chart - {title}')
    plt.yticks(y_pos, completed_df['TaskID'])
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'../results/{filename}.png')
    plt.close()

def plot_metrics(mlfq_results, rr_results):
    # Prepare data for comparison
    metrics = ['SLA Compliance', 'Starvation Rate', 'Average Wait Time']
    mlfq_values = [
        mlfq_results['sla_compliance'],
        mlfq_results['starvation'],
        mlfq_results['avg_wait']
    ]
    rr_values = [
        rr_results['sla_compliance'],
        rr_results['starvation'],
        rr_results['avg_wait']
    ]
    
    x = np.arange(len(metrics))
    width = 0.35
    
    fig, ax = plt.subplots(figsize=(10, 6))
    bars1 = ax.bar(x - width/2, mlfq_values, width, label='MLFQ')
    bars2 = ax.bar(x + width/2, rr_values, width, label='Round Robin')
    
    ax.set_xlabel('Metrics')
    ax.set_ylabel('Values')
    ax.set_title('Scheduler Performance Comparison')
    ax.set_xticks(x)
    ax.set_xticklabels(metrics)
    ax.legend()
    
    # Add value labels on bars
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            ax.annotate(f'{height:.1f}',
                        xy=(bar.get_x() + bar.get_width() / 2, height),
                        xytext=(0, 3),
                        textcoords="offset points",
                        ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig('../results/metrics_comparison.png')
    plt.close()

if __name__ == "__main__":
    # Load data
    mlfq_df = pd.read_csv('../cpp_scheduler/build/data/mlfq_results.csv')
    rr_df = pd.read_csv('../cpp_scheduler/build/data/rr_results.csv')
    
    # Create Gantt charts
    create_gantt_chart(mlfq_df, 'MLFQ Scheduler', 'mlfq_gantt')
    create_gantt_chart(rr_df, 'Round Robin Scheduler', 'rr_gantt')
    
    # Calculate metrics for plotting
    from analyze_results import analyze_results
    results = analyze_results('../cpp_scheduler/build/data/mlfq_results.csv', '../cpp_scheduler/build/data/rr_results.csv')
    
    # Plot metrics comparison
    plot_metrics(results['mlfq'], results['rr'])
    
    print("Visualizations saved to results directory")