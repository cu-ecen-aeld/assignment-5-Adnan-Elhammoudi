#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file_path> <string_to_write>\n", argv[0]);
        return 1;
    }

    char *file_path = argv[1];
    char *string_to_write = argv[2];

    openlog(argv[0], LOG_PID, LOG_USER); // Open syslog with program name

    int fd = open(file_path, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        syslog(LOG_ERR, "Error opening file: %m"); // %m prints strerror(errno)
        perror("Error opening file");
        closelog();
        return 1;
    }

    ssize_t bytes_written = write(fd, string_to_write, strlen(string_to_write));
    if (bytes_written == -1) {
        syslog(LOG_ERR, "Error writing to file: %m");
        perror("Error writing to file");
        close(fd);
        closelog();
        return 1;
    }

    if (close(fd) == -1) {
        syslog(LOG_ERR, "Error closing file: %m");
        perror("Error closing file");
        closelog();
        return 1;
    }

    syslog(LOG_DEBUG, "Writing '%s' to %s", string_to_write, file_path);
    printf("String was successfully written to the file.\n");

    closelog();
    return 0;
}
