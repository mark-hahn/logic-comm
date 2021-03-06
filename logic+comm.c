#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libserialport.h>
// #include "logic+comm.h"

#define FBUF_SIZE 64

void printErr(const char* hdr1, const char* hdr2, sp_return ret) {
  if (ret < 0) {
    char errStr[15];
    switch (ret) {
      case SP_ERR_ARG:  strcpy(errStr, ", SP_ERR_ARG");  break;
      case SP_ERR_FAIL: strcpy(errStr, ", SP_ERR_FAIL"); break;
      case SP_ERR_MEM:  strcpy(errStr, ", SP_ERR_MEM");  break;
      case SP_ERR_SUPP: strcpy(errStr, ", SP_ERR_SUPP"); break;
      default: strcpy(errStr, ""); break;
    }
    if(ret == SP_ERR_FAIL) {
      char* msg = sp_last_error_message();
      printf("%s%s %s, %s\n", hdr1, hdr2, errStr, msg);
      sp_free_error_message(msg);
    } else printf("%s%s%s\n", hdr1, hdr2, errStr);
  }
}

int main(int argc, char** argv)  {
  sp_return ret;
  char *portName = NULL;
  char *fileName = NULL;
  FILE* fd;
  size_t len = 0;
  ssize_t bytesRead;
  bool listPorts = false;
  char fbuf[FBUF_SIZE];
  int c;

  opterr = 0;
  while ((c = getopt (argc, argv, "lp:")) != -1) {
    switch (c) {
      case 'p':
        portName = optarg;
        break;
      case 'l':
        listPorts = true;
        break;
      case '?':
        if (optopt == 'p')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        return 1;
      default:
        abort ();
    }
  }
  if(optind < argc) {
    fileName = argv[optind];
    fd = fopen((const char*) fileName, "r");
    if (!fd) {
      printf ("File not found: %s\n", fileName);
      return 1;
    }
  }
  if (!portName && !listPorts) {
    portName = (char*) "/dev/ttyACM0";
  }

  int i;
  struct sp_port **ports;
  struct sp_port *port = NULL;

  sp_list_ports(&ports);
  for (i=0; ports[i]; i++) {
    if (listPorts) printf ("Found port %s\n", sp_get_port_name(ports[i]));
    if (portName && !strcmp(sp_get_port_name(ports[i]), portName)) port = ports[i];
  }
  if (!port) {
    printErr("Unable to find port ", portName, (sp_return) -99);
    return 1;
  }
  ret = sp_open(port, SP_MODE_READ_WRITE);
  printErr("Unable to open port ", sp_get_port_name(port), ret);

  ret = sp_set_baudrate(port, 115200);
  printErr("sp_set_baudrate err, ", "115200", ret);
  ret = sp_set_bits(port, 8);
  printErr("sp_set_bits err, ", "8", ret);
  ret = sp_set_parity(port, SP_PARITY_NONE);
  printErr("sp_set_parity err, ", "SP_PARITY_NONE", ret);
  ret = sp_set_stopbits(port, 1);
  printErr("sp_set_stopbits err, ", "1", ret);
  ret = sp_flush(port, SP_BUF_BOTH);
  printErr("sp_flush err, ", "1", ret);

  printf("Using   %s\n", portName);
  if (fileName) printf("Sending %s\n", fileName);

  #define PAUSE   0xf0
  #define UNPAUSE 0xf1

  char recvChar;
  bool paused = false;

  while(1) {
    recvChar = 0;
    ret = sp_nonblocking_read(port, &recvChar, 1);
    printErr("sp_nonblocking_read err, ", "", ret);
    if (recvChar) {
      if (recvChar == PAUSE) paused = true;
      else if (recvChar == UNPAUSE) paused = false;
      else printf("%c", recvChar);
    }
    if(!paused) {
      bytesRead = fread(fbuf, 1, FBUF_SIZE, fd);
      if(bytesRead == -1) {
        printf("\nFinished sending file");
        exit(0);
      }
      ret = sp_blocking_write(port, fbuf, bytesRead, 0);
      printErr("sp_blocking_write err, ", "", ret);
    }
  }
}
