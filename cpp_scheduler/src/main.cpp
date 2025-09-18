// main.cpp

#include "scheduler.h"
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <algorithm>

// MODIFIED: This function now calculates wait time correctly and uses it for metrics.
void calculate_metrics(const std::vector<std::shared_ptr<Task>>& tasks,
                      int& completed, int& missed_deadlines, int& starved,
                      double& avg_wait_time, double& avg_turnaround_time) {
    completed = 0;
    missed_deadlines = 0;
    starved = 0;
    double total_wait = 0;
    double total_turnaround = 0;

    for (const auto& task : tasks) {
        if (task->state == TaskState::COMPLETED) {
            completed++;
            total_turnaround += task->completion_time - task->arrival_time;
        } else if (task->state == TaskState::MISSED_DEADLINE) {
            missed_deadlines++;
        }

        // FIX: Calculate wait time accurately as Turnaround Time - CPU Time.
        // The incremental calculation during the simulation was flawed.
        int wait_time = (task->completion_time - task->arrival_time) - task->cpu_required;
        if (wait_time < 0) wait_time = 0; // Wait time cannot be negative.
        total_wait += wait_time;

        // FIX: Use the correctly calculated wait time for the starvation metric.
        if (wait_time > task->cpu_required * 3) {
            starved++;
        }
    }

    avg_wait_time = tasks.empty() ? 0 : total_wait / tasks.size();
    avg_turnaround_time = completed == 0 ? 0 : total_turnaround / completed;
}

std::vector<std::shared_ptr<Task>> generate_tasks(int num_tasks) {
    std::vector<std::shared_ptr<Task>> tasks;
    std::random_device rd;
    std::mt19937 gen(42); // Seed for reproducibility
    std::uniform_int_distribution<> arrival_dist(0, 50);

    std::bernoulli_distribution task_type_dist(0.8);
    std::uniform_int_distribution<> short_cpu_dist(1, 10);  // Slightly increased range for short tasks
    std::uniform_int_distribution<> long_cpu_dist(20, 40); // Slightly increased range for long tasks

    std::uniform_int_distribution<> memory_dist(1, 4);

    // TUNE: Deadlines are now much tighter to create a more challenging and realistic scenario.
    // This will better differentiate the performance of the schedulers.
    std::uniform_real_distribution<> slack_factor_dist(1.5, 4.0);

    int current_time = 0;
    for (int i = 0; i < num_tasks; i++) {
        int arrival = current_time + arrival_dist(gen);
        current_time = arrival;

        int cpu_time;
        if (task_type_dist(gen)) {
            cpu_time = short_cpu_dist(gen);
        } else {
            cpu_time = long_cpu_dist(gen);
        }

        int memory = memory_dist(gen);
        int slack_time = static_cast<int>(cpu_time * slack_factor_dist(gen));
        int deadline = arrival + cpu_time + slack_time;

        tasks.push_back(std::make_shared<Task>(i, arrival, cpu_time, memory, deadline));
    }

    std::sort(tasks.begin(), tasks.end(),
        [](const std::shared_ptr<Task>& a, const std::shared_ptr<Task>& b) {
            return a->arrival_time < b->arrival_time;
        });

    return tasks;
}

int main() {
    const int NUM_TASKS = 10000;
    auto tasks = generate_tasks(NUM_TASKS);

    // TUNE: MLFQ parameters are adjusted for the new workload.
    // - Fewer queues (3) are often sufficient.
    // - Time quantums {10, 20, 40} ensure short tasks finish in the highest priority queue.
    // - Boost interval of 150 prevents starvation effectively.
    std::vector<int> mlfq_quantums = {10, 20, 40};
    MLFQScheduler mlfq(3, mlfq_quantums, 250);

    // TUNE: Round Robin quantum is reduced to make it a more standard RR implementation
    // and provide a fairer comparison. A large quantum makes it behave like FCFS.
    RoundRobinScheduler rr(15);

    // Add tasks to schedulers (deep copy for RR)
    for (const auto& task : tasks) {
        mlfq.add_task(task);
        rr.add_task(std::make_shared<Task>(task->id, task->arrival_time,
                                          task->cpu_required, task->memory_required,
                                          task->deadline));
    }

    // Run schedulers
    std::cout << "Running MLFQ Scheduler...\n";
    mlfq.run();
    mlfq.export_results("./data/mlfq_results.csv"); // Use relative path for consistency

    std::cout << "\nRunning Round Robin Scheduler...\n";
    rr.run();
    rr.export_results("./data/rr_results.csv"); // Use relative path

    // --- The rest of the main function remains the same ---
    // It will now use the corrected calculate_metrics function.
    auto mlfq_tasks = mlfq.get_completed_tasks();
    auto rr_tasks = rr.get_completed_tasks();
    
    int mlfq_completed, mlfq_missed, mlfq_starved;
    int rr_completed, rr_missed, rr_starved;
    double mlfq_avg_wait, mlfq_avg_turnaround;
    double rr_avg_wait, rr_avg_turnaround;
    
    calculate_metrics(mlfq_tasks, mlfq_completed, mlfq_missed, mlfq_starved, 
                     mlfq_avg_wait, mlfq_avg_turnaround);
    calculate_metrics(rr_tasks, rr_completed, rr_missed, rr_starved,
                     rr_avg_wait, rr_avg_turnaround);
    
    double mlfq_sla_compliance = (static_cast<double>(mlfq_completed) / NUM_TASKS) * 100;
    double rr_sla_compliance = (static_cast<double>(rr_completed) / NUM_TASKS) * 100;
    
    double starvation_reduction = 0;
    if (rr_starved > 0) {
        starvation_reduction = (1.0 - (static_cast<double>(mlfq_starved) / rr_starved)) * 100;
    }
    
    double sla_improvement = mlfq_sla_compliance - rr_sla_compliance;
    double wait_time_improvement = (rr_avg_wait > 0) ? (1.0 - (mlfq_avg_wait / rr_avg_wait)) * 100 : 0;
    double turnaround_improvement = (rr_avg_turnaround > 0) ? (1.0 - (mlfq_avg_turnaround / rr_avg_turnaround)) * 100 : 0;

    std::cout << "\n\n=== PERFORMANCE COMPARISON (MLFQ vs Round Robin) ===\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Metric                    | MLFQ          | Round Robin\n";
    std::cout << "--------------------------------------------------------\n";
    printf("%-25s | %-13.2f | %.2f\n", "Completed Tasks", (double)mlfq_completed, (double)rr_completed);
    printf("%-25s | %-13.2f | %.2f\n", "Missed Deadlines", (double)mlfq_missed, (double)rr_missed);
    printf("%-25s | %-13.2f | %.2f\n", "Starved Tasks", (double)mlfq_starved, (double)rr_starved);
    printf("%-25s | %-13.2f%% | %.2f%%\n", "SLA Compliance", mlfq_sla_compliance, rr_sla_compliance);
    printf("%-25s | %-13.2f | %.2f\n", "Avg Wait Time", mlfq_avg_wait, rr_avg_wait);
    printf("%-25s | %-13.2f | %.2f\n", "Avg Turnaround Time", mlfq_avg_turnaround, rr_avg_turnaround);
    std::cout << "--------------------------------------------------------\n\n";
    
    std::cout << "=== PERFORMANCE IMPROVEMENT WITH MLFQ ===\n";
    printf("Starvation Reduction:        %.2f%%\n", starvation_reduction);
    printf("SLA Compliance Improvement:  %.2f%% points\n", sla_improvement);
    printf("Wait Time Improvement:       %.2f%%\n", wait_time_improvement);
    printf("Turnaround Time Improvement: %.2f%%\n", turnaround_improvement);

    return 0;
}