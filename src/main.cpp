/*
 * Copyright 2021 Guilherme Andrade
 *
 * See the included readme for instructions.
 * https://microcontrollerslab.com/esp32-pinout-use-gpio-pins/
 * https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
 * https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
 * 
 */

#include <main.h>

/* Componente: INTERFACE */
bool INTERFACE_tecladoA_event = false;
bool INTERFACE_tecladoB_event = false;
bool INTERFACE_tecladoC_event = false;
volatile int16_t USER_choice = 0;
LiquidCrystal_I2C lcd(0x27,INTERFACE_lcdColumns,INTERFACE_lcdRows);

/* Componente: SLOTs */
String SLOTENTRADA_nomeProduto[] = {"Produto 1","Produto 2","Produto 3","Produto 4","Produto 5","Produto 6"};
double SLOTENTRADA_posProduto[] = {400.0,800.0,1200.0,1600.0,2000.0,2400.0};
bool SLOTSAIDA_sensorIR1_event = false;
bool SLOTENTRADA_sensorTampa_event = false;
bool SLOTENTRADA_sensorIR1_event = false;
bool SLOTENTRADA_sensorIR2_event = false;
bool SLOTENTRADA_sensorIR3_event = false;
bool SLOTENTRADA_sensorIR4_event = false;
bool SLOTENTRADA_sensorIR5_event = false;
bool SLOTENTRADA_sensorIR6_event = false;

/* Componente: WIFI */
String device_name, wifi_network, wifi_pass, WIFI_ip, header; //variaveis de config. inicial
WiFiServer server(80); //variable to store the HTTP request
unsigned long currentTime = millis();
unsigned long previousTime = 0;

/* Componente: CARRO */
volatile double CARRO_posAtual = 0.0; //variavel para armazenar posicao do carro

/* Componente: SYSTEM */
bool TIMERA_burst = false;
uint16_t state = 0; //Variavel de estado da máquina de estados
Preferences preferences; //Biblioteca para salvar dados na FLASH
bool USER_device_configStatus = false; //Variavel para armazenar se device já foi configurado


/* Interruption Routines */

void onTime_TimerA(void)
{
    TIMERA_burst = true;
    stop_timerA();
}

void IRAM_ATTR isr_SLOTSAIDA_sensorIR1(void)
{
    gpio_intr_disable(GPIO_NUM_32);
    SLOTSAIDA_sensorIR1_event = true;
}

void IRAM_ATTR isr_EIXO_encoder_A(void) 
{
	if(((digitalRead(EIXO_encoder_A)==HIGH)&(digitalRead(EIXO_encoder_B)==LOW))||((digitalRead(EIXO_encoder_A)==LOW)&(digitalRead(EIXO_encoder_B)==HIGH)))
	{
		#ifdef ENCODER02_RESOLUTION_MULT_EN
	    	#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual - (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_4X));
			#else
		    	CARRO_posAtual--;
			#endif
		#else
			#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual - (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_1X));
			#else
				CARRO_posAtual--;
			#endif
		#endif
	}
	else
	{
		#ifdef ENCODER02_RESOLUTION_MULT_EN
	    	#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual + (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_4X));
			#else
		    	CARRO_posAtual++;
			#endif
		#else
			#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual + (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_1X));
			#else
				CARRO_posAtual++;
			#endif
		#endif		
	}
}

void IRAM_ATTR isr_EIXO_encoder_B(void) 
{
    if(((digitalRead(EIXO_encoder_A)==HIGH)&(digitalRead(EIXO_encoder_B)==HIGH))||((digitalRead(EIXO_encoder_A)==LOW)&(digitalRead(EIXO_encoder_B)==LOW)))
	{
		#ifdef ENCODER02_RESOLUTION_MULT_EN
	    	#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual - (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_4X));
			#else
		    	CARRO_posAtual--;
			#endif
		#else
			#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual - (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_1X));
			#else
				CARRO_posAtual--;
			#endif
		#endif
	}
	else
	{
		#ifdef ENCODER02_RESOLUTION_MULT_EN
	    	#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual + (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_4X));
			#else
		    	CARRO_posAtual++;
			#endif
		#else
			#ifndef ENCODER_IN_PULSE
				CARRO_posAtual = CARRO_posAtual + (360.0/double(ENCODER02_PPR*ENCODER02_RESOLUTION_MULT_1X));
			#else
				CARRO_posAtual++;
			#endif
		#endif
	}
}

void IRAM_ATTR isr_SLOTENTRADA_sensorIR1(void)
{
    gpio_intr_disable(GPIO_NUM_15);
    SLOTENTRADA_sensorIR1_event = true;
}

void IRAM_ATTR isr_SLOTENTRADA_sensorIR2(void)
{
    gpio_intr_disable(GPIO_NUM_3);
    SLOTENTRADA_sensorIR2_event = true;
}

void IRAM_ATTR isr_SLOTENTRADA_sensorIR3(void)
{
    gpio_intr_disable(GPIO_NUM_18);
    SLOTENTRADA_sensorIR3_event = true;
}

void IRAM_ATTR isr_SLOTENTRADA_sensorIR4(void)
{
    gpio_intr_disable(GPIO_NUM_16);
    SLOTENTRADA_sensorIR4_event = true;
}

void isr_SLOTENTRADA_sensorTampa(void)
{
    gpio_intr_disable(GPIO_NUM_5);
    SLOTENTRADA_sensorTampa_event = true;
    state = 8;
}

void isr_INTERFACE_tecladoA(void)
{
    gpio_intr_disable(GPIO_NUM_25);
    INTERFACE_tecladoA_event = true;
}

void isr_INTERFACE_tecladoB(void)
{
    gpio_intr_disable(GPIO_NUM_26);
    INTERFACE_tecladoB_event = true;
}

void isr_INTERFACE_tecladoC(void)
{
    gpio_intr_disable(GPIO_NUM_23);
    INTERFACE_tecladoC_event = true;
}

/* Functions */

void INTERFACE_setDisplay(String msgL1, String msgL2, uint16_t menuContex, uint16_t delay_time)
{
    /**
    * @param msgL1 Mensagem na linha 1
    * @param msgL2 Mensagem na linha 2
    * @param menuContex Contexto de menu 0: Standart - 1: Escolha - 2: Confirmar
    */
    
    //atualizar display LCD
    lcd.clear();

    /* Contexto de menu */
    switch(menuContex)
    {
        case 0:
            /* Contexto de menu: STANDART */
            lcd.setCursor(17, 0);
            lcd.print("---");
            break;

        case 1:
            /* Contexto de menu: ESCOLHA DE PRODUTO */
            lcd.setCursor(17, 0);
            lcd.print("---");
            lcd.setCursor(0, 3);
            lcd.print("-");
            lcd.setCursor(5, 3);
            lcd.print("+");
            lcd.setCursor(19, 3);
            lcd.print(">");
            break;

        case 2:
            /* Contexto de menu: CONFIRMACAO */
            lcd.setCursor(17, 0);
            lcd.print("---");
            lcd.setCursor(0, 3);
            lcd.print("<cancela");
            lcd.setCursor(11, 3);
            lcd.print("confirma>");
            break;
    }

    lcd.setCursor(1, 1);
    lcd.print(msgL1);
    
    lcd.setCursor(1, 2);
    lcd.print(msgL2);

    delay(delay_time);
}

void INTERFACE_enableAllKeys(void)
{
    //limpar flags de evento
    INTERFACE_tecladoA_event = false;
    INTERFACE_tecladoB_event = false;
    INTERFACE_tecladoC_event = false;
    
    //habilitar interrupcoes do teclado
    gpio_intr_enable(GPIO_NUM_25);
    gpio_intr_enable(GPIO_NUM_26);
    gpio_intr_enable(GPIO_NUM_23);
}

void INTERFACE_disableAllKeys(void)
{
    //limpar flags de evento
    INTERFACE_tecladoA_event = false;
    INTERFACE_tecladoB_event = false;
    INTERFACE_tecladoC_event = false;

    //desabilitar interrupcoes do teclado
    gpio_intr_disable(GPIO_NUM_25);
    gpio_intr_disable(GPIO_NUM_26);
    gpio_intr_disable(GPIO_NUM_23);
}

void INTERFACE_disableKey(int PIN)
{   
    switch (PIN)
    {
        case 25:
            INTERFACE_tecladoA_event = false;
            gpio_intr_disable(GPIO_NUM_25);
            break;
    
        case 26:
            INTERFACE_tecladoB_event = false;
            gpio_intr_disable(GPIO_NUM_26);
            break;

        case 23:
            INTERFACE_tecladoC_event = false;
            gpio_intr_disable(GPIO_NUM_23);
            break;

        default:
            break;
    }
}

bool WIFI_conn(const char* ssid, const char* password) 
{
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){INTERFACE_setDisplay("Conectando WiFi",String(ssid),0,WIFI_RECONN_TIME);}
    WIFI_ip = String(WiFi.localIP());
    
    return true;
}

void EIXO_movimentaMotor(char OPERATION)
{
    if(OPERATION=='A') //Movimento de AVANCO
    {
        digitalWrite(EIXO_releMotorA, LOW);
        while(CARRO_posAtual<=SLOTENTRADA_posProduto[USER_choice]){}
        digitalWrite(EIXO_releMotorA, HIGH);
    }
    else if(OPERATION=='Z') //Movimento de ZERAMENTO
    {
        digitalWrite(EIXO_releMotorB, LOW);
        while(CARRO_posAtual>8.0 || digitalRead(EIXO_chaveIDC)==HIGH){INTERFACE_setDisplay("Chave 0: "+String(digitalRead(EIXO_chaveIDC)),String(CARRO_posAtual),0,200);}
        digitalWrite(EIXO_releMotorB, HIGH);
    }
}

void CARRO_movimentaServo(void)
{
    //1,0ms = 51,2 pulsos = 0 grau
    //1,5ms = 76,8 pulsos = 90 grau
    //2,0ms = 102,4 pulsos = 180 grau
    noInterrupts();
    for(int i = 0; i < 102; i++) // até 180 graus
    {
      ledcWrite(0, i);//Escrevemos no canal 0, o duty cycle "i".
      delay(15);
    }
    delay(50);
    for(int i = 102; i > 0; i--) //até 0 grau
    {
      ledcWrite(0, i);
      delay(15);
    }
    interrupts();
}

int PRODUT_getStock(String SLOT)
{
    preferences.begin("PRODUT_estoque", false);
    uint8_t qtddProduto = uint8_t(preferences.getShort(SLOT.c_str(), 0));
    preferences.end();

    return qtddProduto;
}

bool PRODUT_setStock(String SLOT, char OPERATION)
{
    uint16_t qtddAtual = 0;
    bool isSuccess = false;
    
    preferences.begin("PRODUT_estoque", false);
    
    if(OPERATION == '+')
    {
        qtddAtual = preferences.getShort(SLOT.c_str(), 0);
        qtddAtual++;
        preferences.putShort(SLOT.c_str(), qtddAtual);
        isSuccess = true;
    }
    else if(OPERATION == '-')
    {
        qtddAtual = preferences.getShort(SLOT.c_str(), 0);
        qtddAtual--;
        preferences.putShort(SLOT.c_str(), qtddAtual);
        isSuccess = true;
    }

    preferences.end();

    return isSuccess;
}

void PRODUT_showAllStock(void)
{
    String abc = String(PRODUT_getStock(SLOTENTRADA_nomeProduto[0]));
    abc.concat("  "+String(PRODUT_getStock(SLOTENTRADA_nomeProduto[1]))+"  "+String(PRODUT_getStock(SLOTENTRADA_nomeProduto[2]))+"  "+String(PRODUT_getStock(SLOTENTRADA_nomeProduto[3]))+"  "+String(PRODUT_getStock(SLOTENTRADA_nomeProduto[4]))+"  "+String(PRODUT_getStock(SLOTENTRADA_nomeProduto[5])));
    INTERFACE_setDisplay("S1 S2 S3 S4 S5 S6",abc,0,1000);
}

void SLOT_disableAllSensorsIR(void)
{
    //limpar flags de evento
    SLOTSAIDA_sensorIR1_event = false;
    SLOTENTRADA_sensorIR1_event = false;
    SLOTENTRADA_sensorIR2_event = false;
    SLOTENTRADA_sensorIR3_event = false;
    SLOTENTRADA_sensorIR4_event = false;

    //desabilitar interrupcoes
    gpio_intr_disable(GPIO_NUM_32); //Desabilita interrupcao do SLOTSAIDA_sensorIR1
    gpio_intr_disable(GPIO_NUM_15); //Desabilita interrupcao do SLOTENTRADA_sensorIR1
    gpio_intr_disable(GPIO_NUM_3);  //Desabilita interrupcao do SLOTENTRADA_sensorIR2
    gpio_intr_disable(GPIO_NUM_18); //Desabilita interrupcao do SLOTENTRADA_sensorIR2
    gpio_intr_disable(GPIO_NUM_16); //Desabilita interrupcao do SLOTENTRADA_sensorIR2
}

void SLOT_enableAllSensorsIR(void)
{
    //limpar flags de evento
    SLOTSAIDA_sensorIR1_event = false;
    SLOTENTRADA_sensorIR1_event = false;
    SLOTENTRADA_sensorIR2_event = false;
    SLOTENTRADA_sensorIR3_event = false;
    SLOTENTRADA_sensorIR4_event = false;

    //desabilitar interrupcoes
    gpio_intr_enable(GPIO_NUM_32); //Desabilita interrupcao do SLOTSAIDA_sensorIR1
    gpio_intr_enable(GPIO_NUM_15); //Desabilita interrupcao do SLOTENTRADA_sensorIR1
    gpio_intr_enable(GPIO_NUM_3);  //Desabilita interrupcao do SLOTENTRADA_sensorIR2
    gpio_intr_enable(GPIO_NUM_18); //Desabilita interrupcao do SLOTENTRADA_sensorIR2
    gpio_intr_enable(GPIO_NUM_16); //Desabilita interrupcao do SLOTENTRADA_sensorIR2
}

unsigned char SYSTEM_h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

String SYSTEM_urlDecode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (SYSTEM_h2int(code0) << 4) | SYSTEM_h2int(code1);
        encodedString+=c;
      } else{
        
        encodedString+=c;  
      }
      
      yield();
    }
    
   return encodedString;
}

String SYSTEM_getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void SYSTEM_resetWifiParam(void)
{   
    server.begin();
    WiFiClient client = server.available(); //listen for incoming clients
    if(client) //If a new client connects...
    {
        currentTime = millis();
        previousTime = currentTime;
        String currentLine = ""; //make a String to hold incoming data from the client

        while(client.connected() && currentTime - previousTime <= WIFI_timeoutTime) //loop while the client's connected
        {
            currentTime = millis();
            if (client.available()) //if there's bytes to read from the client...
            { 
                char c = client.read(); // read a byte, then
                //Serial.write(c); // print it out the serial monitor
                header += c;
                if (c == '\n')
                {   // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons 
                        // Feel free to change the background-color and font-size attributes to fit your preferences
                        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                        client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                        client.println(".button2 {background-color: #555555;}</style></head>");

                        // Receber resposta do client e tratar dados
                        header = SYSTEM_getValue(header,'\r',0);
                        header = SYSTEM_getValue(header,'?',1);
                        header = SYSTEM_getValue(header,' ',0); //device_name=xx&wifi_network=yy&wifi_pass=zz

                        device_name = SYSTEM_getValue(header,'&',0);
                        device_name = SYSTEM_urlDecode(SYSTEM_getValue(device_name,'=',1));

                        wifi_network = SYSTEM_getValue(header,'&',1);
                        wifi_network = SYSTEM_urlDecode(SYSTEM_getValue(wifi_network,'=',1));

                        wifi_pass = SYSTEM_getValue(header,'&',2);
                        wifi_pass = SYSTEM_urlDecode(SYSTEM_getValue(wifi_pass,'=',1));

                        // Web Page Heading
                        client.println("<body><h1>Configurar Vending Machine</h1>");

                        // Capturar os inputs do formulario web
                        client.write("<form method=GET>Nome do dispositivo: <input type=text name=device_name></br>");
                        client.write("Rede Wifi: <input type=text name=wifi_network></br>");
                        client.write("Senha Wifi: <input type=text name=wifi_pass></br>");
                        client.write("<input type=submit></form>");

                        client.println("</body></html>");

                        client.println(); //the HTTP response ends with another blank line

                        break; //break out of the while loop

                    }
                    else{currentLine = "";}//if you got a newline, then clear currentLine
                }
                else if(c != '\r') //if you got anything else but a carriage return character,
                { 
                    currentLine += c; //add it to the end of the currentLine
                }
            }
        }

        header = ""; //clear the header variable
        client.stop(); //close the connection
        if(wifi_pass.length()>0 && wifi_network.length()>0) //recebeu dados de client
        {
            WIFI_conn(wifi_network.c_str(),wifi_pass.c_str()); //Conecta ao WiFi com dados recebidos

            // Cria preferences para armazenar dados na FLASH

            // Dados de WiFi
            preferences.begin("USER_network", false);
            preferences.clear();
            preferences.putString("WIFIssid", wifi_network);
            preferences.putString("WIFIpass", wifi_pass);
            preferences.end();

            delay(2000);
            ESP.restart();
        }
    }
}

bool SYSTEM_createAccessPoint(void)
{
    WiFi.disconnect();
    WiFi.softAP("Vend-Machine","123456789"); //Connect to Wi-Fi network
    IPAddress IP = WiFi.softAPIP();
    WIFI_ip = IP.toString(); 

    INTERFACE_setDisplay("Wifi: Vend-Machine","Senha: 123456789",0,5000);
    INTERFACE_setDisplay("Acesse no celular:",WIFI_ip,0,0);

    return true;
}

void SYSTEM_hiddenMenu(void)
{
    ulong buttonTimer = 0;
    ulong longPressTime1 = SYSTEM_longPress1;
    ulong longPressTime2 = SYSTEM_longPress2;
    ulong longPressTime3 = SYSTEM_longPress3;
    set_timer(&onTime_TimerA,1,(longPressTime3 + 1000));  // Timer Interruption each 4s
    while(!TIMERA_burst)
    {
        if(digitalRead(INTERFACE_tecladoC) == HIGH){buttonTimer = millis();}
        if(millis() - buttonTimer > longPressTime1){INTERFACE_setDisplay("longpress","1s detect.",0,750);state = 10;}
        if(millis() - buttonTimer > longPressTime2){INTERFACE_setDisplay("longpress","2s detect.",0,750);state = 11;}
        if(millis() - buttonTimer > longPressTime3){INTERFACE_setDisplay("longpress","4s detect.",0,750);state = 12;break;}
    }
}

void SYSTEM_menu3(void)
{
    INTERFACE_setDisplay("Hidden menu:","Reset device.",0,2000);
    preferences.begin("USER_device", false);
    preferences.putBool("DEVICE_INIT", false); //Dado para salvar que dispositivo deve ser iniciado
    preferences.end();
}

void SYSTEM_menu2(void)
{
    INTERFACE_setDisplay("Hidden menu:","Reset WiFi.",0,2000);
    SYSTEM_createAccessPoint();
    SYSTEM_resetWifiParam();
}

void SYSTEM_menu1(void)
{
    INTERFACE_setDisplay("Hidden menu:","Show Stock.",0,2000);
    PRODUT_showAllStock();
}

void SYSTEM_enableISRpin(int PIN)
{   
    switch(PIN)
    {
        case 25:
            INTERFACE_tecladoA_event = false;
            gpio_intr_enable(GPIO_NUM_25);
            break;

        case 26:
            INTERFACE_tecladoB_event = false;
            gpio_intr_enable(GPIO_NUM_26);
            break;

        case 23:
            INTERFACE_tecladoC_event = false;
            gpio_intr_enable(GPIO_NUM_23);
            break;
        
        case 15:
            SLOTENTRADA_sensorIR1_event = false;
            gpio_intr_enable(GPIO_NUM_15);
            break;
    
        case 3:
            SLOTENTRADA_sensorIR2_event = false;
            gpio_intr_enable(GPIO_NUM_3);
            break;

        case 18:
            SLOTENTRADA_sensorIR3_event = false;
            gpio_intr_enable(GPIO_NUM_18);
            break;

        case 16:
            SLOTENTRADA_sensorIR4_event = false;
            gpio_intr_enable(GPIO_NUM_16);
            break;

        case 34:
            SLOTENTRADA_sensorIR5_event = false;
            gpio_intr_enable(GPIO_NUM_34);
            break;

        case 7:
            SLOTENTRADA_sensorIR6_event = false;
            gpio_intr_enable(GPIO_NUM_7);
            break;

        case 32:
            SLOTSAIDA_sensorIR1_event = false;
            gpio_intr_enable(GPIO_NUM_32);
            break;

        case 5:
            SLOTENTRADA_sensorTampa_event = false;
            gpio_intr_enable(GPIO_NUM_5);
            break;

        default:
            break;
    }
}

void SYSTEM_disableISRpin(int PIN)
{   
    switch(PIN)
    {
        case 25:
            INTERFACE_tecladoA_event = false;
            gpio_intr_disable(GPIO_NUM_25);
            break;

        case 26:
            INTERFACE_tecladoB_event = false;
            gpio_intr_disable(GPIO_NUM_26);
            break;

        case 23:
            INTERFACE_tecladoC_event = false;
            gpio_intr_disable(GPIO_NUM_23);
            break;
        
        case 15:
            SLOTENTRADA_sensorIR1_event = false;
            gpio_intr_disable(GPIO_NUM_15);
            break;
    
        case 3:
            SLOTENTRADA_sensorIR2_event = false;
            gpio_intr_disable(GPIO_NUM_3);
            break;

        case 18:
            SLOTENTRADA_sensorIR3_event = false;
            gpio_intr_disable(GPIO_NUM_18);
            break;

        case 16:
            SLOTENTRADA_sensorIR4_event = false;
            gpio_intr_disable(GPIO_NUM_16);
            break;

        case 34:
            SLOTENTRADA_sensorIR5_event = false;
            gpio_intr_disable(GPIO_NUM_34);
            break;

        case 7:
            SLOTENTRADA_sensorIR6_event = false;
            gpio_intr_disable(GPIO_NUM_7);
            break;

        case 32:
            SLOTSAIDA_sensorIR1_event = false;
            gpio_intr_disable(GPIO_NUM_32);
            break;

        case 5:
            SLOTENTRADA_sensorTampa_event = false;
            gpio_intr_disable(GPIO_NUM_5);
            break;

        default:
            break;
    }
}

/* FSM States */

void STATE_start(void)
{
    server.begin();
    WiFiClient client = server.available(); //listen for incoming clients
    if(client) //If a new client connects...
    {
        currentTime = millis();
        previousTime = currentTime;
        String currentLine = ""; //make a String to hold incoming data from the client

        while(client.connected() && currentTime - previousTime <= WIFI_timeoutTime) //loop while the client's connected
        {
            currentTime = millis();
            if (client.available()) //if there's bytes to read from the client...
            { 
                char c = client.read(); // read a byte, then
                //Serial.write(c); // print it out the serial monitor
                header += c;
                if (c == '\n')
                {   // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons 
                        // Feel free to change the background-color and font-size attributes to fit your preferences
                        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                        client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                        client.println(".button2 {background-color: #555555;}</style></head>");

                        // Receber resposta do client e tratar dados
                        header = SYSTEM_getValue(header,'\r',0);
                        header = SYSTEM_getValue(header,'?',1);
                        header = SYSTEM_getValue(header,' ',0); //device_name=xx&wifi_network=yy&wifi_pass=zz

                        device_name = SYSTEM_getValue(header,'&',0);
                        device_name = SYSTEM_urlDecode(SYSTEM_getValue(device_name,'=',1));

                        wifi_network = SYSTEM_getValue(header,'&',1);
                        wifi_network = SYSTEM_urlDecode(SYSTEM_getValue(wifi_network,'=',1));

                        wifi_pass = SYSTEM_getValue(header,'&',2);
                        wifi_pass = SYSTEM_urlDecode(SYSTEM_getValue(wifi_pass,'=',1));

                        // Web Page Heading
                        client.println("<body><h1>Configurar Vending Machine</h1>");

                        // Capturar os inputs do formulario web
                        client.write("<form method=GET>Nome do dispositivo: <input type=text name=device_name></br>");
                        client.write("Rede Wifi: <input type=text name=wifi_network></br>");
                        client.write("Senha Wifi: <input type=text name=wifi_pass></br>");
                        client.write("<input type=submit></form>");

                        client.println("</body></html>");

                        client.println(); //the HTTP response ends with another blank line

                        break; //break out of the while loop

                    }
                    else{currentLine = "";}//if you got a newline, then clear currentLine
                }
                else if(c != '\r') //if you got anything else but a carriage return character,
                { 
                    currentLine += c; //add it to the end of the currentLine
                }
            }
        }

        header = ""; //clear the header variable
        client.stop(); //close the connection
        if(wifi_pass.length()>0 && wifi_network.length()>0) //recebeu dados de client
        {
            WIFI_conn(wifi_network.c_str(),wifi_pass.c_str()); //Conecta ao WiFi com dados recebidos
            
            // Cria preferences para armazenar dados na FLASH
            // Dados de estoque
            preferences.begin("PRODUT_estoque", false);
            preferences.clear();
            preferences.putShort(SLOTENTRADA_nomeProduto[0].c_str(), 0);
            preferences.putShort(SLOTENTRADA_nomeProduto[1].c_str(), 0);
            preferences.putShort(SLOTENTRADA_nomeProduto[2].c_str(), 0);
            preferences.putShort(SLOTENTRADA_nomeProduto[3].c_str(), 0);
            preferences.putShort(SLOTENTRADA_nomeProduto[4].c_str(), 0);
            preferences.putShort(SLOTENTRADA_nomeProduto[5].c_str(), 0);
            preferences.end(); 

            // Dados de WiFi
            preferences.begin("USER_network", false);
            preferences.clear();
            preferences.putString("WIFIssid", wifi_network);
            preferences.putString("WIFIpass", wifi_pass);
            preferences.end();

            // Dados de usuario
            preferences.begin("USER_device", false);
            preferences.clear();
            preferences.putBool("DEVICE_INIT", true); //Dado para salvar que dispositivo foi iniciado
            preferences.end();

            delay(2000);
            ESP.restart();
        }
    }
}

void STATE_escolheProduto(void)
{
    if(INTERFACE_tecladoA_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(INTERFACE_tecladoA) == HIGH)
        {
            USER_choice--;
            if(USER_choice < 0){USER_choice = 5;}
            INTERFACE_setDisplay("Produto escolhido:",SLOTENTRADA_nomeProduto[USER_choice],1,0);
        }
        SYSTEM_enableISRpin(INTERFACE_tecladoA);
    }
    else if(INTERFACE_tecladoB_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(INTERFACE_tecladoB) == HIGH)
        {
            USER_choice++;
            if(USER_choice >= 6){USER_choice = 0;}
            INTERFACE_setDisplay("Produto escolhido:",SLOTENTRADA_nomeProduto[USER_choice],1,0);
        }
        SYSTEM_enableISRpin(INTERFACE_tecladoB);
    }
    else if(INTERFACE_tecladoC_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(INTERFACE_tecladoC) == HIGH)
        {
            if(PRODUT_getStock(SLOTENTRADA_nomeProduto[USER_choice])>0)
            {
                INTERFACE_setDisplay("Confirmar compra?",SLOTENTRADA_nomeProduto[USER_choice],2,0);
                state = 3;
            }
            else{INTERFACE_setDisplay("Item sem","estoque.",0,750);state = 1;}
        }
        SYSTEM_enableISRpin(INTERFACE_tecladoC);
    }
}

void STATE_confirmaCompra(void)
{
    if(INTERFACE_tecladoA_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(INTERFACE_tecladoA) == HIGH){state = 1;}
        SYSTEM_enableISRpin(INTERFACE_tecladoA);
    }
    else if(INTERFACE_tecladoB_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(INTERFACE_tecladoB) == HIGH){state = 1;}
        SYSTEM_enableISRpin(INTERFACE_tecladoB);
    }
    else if(INTERFACE_tecladoC_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(INTERFACE_tecladoC) == HIGH){state = 4;}
        SYSTEM_enableISRpin(INTERFACE_tecladoC);
    }
}

void STATE_liberarProduto(void)
{  
    //a) Movimenta carro ate a posicao escolhida
    INTERFACE_setDisplay("Iniciando","Motor IDA...",0,0);
    EIXO_movimentaMotor('A');

    //b) Aciona servomotor
    SYSTEM_enableISRpin(SLOTSAIDA_sensorIR1);
    INTERFACE_setDisplay("Iniciando","Servomotor...",0,0);
    CARRO_movimentaServo();
    INTERFACE_setDisplay("Verificando","saida produto...",0,0);
    state = 7;
}

void STATE_caidaProduto(void)
{
    //c) Verifica se produto caiu
    if(SLOTSAIDA_sensorIR1_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(SLOTSAIDA_sensorIR1) == HIGH)
        {	
            if(PRODUT_setStock(SLOTENTRADA_nomeProduto[USER_choice],'-')){INTERFACE_setDisplay("Estoque","atualizado.",0,750);}
            
            //d) Movimenta carro até posicao zero
            INTERFACE_setDisplay("Iniciando","Motor ZERO...",0,750);
            EIXO_movimentaMotor('Z');
            state = 1;
        }

        SYSTEM_enableISRpin(SLOTSAIDA_sensorIR1); //habilita sensor
    }
}

void STATE_confirmarPgto(void)
{
    INTERFACE_disableAllKeys();
    INTERFACE_setDisplay("Aguardando","pagamento...",0,1000);
    state = 6;
}

void STATE_criarOrdemCompra(void)
{
    INTERFACE_setDisplay("Criando ordem de","compra no server...",0,1000);
    SYSTEM_disableISRpin(SLOTENTRADA_sensorTampa); //desabilita sensor SLOTENTRADA_sensorTampa
    state = 5;
}

void STATE_tampaAberta(void)
{
    if(SLOTENTRADA_sensorTampa_event == true)
    {
        SLOT_disableAllSensorsIR();

        delay(DEBOUNCE_TIME);
        if(digitalRead(SLOTENTRADA_sensorTampa) == LOW)
        {
            INTERFACE_setDisplay("Tampa aberta.","Insira com cuidado",0,0);

            SYSTEM_enableISRpin(SLOTENTRADA_sensorIR1);
            SYSTEM_enableISRpin(SLOTENTRADA_sensorIR2);
            SYSTEM_enableISRpin(SLOTENTRADA_sensorIR3);
            SYSTEM_enableISRpin(SLOTENTRADA_sensorIR4);
            state = 9;
        }
        else if(digitalRead(SLOTENTRADA_sensorTampa) == HIGH)
        {
            INTERFACE_setDisplay("Tampa fechada.","",0,750);
            state = 1;
        }

        SYSTEM_enableISRpin(SLOTENTRADA_sensorTampa);
    }
}

void STATE_inserirProduto(void)
{
    if(SLOTENTRADA_sensorIR1_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(SLOTENTRADA_sensorIR1) == HIGH)
        {
            PRODUT_setStock(SLOTENTRADA_nomeProduto[0],'+');
            INTERFACE_setDisplay("+1 registrado","em SLOT1.",0,750);
            INTERFACE_setDisplay("Tampa aberta.","Insira com cuidado",0,0);
        }

        SYSTEM_enableISRpin(SLOTENTRADA_sensorIR1); //habilita sensor
    }
    else if(SLOTENTRADA_sensorIR2_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(SLOTENTRADA_sensorIR2) == HIGH)
        {
            PRODUT_setStock(SLOTENTRADA_nomeProduto[1],'+');
            INTERFACE_setDisplay("+1 registrado","em SLOT2.",0,750);
            INTERFACE_setDisplay("Tampa aberta.","Insira com cuidado",0,0);
        }

        SYSTEM_enableISRpin(SLOTENTRADA_sensorIR2); //habilita sensor
    }
    else if(SLOTENTRADA_sensorIR3_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(SLOTENTRADA_sensorIR3) == HIGH)
        {
            PRODUT_setStock(SLOTENTRADA_nomeProduto[2],'+');
            INTERFACE_setDisplay("+1 registrado","em SLOT3.",0,750);
            INTERFACE_setDisplay("Tampa aberta.","Insira com cuidado",0,0);
        }

        SYSTEM_enableISRpin(SLOTENTRADA_sensorIR3); //habilita sensor
    }
    else if(SLOTENTRADA_sensorIR4_event == true)
    {
        delay(DEBOUNCE_TIME);
        if(digitalRead(SLOTENTRADA_sensorIR4) == HIGH)
        {
            PRODUT_setStock(SLOTENTRADA_nomeProduto[3],'+');
            INTERFACE_setDisplay("+1 registrado","em SLOT4.",0,750);
            INTERFACE_setDisplay("Tampa aberta.","Insira com cuidado",0,0);
        }

        SYSTEM_enableISRpin(SLOTENTRADA_sensorIR4); //habilita sensor
    }
}

/* Execution */

void setup()
{
    // Start
    pinMode(EIXO_chaveIDC, INPUT_PULLDOWN);
    pinMode(EIXO_releMotorB, INPUT_PULLDOWN);
    pinMode(EIXO_releMotorA, INPUT_PULLDOWN);
    pinMode(EIXO_encoder_A, INPUT_PULLDOWN);
    pinMode(EIXO_encoder_B, INPUT_PULLDOWN);
    pinMode(SLOTSAIDA_sensorIR1, INPUT_PULLDOWN);
    pinMode(SLOTENTRADA_sensorIR1, INPUT_PULLDOWN);
    pinMode(SLOTENTRADA_sensorIR2, INPUT_PULLDOWN);
    pinMode(SLOTENTRADA_sensorIR3, INPUT_PULLDOWN);
    pinMode(SLOTENTRADA_sensorIR4, INPUT_PULLDOWN);
    pinMode(SLOTENTRADA_sensorTampa, INPUT_PULLDOWN);
    pinMode(INTERFACE_tecladoA, INPUT_PULLDOWN);
    pinMode(INTERFACE_tecladoB, INPUT_PULLDOWN);
    pinMode(INTERFACE_tecladoC, INPUT_PULLDOWN);
    pinMode(CARRO_servomotor, INPUT_PULLDOWN);
    
    // UART & LCD
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    
    // GPIO Pinout
    pinMode(EIXO_releMotorA, OUTPUT);
    pinMode(EIXO_releMotorB, OUTPUT);
    pinMode(EIXO_encoder_A, INPUT_PULLUP);
    pinMode(EIXO_encoder_B, INPUT_PULLUP);
    pinMode(INTERFACE_tecladoA, INPUT_PULLUP);
    pinMode(INTERFACE_tecladoB, INPUT_PULLUP);
    pinMode(INTERFACE_tecladoC, INPUT_PULLUP);
    pinMode(CARRO_servomotor, OUTPUT);

    //Reles acionam em nível lógico LOW
    digitalWrite(EIXO_releMotorA, HIGH);
    digitalWrite(EIXO_releMotorB, HIGH);

    // PWM & Servos
    ledcAttachPin(CARRO_servomotor, 0); //Atribuimos o pino CARRO_servomotor ao canal 0.
    ledcSetup(0, 50, 10); //Canal 0 recebe frequencia de 50Hz (20ms) com resolucao de 10bits (1024).

    // Interruptions
    attachInterrupt(INTERFACE_tecladoA, isr_INTERFACE_tecladoA, RISING);
    attachInterrupt(INTERFACE_tecladoB, isr_INTERFACE_tecladoB, RISING);
    attachInterrupt(INTERFACE_tecladoC, isr_INTERFACE_tecladoC, RISING);
    attachInterrupt(SLOTSAIDA_sensorIR1, isr_SLOTSAIDA_sensorIR1, RISING);
    attachInterrupt(SLOTENTRADA_sensorIR1, isr_SLOTENTRADA_sensorIR1, RISING);
    attachInterrupt(SLOTENTRADA_sensorIR2, isr_SLOTENTRADA_sensorIR2, RISING);
    attachInterrupt(SLOTENTRADA_sensorIR3, isr_SLOTENTRADA_sensorIR3, RISING);
    attachInterrupt(SLOTENTRADA_sensorIR4, isr_SLOTENTRADA_sensorIR4, RISING);
    attachInterrupt(SLOTENTRADA_sensorTampa, isr_SLOTENTRADA_sensorTampa, RISING);

    #ifdef ENCODER02_RESOLUTION_MULT_EN
    	attachInterrupt(EIXO_encoder_A, isr_EIXO_encoder_A, CHANGE);
    	attachInterrupt(EIXO_encoder_B, isr_EIXO_encoder_B, CHANGE);
    #else
    	attachInterrupt(EIXO_encoder_A, EN02_A_isr, RISING);
    #endif	

    // Timers
    //set_timer(&onTime_TimerA,1,2000);  // Timer Interruption each 0,
    //set_timer(&onTime_TimerB,2,1000); // Timer Interruption each 1s
    //set_timer(&onTime_TimerC,3,2000); // Timer Interruption each 2s

    preferences.begin("USER_device", false);
    USER_device_configStatus = preferences.getBool("DEVICE_INIT", false);
    preferences.end();
    
    INTERFACE_setDisplay("Inicializando","Wifi...",0,750);
    if(USER_device_configStatus == true) //Dispositivo foi configurado
    {
        // Conectar ao WIFI com credenciais salvas na FLASH
        preferences.begin("USER_network", false);
        wifi_network = preferences.getString("WIFIssid", " ");
        wifi_pass = preferences.getString("WIFIpass", " ");
        preferences.end();

        if(WIFI_conn(wifi_network.c_str(),wifi_pass.c_str()) == true){INTERFACE_setDisplay("WiFi conectado","com sucesso.",0,750);}
        else{INTERFACE_setDisplay("Sem conexao com","rede WiFi.",0,750);}
        
        state = 1;
        INTERFACE_setDisplay("Inicializando","menus...",0,250);
        SYSTEM_hiddenMenu();
    }
    else //Dispositivo nao foi configurado
    {
        WiFi.disconnect();
        WiFi.softAP("Vend-Machine","123456789"); //Connect to Wi-Fi network
        IPAddress IP = WiFi.softAPIP();
        WIFI_ip = IP.toString(); 

        INTERFACE_setDisplay("Wifi: Vend-Machine","Senha: 123456789",0,5000);
        INTERFACE_setDisplay("Acesse:",WIFI_ip,0,0);
        state = 0;
    }
}

void loop()
{   
    switch(state)
    {
        case 0:
        {            
            STATE_start();
            break;
        }
        
        case 1:

            USER_choice = 0;
            SYSTEM_enableISRpin(SLOTENTRADA_sensorTampa); //habilita sensor SLOTENTRADA_sensorTampa
            SLOT_disableAllSensorsIR(); //desabilita todos os sensores de entrada/saida de produto
            INTERFACE_enableAllKeys(); //habilita todos os pushbuttons do teclado
            PRODUT_showAllStock(); //mostra estoque salvo na FLASH
            INTERFACE_setDisplay("Produto escolhido:",SLOTENTRADA_nomeProduto[USER_choice],1,0);
            state = 2;
            break;
        
        case 2:

            STATE_escolheProduto();
            break;
        
        case 3:

            STATE_confirmaCompra();
            break;

        case 4:

            STATE_criarOrdemCompra();
            break;

        case 5:

            STATE_confirmarPgto();
            break;

        case 6:

            STATE_liberarProduto();
            break;

        case 7:

            STATE_caidaProduto();
            break;

        case 8:
            
            STATE_tampaAberta();
            break;

        case 9:

            STATE_inserirProduto();
            break;

        case 10:

            SYSTEM_menu1();
            state = 1;
            break;

        case 11:

            SYSTEM_menu2();
            state = 1;
            break;

        case 12:

            SYSTEM_menu3();
            state = 1;
            break;

        default:
            break;
    }
}
