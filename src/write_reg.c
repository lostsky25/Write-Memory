#include "stdlib.h"
#include "../sig_exchange_lib/sig_lib.h"
#include "libbru.h"
#include "opensdk.h"
#include "../tbv_lib/tbv.h"

errcode_t write_drv(PROCESS*, int64_t, int16_t);
void write_memory(void);
void signal_init(SIGSELECT);

//Контекст модуля
//...

int32_t main(void){

	//Модуль находится в ожидании
	while(1){
		rec_w_sig_module("main_disp", signal_init, 50);
	}
	
	stop(current_process());
	
	return 0;
}

errcode_t write_drv(PROCESS* pid, int64_t data, int16_t addr){

	uint8_t tmp[sizeof(struct bru_imsmem_t) + 7] = {0};

	struct bru_imsmem_t * imsmem_p = (struct bru_imsmem_t*)tmp;
	
	imsmem_p->drv_pid = *pid;
	imsmem_p->dev_addr = SPVKR_A;
	imsmem_p->ims_line = IMS_A;
	imsmem_p->addr = ZP_FREEBANK_ADDR | addr;
	imsmem_p->size = 8;
	
	memcpy(imsmem_p->data, &data, sizeof(int64_t));
	
	if(bru_writeIMSMem(imsmem_p)){
		print_debug("Произошла ошибка записи\r\n");
		return ERR_WRITE_DATA;
	}
	
	return NO_ERROR;
}

void signal_init(SIGSELECT sig_no){
	switch(sig_no){
        case PM_STATUS_INI:{
            print_debug("Пришел сигнал от диспетчера о инициализации в write_reg\r\n");
            send_sig_module_em("main_disp", PM_STATUS_INI);
            break;
        }
        case PM_STATUS_READY:{
            print_debug("Модуль готов к работе\r\n");
            break;
        }
        case PM_STATUS_START:{
            print_debug("Произошел старт модуля\r\n");

			write_memory();

            send_sig_module_em("main_disp", PM_STATUS_END);
            break;
        }
		case PM_STATUS_SUCCESS:{
			print_debug("Модуль доступен\r\n");
			break;
		}
		case PM_STATUS_END:{
			print_debug("Модуль завершил работу\r\n");
			break;
		}
		default:{
			print_debug("Сигнал не был проиндексирован\r\n");
			break;
		}
    }
}

void write_memory(void){
	PROCESS drv_pid = 0;

	if(!bru_getPID("regio_bg", &drv_pid)){
		for(int16_t cell = 0; cell < 16 * 10 + 1; cell += 16){
			//Генерация значений тяги
			int64_t engine_thrust = (int64_t)rand() % 1000;
			//Запись времени
			int64_t time = get_tbv_time();
			write_drv(&drv_pid, time, cell);
			delay(250);
			//Запись тяги
			write_drv(&drv_pid, engine_thrust, cell + 8);

			print_debug("Время => Ячейка:[0x%0.8x] Записанные данные: %5lld  \r\n", cell, time);

			print_debug("Тяга => Ячейка:[0x%0.8x] Записанные данные: %5lld  \r\n\r\n", cell + 8, engine_thrust);
		}
	}else{
		print_debug("regio_bg не был открыт\r\n");
	}
}










