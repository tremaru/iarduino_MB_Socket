// ПРИМЕР УПРАВЛЕНИЯ ВСЕМИ РОЗЕТКАМИ НА ШИНЕ:                                      //
// без указания их адресов в скетче.                                               //
                                                                                   //
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_Socket.h>                                                    //   Подключаем библиотеку для работы с розеткой.
                                                                                   //
ModbusClient       modbus(Serial1, 2);                                             //   Создаём объект для работы по протоколу Modbus указывая объект шины UART-RS485 для протокола и вывод DE конвертера UART-RS485.
iarduino_MB_Socket* socket;                                                        //   Создаём указатель который будет переопределён в массив объектов работы с розетками.
                                                                                   //
uint8_t sum=0;                                                                     //   Определяем переменную для хранения количества найденных розеток.
                                                                                   //
void setup(){                                                                      //
     Serial.begin(9600); while(!Serial);                                           //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     Serial1.begin(9600); while(!Serial1);                                         //   Инициируем работу с шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
//   Выполняем поиск всех розеток на шине (занимает несколько секунд):             //
     Serial.println("Поиск розеток ..." );                                         //
     sum = modbus.findID( DEF_MODEL_SOCKET );                                      //   Ищем адреса всех устройств с идентификатором розетки DEF_MODEL_SOCKET.
     uint8_t arrID[sum], num=0;                                                    //   Объявляем массив arrID для хранения найденных адресов.
     while( modbus.available() ){ arrID[num++]=modbus.read(); }                    //   Заполняем массив arrID найденными адресами.
     if( sum ){                                                                    //
     //  Переопределяем указатель socket в массив объектов:                        //
         socket = (iarduino_MB_Socket*) malloc( sizeof(iarduino_MB_Socket)*sum );  //   Выделяем под массив socket требуемый объем памяти.
     //  Инициируем работу с розетками:                                            //
         for(uint8_t i=0; i<sum; i++){                                             //
             socket[i]=modbus;                                                     //   Указываем очередному объекту работы с розеткой, объект протокола Modbus.
             socket[i].begin( arrID[i] );                                          //   Инициируем работу с очередной розеткой, указав очередной адрес из ранее найденных.
         }                                                                         //
     }                                                                             //
//   Выводим сообщение:                                                            //
     Serial.print((String) "Найдено "+sum+" розеток, с адресами: " );              //
     for(uint8_t i=0; i<sum; i++){ Serial.print(arrID[i]); Serial.print(", "); }   //
     Serial.println();                                                             //
}                                                                                  //
                                                                                   //
void loop(){                                                                       //
     for(uint8_t i=0, j; i<sum; i++, j=0){                                         //   Проходим по всем розеткам.
         Serial.print( socket[i].getID() );                                        //   Выводим   ID текущей розетки.
         if( socket[i].setState(true ) ){j++;}                                     //   Включаем  розетку.
         if( socket[i].setIDLED(true ) ){j++;}                                     //   Включаем  светодиод обнаружения устройства (на разъёме).
         delay(1000);                                                              //
         if( socket[i].setState(false) ){j++;}                                     //   Отключаем розетку.
         if( socket[i].setIDLED(false) ){j++;}                                     //   Отключаем светодиод обнаружения устройства (на разъёме).
         Serial.println( (j<4)? "=ERR":"=OK" );                                    //   Выводим   Результат управления розеткой.
         delay(1000);                                                              //
     }                                                                             //
}                                                                                  //
