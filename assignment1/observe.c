#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>

int getProcesser(char** array) {
  int i = 0;
  FILE *fin = fopen("/proc/cpuinfo", "r");
  char line[256];
  while (fgets(line, sizeof line, fin) != NULL) /* read a line */
  {
    if (strncmp(line, "model name", 10) == 0) {
      array[i] = malloc(256);
      strcpy(array[i], line+13);
      i++;
    }
  }
  fclose(fin);
  return i;
}

char* getVersion() {
  char tempStr[100];
  char tempStr2[100];
  char* version = malloc(100);
  FILE *fin = fopen("/proc/version", "r");
  fscanf(fin, "%s %s %s", tempStr, tempStr2, version);
  fclose(fin);
  return version;
}

int getMemory() {
  char tempStr[100];
  int memory;
  FILE *fin = fopen("/proc/meminfo", "r");
  fscanf(fin, "%s\t%d", tempStr, &memory);
  fclose(fin);
  return memory;
}

char* getUpTime() {
  float upTime;
  int sec, min, hour, day;
  char* upTimeStr = malloc(16);
  FILE *fin = fopen("/proc/uptime", "r");
  fscanf(fin, "%f", &upTime);
  fclose(fin);
  sec = (int)upTime % 60;
  min = (int)upTime / 60;
  hour = min / 60;
  min = min % 60;
  day = hour / 24;
  hour = hour % 24;
  snprintf(upTimeStr, 16, "%02dD:%02dH:%02dM:%02dS", day, hour, min, sec);
  return upTimeStr;
}

void printProcessesByUid(const int uid) {
  DIR *dir = opendir("/proc");
  struct dirent *dent;
  while (dent = readdir(dir)) {
    if (atoi(dent->d_name)) {
      char dest[256];
      strcpy(dest, "/proc/");
      strcat(dest, dent->d_name);
      strcat(dest, "/status");
      FILE *fin = fopen(dest, "r");
      char line[256];
      char name[256];
      int _uid = -1;
      while (fgets(line, sizeof line, fin) != NULL) /* read a line */
      {
        if (strncmp(line, "Name", 4) == 0) {
          strcpy(name, line+6);
        }
        else if (strncmp(line, "Uid", 3) == 0) {
          char temp[256];
          int real, effective, saved_set, file_system_uids;
          sscanf(line, "%s %d %d %d %d", temp, &real, &effective, &saved_set, &file_system_uids);
          if (real == uid) {
            _uid = real;
          }
        }
      }
      fclose(fin);
      if (_uid != -1) {
        printf("%s: %s", dent->d_name, name); // name will contain a newline notation
      }
    }
  }
}

void usage1() {
  printf("---Processer Type---\n");
  char** processers = malloc(128); // Assume the maximun processors in a computer is 128
  int num = getProcesser(processers);
  int i;
  for (i = 0; i < num; i++) {
    printf("Processor-%d: %s", i, processers[i]);
  } 
  printf("---Kernel Version---\n");
  printf("Linux version %s\n",getVersion());
  printf("---Amount of Memory---\n");
  printf("Installed Memory: %d kB\n", getMemory());
  printf("---Up Time Since Booted---\n");
  printf("%s\n", getUpTime());
}

void usage2(char* str) {
  struct passwd *user;
  user = getpwnam(str);
  if (user == NULL) {
    printf("Invalid username: %s\n", str);
  }
  else {
    printProcessesByUid(user->pw_uid);
  }
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    usage1();
  }
  else if (argc == 2) {
    usage2(argv[1]);
  }
  else {
    printf("Too many arguments supplied.\n");
  }
  return 0;
}
