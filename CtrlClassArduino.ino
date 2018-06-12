#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <EthernetUdp.h> // UDP library from: bjoern@cs.stanford.edu 12/30/2008       

MFRC522 mfrc522(49, 48); 
 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ipServer(192, 168, 137, 1);
IPAddress ip(192, 168, 137, 2);
unsigned int localPort = 8888;
unsigned int serverPort = 6000;
EthernetUDP Udp; // An EthernetUDP instance to let us send and receive packets over UDP
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // Buffer to hold incoming packet,

void setup(){
  Serial.begin(9600); // config Serial
  Ethernet.begin(mac, ip); // start Ethernet
  Udp.begin(localPort); // start UDP
  SPI.begin(); // start RFID
  mfrc522.PCD_Init();
  pinMode(14, OUTPUT); digitalWrite(14, LOW); // config 14 Saida
  pinMode(15, OUTPUT); digitalWrite(15, LOW); // config 15 Saida
}

void loop(){
  Serial.println("Aproxime o seu cartao do leitor...\n");
  rfid();
}

void sendToServer(String tag){
  Serial.print("Sending:  ["); Serial.print(tag.c_str()); Serial.println("]...");
  Udp.beginPacket(ipServer, serverPort); Udp.write(tag.c_str());
  Udp.endPacket();
}

int receiveFromServer(){
  if (Udp.parsePacket()){
    Serial.print("Received packet from ");
    for (int i = 0; i < 4; i++) Serial.print( (Udp.remoteIP())[i], DEC);
    Serial.print(", port "); Serial.println(Udp.remotePort());

    for(int i=0; i<UDP_TX_PACKET_MAX_SIZE; i++) packetBuffer[i] = 0;
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.print("Received:  ["); Serial.print(packetBuffer); Serial.println("]");
    
    return strcmp(packetBuffer, "true") == 0;
  }
  Serial.println("Failed to receive"); return -1;
}

int isAllowed(String tag){ //retorna se o usuario está permitido ou nao
  Serial.println("Checking if UID is authorized...");
  sendToServer(tag); delay(1000);
  return receiveFromServer();
}

void check(String tag){
  Serial.println("TAG: " + tag);
  int stat = isAllowed(tag);
  if(stat == 1) outOk(); // Autorizado
  if(stat == 0) outNo(); // Não autorizado
  if(stat == -1) outEr(); // Erro 
  delay(500);
  Serial.println("**********************\n");
}

void rfid(){
  while ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) delay(100) ;
  String tag= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) tag.concat(String(mfrc522.uid.uidByte[i], HEX));
  tag.toUpperCase();
  //check(tag.substring(1));
  check(tag);
}

void outOk(){
  Serial.println("Usuário autorizado");
  digitalWrite(15, HIGH);
  digitalWrite(14, HIGH); delay(50); digitalWrite(14, LOW); delay(50); 
  digitalWrite(14, HIGH);  delay(50); digitalWrite(14, LOW);
  digitalWrite(15, LOW);
}

void outNo(){
  Serial.println("Não está autorizado");
  digitalWrite(14, HIGH); delay(150); digitalWrite(14, LOW); delay(50); 
  digitalWrite(14, HIGH); delay(50); digitalWrite(14, LOW); delay(50); 
  digitalWrite(14, HIGH); delay(50); digitalWrite(14, LOW); delay(50); 
  digitalWrite(14, HIGH); delay(50); digitalWrite(14, LOW);
}

void outEr(){
  Serial.println("ERROR");
  digitalWrite(14, HIGH); delay(3000); digitalWrite(14, LOW);
}
