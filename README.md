# Linux File Integrity Monitor (FIM)

A lightweight, high-performance Host-based Intrusion Detection tool written in C++ for Linux.

## Overview
This tool utilizes the Linux kernel's `inotify` API to monitor changes in a specified directory in real-time. It is designed to detect unauthorized file modifications, deletions, or creations, making it a fundamental tool for Blue Team operations and system administration.

## Features
*   **Real-time monitoring:** Uses `inotify` for instant alerts on file system changes.
*   **Zero CPU overhead:** Utilizes `poll()` to wait for events efficiently without busy-waiting.
*   **Detailed logging:** Captures timestamps, file types (file/directory), and specific events (MODIFY, CREATE, DELETE, ATTRIB).

## Requirements
*   Linux OS (Debian, Ubuntu, Arch, etc.)
*   `g++` compiler
*   Root privileges (recommended for monitoring system directories like `/etc`)

## Compilation & Usage
```bash
g++ monitor.cpp -o monitor -std=c++11
sudo ./monitor /etc