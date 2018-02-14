#include <stdio.h>
#include <string.h>

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

void usage1() {
  printf("---Processer Type---\n");
  
  printf("---Kernel Version---\n");
  printf("Linux version %s\n",getVersion());
  printf("---Amount of Memory---\n");
  printf("Installed Memory: %d kB\n", getMemory());
  printf("---Up Time Since Booted---\n");
  printf("%s\n", getUpTime());
}

void usage2(char* str) {
  printf("Usage2 + %s\n", str);
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
