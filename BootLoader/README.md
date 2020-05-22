# Micro Controller Data frame Transport Protocol BootLoader documentation:


- adding the application marker at the last index in flash in the memory file
- including all the layers to the project without adding the absolute path from properties -> C/C++ build -> settings -> tool settings -> GNU ARM Cross C Compilers -> includes
- error can't resolve std_types.h (no such file or directory) -> put in folder includes
- error can't resolve gpio (no such file or directory) -> all the problems are solved by removing the GP/FOTA folder inlusions from (properties -> C/C++ build -> settings -> tool settings -> GNU ARM Cross C Compilers -> includes)
```
Sending Sequence: 
```
- PC sends an erase command with the sector numbers to be erased
- RCC enable the UART1 and Port A
- UART_ReceiveBuffer listens on UART to receive the data frame from the PC
- Add global variables for the buffers and their sizes (TransmitterBuffer - RXBuffer - MessageID - Command Flag)
- marker is made a global variable to be allocated in the flash as needed in first point
- make a switch case to check on the marker if it has the address of the application the system will jump to it, if not it will go to the default case which listens till it gets the address of the application
- a switch case on the ID_Command sent by the Sender
```
EraseCommand
```
- in ID_EraseCommand we receive by tprotocol_ReceiveFrame the buffer and the erase command and the message id to check that the mcu received the command correctly
- then we calculate checksum of that frame to check that we received the frame bytes correctly
- we enter a while loop to erase the amount of sections stated in EraseCommand.SectionCount and also check on the ErrorCounter if its less than MAXERRORCOUNT
- we check the local error returned from the Flash_ErasePage(MAINADDRESS+(EraseCommand.SectionOffset)+(PAGESIZE*SectionCount)); here we erase the memory sections given by the sender
- an error counter is used to check for error timeout if the code is stuck on local error not_ok to return not_ok
- check if local_error is ok set the response in the responseCommand = R_OK;
- check if local_error is not_ok set the response in the responseCommand = R_NOT_EraseFailure;
- Fill the transmitterBuffer in the TProtocol_SendFrame with the responseCommand and send it through UART to the Sender with the responseFrame
- if the Checksum of the EraseFrame is not equal to the EraseCommand.Checksum then set the response in responseCommand = R_NOT_ 
- Send the ResponseFrame to the TProtocol_SendFrame to fill the TransmitterBuffer and send it through UART to the Sender
```
DataCommand
```
- in ID_DataCommand we receive by tprotocol_ReceiveFrame the buffer and the DataCommand and the message id to check that the mcu received the command correctly
- Receive the data from the DataCommand.Data[FrameBytes] in the DataBytes[DataIterator]
- Calculate the Checksum for the Data send in the DataCommand.Data[FrameBytes]
```
VerifyCommand
```
- in ID_VerifyCommand we receive by tprotocol_ReceiveFrame the buffer and the VerifyCommand and the message id to check that the mcu received the command correctly
- check if the DataCheckSum is equal VerifyCommand.CheckSum then set the response in the responseCommand = R_OK
- in case (R_OK) we Enable PRIMASK to stop the interrupts (make a critical section)
- Start Writing the Data Bytes received to the Flash Memory at the specified offset for each page size (the offset is incremented each page size flashed)
- check if all the required sections are filled with data flash the marker to the new application in the flash memory
- Disable the PRIMASK to end the critical section
- check if the DataCheckSum is  not equal VerifyCommand.CheckSum then set the response in the responseCommand = R_NOT_MismatchData
- Send the ResponseFrame to the TProtocol_SendFrame to fill the TransmitterBuffer and send it through UART to the Sender
