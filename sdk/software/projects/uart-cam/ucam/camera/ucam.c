/***************************************************************************
* Project                               :  shakti devt board
* Name of the file                      :  ucam.c
* Brief Description of file             :  Device driver to communicate with ucam-III module
* Name of Author                        :  P Girinath, Shivaram Prasad Meesa
* Email ID                              :  palanigirinath@gmail.com, shivaramprasadmeesa@gmail.com

    Copyright (C) 2020  IIT Madras. All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

***************************************************************************/
/**
  @file ucam.c
  @brief Device driver to communicate with ucam-III module
  @details Contains implementation of init_cam() and get_pic() functions
**/




#include "ucam.h"

/** @fn assert_check(int bool,int track)
 * @brief assert function, exits if assert failed.
 * @param int bool-false or true
 * @param int track - 0 if host, 1 if camera
 */
void assert_check(int bool,int track) {
    if(bool) return;
    if(track) 
      printf("Camera response error!!\n");
    else {
      printf("Host response error!!\nIs host connected?");
      return;
    }
    exit(-1);
}

/** @fn delay_milli(unsigned long milli_seconds)
 * @brief delay function 
 * @param unsigned long milli_seconds - delay for given milliseconds
 */
void delay_milli(unsigned long milli_seconds)
{
	unsigned long cntr1 = milli_seconds;
	unsigned long tmpCntr;

	while (cntr1--) {
		tmpCntr = 1000;
		while (tmpCntr--);
	}
}

/** @fn send(const uint8_t* str,int len)
 * @brief function to send command to camera
 * @param const uint8_t* str-pointer to command string  
 * @param int len- length of command string
 */
void send(const uint8_t* str,int len){
  int i;
  flush_uart(CAM);
  for(i =0;i<len;i++){
    write_uart_character(CAM,*str);
    str++;
  }
}

/** @fn recieve_ack(int cmdno,int pid)
 * @brief Read 6 bytes and check whether they match with ACK_COMMAND of specific cmdno,pid
 * @param int cmdno-command number of ACK 
 * @param int pid- packet ID
 */
void recieve_ack(int cmdno,int pid)
{
  uint8_t str[6]; 
  int i = 0;
  for(i=0;i<6;i++){
    read_uart_character(CAM,(str+i));
  }
  uint8_t cc = (uint8_t)cmdno; 
  uint8_t pn[] = {(uint8_t)(pid>>8),(uint8_t)(pid%256)};
  uint8_t ack_ex[] = {(uint8_t)(0xAA),(uint8_t)(0x0E),cc,str[3],pn[1],pn[0]};
  assert_check(strcmp(ack_ex,str)==0,1);
}

/** @fn get_ack(int cmdno,int pid, uint8_t str[])
 * @brief Fill ACK command with given specifics
 * @param int cmdno-command number of ACK 
 * @param int pid- packet ID 
 * @param uint8_t str[]- pointer to command string
 */
void get_ack(int cmdno,int pid, uint8_t str[])
{
  str[0] = (uint8_t)(0xAA);
  str[1] = (uint8_t)(0x0E);
  str[2] = (uint8_t)cmdno;
  str[3] = 0;
  str[4] = (uint8_t)(pid%256);
  str[5] = (uint8_t) (pid>>8);
}

/** @fn recieve_gen(uint8_t str[],int len)
 * @brief function to read len bytes from camera
 * @param uint8_t str[]- pointer to string 
 * @param int len - number of bytes to recieve from camera
 */
void recieve_gen(uint8_t str[],int len)
{
  int i;
  for(i = 0;i < len; i++)
  {
    read_uart_character(CAM,str+i);
  }
}

/** @fn recieve_img(int count,int limit)
 * @brief Function to recieve one data packet from camera
 * @param int count- Packet ID of current packet 
 * @param int limit- number of bytes in current packet
 */ 
void recieve_img(int count,int limit){
  uint8_t ptr[256];
  int i=0;
  uint8_t ack_temp[6];
  get_ack(0,count,ack_temp);
  send(ack_temp, 6);
  // if limit=6 it means we don't expect any data bytes
  if(limit==6) return;
  while(1){
    read_uart_character(CAM,(ptr+i));
    i++;
    if(i==limit) break;
  }
  for(int j=4;j<i-2;j++){
    write_uart_character(HOST,ptr[j]);
  }
}

/** @fn init_cam()
 * @brief Function to wake up camera
 * @return int - 0 if success, -1 if error
 */ 
int init_cam()
{
  int delay_time = 4;
  int no=0;
  const uint8_t  sync_command[] = {(uint8_t)(0xAA),(uint8_t)(0x0D), (uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  const uint8_t  ack_command[] = {(uint8_t)(0xAA),(uint8_t)(0x0E), (uint8_t)(0x0D),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  const uint8_t  ack_command_reply[] = {(uint8_t)(0xAA),(uint8_t)(0x0E), (uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  start:
  {
    no++;
    printf("Round : %d\n",no);
    send(sync_command,6);
    delay_milli(delay_time++); 
    if(CAM->status & STS_RX_NOT_EMPTY)
    {
        // code to read 12 characters from the buffer and if it matches with the ACK and sync commands go to succes
        int i;
        for(i = 0;i<6;i++) 
        {
           uint8_t ch;
           read_uart_character(CAM,&ch);
           if(i == 3)continue; // don't care xx thing
           if(ch != ack_command[i]) goto fail;
        }
        for(i = 0;i<6;i++)
        {
            uint8_t ch;
            read_uart_character(CAM,&ch);
            if(ch != sync_command[i]) goto fail;
        }
        goto success;
    }
  }
  fail:
  if(delay_time>15000) {
    printf("Can't wake up camera! Is camera connected?\n");
    return -1;
  }
  goto start;
  success:
  flush_uart(CAM);
  send(ack_command_reply,6);
  printf("\n synchronisation process done waiting for 2 seconds for stabilisation \n");
  delay(2);
  printf("\n you are all set \n");
  return 0;
}

/** @fn send_full_reset()
 * @brief Send Full reset to camera
 */ 
void send_full_reset() {
  uint8_t reset_command[] = {(uint8_t)(0xAA),(uint8_t)(0x08),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  send(reset_command,6);
  recieve_ack(8,0);
}

/** @fn get_pic(int mode)
 * @brief Function to get picture
 * @param int mode - 0 for one image, 1 for continuous
 */ 
void get_pic(int mode)
{
  //INITIAL_command : AA 01 00 07 xx 07 ( specific for JPEG,640x480 xx=don't care )
  uint8_t initial_command[] = {(uint8_t)(0xAA),(uint8_t)(0x1),(uint8_t)(0x0),(uint8_t)(0x7),(uint8_t)(0x3),(uint8_t)(0x7)};
  send(initial_command,6);
  recieve_ack(1,0);

  //SET_PACKAGE_SIZE_command : AA 06 08 00 01 00 (size=256)
  uint8_t set_package_command[] = {(uint8_t)(0xAA),(uint8_t)(0x6),(uint8_t)(0x8),(uint8_t)(0x0),(uint8_t)(0x1),(uint8_t)(0x0)};
  send(set_package_command,6);
  recieve_ack(6,0);

  //GET_PICTURE_command : AA 04 05 00 00 00 ( Current JPEG  )
  uint8_t get_pic_command[] = {(uint8_t)(0xAA),(uint8_t)(0x4),(uint8_t)(0x5),(uint8_t)(0x0),(uint8_t)(0x0),(uint8_t)(0x0)};
  
  start : 
  send(get_pic_command,6);
  recieve_ack(4,0);
  uint8_t data[6];
  recieve_gen(data,6);

  //DATA_command : AA 0A 05 xx yy zz ( current JPEG mode )
  assert_check(data[0]==0xAA,1);
  assert_check(data[1]==0xA,1);
  assert_check(data[2]==0x5,1);

  long no_of_pg = (int)data[3] + (((long)data[4])<<8) + (((long)data[5])<<16) ;
  printf("New size : %ld\n",no_of_pg);

  // Send data to host
  flush_uart(HOST);
  for(int i=0;i<6;i++)
    write_uart_character(HOST,data[i]);

  int counter = no_of_pg/250 ;
  int i = 0;
  char recv;
  //Wait for HOST to confirm that DATA_command is recieved.
  //Host should reply with 'K'
  read_uart_character(HOST,&recv);
  assert_check(recv=='K',0);
  for(i=0;i<counter;i++){
    recieve_img(i,256);
  }
  recieve_img(counter,6+(no_of_pg%250));
  if(no_of_pg%250 != 0) {     
    uint8_t ack_temp[6];
    get_ack(0,counter+1,ack_temp);
    send(ack_temp, 6);
  }
  //Wait for HOST to confirm that image is recieved.
  //Host should reply with 'O'
  read_uart_character(HOST,&recv);
  assert_check(recv=='O',0);
  printf("Imgdone\n");

  if(mode) goto start;
}
