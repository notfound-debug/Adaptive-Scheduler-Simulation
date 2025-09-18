#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <memory>
#include <vector>
#include <queue>
#include <string>

enum class TaskState {
    PENDING,
    RUNNING,
    COMPLETED,
    MISSED_DEADLINE
};

class Task {
public:
    int id;
    int arrival_time;
    int cpu_required;
    int memory_required;
    int deadline;
    TaskState state;
    int remaining_time;
    int start_time;
    int completion_time;
    int current_queue;
    int wait_time;
    int last_run_time;

    Task(int id, int arrival, int cpu, int memory, int deadline);
    std::string to_string() const;
    bool operator<(const Task& other) const;
};

class MLFQScheduler {
private:
    std::vector<std::queue<std::shared_ptr<Task>>> queues;
    std::vector<int> time_quantums;
    int boost_interval;
    int current_time;
    int total_tasks;
    std::vector<std::shared_ptr<Task>> completed_tasks;

public:
    MLFQScheduler(int num_queues, const std::vector<int>& quantums, int boost);
    void add_task(std::shared_ptr<Task> task);
    void run();
    //std::vector<std::shared_ptr<Task>> get_completed_tasks();
    void print_stats() const;
    void export_results(const std::string& filename) const;
    std::vector<std::shared_ptr<Task>> get_completed_tasks() { return completed_tasks; }
};

class RoundRobinScheduler {
private:
    std::queue<std::shared_ptr<Task>> task_queue;
    int time_quantum;
    int current_time;
    int total_tasks;
    std::vector<std::shared_ptr<Task>> completed_tasks;

public:
    RoundRobinScheduler(int quantum);
    void add_task(std::shared_ptr<Task> task);
    void run();
    //std::vector<std::shared_ptr<Task>> get_completed_tasks();
    void print_stats() const;
    void export_results(const std::string& filename) const;
    std::vector<std::shared_ptr<Task>> get_completed_tasks() { return completed_tasks; }
};

#endif