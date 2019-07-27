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
  
  // ��������x���_ID
  unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID};
  
  // ��������f�o�C�XID
  unsigned char   rawPid[2] = {USB_CFG_DEVICE_ID};
  
  // �x���_��
  char            vendorName[] = {USB_CFG_VENDOR_NAME, 0};
  // �v���_�N�g��
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
  char        buffer[17];    /* 16�o�C�g�̃o�b�t�@+���|�[�gID�p1�o�C�g */
  int recv_len = sizeof(buffer);
  int         err;
  
  if(argc < 2) {
    usage(argv[0]);
    exit(1);
  }
  
  if(strcmp(argv[1], "list") == 0) {		
    // �V���A���ԍ��̃��X�g��1�s1��ŕ\�����܂�		
    if ((dev = openDevice(NULL)) == NULL ) exit(1);

  } else if( (strcmp(argv[1], "loop") == 0) || (strcmp(argv[1],"one") == 0) ){
    // �w�肳�ꂽ�V���A���ԍ��̃Z���T�[�l���擾����
    
    if( argc < 3 ) {
      fprintf(stderr, "Invalid arguments.\n");
      usage(argv[0]);
      exit(1);
    }

    // �V���A���ԍ����w�肵��USB�f�o�C�X���J��
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
    buffer[0] = 0;    // �_�~�[�̃��|�[�gID
    buffer[1] = 0x82; // �V���A���ԍ�����������
    
    // �G�|�b�N�b�𓾂āA10�����̕�����ɕϊ�����
    time(&epoc);
    sprintf(&buffer[2], "%i", (int)epoc);
	  
    // �f�o�C�X���J��
    if((dev = openDevice("ANY")) == NULL) exit(1);
	  
    // �ڑ�����Ă���ŏ���1��̃V���A���ԍ�������������							
    if((err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0) 
      fprintf(stderr, "error init serial: %s\n", usbErrorMessage(err));
	  
    printf("Set serial number to %s\n",&buffer[2]);
	  
  } else {
    usage(argv[0]);
    exit(1);
  }
	
  // �f�o�C�X���N���[�Y
  usbhidCloseDevice(dev);
	
  return 0;
}
