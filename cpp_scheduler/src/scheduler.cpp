// scheduler.cpp

#include "scheduler.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
// Required for mkdir on Windows/Linux
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

// --- Task implementation remains the same ---
Task::Task(int id, int arrival, int cpu, int memory, int deadline)
    : id(id), arrival_time(arrival), cpu_required(cpu), 
      memory_required(memory), deadline(deadline),
      state(TaskState::PENDING), remaining_time(cpu),
      start_time(-1), completion_time(-1), current_queue(0),
      wait_time(0), last_run_time(arrival) {}

std::string Task::to_string() const { /* ... */ return "";}
bool Task::operator<(const Task& other) const { return deadline < other.deadline; }
// --- End of Task implementation ---


// --- MLFQScheduler implementation ---
MLFQScheduler::MLFQScheduler(int num_queues, const std::vector<int>& quantums, int boost)
    : queues(num_queues), time_quantums(quantums), 
      boost_interval(boost), current_time(0), total_tasks(0) {
    if (time_quantums.size() != queues.size()) {
        throw std::invalid_argument("Number of time quantums must match number of queues");
    }
}

void MLFQScheduler::add_task(std::shared_ptr<Task> task) {
    if (queues.empty()) return;
    queues[0].push(task);
    total_tasks++;
}

void MLFQScheduler::run() {
    current_time = 0;
    int tasks_processed = 0;
    
    while (tasks_processed < total_tasks) {
        if (boost_interval > 0 && current_time > 0 && current_time % boost_interval == 0) {
            for (int i = 1; i < queues.size(); i++) {
                while (!queues[i].empty()) {
                    queues[0].push(queues[i].front());
                    queues[i].pop();
                }
            }
        }

        bool found_task = false;
        int earliest_arrival = -1;

        for (int i = 0; i < queues.size(); i++) {
            if (queues[i].empty()) continue;
            
            auto task = queues[i].front();
            
            if (task->arrival_time <= current_time) {
                queues[i].pop();
                found_task = true;
                
                int time_quantum = time_quantums[i];
                int time_used = std::min(time_quantum, task->remaining_time);
                
                if (task->start_time == -1) task->start_time = current_time;
                
                current_time += time_used;
                task->remaining_time -= time_used;
                
                if (task->remaining_time <= 0) {
                    task->completion_time = current_time;
                    task->state = (task->completion_time > task->deadline) ? TaskState::MISSED_DEADLINE : TaskState::COMPLETED;
                    completed_tasks.push_back(task);
                    tasks_processed++;
                } else {
                    int next_queue_index = std::min((int)queues.size() - 1, i + 1);
                    queues[next_queue_index].push(task);
                }
                break;
            } else {
                if (earliest_arrival == -1 || task->arrival_time < earliest_arrival) {
                    earliest_arrival = task->arrival_time;
                }
            }
        }
        
        if (!found_task) {
            // If no task is ready, jump time forward to the earliest arrival time.
            current_time = (earliest_arrival > current_time) ? earliest_arrival : current_time + 1;
        }
    }
}


void MLFQScheduler::export_results(const std::string& filename) const {
    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string dir = filename.substr(0, pos);
        MKDIR(dir.c_str());
    }
    
    std::ofstream file(filename);
    file << "TaskID,ArrivalTime,CPURequired,MemoryRequired,Deadline,StartTime,CompletionTime,WaitTime,Status\n";
    
    for (const auto& task : completed_tasks) {
        // FIX: Calculate wait time correctly before exporting.
        int wait_time = (task->completion_time - task->arrival_time) - task->cpu_required;
        if (wait_time < 0) wait_time = 0;

        file << task->id << ","
             << task->arrival_time << ","
             << task->cpu_required << ","
             << task->memory_required << ","
             << task->deadline << ","
             << task->start_time << ","
             << task->completion_time << ","
             << wait_time << "," // Use the correct wait time
             << (task->state == TaskState::COMPLETED ? "COMPLETED" : "MISSED_DEADLINE")
             << "\n";
    }
    file.close();
}

// --- RoundRobinScheduler implementation ---
RoundRobinScheduler::RoundRobinScheduler(int quantum) 
    : time_quantum(quantum), current_time(0), total_tasks(0) {}

void RoundRobinScheduler::add_task(std::shared_ptr<Task> task) {
    task_queue.push(task);
    total_tasks++;
}

void RoundRobinScheduler::run() {
    current_time = 0;
    int tasks_processed = 0;
    
    while (tasks_processed < total_tasks) {
        if (task_queue.empty()) break; // No more tasks to process
        
        auto task = task_queue.front();
        task_queue.pop();
        
        // --- THIS IS THE BUG FIX ---
        // If the task hasn't arrived, jump time forward instead of looping.
        if (task->arrival_time > current_time) {
            current_time = task->arrival_time;
        }
        
        if (task->start_time == -1) {
            task->start_time = current_time;
        }
        
        int time_used = std::min(time_quantum, task->remaining_time);
        current_time += time_used;
        task->remaining_time -= time_used;
        
        if (task->remaining_time <= 0) {
            task->completion_time = current_time;
            task->state = (task->completion_time > task->deadline) ? TaskState::MISSED_DEADLINE : TaskState::COMPLETED;
            completed_tasks.push_back(task);
            tasks_processed++;
        } else {
            task_queue.push(task); // Add back to the end of the queue
        }
    }
}
void RoundRobinScheduler::export_results(const std::string& filename) const {
    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string dir = filename.substr(0, pos);
        MKDIR(dir.c_str());
    }
    
    std::ofstream file(filename);
    file << "TaskID,ArrivalTime,CPURequired,MemoryRequired,Deadline,StartTime,CompletionTime,WaitTime,Status\n";
    
    for (const auto& task : completed_tasks) {
        // FIX: Calculate wait time correctly before exporting.
        int wait_time = (task->completion_time - task->arrival_time) - task->cpu_required;
        if (wait_time < 0) wait_time = 0;

        file << task->id << ","
             << task->arrival_time << ","
             << task->cpu_required << ","
             << task->memory_required << ","
             << task->deadline << ","
             << task->start_time << ","
             << task->completion_time << ","
             << wait_time << "," // Use the correct wait time
             << (task->state == TaskState::COMPLETED ? "COMPLETED" : "MISSED_DEADLINE")
             << "\n";
    }
    file.close();
}

// NOTE: The `print_stats` functions are no longer needed as main.cpp handles the final report.
// You can remove them to reduce code duplication.
void MLFQScheduler::print_stats() const {}
void RoundRobinScheduler::print_stats() const {}