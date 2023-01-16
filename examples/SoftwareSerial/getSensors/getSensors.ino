// ПРИМЕР ЧТЕНИЯ ДАННЫХ С ДАТЧИКОВ УРОВНЯ ЖИДКОСТИ:                                //
                                                                                   //
#include <SoftwareSerial.h>                                                        //   Подключаем библиотеку для работы с программной шиной UART.
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_Socket.h>                                                    //   Подключаем библиотеку для работы с розеткой.
                                                                                   //
SoftwareSerial     rs485(8,9);                                                     //   Создаём объект для работы с программной шиной UART-RS485 указывая выводы RX, TX.
ModbusClient       modbus(rs485, 2);                                               //   Создаём объект для работы по протоколу Modbus указывая объект программной шины UART-RS485 и вывод DE конвертера UART-RS485.
iarduino_MB_Socket socket(modbus);                                                 //   Создаём объект для работы с розеткой указывая объект протокола Modbus.
                                                                                   //
void setup(){                                                                      //
     Serial.begin(9600 ); while(!Serial);                                          //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     rs485.begin(9600); while(!rs485);                                             //   Инициируем работу с программной шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
     socket.begin(1);                                                              //   Инициируем работу с розеткой, указав её адрес.
}                                                                                  //   Если адрес не указан socket.begin(), то он будет найден, но это займёт некоторое время.
                                                                                   //
void loop(){                                                                       //
     int i;                                                                        //
                                                                                   //
//   Получаем состояние каждого датчика уровня жидкости по отдельности:            //
     i=socket.digitalRead(SOCKET_SENSOR_MIN);                                      //   Запрашиваем состояние датчика MIN.
     Serial.print(  "min="); if(i<0){Serial.print("err");}else{Serial.print(i);}   //   Выводим     состояние датчика MIN в монитор последовательного порта.
     i=socket.digitalRead(SOCKET_SENSOR_MID);                                      //   Запрашиваем состояние датчика MID.
     Serial.print(", mid="); if(i<0){Serial.print("err");}else{Serial.print(i);}   //   Выводим     состояние датчика MID в монитор последовательного порта.
     i=socket.digitalRead(SOCKET_SENSOR_MAX);                                      //   Запрашиваем состояние датчика MAX.
     Serial.print(", max="); if(i<0){Serial.print("err");}else{Serial.print(i);}   //   Выводим     состояние датчика MAX в монитор последовательного порта.
     Serial.println('.');                                                          //
     delay(1000);                                                                  //
                                                                                   //
//   Получаем состояние всех датчиков уровня жидкости одним запросом:              //
     i=socket.digitalRead(SOCKET_SENSOR_ALL);                                      //   Запрашиваем состояние всех датчиков. Ответом является байт i, три младших бита которого отражают состояние датчиков.
     if( i<0 ){                                                                    //
         Serial.print("min=err, mid=err, max=err");                                //   Выводим сообщение об ошибке.
     }else{                                                                        //
         Serial.print(  "min="); Serial.print( bitRead(i,0) );                     //   Выводим значение 0 бита байта i, который отражает состояние первого  датчика MIN.
         Serial.print(", mid="); Serial.print( bitRead(i,1) );                     //   Выводим значение 1 бита байта i, который отражает состояние второго  датчика MID.
         Serial.print(", max="); Serial.print( bitRead(i,2) );                     //   Выводим значение 2 бита байта i, который отражает состояние третьего датчика MAX.
     }                                                                             //
     Serial.println('.');                                                          //
     delay(1000);                                                                  //
                                                                                   //
//   Данные выводимые в монитор последовательного порта одинаковы для 2 вариантов. //
}                                                                                  //
