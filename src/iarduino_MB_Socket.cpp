#include "iarduino_MB_Socket.h"		//	╔═══════════════════════╗
									//	║ РЕГИСТР ДИАГНОСТИКИ   ║
									//	╠═══════════╤═══════════╣
									//	║ --------  │ -----2--  ║ Флаг остуствия датчика уровня жидкости MAX.
									//	║ --------  │ ------1-  ║ Флаг остуствия датчика уровня жидкости MID.
									//	║ --------  │ -------0  ║ Флаг остуствия датчика уровня жидкости MIN.
									//	╚═══════════╧═══════════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «DO» ║ Биты: ║
									//	╠═══════════════╬═══════╣
#define DO_HL_DATA		0			//	║ DO_HL_DATA    ║   0   ║ Включение светодиода обнаружения устройства.
#define DO_RELAY		1			//	║ DO_RELAY      ║   0   ║ Включение реле.
#define DO_WDT			2			//	║ DO_WDT        ║   0   ║ Включение сторожевого таймера реле.
									//	╟───────────────╫───────╢
//						0x0100		//	║ DO_CHANGE_ID  ║   0   ║ Флаг устанавливается самостоятельно после подтверждения смены адреса на шине.
//						0x0101		//	║ DO_BAN_ID 1   ║   0   ║ Бит  запрещает изменять или устанавливать адрес 1.      Только для режима смены адреса на случайное симло.
//						...			//	║ DO_BAN_ID ... ║   0   ║ Биты запрещают изменять или устанавливать адреса 2-246. Только для режима смены адреса на случайное симло.
//						0x01F7		//	║ DO_BAN_ID 247 ║   0   ║ Бит  запрещает изменять или устанавливать адрес 147.    Только для режима смены адреса на случайное симло.
									//	╚═══════════════╩═══════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «DI» ║ Биты: ║
									//	╠═══════════════╬═══════╣
#define DI_MIN			0			//	║ DI_MIN        ║   0   ║ Флаг нижнего  датчика уровня жидкости (1-жидкость есть, 0-жидкости нет).
#define DI_MID			1			//	║ DI_MID        ║   0   ║ Флаг среднего датчика уровня жидкости (1-жидкость есть, 0-жидкости нет).
#define DI_MAX			2			//	║ DI_MAX        ║   0   ║ Флаг верхнего датчика уровня жидкости (1-жидкость есть, 0-жидкости нет).
									//	╚═══════════════╩═══════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «AO» ║Данные:║
									//	╠═══════════════╬═══════╣
#define AO_WDT			0			//	║ AO_WDT        ║ 10000 ║ Сторожевой таймер реле, хранит мс до сброса реле.
									//	╟───────────────╫───────╢
//						0x0100		//	║ AO_ACK_ID     ║   0   ║ Запись значения 0xF0A5 подтверждает смену адреса на шине. При чтении всегда возвращает 0.
#define AO_ACK_SPEED	0x0101		//	║ AO_ACK_SPEED  ║   0   ║ Запись значения 0xF0A5 подтверждает смену скорости  шины. При чтении всегда возвращает 0.
#define AO_ACK_TYPE		0x0102		//	║ AO_ACK_TYPE   ║   0   ║ Запись значения 0xF0A5 подтверждает смену типа протокола. При чтении всегда возвращает 0.
//						0x0110		//	║ AO_SET_ID     ║  xxID ║ Запись приводит к смене адреса на шине, xx=00 до отключения питания, xx=FF с сохранеием в ПЗУ. Требуется подтверждение. При чтении xx=случайное число.
#define AO_SET_SPEED	0x0111		//	║ AO_SET_SPEED  ║  xxSP ║ Запись приводит к смене скорости  шины, xx=00 до отключения питания, xx=FF с сохранеием в ПЗУ. Требуется подтверждение. При чтении xx=случайное число.
#define AO_SET_TYPE		0x0112		//	║ AO_SET_TYPE   ║  xxTP ║ Запись приводит к смене типа протокола, xx=00 до отключения питания, xx=FF с сохранеием в ПЗУ. Требуется подтверждение. При чтении xx=случайное число.
#define AO_USER_DATA	0x0120		//	║ AO_USER_DATA  ║   0   ║ Пользовательское число хранится в Flash памяти модуля.
									//	╚═══════════════╩═══════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «AI» ║Данные:║
									//	╠═══════════════╬═══════╣
#define AI_VIN			0			//	║ AI_VIN        ║   0   ║ Напряжение питания шины RS-485 в мВ.
									//	╚═══════════════╩═══════╝
																															//
//		КОНСТРУКТОР КЛАССА:																									//
		iarduino_MB_Socket::iarduino_MB_Socket(ModbusClient &obj){objModbus = &obj;}										//	&obj - ссылка на объект для работы по протоколу Modbus.
																															//
//		ФУНКЦИЯ ИНИЦИАЛИЗАЦИИ РОЗЕТКИ:																						//	Возвращает результат инициализации: true-успех / false-провал.
bool	iarduino_MB_Socket::begin(uint8_t id){																				//	id - адрес модуля на шине.
			objModbus->codeError=ERROR_GATEWAY_NO_DEVICE;																	//	Ошибкой выполнения данной функции может быть только отсутствие устройства.
		//	Самостоятельный поиск id устройства:																			//
			if( id==0 )								{ if( objModbus->findID(DEF_MODEL_SOCKET) ){ id=objModbus->read(); }}	//	Если адрес не указан, ищем адрес первого устройства с совпавшим идентификатором.
		//	Проверяем устройство:																							//
			if( id==0 )								{ return false; }														//	Адрес устройства не указан и не найден.
			if( objModbus->getInfo(id)<15 )			{ return false; }														//	Устройство не найдено, или информация об устройстве неполная.
			if( objModbus->read()!=0x77 )			{ return false; }														//	Идентификатор линейки устройств не соответствует устройствам iArduino.
				objModbus->read();																							//	Индикатор пуска не проверяем (00=OFF, FF=ON).
			if( objModbus->read()!=id )				{ return false; }														//	Адрес полученный из информации об устройстве не совпадает с фактическим адресом устройства.
			if( objModbus->read()!=DEF_MODEL_SOCKET){ return false; }														//	Идентификатор устройства не совпадает с DEF_MODEL_SOCKET.
		//	Устройство прошло проверку:																						//
			valID=id;																										//	Сохраняем адрес устройства.
			valVers=objModbus->read();																						//	Сохраняем версию прошивки устройства.
			return true;																									//
}																															//
																															//
//		ФУНКЦИЯ ПЕРЕЗАГРУЗКИ РОЗЕТКИ:																						//	Возвращает результат перезагрузки: true-успех / false-провал.
bool	iarduino_MB_Socket::reset(void){																					//	
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return ( objModbus->diagnostic(valID,1,0xFF00)<0? false:true );													//	Выполняем команду диагностики, функция 0x0001 с очисткой журнала событий 0xFF00.
}																															//
																															//
//		ФУНКЦИЯ СМЕНЫ ID РОЗЕТКИ:																							//	Возвращает результат смены ID: true-успех / false-провал.
bool	iarduino_MB_Socket::changeID(uint8_t id){																			//	id - новый адрес модуля на шине (1-247).
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !objModbus->changeID(valID, id) ){ return false; }															//	Меняем адрес устройства с valID на id.
			valID=id; return true;																							//	Сохраняем навый адрес и возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ СМЕНЫ СКОРОСТИ ПЕРЕДАЧИ ДАННЫХ:																				//	Возвращает результат смены скорости: true-успех / false-провал. Скорость необходимо подтвердить в течении 2 секунд.
bool	iarduino_MB_Socket::setSpeedMB(uint32_t s){																			//	s - скорость передачи данных (2400/4800/9600/19200/38400/57600/115200)
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( s!=2400 && s!=4800 && s!=9600 && s!=19200 && s!=38400 && s!=57600 && s!=115200 ){ return false; }			//	Проверяем значение скорости.
			return objModbus->holdingRegisterWrite(valID, AO_SET_SPEED, 0xFF00|(s/2400));									//	Записываем значение 0xFFXX в регистр "Holding Register" AO[AO_SET_SPEED]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОДТВЕРЖДЕНИЯ СКОРОСТИ ПЕРЕДАЧИ ДАННЫХ:																		//	Возвращает результат подтверждения скорости: true-успех / false-провал. 
bool	iarduino_MB_Socket::ackSpeedMB(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !objModbus->holdingRegisterWrite(valID, AO_ACK_SPEED, 0xF0A5) ){ return false; }							//	Записываем значение 0xF0A5 в регистр "Holding Register" AO[AO_ACK_SPEED]. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит новую скорость в flash память.
			return true;																									//	Возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ СМЕНЫ ТИПА ПРОТОКОЛА MODBUS:																				//	Возвращает результат смены типа протокола Modbus: true-успех / false-провал. Тип необходимо подтвердить в течении 2 секунд.
bool	iarduino_MB_Socket::setTypeMB(uint8_t type){																		//	type - тип протокола Modbus (MODBUS_RTU/MODBUS_ASCII).
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( type!=MODBUS_RTU && type!=MODBUS_ASCII ){ return false; }													//	Проверяем значение типа протокола Modbus.
			return objModbus->holdingRegisterWrite(valID, AO_SET_TYPE, (type==MODBUS_ASCII?0xFFFF:0xFF00));					//	Записываем значение 0xFFXX в регистр "Holding Register" AO[AO_SET_TYPE]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОДТВЕРЖДЕНИЯ ТИПА ПРОТОКОЛА MODBUS:																		//	Возвращает результат подтверждения типа протокола Modbus: true-успех / false-провал. 
bool	iarduino_MB_Socket::ackTypeMB(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !objModbus->holdingRegisterWrite(valID, AO_ACK_TYPE, 0xF0A5) ){ return false; }								//	Записываем значение 0xF0A5 в регистр "Holding Register" AO[AO_ACK_TYPE]. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит новый тип протокола Modbus в flash память.
			return true;																									//	Возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ ЗАПИСИ ПОЛЬЗОВАТЕЛЬСКОГО ЗНАЧЕНИЯ В FLASH ПАМЯТЬ МОДУЛЯ:													//	Возвращает результат записи: true-успех / false-провал.
bool	iarduino_MB_Socket::writeFlash(uint16_t data){																		//	data - целое беззнаковое число от 0 до 65535.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !objModbus->holdingRegisterWrite(valID, AO_USER_DATA, data) ){ return false; }								//	Записываем значение data в регистр "Holding Register" AO[AO_USER_DATA]. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит записанное значение в flash память.
			return true;																									//	Возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ ЧТЕНИЯ ПОЛЬЗОВАТЕЛЬСКОГО ЗНАЧЕНИЯ ИЗ FLASH ПАМЯТИ:															//	Возвращает прочитанное значение, или -1 при провале чтения.
int32_t	iarduino_MB_Socket::readFlash(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->holdingRegisterRead(valID, AO_USER_DATA);														//	Читаем значение из регистра "Holding Register" AO[AO_USER_DATA]. Функция возвращает значение, или -1 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ НАПРЯЖЕНИЯ ПИТАНИЯ:																				//	Возвращает напряжение питания в Вольтах, или -1 при провале чтения.
float	iarduino_MB_Socket::getPWR(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			int32_t Vcc = objModbus->inputRegisterRead(valID, AI_VIN);														//	Читаем напряжение из регистра "Input Register" AI[AI_VIN]. Функция возвращает значение, или -1 при неудаче.
			if( Vcc<0 ){ return -1.0f; }else{ return ((float)Vcc)/1000.0f; }												//	Возвращаем -1 (ошибка), или напряжение в Вольтах.
}																															//
																															//
//		ФУНКЦИЯ ИЗМЕНЕНИЯ СОСТОЯНИЯ СВЕТОДИОДА ОБНАРУЖЕНИЯ УСТРОЙСТВА:														//	Возвращает результат изменения состояния светодиода: true-успех / false-провал.
bool	iarduino_MB_Socket::setIDLED(bool f){																				//	f - состояние светодиода (true/false)
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->coilWrite(valID, DO_HL_DATA, f);																//	Записываем f в регистр "Coil" DO[DO_HL_DATA].
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ФЛАГОВ ОШИБОК ИЗ РЕГИСТРА ДИАГНОСТИКИ:															//	Возвращает значение регистра диагностики, каждый из 16 бит которого является флагом ошибки, или -1 при провале чтения.
int32_t	iarduino_MB_Socket::getErr(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->diagnostic(valID, 2);																			//	Выполняем команду диагностики, номер функции диагностики 0x0002 = получить регистр диагностики. Функция возвращает данные результата выполнения функции диагностики, или -1 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ИЗМЕНЕНИЯ СОСТОЯНИЯ РОЗЕТКИ:																				//	Возвращает результат изменения состояния розетки: true-успех / false-провал.
bool	iarduino_MB_Socket::setState(bool f){																				//	f - состояние розетки (true/false)
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->coilWrite(valID, DO_RELAY, f);																//	Записываем f в регистр "Coil" DO[DO_RELAY].
}																															//
																															//
//		ФУНКЦИЯ ЧТЕНИЯ СОСТОЯНИЯ РОЗЕТКИ:																					//	Возвращает состояние розетки: true-успех / false-провал, или -1 при провале чтения состояния розетки.
int8_t	iarduino_MB_Socket::getState(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->coilRead(valID, DO_RELAY);																	//	Читаем регистр "Coil" DO[DO_RELAY]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ЧТЕНИЯ СОСТОЯНИЯ ДАТЧИКОВ УРОВНЕЙ ЖИДКОСТИ:																	//	Возвращает уровень датчика 0/1, байт битов если читаются все датчики, или -1 при провале чтения уровня датчика.
int8_t	iarduino_MB_Socket::digitalRead(uint8_t num){																		//	num - один из вариантов SOCKET_SENSOR_XXX.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			if( num==SOCKET_SENSOR_ALL ){																					//
			//	Читаем состояние всех датчиков одним запросом:																//
				if( objModbus->requestFrom(valID, DISCRETE_INPUTS, DI_MIN, 3) != 3 ){ return -1; }							//	Читаем регистры "Discrete Inputs" начиная с регистра DI[DI_MIN], всего 3 регистра. Функция возвращает количество прочитанных значений, или 0 при неудаче.
				return (objModbus->read()?1:0)|(objModbus->read()?2:0)|(objModbus->read()?4:0);								//	Собираем байт с тремя прочитанными битами и возвращаем его.
			}else																											//
			//	Читаем состояние указанного датчика:																		//
			if( num==SOCKET_SENSOR_MIN ){ return objModbus->discreteInputRead(valID, DI_MIN); }else							//	Читаем регистр "Discrete Input" DI[DI_MIN]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
			if( num==SOCKET_SENSOR_MID ){ return objModbus->discreteInputRead(valID, DI_MID); }else							//	Читаем регистр "Discrete Input" DI[DI_MID]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
			if( num==SOCKET_SENSOR_MAX ){ return objModbus->discreteInputRead(valID, DI_MAX); }else							//	Читаем регистр "Discrete Input" DI[DI_MAX]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
			//	Параметр num отличается от значений SOCKET_SENSOR_XXX:														//
				{ return -1; }																								//
}																															//
																															//
//		ФУНКЦИЯ ЧТЕНИЯ НАЛИЧИЯ ДАТЧИКОВ УРОВНЕЙ ЖИДКОСТИ:																	//	Возвращает флаг наличия датчика 0/1, байт битов если читается наличие всех датчиков, или -1 при провале чтения наличия датчика.
int8_t	iarduino_MB_Socket::connectRead(uint8_t num){																		//	num - один из вариантов SOCKET_SENSOR_XXX.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			int32_t result = objModbus->diagnostic(valID,2);																//	Выполняем команду диагностики, функция 0x0002 возвращает значение регистра диагностики.
			if( num==SOCKET_SENSOR_ALL ){ return (int8_t)((~result   ) & 0x0007); }else										//	Возвращаем       3 последних бита регистра диагностики с инферсией.
			if( num==SOCKET_SENSOR_MIN ){ return (int8_t)((~result   ) & 0x0001); }else										//	Возвращаем         последний бит  регистра диагностики с инферсией.
			if( num==SOCKET_SENSOR_MID ){ return (int8_t)((~result>>1) & 0x0001); }else										//	Возвращаем     предпоследний бит  регистра диагностики с инферсией.
			if( num==SOCKET_SENSOR_MAX ){ return (int8_t)((~result>>2) & 0x0001); }else										//	Возвращаем предпредпоследний бит  регистра диагностики с инферсией.
		//	Параметр num отличается от значений SOCKET_SENSOR_XXX:															//
			{ return -1; }																									//
}																															//
																															//
//		ФУНКЦИЯ РАЗРЕШЕНИЯ РАБОТЫ СТОРОЖЕВОГО ТАЙМЕРА РОЗЕТКИ:																//	Возвращает результат включения сторожевого таймера: true-успех / false-провал.
bool	iarduino_MB_Socket::enableWDT(uint16_t ms){																			//	ms - время в миллисекундах от 1 до 65535.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			valWDT = ms;																									//	Сохраняем время для функции resetWDT().
			if( !objModbus->holdingRegisterWrite(valID, AO_WDT, ms) ){ return false; }										//	Записываем значение ms в регистр "Holding Register" AO[AO_WDT]. Функция возвращает 1 при успехе, 0 при неудаче.
			return objModbus->coilWrite(valID, DO_WDT, 1);																	//	Устанавливаем регистр "Coil" DO[DO_WDT]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ЗАПРЕТА РАБОТЫ СТОРОЖЕВОГО ТАЙМЕРА РОЗЕТКИ:																	//	Возвращает результат отключения сторожевого таймера: true-успех / false-провал.
bool	iarduino_MB_Socket::disableWDT(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->coilWrite(valID, DO_WDT, 0);																	//	Сбрасываем регистр "Coil" DO[DO_WDT]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ СБРОСА (ПЕРЕЗАПУСК) СТОРОЖЕВОГО ТАЙМЕРА РОЗЕТКИ:															//	Возвращает результат сброса сторожевого таймера: true-успех / false-провал.
bool	iarduino_MB_Socket::resetWDT(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->holdingRegisterWrite(valID, AO_WDT, valWDT);													//	Записываем значение valWDT в регистр "Holding Register" AO[AO_WDT]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ЧТЕНИЯ СОСТОЯНИЯ СТОРОЖЕВОГО ТАЙМЕРА РОЗЕТКИ:																//	Возвращает состояние сторожевого таймера: 1-включён, 0-выключён, или -1 при провале чтения состояния сторожевого таймера.
int8_t	iarduino_MB_Socket::getStateWDT(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			return objModbus->coilRead(valID, DO_WDT);																		//	Читаем регистр "Coil" DO[DO_WDT]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
}																															//
