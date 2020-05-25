
//====================================================================================================//
// Sellecting the COM port Number                                                                     //
//----------------------------------------------------------------------------------------------------//
// Use "Device Manager" in Windows to find out the COM Port number allotted to USB2SERIAL converter-  // 
// -in your Computer and substitute in the  "ComPortName[]" array.                                    //
//                                                                                                    //
// for eg:-                                                                                           //
// If your COM port number is COM32 in device manager(will change according to system)                //
// then                                                                                               //
//			char   ComPortName[] = "\\\\.\\COM32";                                                    //
//====================================================================================================//

#include <Windows.h>
#include <stdio.h>
#include "elf_parser.h"
#include "STD_TYPES.h"
#include "TProtocol.h"
#include "TProtocolMessages_Config.h"



#define DATA_BLOCK_SIZE					1000

HANDLE hComm;                          // Handle to the Serial port
unsigned char  ComPortName[] = "COM7";  // Name of the Serial port(May Change) to be opened,
BOOL  Status;                          // Status of the various operations 
DWORD dwEventMask;                     // Event mask to trigger
unsigned char  TempChar;                        // Temperory Character
unsigned char  SerialBuffer[256];               // Buffer Containing Rxed Data
DWORD NoBytesRead;                     // Bytes read by ReadFile()
unsigned int i = 0;
unsigned int m = 0;
unsigned char * lpBuffer;		       		   // lpBuffer should be  char or byte array, otherwise write wil fail
DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port
char userChoice=0;
unsigned char CommandToSend[9]={0};				//total Frame Size + Null Terminator
unsigned long long z=0,x=0;						//Iterators used for producing delay
unsigned char PortClosedFlag=0;

unsigned char ProgramData[1024*1024]={0};		// 60 K Program Data Buffer
unsigned char ProgramDataToSend[30*1024]={0};	//30K only Program Header Sections
unsigned char ElfFileName[]="Main_APP.elf";
FILE * ElfFileDescriptor;
unsigned long size=0;
unsigned static long SentDataIDX=0;
unsigned static long SentDataBlockIDX=0;
unsigned static long CheckSum = 0;
unsigned char MessageID=0;
unsigned long lastSavedIDX=0;
unsigned long ISR_Offset=0;
unsigned char Program_Size=0;

Elf32_Ehdr * Header = NULL;
Elf32_Phdr * ProgramHeader = NULL;
Elf32_Shdr * SectionHeader = NULL;

void ElfHeaderParser(void);
void ProgramHeaderParser(void);

void InitSerialPort(void);
void SetTimeOut(void);
void sendCommand(char * command);
void receiveCommand(void);
void closePort(void);
void openPort(void);

void main(void)
{
	uint8_t CommandType =0;
	EraseCommand_t EraseCommand={0};
	ResponseCommand_t ResponseCommand={0};
	DataCommand_t DataCommand={0};
	VerifyCommand_t VerifyCommand={0};

	//printf("Enter File Name\n");
	//scanf("%s",ElfFileName);
	ElfFileDescriptor = fopen(ElfFileName, "rb"); // read mode

	long long ProgramIterator=0;
	for(ProgramIterator=0;ProgramIterator <sizeof(ProgramDataToSend);ProgramIterator++){
		ProgramDataToSend[ProgramIterator] = 0xFF;
	}

	InitSerialPort();
	printf("-				Welcome To MCU Port					\n");

	if (ElfFileDescriptor == NULL)
	{
		printf("Error while opening the file.\n");
	}else{

		/*get size of Elf File*/
		fseek(ElfFileDescriptor,0,SEEK_END);
		size = ftell(ElfFileDescriptor);
		fseek(ElfFileDescriptor,0,SEEK_SET);
		rewind(ElfFileDescriptor);

		/*save elf file content in buffer*/
		char ch = fgetc(ElfFileDescriptor);
		printf("%d\n",size);
		long long i=0;
		while (i != size)
		{
			//printf("Here");
			ProgramData[i]=ch;
			i++;
			ch = fgetc(ElfFileDescriptor);
		}

		/*close elf file to open the output file*/
		fclose(ElfFileDescriptor);


		Header = (Elf32_Ehdr *) &ProgramData[0];
		ProgramHeader = (Elf32_Phdr *)(&ProgramData[0] + Header->e_phoff);
		SectionHeader = (Elf32_Shdr *)(&ProgramData[Header->e_shoff]);

		ISR_Offset = ( ( SectionHeader + 1 )->sh_offset );
		Program_Size = ceil( ( (ProgramHeader[0].p_memsz) + (ProgramHeader[1].p_memsz) - ISR_Offset ) / (float)DATA_BLOCK_SIZE );

		
		/*TODO: to add section header parser*/
		for(i= 0 ; i < ((ProgramHeader[0].p_filesz) - ISR_Offset ); i++ ){

			ProgramDataToSend[i] = ProgramData[ ISR_Offset + ProgramHeader[0].p_offset + i ];

		}


		for(i= 0 ; i < (ProgramHeader[1].p_filesz) ; i++ ){

			ProgramDataToSend[  (ProgramHeader[0].p_memsz - ISR_Offset ) + 1 + i  ] = ProgramData[  ProgramHeader[1].p_offset + i   ];

		}
		
		
	/************************************ Erasing Sequence *******************************************/
	/*******Sending Erase Command*********/
	openPort();
	EraseCommand.SectionsCount = Program_Size;
	EraseCommand.SectionOffset = ISR_Offset;
	EraseCommand.CheckSum = ( (uint8_t)EraseCommand.SectionsCount  ) + (  (uint8_t)EraseCommand.SectionOffset );
	TProtcol_sendFrame((void*)&EraseCommand, CommandToSend, ID_EraseCommand );
	sendCommand(CommandToSend);
	closePort();
	
	/*******waiting for response to sent Erase Command*********/
	openPort();
	receiveCommand();
	TProtocol_ReceiveFrame( SerialBuffer, &ResponseCommand, &MessageID);
	printf("\nThe received Response is %d", MessageID);
	printf("\nThe received Response is %d\n\n", ResponseCommand.Response);
	closePort();
		
		
	/************************************ Data Transmission Sequence *******************************************/
		while ( (SentDataBlockIDX< EraseCommand.SectionsCount ) ){

			/*send 1000 Byte*/
			for ( i=0; i < (DATA_BLOCK_SIZE/FRAME_DATA_BYTES) ; i++ ){

				SentDataIDX = i*5 ;
				TProtcol_sendFrame(&ProgramDataToSend[lastSavedIDX + SentDataIDX], CommandToSend, ID_DataCommand);

				CheckSum += (ProgramDataToSend[lastSavedIDX + SentDataIDX] + ProgramDataToSend[lastSavedIDX + SentDataIDX + 1] + ProgramDataToSend[lastSavedIDX + SentDataIDX + 2] + ProgramDataToSend[lastSavedIDX + SentDataIDX + 3] + ProgramDataToSend[lastSavedIDX + SentDataIDX + 4 ]);
				openPort();
				sendCommand(CommandToSend);
				closePort();
			}
			/*verify with checksome*/
			VerifyCommand.CheckSum = CheckSum;
			TProtcol_sendFrame( &VerifyCommand, CommandToSend, ID_VerifyCommand);
			openPort();
			sendCommand(CommandToSend);
			closePort();
			/*wait for response*/
			openPort();
			receiveCommand();
			closePort();
			TProtocol_ReceiveFrame( SerialBuffer, &ResponseCommand, &MessageID);
			//ResponseCommand.Response = R_OK;
			if(ResponseCommand.Response == R_OK){
				SentDataBlockIDX++;
				lastSavedIDX += SentDataIDX + 5; //0	1000	2000		30000
				printf("\n\n/*************************************/\n");
				printf("Last Saved IDX %ld\n",lastSavedIDX);
				printf("/*************************************/\n\n");
				//}
				CheckSum=0;
			}
			/*repeat till end*/
		}


	}



}//End of Main()


void InitSerialPort(void){

	printf("\n\n +==========================================+");
	printf("\n |    Serial Port  Reception (Win32 API)    |");
	printf("\n +==========================================+\n");

}

void SetTimeOut(void){
	/*------------------------------------ Setting Timeouts --------------------------------------------------*/

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 50;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(hComm, &timeouts) == FALSE)
		printf("\n\n    Error! in Setting Time Outs");
	else
		printf("\n\n    Setting Serial Port Timeouts Successfull");

}

void sendCommand(char * command){
	//FlushFileBuffers(hComm);
	/*----------------------------- Writing a Character to Serial Port----------------------------------------*/
	lpBuffer = command;		       // lpBuffer should be  char or byte array, otherwise write wil fail
	dNoOfBytesWritten = 0;          // No of bytes written to the port

	dNoOFBytestoWrite = sizeof(CommandToSend); // Calculating the no of bytes to write into the port

	Status = WriteFile(hComm,               // Handle to the Serialport
			lpBuffer,            // Data to be written to the port 
			dNoOFBytestoWrite,   // No of bytes to write into the port
			&dNoOfBytesWritten,  // No of bytes written to the port
			NULL);

	if (Status == TRUE)
		printf("\n\n    %s - Written to %s with size %d\n", lpBuffer, ComPortName,dNoOfBytesWritten);
	else
		printf("\n\n   Error %d in Writing to Serial Port",GetLastError());

	for (z=0;z<100000000;z++){
		x++;
	}



	//FlushFileBuffers(hComm);
}

void receiveCommand(void){
	/*------------------------------------ Setting Receive Mask ----------------------------------------------*/
	SerialBuffer[0] = 'A';
	m=1;
	/*------------------------------------ Setting WaitComm() Event   ----------------------------------------*/
	Status = SetCommMask(hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

	if (Status == FALSE)
		printf("\n\n    Error! in Setting CommMask");
	else
		printf("\n\n    Setting CommMask successfull");


	printf("\n\n    Waiting for Data Reception");

	Status = WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received

	/*-------------------------- Program will Wait here till a Character is received ------------------------*/				

	if (Status == FALSE)
	{
		printf("\n    Error! in Setting WaitCommEvent()");
	}
	else //If  WaitCommEvent()==True Read the RXed data using ReadFile();
	{
		printf("\n\n    Characters Received");
		do
		{
			Status = ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
			SerialBuffer[m] = TempChar; 
			m++;
			printf("\nNoBytesRead is : %d and received Data is %c \n", NoBytesRead, TempChar);
		}
		while (NoBytesRead > 0);



		/*------------Printing the RXed String to Console----------------------*/

		printf("\n\n    ");
		int j =0;
		for (j = 0; j < m -1; j++)		// j < i-1 to remove the dupliated last character
			printf("%c", SerialBuffer[j]);

	}	
	z=0;x=0;
	for (z=0;z<100000000;z++){
		x++;
	}
	x=0;
	for (z=0;z<100000000;z++){
		x++;
	}
	//FlushFileBuffers(hComm);
}

void openPort(void){
	/*---------------------------------- Opening the Serial Port -------------------------------------------*/

	hComm = CreateFile( ComPortName,                  // Name of the Port to be Opened
			GENERIC_READ | GENERIC_WRITE, // Read/Write Access
			0,                            // No Sharing, ports cant be shared
			NULL,                         // No Security
			OPEN_EXISTING,                // Open existing port only
			0,                            // Non Overlapped I/O
			NULL);                        // Null for Comm Devices

	if (hComm == INVALID_HANDLE_VALUE)
		printf("\n    Error! - Port %s can't be opened\n", ComPortName);
	else
		printf("\n    Port %s Opened\n ", ComPortName);

	/*------------------------------- Setting the Parameters for the SerialPort ------------------------------*/

	DCB dcbSerialParams = { 0 };                         // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	Status = GetCommState(hComm, &dcbSerialParams);      //retreives  the current settings

	if (Status == FALSE)
		printf("\n    Error! in GetCommState()");

	dcbSerialParams.BaudRate = CBR_9600;      // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;        // Setting Parity = None 

	Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

	if (Status == FALSE)
	{
		printf("\n    Error! in Setting DCB Structure");
	}
	else //If Successfull display the contents of the DCB Structure
	{
		//printf("\n\n    Setting DCB Structure Successfull\n");
		//printf("\n       Baudrate = %d", dcbSerialParams.BaudRate);
		//printf("\n       ByteSize = %d", dcbSerialParams.ByteSize);
		//printf("\n       StopBits = %d", dcbSerialParams.StopBits);
		//printf("\n       Parity   = %d", dcbSerialParams.Parity);
	}

	SetTimeOut();
}

void closePort(void){
	CloseHandle(hComm);//Closing the Serial Port
}