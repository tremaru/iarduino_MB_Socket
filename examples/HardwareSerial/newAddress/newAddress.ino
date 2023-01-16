// ПРИМЕР СМЕНЫ АДРЕСА:                                                            //
                                                                                   //
uint8_t nowAddress = 1;                                                            //   Текущий адрес ( 1 - 247 ).
uint8_t newAddress = 10;                                                           //   Новый адрес ( 1 - 247 ).
                                                                                   //
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_Socket.h>                                                    //   Подключаем библиотеку для работы с розеткой.
                                                                                   //
ModbusClient       modbus(Serial1, 2);                                             //   Создаём объект для работы по протоколу Modbus указывая объект шины UART-RS485 для протокола и вывод DE конвертера UART-RS485.
iarduino_MB_Socket socket(modbus);                                                 //   Создаём объект для работы с розеткой указывая объект протокола Modbus.
                                                                                   //
void setup(){                                                                      //
     int f;                                                                        //
     Serial.begin(9600); while(!Serial);                                           //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     Serial1.begin(9600); while(!Serial1);                                         //   Инициируем работу с шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
//   Инициируем работу с розеткой:                                                 //
     f =    socket.begin(nowAddress);                                              //   Инициируем работу с розеткой, указав её текущий адрес nowAddress.
     if(f){ Serial.println("Розетка найдена");            }                        //   Если адрес не указан f=socket.begin(), то он будет найден, но это займёт некоторое время.
     else { Serial.println("Розетка не найдена"); return; }                        //
//   Меняем адрес розетки:                                                         //
     f =    socket.changeID(newAddress);                                           //   Меняем адрес розетки на новый newAddress.
     if(f){ Serial.println("Адрес изменён");            }                          //
     else { Serial.println("Адрес не изменён"); return; }                          //
//   Выводим сообшение о розетке:                                                  //
     Serial.println((String) "Текущий адрес   = "+socket.getID()      );           //   Выводим текущий адрес розетки.
     Serial.println((String) "Версия прошивки = "+socket.getVersion() );           //   Выводим версию прошивки розетки.
}                                                                                  //
                                                                                   //
void loop(){                                                                       //
//   Мигаем светодиодом обнаружения устройства (на разъёме):                       //
     socket.setIDLED(false); delay(1000);                                          //
     socket.setIDLED(true ); delay(1000);                                          //
}                                                                                  //
