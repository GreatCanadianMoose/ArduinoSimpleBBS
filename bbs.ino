 /* Simple Serial Arduino BBS
 *  To do:
 *  Put all strings in program memory with F()
 *  Put everything else in program mem that can safely be in program mem
 *  Double check bytes are 0-255
 */


#include <SD.h> // Load SD card library
#include <SPI.h> // Load SPI Library

File users; // File in format: Username,pass
File posts; // File in format: User,title,msg

void registerUser();
void loginPage();
void printBulletin();
void readAllBulletins();
void postBulletin();
void cleanBuff();
void cleanBuffAfterUsername();

// This logo is 249 bytes
const static String logo PROGMEM = " AA         d                  BBBB  BBBB   SSS  \nA  A        d      ii          B   B B   B S     \nAAAA rrr  ddd u  u    nnn  ooo BBBB  BBBB   SSS  \nA  A r   d  d u  u ii n  n o o B   B B   B     S \nA  A r    ddd  uuu ii n  n ooo BBBB  BBBB  SSSS";

// byte at pos[0] is for menu responses, bytes at positions [1 - 17] are for the username, the rest is for passwords, and msgs
char buff[256];

bool userConnected = 0;
byte looper = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  while(!Serial){} // If serial is not initialized, wait
  if(!SD.begin(4)){Serial.println(F("initialization failed!"));while (1);} // If can't connect to SD, stop running
  if(SD.exists("users.txt")){SD.remove("users.txt");}
}

void loop() {
  if(!userConnected){ // If logged out
    Serial.println(logo);
    Serial.print(F("Welcome To Arduino BBS! (N)ew User? (L)ogon? Your Selection: "));
    looper = 1;
    while(!Serial.available()){}
    buff[0] = Serial.read();
    while(Serial.available()){
      buff[looper] = Serial.read();
      looper++;
    }    
    Serial.println(buff[0]);

    if(buff[0] == 'N'){ cleanBuff(); registerUser(); cleanBuff();} // Decide what to do
    if(buff[0] == 'L'){ cleanBuff(); loginPage(); cleanBuffAfterUsername();}

  }

  else{ // If logged in
    Serial.print(F("====================\n\n\n====================\n\t(R)ead Bulletins?\n\t(P)ost Bulletin?\n\t(L)ogoff?\n====================\n\n\n===================="));

    while(Serial.available()){ // Read input
          buff[looper] = Serial.read();
          looper ++;
      }

    if(buff[0] == 'R'){readAllBulletins();} // Do options
    if(buff[0] == 'P'){postBulletin();  cleanBuffAfterUsername();}
    if(buff[0] == 'L'){userConnected = 0; cleanBuff();}
  }
}

/*
 * INPUT:
 *  Username : 1 - 32
 *  Password : 65 - 96
 * SD CARD:
 *  Username : 33 - 64
    if(buff[0] == "P"){postBulletin();  cleanBuffAfterUsername();}
 *
 */

void registerUser(){
  Serial.print(F("Please Enter Your Username:\t"));

  looper = 1;
  while(!Serial.available()){}
  
  while(Serial.available()){ // Read input
     buff[looper] = Serial.read();
     looper ++;
  }

  users = SD.open("users.txt");
  looper = 33; // Start of message bytes

  bool beforeComma = 1;
  bool searchForUser = 1;

  while(users.available() && searchForUser){
    buff[0] = users.read();
    if(beforeComma){ // Read username and pass to char array
      if(buff[0] == ","){
        beforeComma = 0;
        looper = 65;
      }else{
        buff[looper] = buff[0];
        looper ++;
      }
    }else{ // Read password to buff
      if(buff[0] == "\n"){
        beforeComma = 1;
        looper = 33;
        searchForUser = 0;
        while(looper < 65){if(buff[looper] != buff[looper - 32]){searchForUser = 1;}}
        if(searchForUser){cleanBuffAfterUsername();}
      }else{
        buff[looper] = buff[0];
        looper ++;
      }
    }
  }

  users.close();

  if(searchForUser){ // If not found, register
    users = SD.open("users.txt", FILE_WRITE);
    Serial.print(F("Please Enter Your Password:\t"));
    looper = 33;
    while(!Serial.available()){}
    while(Serial.available()){
      buff[looper] = Serial.read();
      looper ++;
      if(looper >= 65){Serial.println(F("\nHEY THAT'S A LONG PASSWORD ONLY FIRST 31 BYTES WILL BE USED.")); break;}
    }
    // IMPLEMENT WRITE
    looper = 1;
    while(looper < 33){users.print(buff[looper]); looper++;} // Print username
    users.print(",");
    while(looper < 65){users.print(buff[looper]); looper++;} // Print pass
    users.print("\n");
    users.close();
  }else{
    Serial.println(F("\nUsername already taken!"));
     cleanBuff();
  }
}

/*
 * INPUT:
 *  Username : 1 - 32
 *  Password : 33 - 64
 * SD CARD:
 *  Username : 65 - 96
 *  Password : 97 - 128
 */

void loginPage(){
  cleanBuff();
  looper = 1;
  Serial.print(F("Please enter your username!"));
  while(Serial.available()){ // Read input
     buff[looper] = Serial.read();
     looper ++;
  }
  Serial.println("");
  
  looper = 33;
  Serial.print(F("Please enter your password!"));
  while(Serial.available()){ // Read input
     buff[looper] = Serial.read();
     looper ++;
  }

  users = SD.open("users.txt");
  looper = 65; // Start of message bytes

  bool beforeComma = 1;
  bool searchForUser = 1;
  
  while(users.available() && searchForUser){
    buff[0] = users.read();
    if(beforeComma){ // Read username and pass to char array
      if(buff[0] == ","){
        beforeComma = 0;
        looper = 97;
      }else{
        buff[looper] = buff[0];
        looper ++;
      }
    }else{ // Read password to input
      if(buff[0] == "\n"){
        beforeComma = 1;
        looper = 65;
        searchForUser = 0;
        while(looper < 97){if(buff[looper] != buff[looper - 64]){searchForUser = 1;}}
        if(searchForUser){cleanBuffAfterUsername();}
      }else{
        buff[looper] = buff[0];
        looper ++;        
      }
    }
  }
  users.close();

  if(searchForUser){
    Serial.println(F("Username not found!"));
    cleanBuff();
  }else{
    looper = 97;
    while(looper < 129){if(buff[looper] != buff[looper - 64]){searchForUser = 1;}}
    if(searchForUser){userConnected = 1; Serial.println(F("Sucessful Login! Enjoy Your Stay!"));}
    cleanBuffAfterUsername();
  }

}

/*
 * INPUT:
 *  Username : 1 - 32
 *  Title : 33 - 64
 *  Message : 65 - 255
 *
 */

void postBulletin(){

  looper = 33;
  while(looper < 256){buff[looper] = "\0"; looper ++;} // Cleanse the title and message area
  
  Serial.print("Title:\t");
  looper = 33;
  while(Serial.available()){ // Read title
    buff[looper] = Serial.read();  
    looper ++;
  }

  Serial.println("");
  Serial.println("Bulletin Post:");
  looper = 65;
  while(Serial.available()){ // 190 bytes for a post... nice
    buff[looper] = Serial.read();
    looper ++;
  }
  posts = SD.open("posts.txt", FILE_WRITE);

  looper = 1;
  while(looper < 33){posts.print(buff[looper]); looper ++;} // Write Username
  posts.print(",");
  while(looper < 64){posts.print(buff[looper]); looper ++;} // Write Title
  posts.print(",");
  while(looper < 254){posts.print(buff[looper]); looper ++;} // Write Post
  posts.print("\n");
  posts.close();

  Serial.println("Sucessful post!");

  looper = 33;
  while(looper < 256){buff[looper] = "\0"; looper ++;} // Cleanse the title and message

}

void printBulletin(){
  Serial.println(F("\n==================="));
  Serial.print("From: ");
  looper = 33;
  while(looper < 65){Serial.print(buff[looper]); looper++;} // Print Name

  Serial.print("\nTitle: ");
  looper = 65;
  while(looper < 97){Serial.print(buff[looper]); looper++;} // Print Title
  Serial.println(F("\n___________________\n"));

  looper = 97;
  while(buff[looper] != "\n"){Serial.print(buff[looper]); looper++;} // Print Message
  
  Serial.println(F("Please Send Any Char To Continue"));
  cleanBuffAfterUsername();
  while(!Serial.available()){} // Wait for keystroke
  
}

void readAllBulletins(){
  cleanBuffAfterUsername();
  
  posts = SD.open("posts.txt");
  unsigned int i = 1;
  
  while(posts.available()){
    buff[0] = posts.read();
    // Need to implement a read into the buffer here 

    looper = 33;
    while(buff[0] != ","){buff[looper] = buff[0]; looper ++;} // Read User    

    looper = 65;
    while(buff[0] != ","){buff[looper] = buff[0]; looper ++;} // Read Title    

    looper = 97;
    while(buff[0] != "\n"){buff[looper] = buff[0]; looper ++;} // Read Message

    Serial.print("\nPost Number: ");
    Serial.println(i);
    printBulletin();
    i++;
  }
}

void cleanBuff(){
  looper = 0;
  while(looper != 255){buff[looper] = '\0'; looper ++;} // Cleanse the whole buffer
}

void cleanBuffAfterUsername(){
  looper = 33;
  while(looper != 255){buff[looper] = "\0"; looper ++;} // Cleanse the title and message area
}
