#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

typedef enum direction_t {
    NONE = 0,
    NORTH = 1,
    SOUTH = 2,
    EAST = 4,
    WEST = 8,

    NORTH_EAST = NORTH | EAST,
    NORTH_WEST = NORTH | WEST,
    SOUTH_EAST = SOUTH | EAST,
    SOUTH_WEST = SOUTH | WEST,
    NORTH_SOUTH = NORTH | SOUTH,
    EAST_WEST = EAST | WEST,
    NORTH_SOUTH_EAST = NORTH | SOUTH | EAST,
    NORTH_SOUTH_WEST = NORTH | SOUTH | WEST,
    NORTH_EAST_WEST = NORTH | EAST | WEST,
    SOUTH_EAST_WEST = SOUTH | EAST | WEST,
    NORTH_SOUTH_EAST_WEST = NORTH | SOUTH | EAST | WEST,
} direction_t;

typedef struct dir_offset_t {
    ptrdiff_t x;
    ptrdiff_t y;
} dir_offset_t;

size_t constexpr MAZE_WIDTH = 32;
size_t constexpr MAZE_HEIGHT = 32;
direction_t maze[MAZE_HEIGHT][MAZE_WIDTH];
size_t filled = 0;

unsigned long xorshift_state = 0;

void print_maze(void);

unsigned long xorshift(void) {
    unsigned long x = xorshift_state;
    x ^= x << 7;
    x ^= x >> 9;
    return xorshift_state = x;
}

direction_t get_rand_direction(void) {
    unsigned long rand = xorshift() % 4;
    return (direction_t) 1 << rand;
}

direction_t rotate_clockwise(direction_t current_direction) {
    switch (current_direction) {
        case NORTH:
            return EAST;
        case EAST:
            return SOUTH;
        case SOUTH:
            return WEST;
        case WEST:
            return NORTH;
        case NONE:
        default:
            return get_rand_direction();
    }
}

direction_t rotate_counterclockwise(direction_t current_direction) {
    switch (current_direction) {
        case NORTH:
            return WEST;
        case WEST:
            return SOUTH;
        case SOUTH:
            return EAST;
        case EAST:
            return NORTH;
        case NONE:
        default:
            return get_rand_direction();
    }
}

direction_t rotate_180(direction_t current_direction) {
    return rotate_clockwise(rotate_clockwise(current_direction));
}

direction_t get_next_direction(direction_t current_direction, direction_t blocked_mask) {
    unsigned long rand = xorshift() % 16;
    if (rand < 12 && !(current_direction & blocked_mask)) {
        return current_direction;
    } else if ((rand < 14 && !(rotate_clockwise(current_direction) & blocked_mask)) || (rand < 12 && !(rotate_clockwise(current_direction) & blocked_mask))) {
        return rotate_clockwise(current_direction);
    } else if ((rand < 16 && !(rotate_counterclockwise(current_direction) & blocked_mask)) || (rand < 14 && !(rotate_counterclockwise(current_direction) & blocked_mask))) {
        return rotate_counterclockwise(current_direction);
    } else if (rotate_180(current_direction) & blocked_mask) {
        return rotate_180(current_direction);
    }
    direction_t direction = get_rand_direction();
    while (blocked_mask & direction) {
        direction = get_rand_direction();
    }
    return direction;
}

dir_offset_t get_offset(direction_t current_direction) {
    switch (current_direction) {
        case NONE:
        default:
            return (dir_offset_t){0, 0};
        case NORTH:
            return (dir_offset_t){0, -1};
        case SOUTH:
            return (dir_offset_t){0, 1};
        case EAST:
            return (dir_offset_t){1, 0};
        case WEST:
            return (dir_offset_t){-1, 0};
    }
}

void fill_cell(size_t x, size_t y, direction_t current_direction) {
    if (current_direction == NONE)
        return;
    maze[y][x] = current_direction;
    filled++;
}

void add_to_cell(size_t x, size_t y, direction_t current_direction) {
    if (current_direction == NONE)
        return;
    if (maze[y][x] == NONE)
        filled++;
    maze[y][x] |= current_direction;
}

bool make_path(ptrdiff_t x, ptrdiff_t y, direction_t use_direction, bool fill_edge) { //return true if hit an edge, false if hit a tile
    direction_t current_direction;
    if (use_direction != NONE) {
        current_direction = use_direction;
    } else {
        current_direction = get_rand_direction();
    }
    direction_t blocked_mask = rotate_180(current_direction);
    while (true) {
        dir_offset_t offset = get_offset(current_direction);
        ptrdiff_t new_x = x + offset.x;
        ptrdiff_t new_y = y + offset.y;
        if (new_x < 0 || new_x >= MAZE_WIDTH || new_y < 0 || new_y >= MAZE_HEIGHT) {
            if (fill_edge) {
                add_to_cell(x, y, current_direction);
                // print_maze();
                return true;
            }
            blocked_mask |= current_direction;
            current_direction = get_next_direction(current_direction, blocked_mask);
            continue;
        }
        bool cell_filled = maze[new_y][new_x] != NONE;
        add_to_cell(x, y, current_direction);
        add_to_cell(new_x, new_y, rotate_180(current_direction));
        if (cell_filled) {
            // print_maze();
            return false;
        }

        x = new_x;
        y = new_y;
        blocked_mask = rotate_180(current_direction);
        current_direction = get_next_direction(current_direction, blocked_mask);
    }
}

bool first_run(void) {
    memset(maze, 0, sizeof(maze));
    filled = 0;
    direction_t current_direction = SOUTH;
    ptrdiff_t x = xorshift() % MAZE_WIDTH;
    ptrdiff_t y = 0;
    add_to_cell(x, y, rotate_180(current_direction));
    return make_path(x, y, current_direction, true);

    // bool exited = false;
    // while (!exited) {
    //     add_to_cell(x, y, current_direction);
    //     add_to_cell(x, y, rotate_180(current_direction));
    //
    //     direction_t next_direction = get_next_direction(current_direction);
    //     dir_offset_t offset = get_offset(next_direction);
    //     x += offset.x;
    //     y += offset.y;
    //     if (x < 0 || x >= MAZE_WIDTH || y < 0 || y >= MAZE_HEIGHT) {
    //         exited = true;
    //     }
    //     if (maze[y][x] != NONE) {
    //         return false;
    //     }
    //     current_direction = next_direction;
    // }

    return true;
}

void print_maze(void) {
    // printf("╔");
    // for (size_t x = 1; x <= MAZE_WIDTH; x++)
    //     printf("═");
    // printf("╗\n");
    printf(" ");
    for (size_t x = 0; x < MAZE_WIDTH; x++) {
        if (maze[0][x] & NORTH) {
            printf("V");
        } else {
            printf(" ");
        }
    }
    printf("\n");

    for (size_t y = 0; y < MAZE_HEIGHT; y++) {
        // printf("║");
        if (maze[y][0] & WEST) {
            printf(">");
        } else {
            printf(" ");
        }
        for (size_t x = 0; x < MAZE_WIDTH; x++) {
            switch (maze[y][x]) {
                case NONE:
                    printf("X");
                    break;
                case NORTH:
                    printf("╹");
                    break;
                case SOUTH:
                    printf("╻");
                    break;
                case EAST:
                    printf("╺");
                    break;
                case WEST:
                    printf("╸");
                    break;
                case NORTH_EAST:
                    printf("┗");
                    break;
                case NORTH_WEST:
                    printf("┛");
                    break;
                case SOUTH_EAST:
                    printf("┏");
                    break;
                case SOUTH_WEST:
                    printf("┓");
                    break;
                case NORTH_SOUTH:
                    printf("┃");
                    break;
                case EAST_WEST:
                    printf("━");
                    break;
                case NORTH_SOUTH_EAST:
                    printf("┣");
                    break;
                case NORTH_SOUTH_WEST:
                    printf("┫");
                    break;
                case NORTH_EAST_WEST:
                    printf("┻");
                    break;
                case SOUTH_EAST_WEST:
                    printf("┳");
                    break;
                case NORTH_SOUTH_EAST_WEST:
                    printf("╋");
                    break;
            }
        }
        // printf("║");
        if (maze[y][MAZE_WIDTH - 1] & EAST) {
            printf("<");
        } else {
            printf(" ");
        }
        printf("\n");
    }
    // printf("╚");
    // for (size_t x = 1; x <= MAZE_WIDTH; x++)
    //     printf("═");
    // printf("╝\n");
    printf(" ");
    for (size_t x = 0; x < MAZE_WIDTH; x++) {
        if (maze[MAZE_HEIGHT - 1][x] & SOUTH) {
            printf("^");
        } else {
            printf(" ");
        }
    }
    printf("\n");
    // printf("filled: %zu vs total %zu\n", filled, MAZE_WIDTH * MAZE_HEIGHT);
}

int main(void) {
    while (xorshift_state == 0) {
        xorshift_state = time(nullptr);
        // xorshift_state = 4;
    }

    while (!first_run());
    // print_maze();
    while (filled < MAZE_WIDTH * MAZE_HEIGHT) {
        ptrdiff_t x = 0;
        ptrdiff_t y = 0;
        for (y = 0; y < MAZE_HEIGHT; y++) {
            for (x = 0; x < MAZE_WIDTH; x++) {
                if (maze[y][x] == NONE) {
                    goto found_empty;
                }
            }
        }
found_empty:
        make_path(x, y, NONE, false);
    }
    print_maze();
}
