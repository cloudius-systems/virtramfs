/*
 * Copyright 2015 Cloudius Systems
 */

#include <sys/mount.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

void domount(const char* source, const char* target,
        const char* fstype, unsigned long flags, const void* options) {
    int r = mount(source, target, fstype, flags, options);
    if (r == -1) {
        printf("Failed to mount %s on %s: %m\n", source, target);
    }
}

int main(int ac, char** av) {
    printf("Virtramfs starting\n");
    FILE* fp = fopen("modules", "r");
    char* line = NULL;
    size_t sz = 0;
    int n;
    while ((n = getline(&line, &sz, fp)) != -1) {
        line[n - 1] = '\0';
        int fd = open(line, O_RDONLY);
        if (fd == -1) {
            printf("Cannot open module file \"%s\": %m\n", line);
            exit(1);
        }
        int r = syscall(SYS_finit_module, fd, "", 0);
        if (r == -1) {
            printf("Cannot load module %s: %m\n", line);
            exit(1);
        }
        close(fd);
    }
    domount("root", "/newroot", "9p", 0, "trans=virtio,version=9p2000.L,cache=none");
    domount("tmpfs", "/newroot/tmp", "tmpfs", 0, "");
    chdir("/newroot");
    domount(".", "/", "none", MS_MOVE, "");
    chroot(".");
    chdir("/");
    domount("proc", "/proc", "proc", 0, "");
    domount("sys", "/sys", "sysfs", 0, "");
    domount("hugetlbfs", "/dev/hugepages", "hugetlbfs", 0, "");
    domount("tmpfs", "/var/run", "tmpfs", 0, "");
    execve("/bin/bash", NULL, NULL);
    return 0;
}
