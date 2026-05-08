#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

size_t tick = 0;
size_t world_width;
size_t world_height;
bool* world;
bool* world_new;

bool get_cell(ptrdiff_t x, ptrdiff_t y) {
    if (x < 0) {
        x = world_width - 1;
    } else if (x >= world_width) {
        x = 0;
    }
    if (y < 0) {
        y = world_height - 1;
    } else if (y >= world_height) {
        y = 0;
    }
    return world[y * world_width + x];
}

bool iterate_world(void) {
    tick++;
    bool all_dead = true;
    for (ptrdiff_t y = 0; y < world_height; y++) {
        for (ptrdiff_t x = 0; x < world_width; x++) {
            if (world[y * world_width + x]) {
                all_dead = false;
            }
            int alive_neighbors = 0;
            ptrdiff_t const offsets[8][2] = {
                {-1, -1}, {0, -1}, {1, -1},
                {-1, 0}, /*{0, 0},*/ {1, 0},
                {-1, 1}, {0, 1}, {1, 1},
            };
            for (size_t i = 0; i < 8; i++) {
                bool neighbor = get_cell(x + offsets[i][0], y + offsets[i][1]);
                if (neighbor) {
                    alive_neighbors++;
                }
            }
            if (alive_neighbors < 2 || alive_neighbors > 3) {
                world_new[y * world_width + x] = false;
            } else if (alive_neighbors == 2) {
                world_new[y * world_width + x] = world[y * world_width + x];
            }
            else if (alive_neighbors == 3) {
                world_new[y * world_width + x] = true;
                all_dead = false;
            }
        }
    }
    memcpy(world, world_new, sizeof(bool) * world_height * world_width);
    return !all_dead;
}

void print_world(void) {
    printf("Tick %zu\n", tick);
    for (ptrdiff_t y = 0; y < world_height; y++) {
        for (ptrdiff_t x = 0; x < world_width; x++) {
            printf("%c", world[y * world_width + x] ? 'X' : ' ');
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    printf("Enter world width: ");
    scanf("%zu", &world_width);
    printf("Enter world height: ");
    scanf("%zu", &world_height);
    // world_width = 4;
    // world_height = 4;
    world = calloc(sizeof(bool), world_height * world_width);
    world_new = calloc(sizeof(bool), world_height * world_width);

    // world[0 * world_width + 1] = true;
    // world[1 * world_width + 1] = true;
    // world[2 * world_width + 1] = true;
    srand(time(nullptr));
    for (size_t y = 0; y < world_height; y++) {
        for (size_t x = 0; x < world_width; x++) {
            world[y * world_width + x] = rand() % 2;
        }
    }

    print_world();
    while (iterate_world()) {
        print_world();
        if (tick % 1000 == 0) {
            break;
        }
    }
    // print_world();
}
