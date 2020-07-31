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



//=============  此处必须修改============
String DEVICEID = "18934"; // 你的设备ID=======
String APIKEY = "85c6c1b1e"; // 设备密码==
String INPUTID = "16963"; //接口ID==============
//=======================================
unsigned long lastCheckStatusTime = 0; //记录上次报到时间
unsigned long lastUpdateTime = 0;//记录上次上传数据时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次
const unsigned long updateInterval = 5000; // 数据上传间隔时间5秒
unsigned long checkoutTime = 0;//登出时间
void setup() {
    Serial.begin(115200);
    delay(5000);//等一会儿ESP8266
        // start serial port
    Serial.println("test");
    // Start up the library
    sensors.begin();
}
void loop() {
    //每一定时间查询一次设备在线状态，同时替代心跳
    if (millis() - lastCheckStatusTime > postingInterval) {
        checkStatus();
    }
    //checkout 50ms 后 checkin
    if (checkoutTime != 0 && millis() - checkoutTime > 50) {
        checkIn();
        checkoutTime = 0;
    }
    //每隔一定时间上传一次数据
    if (millis() - lastUpdateTime > updateInterval) {
        sensors.requestTemperatures(); // Send the command to get temperatures
        Serial.println("DONE");
        float val = sensors.getTempCByIndex(0);        //dat = analogRead(LM35); // 读取传感器的模拟值并赋值给dat
        update1(DEVICEID, INPUTID, val);
        lastUpdateTime = millis();
    }
    //读取串口信息
    while (Serial.available()) {
        String inputString = Serial.readStringUntil('\n');
        //检测json数据是否完整
        int jsonBeginAt = inputString.indexOf("{");
        int jsonEndAt = inputString.lastIndexOf("}");
        if (jsonBeginAt != -1 && jsonEndAt != -1) {
            //净化json数据
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
//设备登录
//{"M":"checkin","ID":"xx1","K":"xx2"}\n
void checkIn() {
    Serial.print("{\"M\":\"checkin\",\"ID\":\"");
    Serial.print(DEVICEID);
    Serial.print("\",\"K\":\"");
    Serial.print(APIKEY);
    Serial.print("\"}\n");
}
//强制设备下线，用消除设备掉线延时
//{"M":"checkout","ID":"xx1","K":"xx2"}\n
void checkOut() {
    Serial.print("{\"M\":\"checkout\",\"ID\":\"");
    Serial.print(DEVICEID);
    Serial.print("\",\"K\":\"");
    Serial.print(APIKEY);
    Serial.print("\"}\n");
}

//查询设备在线状态
//{"M":"status"}\n
void checkStatus() {
    Serial.print("{\"M\":\"status\"}\n");
    lastCheckStatusTime = millis();
}

//处理来自ESP8266透传的数据
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
//上传一个接口数据
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
//同时上传两个接口数据调用此函数
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

