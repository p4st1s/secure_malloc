#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <alloca.h>
#include <string.h>
#include <fcntl.h> 
#include <stdint.h> 
#include <stdint.h> 

#include "utils.h"

#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[34m"

#define LOG_INFO "[INFO]"
#define LOG_DEBUG "[DEBUG]"
#define LOG_ERROR "[ERROR]"

int log_fd =-1; // Default log file descriptor
int log_lvl = 0;
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
        if (log_lvl<1)
            return;   
        color = get_log_color(LOG_INFO);
    }
    else if (strstr(buffer, LOG_DEBUG) != NULL)
    {
        if (log_lvl<3)
            return;
        color = get_log_color(LOG_DEBUG);
    }
    else if (strstr(buffer, LOG_ERROR) != NULL)
    {
        if (log_lvl<2)
            return;
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
    log_fd= open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd == -1)
        return -1;
    return log_fd;
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
        log_fd = -1;
        return;
    }


    // If stdout, set the file descriptor to stdout
    if (strcmp(path, "stdout") == 0)
    {

        log_fd = STDOUT_FILENO;
    }

    const char *lvl=getenv("LOG_LVL");
    if(lvl!=NULL){
        if(strcmp(lvl,"INFO")==0){
            log_lvl=1;
        }
        else if(strcmp(lvl,"ERROR")==0){
            log_lvl=2;
        }
        else if(strcmp(lvl,"DEBUG")==0){
            log_lvl=3;
        }
    }

    // If already opened, return
    if (log_fd != -1){
        return;

    }

    else{
        my_log("[INFO] Creating log file at %s", path);
        log_fd = create_log_file(path);
    }

}

/**
 * @brief Close the log file.
 *
 */
void close_logging()
{
    if (log_fd == -1)
        return;

    my_log("[INFO] Closing log file\n");

    close(log_fd);
}


/**
 * Generates a random canary value.
 *
 * This function opens the /dev/urandom device file and reads 4 bytes from it to generate a random canary value.
 * The canary value is used as a security measure to detect buffer overflows by placing it before the allocated memory
 * and checking if it has been modified before freeing the memory.
 *
 * @return The generated random canary value.
 *         If an error occurs during the generation process, 0 is returned.
 */
uint32_t get_random_canary()
{
    // Open /dev/urandom
    uint32_t fd = open("/dev/urandom", O_RDONLY);
    if (fd == (uint32_t)-1)
        return 0;

    // Read 4 bytes from /dev/urandom
    uint32_t random = 0;
    if (lseek(fd, -4, SEEK_END) == -1)
        return 0;

    // Put the random value in the canary
    if (read(fd, &random, sizeof(random)) == -1)
        return 0;

    close(fd);

    return random;
}