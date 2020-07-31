#include <aJSON.h>
// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);



//=============  �˴������޸�============
String DEVICEID = "18934"; // ����豸ID=======
String APIKEY = "85c6c1b1e"; // �豸����==
String INPUTID = "16963"; //�ӿ�ID==============
//=======================================
unsigned long lastCheckStatusTime = 0; //��¼�ϴα���ʱ��
unsigned long lastUpdateTime = 0;//��¼�ϴ��ϴ�����ʱ��
const unsigned long postingInterval = 40000; // ÿ��40�������������һ��
const unsigned long updateInterval = 5000; // �����ϴ����ʱ��5��
unsigned long checkoutTime = 0;//�ǳ�ʱ��
void setup() {
    Serial.begin(115200);
    delay(5000);//��һ���ESP8266
        // start serial port
    Serial.println("test");
    // Start up the library
    sensors.begin();
}
void loop() {
    //ÿһ��ʱ���ѯһ���豸����״̬��ͬʱ�������
    if (millis() - lastCheckStatusTime > postingInterval) {
        checkStatus();
    }
    //checkout 50ms �� checkin
    if (checkoutTime != 0 && millis() - checkoutTime > 50) {
        checkIn();
        checkoutTime = 0;
    }
    //ÿ��һ��ʱ���ϴ�һ������
    if (millis() - lastUpdateTime > updateInterval) {
        sensors.requestTemperatures(); // Send the command to get temperatures
        Serial.println("DONE");
        float val = sensors.getTempCByIndex(0);        //dat = analogRead(LM35); // ��ȡ��������ģ��ֵ����ֵ��dat
        update1(DEVICEID, INPUTID, val);
        lastUpdateTime = millis();
    }
    //��ȡ������Ϣ
    while (Serial.available()) {
        String inputString = Serial.readStringUntil('\n');
        //���json�����Ƿ�����
        int jsonBeginAt = inputString.indexOf("{");
        int jsonEndAt = inputString.lastIndexOf("}");
        if (jsonBeginAt != -1 && jsonEndAt != -1) {
            //����json����
            inputString = inputString.substring(jsonBeginAt, jsonEndAt + 1);
            int len = inputString.length() + 1;
            char jsonString[len];
            inputString.toCharArray(jsonString, len);
            aJsonObject* msg = aJson.parse(jsonString);
            processMessage(msg);
            aJson.deleteItem(msg);
        }
    }
}
//�豸��¼
//{"M":"checkin","ID":"xx1","K":"xx2"}\n
void checkIn() {
    Serial.print("{\"M\":\"checkin\",\"ID\":\"");
    Serial.print(DEVICEID);
    Serial.print("\",\"K\":\"");
    Serial.print(APIKEY);
    Serial.print("\"}\n");
}
//ǿ���豸���ߣ��������豸������ʱ
//{"M":"checkout","ID":"xx1","K":"xx2"}\n
void checkOut() {
    Serial.print("{\"M\":\"checkout\",\"ID\":\"");
    Serial.print(DEVICEID);
    Serial.print("\",\"K\":\"");
    Serial.print(APIKEY);
    Serial.print("\"}\n");
}

//��ѯ�豸����״̬
//{"M":"status"}\n
void checkStatus() {
    Serial.print("{\"M\":\"status\"}\n");
    lastCheckStatusTime = millis();
}

//��������ESP8266͸��������
void processMessage(aJsonObject* msg) {
    aJsonObject* method = aJson.getObjectItem(msg, "M");
    if (!method) {
        return;
    }
    String M = method->valuestring;
    if (M == "WELCOME TO BIGIOT") {
        checkOut();
        checkoutTime = millis();
        return;
    }
    if (M == "connected") {
        checkIn();
    }
}
//�ϴ�һ���ӿ�����
//{"M":"update","ID":"2","V":{"2":"120"}}\n
void update1(String did, String inputid, float value) {
    Serial.print("{\"M\":\"update\",\"ID\":\"");
    Serial.print(did);
    Serial.print("\",\"V\":{\"");
    Serial.print(inputid);
    Serial.print("\":\"");
    Serial.print(value);
    Serial.println("\"}}");
}
//ͬʱ�ϴ������ӿ����ݵ��ô˺���
//{"M":"update","ID":"112","V":{"6":"1","36":"116"}}\n
void update2(String did, String inputid1, float value1, String inputid2, float value2) {
    Serial.print("{\"M\":\"update\",\"ID\":\"");
    Serial.print(did);
    Serial.print("\",\"V\":{\"");
    Serial.print(inputid1);
    Serial.print("\":\"");
    Serial.print(value1);
    Serial.print("\",\"");
    Serial.print(inputid2);
    Serial.print("\":\"");
    Serial.print(value2);
    Serial.println("\"}}");
}

