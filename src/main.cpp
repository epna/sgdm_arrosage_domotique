#include <ArduinoHttpClient.h>
#include <Ethernet.h>
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>         
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
#include <stdio.h>

#define Arro1 5
#define Arro2 6
#define Arro3 7
#define Arro4 8

char buffer[1000];
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAB }; //192.168.1.48
byte mac[] = { 0xFE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAB }; //192.168.1.49
boolean reading;

EthernetClient client;
EthernetClient client2;
EthernetClient client3;

EthernetUDP Udp;
IPAddress ip(192, 168, 1, 75);
IPAddress server_ip;  // IP of the MySQL *server* here
EthernetServer server(80);
boolean Manuel[9],bonget;
int count=0; 
int MaxIdx; 

long DemManuel[5];
int MinuteNow;
int old_minute, old_day;
int debutV, finV;
unsigned int MaxManuel1;
unsigned int MaxManuel2;
String MonHeure;

NTPClient timeClient(Udp, "europe.pool.ntp.org", 3600);


typedef struct
 {
     String Akey;
     boolean Aactivation;
     int Aarroseur;
     int Aduree;
     int Ademarrage;
     String Afrequence;
	   boolean Automatique; 
 }  MonArro;
MonArro Arro[50];

String lib_equipement[10];
boolean Prevenu[10];
boolean ecrire = false;
String strJSON; 

void ReponseServeur(String MonParam) {
    // send a standard http response header
    client.println("HTTP/1.1 200 OK");           // This tells the browser that the request to provide data was accepted
    client.println("Access-Control-Allow-Origin: *");  //Tells the browser it has accepted its request for data from a different domain (origin).
    client.println("Content-Type: text/html");
    client.println("Server: Arduino2");           // The data is coming from an Arduino Web Server (this line can be omitted)    
    client.println("Connection: close");         // Will close the connection at the end of data transmission.
    client.println();                            // You need to include this blank line - it tells the browser that it has reached the end of the Server reponse header.
    // output the value of each analog input pin
		client.print("{\"manuel\":[");
		
		for (int Idx = 0; Idx < 4; Idx++)
		{
			if (Idx > 0) {client.print(",");} 
          if (!Manuel[Idx])
	      		{
	      		  client.print("{\"duree\": \"-1\",");
	      		  client.print("\"actif\": \"off\"}");
	      		}
			    else
			      {
			      client.print("{\"duree\": \""); 
			      client.print((millis()-DemManuel[Idx])/60000);
			      client.print("\",");
      		  client.print("\"actif\": \"on\"}");

			      }
		}
		client.print("],");
		client.print("\"automatique\":[");
		boolean deja=false; 
		for (int yy=0; yy <= MaxIdx-1 ;yy++)
		{
			if (Arro[yy].Automatique)
			{
			if (deja) {client.print(",");}
			deja=true;
			  
			client.print("{\"arroseur\": \"");
			client.print(Arro[yy].Aarroseur);
			client.print("\",\"Key \": \"");
			client.print(Arro[yy].Akey);
			client.print("\",\"depuis\": \"");
			client.print(Arro[yy].Ademarrage);
			client.print("\",\"duree\": \"");
			client.print(Arro[yy].Aduree);
			client.print("\",\"freequence\": \"");
			client.print(Arro[yy].Afrequence);
			client.print("\"}");
			}
		}
		client.print("],");
		client.print("\"sessions\":[");
		deja=false; 
		for (int yy=0; yy <= MaxIdx-1 ;yy++)
		{
		
			if (deja) {client.print(",");}
			deja=true;	
			client.print("{\"Key \": \"");
			client.print(Arro[yy].Akey);
			client.print("\",\"arroseur\": \"");
			client.print(Arro[yy].Aarroseur);
			client.print("\",\"démarrage\": \"");
			client.print(Arro[yy].Ademarrage);
			client.print("\",\"Activation\": \"");
			if (Arro[yy].Aactivation) 
			  {client.print("True");}
			  else 
			  {client.print("false");}
			  
			client.print("\",\"duree\": \"");
			client.print(Arro[yy].Aduree);
			client.print("\",\"frequence\": \"");
			client.print(Arro[yy].Afrequence);
			client.print("\"}");
			
		
		}
    client.print("],\"message\":[{\"resultat\":\"");
    client.print(MonParam);
    client.print("\"}]");    
    client.print(",\"datetime\":[{\"heure\":\"");
    client.print (String(timeClient.getHours()) + ":" + String(timeClient.getMinutes()));
    client.print("\"},{\"DOW\":\"");
    client.print(String(timeClient.getDay()));
    client.println("\"}]}");
  	delay(1);
    client.stop();
     }


void Trace( String MaTrace)
{ 
	
	MonHeure = String(String(timeClient.getDay()) + "  " + timeClient.getHours()) + ":" + String(timeClient.getMinutes());   
	Serial.println(MonHeure + " -- " + MaTrace);
 }
////////////////////////////////////////////

void PBSend(int codemess, int arro, int porte )
{
    if (client3.connect("boat.alwaysdata.net", 80))
		{   
        client3.println("GET /sgdmdomo.php?action=message&codemess=" + String(codemess) + "&porte=" + String(porte) + "&arro=" + String(arro) +  " HTTP/1.1" );
        Serial.println("GET /sgdmdomo.php?action=message&codemess=" + String(codemess) + "&porte=" + String(porte) + "&arro=" + String(arro) +  " HTTP/1.1" );
		    client3.println("Host: boat.alwaysdata.net");
		    client3.println("Connection: close");
	    	client3.println();
        client3.stop(); 
        delay(5);
		} 
}


void Automate () 
{
  int arr_debut;
  int arr_fin; 
  bool cDay;
  MinuteNow= (timeClient.getHours() * 60 ) + timeClient.getMinutes();
  String Daynow= String(timeClient.getDay()); 
  for (int yy=0; yy <= MaxIdx-1 ;yy++)
  {
   if (Arro[yy].Akey!="")
    {   
      //==========================================
      //            DÃ©marrage automatique 
      //==========================================
      arr_debut =  Arro[yy].Ademarrage;
      arr_fin = Arro[yy].Ademarrage + Arro[yy].Aduree ;
      //      Serial.print(Arro[yy].Akey + " -- ");
      //      Serial.print(String(arr_debut));
      //      Serial.print(" -- " + String(arr_fin)); 
      //      Serial.print(" Minute Now " + String(MinuteNow ));
      //      Serial.print(" Day Now " + String(Daynow) + " " + String(Arro[yy].Afrequence));
      //      if (Arro[yy].Aactivation) {Serial.print(" activation true" );}
      //      if (!Arro[yy].Aactivation) {Serial.print(" activation false") ;}
      //      Serial.println (" Arroseur   " + String(Arro[yy].Aarroseur));
	      if ((!Arro[yy].Automatique) && (!Manuel[Arro[yy].Aarroseur]))  
            {
              String xxxx = Arro[yy].Afrequence; 
              int yyyy = xxxx.indexOf(Daynow); 
              
			        cDay = ((yyyy > -1 )  );
			        
			        if ( (cDay) and (arr_debut < MinuteNow) and (arr_fin > MinuteNow)  and (Arro[yy].Aactivation))
			            {
			              int xxx = Arro[yy].Aarroseur+5;
			              Trace("Démarrage automatique de " + String(Arro[yy].Akey)+ "  "+ String(xxx));
			              PBSend(3,Arro[yy].Aarroseur,0);
			              digitalWrite(xxx,HIGH);
			              Arro[yy].Automatique=true;
                  }
            }
        
        //==========================================
        //            ArrÃªt automatique 
        //==========================================
  
        if ((arr_fin < MinuteNow) and (Arro[yy].Automatique))
          {  
            Trace("Arret automatique de " + String(Arro[yy].Akey));
	  	      PBSend(4,Arro[yy].Aarroseur,0 );
	  	      digitalWrite(Arro[yy].Aarroseur+5,LOW);
            Arro[yy].Automatique=false;
          }
      }
   }
}

long detectV(int  starter) 
{
  debutV=finV+1;
  int xx = strJSON.indexOf(";", debutV);
  return xx ;
   
}

void RemplirSession (int CurrIndex)
{
		Serial.println("RemplirSession");  
		
    String temp;
    debutV=strJSON.indexOf("{",debutV)+1; 
    finV = strJSON.indexOf(";", debutV);  
    Arro[CurrIndex].Akey=strJSON.substring(debutV,finV);
    Serial.println(" ===key = " + Arro[CurrIndex].Akey  );
      
      
    finV = detectV(debutV);  
    if (strJSON.substring(debutV,finV)== "1")
    {
      Arro[CurrIndex].Aactivation=true;
      Serial.print(" ===activation " );
    Serial.println("True");
    }
    else
    {
      Arro[CurrIndex].Aactivation=false;
      Serial.print(" ===activation " );
    Serial.println("False ")  ;
    }
    
    
    finV = detectV(debutV); 
    temp = strJSON.substring(debutV,finV);
    Arro[CurrIndex].Aduree= temp.toInt();
    Serial.print(" ===duree ");
    Serial.println( Arro[CurrIndex].Aduree  );


    finV = detectV(debutV);   
    temp = strJSON.substring(debutV,finV);
    Arro[CurrIndex].Aarroseur=temp.toInt();
    Serial.print(" ===arroseur ");
    Serial.println(Arro[CurrIndex].Aarroseur  );

      
      
    finV = detectV(debutV);   
    temp = strJSON.substring(debutV,finV);
    Arro[CurrIndex].Ademarrage=temp.toInt();
    Serial.print(" ===demarrage ");
    Serial.println( Arro[CurrIndex].Ademarrage  );

    debutV= finV+1;
    finV= strJSON.indexOf("}", debutV); 
     
    Arro[CurrIndex].Afrequence= strJSON.substring(debutV,finV);
    Serial.print(" ===frequence " );
    Serial.println(Arro[CurrIndex].Afrequence  );
    }

int ParsePayload(int transfo)
{
  if (transfo==1) 
    {
      String temp;
      int idxArro=0;
      debutV=0; 
      finV=1;
      while (strJSON.indexOf("{", finV) > -1 ) 
        { 
          RemplirSession(idxArro);
          idxArro++;
        }
      MaxIdx=idxArro;
      Serial.print("Sessions intégrées : ");
      Serial.println(MaxIdx);
    }
  if (transfo==2)
    {
      MaxManuel1 = 30 ; 
      MaxManuel2 = 60;  
      Serial.print("Max1 ");
      Serial.println(MaxManuel1);
      Serial.print("Max2 ");
      Serial.println(MaxManuel2);
    }
  return 0; 
}




void SendHTTPRequest(String req, int transfo) {
  if (client2.connect("boat.alwaysdata.net", 80)) 
  {
    Serial.println("connected");
    client2.println(req);
    client2.println("Host: boat.alwaysdata.net");
    client2.println("Connection: close");
    delay(100);
    client2.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  delay(1000);
  ecrire=false;
  strJSON="";
  //int counter = 0;
  //String last; 
  
  while (client2.available())
  {
    char c = client2.read();
    if (c == '{')  { ecrire = true; }
    if (c=='\n') { ecrire = false; }
    if (ecrire) 
    {
      strJSON += c;
     }
  }
 
  // if the server's disconnected, stop the client:
  if (!client2.connected()) {
      client2.stop();
      ParsePayload(transfo);
  }
  client2.stop();
}



void doaction(){
	String MonBuffer = (buffer);
  char *resultat; 
  resultat =strstr(buffer,"arro");
  for( int i = 0; i < sizeof(buffer);  ++i )
  buffer[i] = (char)0;
  
  ////////////////////////////////////////////////////
  // Arrosage Manuel  
  ////////////////////////////////////////////////////
	if (MonBuffer.indexOf("arro=") > 0) 
		{
		  Serial.print (" Arrosage manuel" + MonBuffer);
		int MonArro =  MonBuffer.substring(6,7).toInt();
		Serial.print(" Action ");
		Serial.println(MonBuffer.substring(7,11));
		if (MonBuffer.substring(7,11) == "true") 
			{
			Manuel[MonArro]=true;
			Trace("HIGH "   + String(MonArro) );
			digitalWrite(MonArro+5,HIGH); 
			PBSend(1,  MonArro,0);
			DemManuel[MonArro]= millis();  
      
			Prevenu[MonArro]=false;
			ReponseServeur("arro"+String(MonArro)+"on");

			}
		else 
			{
			Manuel[MonArro]=false;
			Trace("LOWXXXX"   + String(MonArro) );
			DemManuel[MonBuffer.substring(6,7).toInt()]= 0;
			digitalWrite(MonArro+5,LOW);
			PBSend(2,MonArro,0);
			ReponseServeur("arro"+String(MonArro)+"off");
			}
		}
	/////////////////////////////////////////////////////////////////
  ////  Mises à jour données automatiques 
	/////////////////////////////////////////////////////////////////

	if (MonBuffer.indexOf("action=periodeupdate") > 0)  
		{
		Serial.println("periodeupdate");  
    SendHTTPRequest("GET /sgdmdomo.php?action=listeperiode HTTP/1.1",1);
		ReponseServeur("periodeupdate");
		}

  //creation 
	if (MonBuffer.indexOf("action=createarro") > 0)  
		{
		  int deba= MonBuffer.indexOf("{")+1;
		  int fina= MonBuffer.indexOf("}");
		  
		  strJSON = MonBuffer.substring(deba,fina);
		  Serial.println(strJSON);
		
      RemplirSession(MaxIdx);
      MaxIdx++;
		}
	//suppression 
	if (MonBuffer.indexOf("action=supprarro") > 0)  
		{
	int deb = MonBuffer.indexOf("key=")+4; 
	
		String CurrKey = MonBuffer.substring(deb); 
		CurrKey.trim();
		Serial.println(CurrKey.length());
		String Tempxxx = Arro[2].Akey;
		Serial.println(Tempxxx.length());
		Serial.print ("Key à supprimer " ); 
		Serial.println(CurrKey); 
		for (int i = 0; i <= MaxIdx-1; i++) 
		  {
		    Serial.println(Arro[i].Akey);
		    if (CurrKey == Arro[i].Akey) 
		    {
		      Arro[i].Akey  = "";
		      Serial.print("session supprimée " ); 
		      Serial.println(CurrKey);
		      return;
		    }
		  }
		}
	//modification 
	if (MonBuffer.indexOf("action=modifarro") > 0)  
		{
	
	    int deba= MonBuffer.indexOf("{")+1;
		  int fina= MonBuffer.indexOf("}");
		  strJSON = MonBuffer.substring(deba,fina);
		  int debutb=strJSON.indexOf("{")+1; 
      int finb = strJSON.indexOf(";");  
		  String CurrKey = strJSON.substring(debutb,finb); 
		  for (int i = 0; i <= MaxIdx-1; i++) 
		    {
		      if (CurrKey == Arro[i].Akey) 
		      {
		        RemplirSession(i);
		        return;
		      }
		    }
		  ReponseServeur("modifarro");
		  }
	if (MonBuffer.indexOf("action=maxupdate") > 0)  
		{
		Serial.println("maxupdate");  
		SendHTTPRequest("GET /sgdmdomo.php?action=listemax HTTP/1.1",2);
		ReponseServeur("maxupdate");
		}
	/////////////////////////////////////////////////////////////////
  // Données de service 
	/////////////////////////////////////////////////////////////////
	if (MonBuffer.indexOf("getstatus") > 0) 
		{
		ReponseServeur("getstatus");
		}
			if (MonBuffer.indexOf("refresh") > 0) 
		{
		SendHTTPRequest("GET /sgdmdomo.php?action=listeperiode HTTP/1.1",1);
		ReponseServeur("getstatus");
		}
}

boolean  ReceiveRequest (){
  client =  server.available();
	count=0; 
    if(client){
        boolean currentLineIsBlank = true;
        //boolean sentHeader = false;
        bonget=true;
		    while(client.connected()){
		        if(client.available()>0){
		            //Serial.print("available : ");
		            //Serial.println(client.available());
                char c = client.read();
                //Serial.print(c);
                if(reading && c == ' '){reading = false;}
                if(c == '?'){reading = true;}
                if(reading){
					          buffer[count++]=c; 
					          
					          
                }
                if(c == '\n' && currentLineIsBlank){
                  break;
                  return true;
                  }
                if(c == '\n'){
                    currentLineIsBlank = true;
                }else if(c != '\r'){
                    currentLineIsBlank = false;
                }
            }
        }
      delay(3);
      Serial.println(buffer);
      return true;
    }
    
    return false;
}


void DemarrageManuel()
//
// Forcage Arrêt manuel 
//
{
for (int Idx = 0; Idx < 4; Idx++) 
	{
	  
	 
	if (Manuel[Idx])
	{
  //Serial.println("------------------------------------------------------------------");
  //Serial.print("Max1 : ");
  //Serial.print(MaxManuel1); 
  //Serial.print("  Max2 : ");
  //Serial.print(MaxManuel2); 
  //Serial.print("  Demmarrage ");
  //Serial.print(DemManuel[Idx]); 
  //Serial.print("  Maintenant ");
  //Serial.print(millis()); 
  //Serial.print(" Calcul   ");
  //Serial.println((millis()-DemManuel[Idx])/60000);
  //Serial.println("------------------------------------------------------------------");  
	if ((millis()-DemManuel[Idx])/60000 > MaxManuel2) 
		{
		Manuel[Idx]=false;
		Trace( "Arret force "   + String(Idx) );
		DemManuel[Idx]=0;
		digitalWrite(Idx+5,LOW);
		PBSend(5,Idx,0);
		}
        else
		if (((millis()-DemManuel[Idx])/60000 > MaxManuel1) && (!Prevenu[Idx]))
        {

		Prevenu[Idx]=true;
		PBSend(6,  Idx,0);	
		}
	}
	}
}


//=========================================================
//                Loop 
//=========================================================


void loop() 
{
  if (timeClient.getMinutes() != old_minute)
	  {
    Automate();
    old_minute = timeClient.getMinutes(); 
    Trace("Minute :" + String(old_minute)); 
    
	  if (timeClient.getDay() != old_day)
	    {
        timeClient.update(); 
        old_day =timeClient.getDay();
        //SendHTTPRequest("GET /sgdmdomo.php?action=listeperiode HTTP/1.1",1);
        PBSend(10,0,1);
    
      }
	  }
  
  if (ReceiveRequest())  doaction();
	DemarrageManuel();
}












/////////////////////////////////////////////////////////////








void setup() {
  //************************************
  //  Acquisition des donnÃ©es MYSQL 
  //************************************
 Serial.begin(9600);

 Serial.println("C'est parti");
 pinMode(Arro1,OUTPUT);
 pinMode(Arro2,OUTPUT);
 pinMode(Arro3,OUTPUT);
 pinMode(Arro4,OUTPUT);
 
 digitalWrite(Arro1,LOW);
 digitalWrite(Arro2,LOW);
 digitalWrite(Arro3,LOW);
 digitalWrite(Arro4,LOW);
 
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  Serial.println(Ethernet.localIP());
  delay(1000);
  Serial.println("connecting...");
  timeClient.begin();
  delay(3000);
  timeClient.update();
  old_day = timeClient.getDay(); 
  old_day = 9; 
  SendHTTPRequest("GET /sgdmdomo.php?action=listeperiode HTTP/1.1",1);
  SendHTTPRequest("GET /sgdmdomo.php?action=listemax HTTP/1.1",2);
  
  
  for (int xx = 0; xx < 5; xx++) 
    {
      Manuel[xx]=false;
      DemManuel[xx]=0;
    }
	PBSend(9,0,0);
}