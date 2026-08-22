#include "domain/scheduling/TaskQueue.h"

#include <gtest/gtest.h>

using fleetsim::core::Task;
using fleetsim::core::TaskStatus;
using fleetsim::domain::scheduling::TaskQueue;

TEST(TaskQueueTest, PendingAndAssign)
{
    TaskQueue queue;
    Task task;
    task.id = "task_0";
    task.pickup = {1.0, 2.0, 0.0};
    task.dropoff = {5.0, 6.0, 0.0};
    queue.addTask(task);

    EXPECT_EQ(queue.pendingCount(), 1);
    ASSERT_TRUE(queue.markAssigned("task_0", "agv_0"));
    EXPECT_EQ(queue.pendingCount(), 0);
    EXPECT_EQ(queue.assignedCount(), 1);
}

TEST(TaskQueueTest, MarkDone)
{
    TaskQueue queue;
    Task task;
    task.id = "task_1";
    queue.addTask(task);
    queue.markAssigned("task_1", "agv_1");
    ASSERT_TRUE(queue.markDone("task_1"));
    EXPECT_EQ(queue.doneCount(), 1);
}
