#include <iostream>
#include <string>
#include <vector>
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <poll.h>
#include <cstring>
#include <ctime>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

std::string GetEventName(uint32_t mask) {
    if (mask & IN_ACCESS)        return "FILE ACCESSED";
    if (mask & IN_MODIFY)        return "FILE MODIFIED";
    if (mask & IN_ATTRIB)        return "METADATA CHANGED";
    if (mask & IN_CLOSE_WRITE)   return "FILE CLOSED AFTER WRITE";
    if (mask & IN_CLOSE_NOWRITE) return "FILE CLOSED WITHOUT WRITE";
    if (mask & IN_OPEN)          return "FILE OPENED";
    if (mask & IN_MOVED_FROM)    return "FILE MOVED FROM";
    if (mask & IN_MOVED_TO)      return "FILE MOVED TO";
    if (mask & IN_CREATE)        return "FILE CREATED";
    if (mask & IN_DELETE)        return "FILE DELETED";
    if (mask & IN_DELETE_SELF)   return "WATCHED FILE DELETED";
    if (mask & IN_MOVE_SELF)     return "WATCHED FILE MOVED";
    return "UNKNOWN EVENT";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        std::cerr << "Example: sudo ./monitor /etc" << std::endl;
        return 1;
    }

    std::string pathToWatch = argv[1];
    
    std::cout << "===========================================\n";
    std::cout << "    LINUX FILE INTEGRITY MONITOR (FIM)     \n";
    std::cout << "===========================================\n\n";
    std::cout << "[*] Starting monitor on directory: " << pathToWatch << "\n";

    int fd = inotify_init();
    if (fd < 0) {
        std::cerr << "[-] Failed to initialize inotify. Try running as root (sudo)." << std::endl;
        return 1;
    }

    int wd = inotify_add_watch(fd, pathToWatch.c_str(), 
        IN_MODIFY | IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MOVED_FROM | IN_MOVED_TO);
        
    if (wd < 0) {
        std::cerr << "[-] Failed to watch directory: " << pathToWatch << std::endl;
        close(fd);
        return 1;
    }

    std::cout << "[+] Watch established. Waiting for events... (Press Ctrl+C to stop)\n\n";

    char buffer[BUF_LEN];
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (true) {
        int poll_num = poll(&pfd, 1, -1); 
        
        if (poll_num == -1) {
            std::cerr << "[-] Poll error." << std::endl;
            break;
        }

        if (poll_num > 0) {
            ssize_t length = read(fd, buffer, BUF_LEN);
            if (length < 0) {
                std::cerr << "[-] Read error." << std::endl;
                break;
            }

            int i = 0;
            while (i < length) {
                struct inotify_event *event = (struct inotify_event *) &buffer[i];
                
                if (event->len) {
                    std::string action = GetEventName(event->mask);
                    
                    char timeStr[100];
                    time_t now = time(0);
                    struct tm tstruct;
                    localtime_r(&now, &tstruct);
                    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tstruct);

                    std::cout << "[" << timeStr << "] [" << (event->mask & IN_ISDIR ? "DIR " : "FILE") << "] ";
                    std::cout << action << " -> " << event->name << std::endl;
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}