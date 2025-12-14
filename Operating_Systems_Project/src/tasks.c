#include "tasks.h"

/* Her task icin farkli bir renk donduruyor.
   Ciktida hangisi hangisi karismasin diye. */
const char* get_task_color(int id) {
    const char* colors[] = {
        "\x1b[31m", // Kirmizi
        "\x1b[32m", // Yesil
        "\x1b[33m", // Sari
        "\x1b[34m", // Mavi
        "\x1b[35m", // Eflatun
        "\x1b[36m"  // Cyan
    };
    // ID numarasina gore sirayla renk sec
    return colors[id % 6];
}
