/*
 * IWT303 Control Program
 * Copyright (C) 2014 Tokyo Devices, I.W. Technology Firm, Inc.
 * http://tokyodevices.jp/
 * License: GNU GPL v2 (see License.txt)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "hiddata.h"
#include "usbconfig.h"  /* for device VID, PID, vendor name and product name */

static char *usbErrorMessage(int errCode)
{
	static char buffer[80];

	switch(errCode)
	{
		case USBOPEN_ERR_ACCESS:      return "Access to device denied";
		case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
		case USBOPEN_ERR_IO:          return "Communication error with device";
		default:
			sprintf(buffer, "Unknown USB error %d", errCode);
			return buffer;
	}

	return NULL;    /* not reached */
}

static usbDevice_t  *openDevice(char *serialNumber)
{
  usbDevice_t     *dev = NULL;
  
  // 検索するベンダID
  unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID};
  
  // 検索するデバイスID
  unsigned char   rawPid[2] = {USB_CFG_DEVICE_ID};
  
  // ベンダ名
  char            vendorName[] = {USB_CFG_VENDOR_NAME, 0};
  // プロダクト名
  char			productName[] = {USB_CFG_DEVICE_NAME, 0};
  
  int             vid = rawVid[0] + 256 * rawVid[1];
  int             pid = rawPid[0] + 256 * rawPid[1];	
  int             err;
  
  if((err = usbhidOpenDevice(&dev, vid, vendorName, pid, productName, serialNumber, 0)) != 0){
    if( serialNumber != NULL ) {
      fprintf(stderr, "error finding %s: %s\n", serialNumber, usbErrorMessage(err));
    }
    return NULL;
  }
  
  return dev;
}

static void usage(char *myName)
{
  fprintf(stderr, "IWS600-CM host controller Version 0.1.0\n");
  fprintf(stderr, "2013-2015 (C) Tokyo Devices, I.W. Technology Firm, Inc.\n");
  fprintf(stderr, "usage:\n");
  fprintf(stderr, "  %s list ... List all serial number of detected device(s).\n", myName);
  fprintf(stderr, "  %s <\"loop\"|\"one\"> <SerialNumber|\"ANY\"> ... Get state of the specified device.\n", myName);
}

int main(int argc, char **argv)
{
  usbDevice_t *dev;
  char        buffer[17];    /* 16バイトのバッファ+レポートID用1バイト */
  int recv_len = sizeof(buffer);
  int         err;
  
  if(argc < 2) {
    usage(argv[0]);
    exit(1);
  }
  
  if(strcmp(argv[1], "list") == 0) {		
    // シリアル番号のリストを1行1台で表示します		
    if ((dev = openDevice(NULL)) == NULL ) exit(1);

  } else if( (strcmp(argv[1], "loop") == 0) || (strcmp(argv[1],"one") == 0) ){
    // 指定されたシリアル番号のセンサー値を取得する
    
    if( argc < 3 ) {
      fprintf(stderr, "Invalid arguments.\n");
      usage(argv[0]);
      exit(1);
    }

    // シリアル番号を指定してUSBデバイスを開く
    if((dev = openDevice(argv[2])) == NULL) exit(1);
    
    while(1) {
      if( (err = usbhidGetReport( dev, 0, buffer, &recv_len )) != 0 ) {
	printf("ERR\n");
      } else {
	printf("%d\n", (unsigned char) buffer[1]);
      }

      if( strcmp(argv[1], "one") == 0 ) break;
      
      fflush(stdout);
      
      usleep( 100000 );
    }
     
  } else if(strcmp(argv[1], "init") == 0){
 	  
    time_t epoc;
    
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0;    // ダミーのレポートID
    buffer[1] = 0x82; // シリアル番号初期化命令
    
    // エポック秒を得て、10文字の文字列に変換する
    time(&epoc);
    sprintf(&buffer[2], "%i", (int)epoc);
	  
    // デバイスを開く
    if((dev = openDevice("ANY")) == NULL) exit(1);
	  
    // 接続されている最初の1台のシリアル番号を初期化する							
    if((err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0) 
      fprintf(stderr, "error init serial: %s\n", usbErrorMessage(err));
	  
    printf("Set serial number to %s\n",&buffer[2]);
	  
  } else {
    usage(argv[0]);
    exit(1);
  }
	
  // デバイスをクローズ
  usbhidCloseDevice(dev);
	
  return 0;
}
