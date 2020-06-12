/*========================================================================================================================================*/
/*                                                                    Includes                                                            */
/*========================================================================================================================================*/

/* Include Firebase ESP8266 library (this library) */
#include "FirebaseESP8266.h"

/* Include ESP8266WiFi.h and must be included after FirebaseESP8266.h */
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include "HardwareSerial.h"

/*========================================================================================================================================*/
/*                                                                     Macros                                                             */
/*========================================================================================================================================*/
/* Network - Parameters for Firebase. */
#define FIREBASE_HOST "fota-905e1.firebaseio.com"
#define FIREBASE_AUTH "Bpdn4xtRL1NG1vP1r2mgb85QpTxbVeWsEYgzpnky"

/* Network - Parameters for Local Network. 
#define WIFI_SSID "OrangeDSL-Gomaa"
#define WIFI_PASSWORD "MeMes159753" */

#define CHIP_NAME       "Esraa"   //write the name you want for the Chip here

/* Data Frame Size */
#define DATA_FRAMES_COUNT           200
#define DATA_FRAME_BYTES_SIZE       8
/*========================================================================================================================================*/
/*                                                                  Definitions                                                           */
/*========================================================================================================================================*/
/* Declare the Firebase Data object in the global scope */
FirebaseData firebaseData;

/* Global String that Holds the Received and Transmitted Strings */
String TX_Need_Resp_Buffer , RX_string_buffer , Firebase_marker_string;
String TX_string_buffer = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
/* Status Boolean Variables */
bool FlashNewApp_Flag , SendRQT , ResponseRQT, MarkerRQT;

/* Local Parsed Data to be Send and Received in Non-Data Frames*/
unsigned char TxBuffer[8] , RxBuffer[8], stmMarkerStatus[8];

/* Local Parsed Data to be Send in Data Frames  """""""Options - need to be decided""""""" */
//unsigned char DataTxBuffer_Option1[200][8] ;
unsigned char DataTxBuffer_Option2[ 200*8 ];
/* TODO: Testing Paramete """"""""""""""""""TO BE REMOVED"""""""""""""""""" */
String myString, incomingResponse;

HardwareSerial Serial2(2);

int DIOPin = 4 ;

/*========================================================================================================================================*/
/*                                                                     Setup                                                              */
/*========================================================================================================================================*/
void setup() {
  /* Re-setup the WatchDog - Timer for Execption handling */
  ESP.wdtDisable(); 
  ESP.wdtEnable(WDTO_8S);


  /* Serial Setup */
  Serial2.begin(9600);
  Serial.begin(500000);


  /* connecting to wifi Sequence */
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
 // wifiManager.resetSettings();
  wifiManager.autoConnect("NodeMCU_Network");
  Serial.print("Yaay");

  /* Setup the Firebase */
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  /* Optional, set the size of BearSSL WiFi to receive and transmit buffers */ 
  firebaseData.setBSSLBufferSize(4000, 4000); //minimum size is 4096 bytes, maximum size is 16384 bytes

  /* Optional, set the size of HTTP response buffer */
  firebaseData.setResponseSize(4000); //minimum size is 400 bytes

  /* TODO: to be removed, these variable controlled by PC  """"""""""""""""""TO BE REMOVED""""""""""""""""""  */
  /*Firebase.setBool(firebaseData, "FlashNewApp", false );
  Firebase.setBool(firebaseData, "Send", true );
  Firebase.setBool(firebaseData, "ResponseRQT", false );
  Firebase.setString(firebaseData, "Frame", "");
  Firebase.setString(firebaseData, "Marker", "Mostafa");
  Firebase.setBool(firebaseData, "MarkerRQT", false );*/
    
 bool Semphaore;
 while(1)
 {
  Firebase.getBool(firebaseData, "NodeMCUSemaphore"); 
  Semphaore = firebaseData.boolData();
  if ( Semphaore == false )
    break; 
 } 
String chipId;
String Node = "0000000000000000000";
String Node2 = "000000000000000000";
Firebase.setBool(firebaseData, "NodeMCUSemaphore", true );
chipId = String(CHIP_NAME) + "-" + String(ESP.getChipId());

Firebase.getString(firebaseData, "NodeMCUSs"); 
Node = firebaseData.stringData();

Node2 = Node + " " + chipId;
Firebase.setString(firebaseData, "NodeMCUs", Node2);
Firebase.setBool(firebaseData, "NodeMCUSemaphore", false );

 
/* Firebase.setString(firebaseData, "NodeMCUs", CHIP_NAME+"-"+String(ESP.getChipId())); */


  pinMode(DIOPin, OUTPUT);
  digitalWrite(DIOPin, 0); 

}



/*========================================================================================================================================*/
/*                                                                     LOOP                                                               */
/*========================================================================================================================================*/
void loop()
{ 

  /* Feeding the Watch Dog Timer */
  ESP.wdtFeed();

  /* Get the Current Status of FlashNewApp, if true then begin the sequence of receiveing new application */
  Firebase.getBool(firebaseData, "FlashNewApp");
  FlashNewApp_Flag = firebaseData.boolData();


  /* Check if a new program has been uploaded */
  if (FlashNewApp_Flag == true)
  {


    /* Get the Current Status of the Send and ResponseRQT */
    Firebase.getBool(firebaseData, "Send"); 
    SendRQT = firebaseData.boolData();
    Firebase.getBool(firebaseData, "ResponseRQT");
    ResponseRQT = firebaseData.boolData();  
    Firebase.getBool(firebaseData, "MarkerRQT"); 
    MarkerRQT = firebaseData.boolData();

    if( MarkerRQT == true)
    {
      digitalWrite(DIOPin, 1); 


      /* TODO: Receive Data from STM  in RXBuffer*/
      while( !Serial.available() ); /* not enter this cond till it recive something on the channel */
      if (Serial.available() > 0)
      {
        // read the incoming byte:
        Serial.readBytes(stmMarkerStatus,4);
      }

     /* if( stmMarkerStatus[0] == 'A' && stmMarkerStatus[1] == 0x54 )
      {*/
        if ( stmMarkerStatus[0] == 's' &&  stmMarkerStatus[1] == 'e' && stmMarkerStatus[2] == 'n' && stmMarkerStatus[3] == 'd'  )
        {
          /* Get the Current Frame value */
          Firebase.getString(firebaseData, "Marker");
          Firebase_marker_string = firebaseData.stringData();

          /* Divide the Frame into 8 bytes for UART to be send */
          TxBuffer[0] = strtoul(Firebase_marker_string.substring(0,2).c_str()  ,NULL,16);
          TxBuffer[1] = strtoul(Firebase_marker_string.substring(2,4).c_str()  ,NULL,16);
          TxBuffer[2] = strtoul(Firebase_marker_string.substring(4,6).c_str()  ,NULL,16);
          TxBuffer[3] = strtoul(Firebase_marker_string.substring(6,8).c_str()  ,NULL,16);
          TxBuffer[4] = strtoul(Firebase_marker_string.substring(8,10).c_str() ,NULL,16);
          TxBuffer[5] = strtoul(Firebase_marker_string.substring(10,12).c_str(),NULL,16);
          TxBuffer[6] = strtoul(Firebase_marker_string.substring(12,14).c_str(),NULL,16);
          TxBuffer[7] = strtoul(Firebase_marker_string.substring(14,16).c_str(),NULL,16);

          /*============== UART SEQUENCE FROM HERE NEED TO APPLIED ON THE TXBUFFER ==============*/      
          /*TODO: Send Data over UART to STM */
          for(int index=0;index<8;index++)
          {
            Serial.write(TxBuffer[index]);
          }

          for(int l=0;l<8;l++){
          stmMarkerStatus[l]=0;
          }

          /* TODO: Receive Data from STM  in RXBuffer*/
          while( !Serial.available() ); /* not enter this cond till it recive something on the channel */
          if (Serial.available() > 0)
          {
            // read the incoming byte:
            Serial.readBytes(stmMarkerStatus,8);
            //delay(500);
          }
          
          if( stmMarkerStatus[0] == 'A' && stmMarkerStatus[1] == 'R' && stmMarkerStatus[2] == 0 )
          { 
              digitalWrite(DIOPin, 0);     
            /*this the resp to GUI that Marker is send correctly */
              Firebase.setBool(firebaseData, "MarkerRQT", false );
    
          }
          else
          {
            digitalWrite(DIOPin, 0);  
            /*the marker has been send wrong */
             Firebase.setString(firebaseData, "Marker", "Same_Marker");
             Firebase.setBool(firebaseData, "MarkerRQT", false );

         
          }
        }
          else 
          {
            /* Failed */
          }

        //}



      }








      /*-------------------------------------------------------- Non-Data Frames------------------------------------------------------------*/
      /* If it is Send and Receive Operation */
      if (SendRQT == true && ResponseRQT == true)
      {
        /* Get the Current Frame value */
        Firebase.getString(firebaseData, "Frame");
        TX_Need_Resp_Buffer = firebaseData.stringData();

        /* Divide the Frame into 8 bytes for UART to be send */
        TxBuffer[0] = strtoul(TX_Need_Resp_Buffer.substring(0,2).c_str()  ,NULL,16);
        TxBuffer[1] = strtoul(TX_Need_Resp_Buffer.substring(2,4).c_str()  ,NULL,16);
        TxBuffer[2] = strtoul(TX_Need_Resp_Buffer.substring(4,6).c_str()  ,NULL,16);
        TxBuffer[3] = strtoul(TX_Need_Resp_Buffer.substring(6,8).c_str()  ,NULL,16);
        TxBuffer[4] = strtoul(TX_Need_Resp_Buffer.substring(8,10).c_str() ,NULL,16);
        TxBuffer[5] = strtoul(TX_Need_Resp_Buffer.substring(10,12).c_str(),NULL,16);
        TxBuffer[6] = strtoul(TX_Need_Resp_Buffer.substring(12,14).c_str(),NULL,16);
        TxBuffer[7] = strtoul(TX_Need_Resp_Buffer.substring(14,16).c_str(),NULL,16);
        /*============== UART SEQUENCE FROM HERE NEED TO APPLIED ON THE TXBUFFER ==============*/


        /*TODO: Send Data over UART to STM */
        for(int index=0;index<8;index++)
        {
          Serial.write(TxBuffer[index]);
        }


        /* TODO: Receive Data from STM  in RXBuffer*/
        while( !Serial.available() );
        if (Serial.available() > 0) {
          // read the incoming byte:
          Serial.readBytes(RxBuffer,8);
        }

        /*============== UART SEQUENCE ENDED ================*/

        /*TODO: Concatenate the RxBuffer into string to Send it to FireBase */
        RX_string_buffer  = String(RxBuffer[0],HEX);
        RX_string_buffer += String(RxBuffer[1],HEX);
        RX_string_buffer += String(RxBuffer[2],HEX);
        RX_string_buffer += String(RxBuffer[3],HEX);
        RX_string_buffer += String(RxBuffer[4],HEX);
        RX_string_buffer += String(RxBuffer[5],HEX);
        RX_string_buffer += String(RxBuffer[6],HEX);
        RX_string_buffer += String(RxBuffer[7],HEX);
        // RX_string_buffer += "\0";


        /* Put the Response and Set the Send to false in firebase*/
        SendRQT = false;
        Firebase.setString(firebaseData, "Frame", RX_string_buffer);
        Firebase.setBool(firebaseData, "Send", SendRQT);
        RX_string_buffer=" ";
      }

      /*---------------------------------------------------------- Data Frames------------------------------------------------------------*/

      /* If it is Send Operation Only */
      else if(SendRQT == true && ResponseRQT == false)
      {
        /* Get the Current Frame */
        TX_string_buffer =" ";
        Firebase.getString(firebaseData, "Frame");
        TX_string_buffer = firebaseData.stringData(); 

        /* """""""""""""""""""""""""""Choose One Option to How to send your data accross UART""""""""""""""""""""""""" */


        /*============== Parsing Option 1 ==============*/
        /*for (int index = 0 ; index < DATA_FRAMES_COUNT ; index++)
          {
                DataTxBuffer_Option1[index][0] = strtoul(TX_string_buffer.substring( 0 +(16*index) , 2 +(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][1] = strtoul(TX_string_buffer.substring( 2 +(16*index) , 4 +(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][2] = strtoul(TX_string_buffer.substring( 4 +(16*index) , 6 +(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][3] = strtoul(TX_string_buffer.substring( 6 +(16*index) , 8 +(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][4] = strtoul(TX_string_buffer.substring( 8 +(16*index) , 10+(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][5] = strtoul(TX_string_buffer.substring( 10+(16*index) , 12+(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][6] = strtoul(TX_string_buffer.substring( 12+(16*index) , 14+(16*index) ).c_str() , NULL,16);
                DataTxBuffer_Option1[index][7] = strtoul(TX_string_buffer.substring( 14+(16*index) , 16+(16*index) ).c_str() , NULL,16); 
          }*/

        /*============== Parsing Option 2 ==============*/

        for (int index = 0 ; index < (DATA_FRAMES_COUNT * DATA_FRAME_BYTES_SIZE) ; index++)
        {
          DataTxBuffer_Option2[index] = strtoul(TX_string_buffer.substring( (2*index ) , (2*index )+2 ).c_str()  , NULL,16); 
          Serial.write(DataTxBuffer_Option2[index]);
        }


        /*============== UART SEQUENCE FROM HERE NEED TO APPLIED ON THE TXBUFFER ==============*/
        /* Send Data over UART to STM */


        /* for(int index=0;index<8;index++)
        {
          //uart.write(0,TxBuffer[index]);  
          Serial.printf("%02x",TxBuffer[index]);        
        }*/

        /*============== UART SEQUENCE ENDED ================*/
        /*Set the Send flag on firebase to false */
        SendRQT = false;
        Firebase.setBool(firebaseData, "Send", SendRQT);
      }
    }      
}
  
