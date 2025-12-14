#include "tasks.h"

const char* get_task_color(int id) {
    const char* colors[] = {
        "\x1b[31m",
        "\x1b[32m",
        "\x1b[33m",
        "\x1b[34m",
        "\x1b[35m",
        "\x1b[36m"
    };
    return colors[id % 6];
}
