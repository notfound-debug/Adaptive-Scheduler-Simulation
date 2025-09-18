# python_analysis/tune_parameters.py
import subprocess
import os
import pandas as pd
from analyse_results import analyze_results

def tune_parameters():
    best_params = None
    best_improvement = 0
    
    # Test different parameter combinations
    for boost_interval in [30, 50, 70, 100]:
        for q1_quantum in [5, 10, 15]:
            for q2_quantum in [10, 20, 30]:
                print(f"Testing boost={boost_interval}, quantums=[{q1_quantum}, {q2_quantum}, {q2_quantum*2}]")
                
                # Modify the C++ code with these parameters
                modify_cpp_code(boost_interval, [q1_quantum, q2_quantum, q2_quantum*2])
                
                # Rebuild and run the C++ scheduler
                os.chdir('../cpp_scheduler/build')
                subprocess.run(['cmake', '--build', '.'])
                subprocess.run(['./scheduler'])
                os.chdir('../../python_analysis')
                
                # Analyze results
                results = analyze_results('../cpp_scheduler/data/mlfq_results.csv', '../cpp_scheduler/data/rr_results.csv')
                
                # Calculate improvement
                starvation_reduction = ((results['rr']['starvation'] - results['mlfq']['starvation']) / 
                                       results['rr']['starvation']) * 100
                sla_improvement = ((results['mlfq']['sla_compliance'] - results['rr']['sla_compliance']) / 
                                  results['rr']['sla_compliance']) * 100
                
                total_improvement = starvation_reduction + sla_improvement
                
                if total_improvement > best_improvement:
                    best_improvement = total_improvement
                    best_params = (boost_interval, [q1_quantum, q2_quantum, q2_quantum*2])
                    print(f"New best: starvation reduction={starvation_reduction:.2f}%, "
                          f"SLA improvement={sla_improvement:.2f}%")
    
    print(f"\nBest parameters: boost_interval={best_params[0]}, quantums={best_params[1]}")
    return best_params

def modify_cpp_code(boost_interval, quantums):
    # This function would modify the C++ code with the new parameters
    # Implementation depends on how you structure your code for parameter tuning
    pass

if __name__ == "__main__":
    tune_parameters()