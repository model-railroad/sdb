#pragma once

struct QueueDefinition {
    QueueDefinition() : counter(0) { }
    int counter;
};
typedef struct QueueDefinition   * QueueHandle_t;

