// ПРИМЕР ВИЗУАЛЬНОГО ОБНАРУЖЕНИЯ УСТРОЙСТВА:                                      //
                                                                                   //
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_Socket.h>                                                    //   Подключаем библиотеку для работы с розеткой.
                                                                                   //
ModbusClient       modbus(Serial1, 2);                                             //   Создаём объект для работы по протоколу Modbus указывая объект шины UART-RS485 для протокола и вывод DE конвертера UART-RS485.
iarduino_MB_Socket socket(modbus);                                                 //   Создаём объект для работы с розеткой указывая объект протокола Modbus.
                                                                                   //
void setup(){                                                                      //
     Serial1.begin(9600); while(!Serial1);                                         //   Инициируем работу с шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
     socket.begin(1);                                                              //   Инициируем работу с розеткой, указав её адрес.
}                                                                                  //   Если адрес не указан socket.begin(), то он будет найден, но это займёт некоторое время.
                                                                                   //
void loop(){                                                                       //
//   Мигаем светодиодом обнаружения устройства (на разъёме):                       //
     socket.setIDLED(true ); delay(1000);                                          //   Включаем  светодиод на 1 секунду.
     socket.setIDLED(false); delay(1000);                                          //   Отключаем светодиод на 1 секунду.
}                                                                                  //
