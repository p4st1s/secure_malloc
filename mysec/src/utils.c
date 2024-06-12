#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <alloca.h>
#include <string.h>

#include "utils.h"

#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[34m"

#define LOG_INFO "[INFO]"
#define LOG_DEBUG "[DEBUG]"
#define LOG_ERROR "[ERROR]"

int log_fd = -1; // Default log file descriptor

/**
 * Returns the color associated with a given log level.
 *
 * @param log_level The log level for which to retrieve the color.
 * @return The color associated with the log level.
 */
const char *get_log_color(const char *log_level)
{
    if (strcmp(log_level, LOG_INFO) == 0)
    {
        return COLOR_GREEN;
    }
    else if (strcmp(log_level, LOG_DEBUG) == 0)
    {
        return COLOR_YELLOW;
    }
    else if (strcmp(log_level, LOG_ERROR) == 0)
    {
        return COLOR_RED;
    }
    else
    {
        return COLOR_RESET;
    }
}

/**
 * @brief general function to print without using printf
 * @param fmt format of the string
 * @param ... arguments
 */
void my_log(const char *fmt, ...)
{
    va_list ap_list;
    char buffer[1024]; // Tampon statique pour les messages de log
    va_start(ap_list, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap_list);
    va_end(ap_list);

    const char *color = COLOR_RESET;

    if (strstr(buffer, LOG_INFO) != NULL)
    {
        color = get_log_color(LOG_INFO);
    }
    else if (strstr(buffer, LOG_DEBUG) != NULL)
    {
        color = get_log_color(LOG_DEBUG);
    }
    else if (strstr(buffer, LOG_ERROR) != NULL)
    {
        color = get_log_color(LOG_ERROR);
    }

    // Utiliser write pour éviter malloc interne de fprintf
    write(log_fd, color, strlen(color));
    write(log_fd, buffer, strlen(buffer));
    write(log_fd, COLOR_RESET, strlen(COLOR_RESET));
}



/**
 * @brief Create a log file with the specified path
 * If the file already exists, it will be overwritten.
 * The file descriptor is returned.
 * If the file cannot be created, -1 is returned.
 *
 * @param path pointer to the path of the log file
 * @return file descriptor
 */
int create_log_file(const char *path)
{
    FILE *log_file = fopen(path, "w");
    if (log_file == NULL)
        return -1;

    return log_file->_fileno;
}

/**
 * @brief Initialize the log file.
 * Handles the presence of the MSM_OUTPUT environment variable
 * and creates the log file accordingly.
 *
 */
void init_logging()
{
    // Get the path from the environment variable
    const char *path = getenv("MSM_OUTPUT");
    // write(STDOUT_FILENO, path, strlen(path));
    if (path == NULL)
    {
        log_fd = DEACTIVATE_LOGGING;
        return;
    }

    // If stdout, set the file descriptor to stdout
    if (strcmp(path, "stdout") == 0)
    {
        log_fd = STDOUT_FILENO;
        return;
    }

    // If already opened, return
    if (log_fd != -1)
        return;

    log_fd = create_log_file(path);
    if (log_fd == -1)
    {
        my_log("[ERROR] Failed to create log file");
        return;
    }
}

/**
 * @brief Close the log file.
 *
 */
void close_logging()
{
    if (log_fd == DEACTIVATE_LOGGING)
        return;

    my_log("[INFO] Closing log file");

    close(log_fd);
}