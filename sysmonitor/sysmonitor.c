#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

/* Function prototypes */
void show_cpu_info();
void show_mem_info();
void show_uptime();
void show_processes();
void print_separator();

/* Main function */
int main() {
    print_separator();
    printf(" LINUX SYSTEM MONITOR - /proc Demo\n");
    print_separator();
    
    show_cpu_info();
    show_mem_info();
    show_uptime();
    show_processes();
    
    print_separator();
    return 0;
}

/* Utility function to print a separator line */
void print_separator() {
    printf("====================================\n");
}

/* Function to display CPU information from /proc/cpuinfo */
void show_cpu_info() {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        perror("Cannot open /proc/cpuinfo");
        return;
    }
    
    char line[256];
    int processor_count = 0;
    int found_model = 0;
    
    printf("\n=== CPU INFORMATION ===\n");
    
    while (fgets(line, sizeof(line), fp)) {
        /* Count processors */
        if (strncmp(line, "processor", 9) == 0) {
            processor_count++;
        }
        
        /* Print model name (only once) */
        if (strncmp(line, "model name", 10) == 0 && !found_model) {
            printf("%s", line);
            found_model = 1;
        }
        
        /* Print CPU cores information */
        if (strncmp(line, "cpu cores", 9) == 0) {
            printf("%s", line);
        }
    }
    
    printf("Total processors: %d\n", processor_count);
    fclose(fp);
}

/* Function to display memory information from /proc/meminfo */
void show_mem_info() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Cannot open /proc/meminfo");
        return;
    }
    
    char line[256];
    printf("\n=== MEMORY INFORMATION ===\n");
    
    /* Print the first 5 lines (MemTotal, MemFree, MemAvailable, Buffers, Cached) */
    for (int i = 0; i < 5 && fgets(line, sizeof(line), fp); i++) {
        printf("%s", line);
    }
    
    fclose(fp);
}

/* Function to display system uptime from /proc/uptime */
void show_uptime() {
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) {
        perror("Cannot open /proc/uptime");
        return;
    }
    
    double uptime_secs, idle_secs;
    if (fscanf(fp, "%lf %lf", &uptime_secs, &idle_secs) != 2) {
        printf("Error reading uptime information\n");
        fclose(fp);
        return;
    }
    
    fclose(fp);
    
    int days = (int)(uptime_secs / 86400);
    int hours = (int)((uptime_secs - days * 86400) / 3600);
    int minutes = (int)((uptime_secs - days * 86400 - hours * 3600) / 60);
    
    printf("\n=== SYSTEM UPTIME ===\n");
    printf("System up for: %d days, %d hours, %d minutes\n", days, hours, minutes);
}

/* Function to list running processes from /proc */
void show_processes() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Cannot open /proc");
        return;
    }
    
    struct dirent *entry;
    printf("\n=== TOP 10 PROCESSES (PID | NAME | STATE) ===\n");
    
    int count = 0;
    while ((entry = readdir(dir)) != NULL && count < 10) {
        /* Check if the entry is a process directory (numeric) */
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            char status_path[64];
            snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);
            
            FILE *fp = fopen(status_path, "r");
            if (!fp) continue;
            
            char name[64] = "";
            char state[16] = "";
            char line[128];
            
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Name:", 5) == 0) {
                    sscanf(line, "Name:\t%s", name);
                }
                if (strncmp(line, "State:", 6) == 0) {
                    sscanf(line, "State:\t%s", state);
                }
            }
            
            fclose(fp);
            printf("%-8s | %-20s | %s\n", entry->d_name, name, state);
            count++;
        }
    }
    
    closedir(dir);
}