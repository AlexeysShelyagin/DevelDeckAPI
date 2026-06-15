#include "vibro.h"

struct Vibro_task_param_t{
	uint16_t t1, t2;
	uint8_t n;
	uint8_t strength_;
	uint8_t ledc_ch;
};



void vib_params_free_memory(Vibro_task_param_t *params){
	if(params == nullptr)
		return;

	delete params;
	params = nullptr;
}

void vibro_task(void *params){
	Vibro_task_param_t *job = (Vibro_task_param_t *) params;

	for(uint8_t i = 0; i < job->n; i++){
		ledcWrite(job->ledc_ch, job->strength_);
		vTaskDelay(pdMS_TO_TICKS(job->t1));
		ledcWrite(job->ledc_ch, 0);
		vTaskDelay(pdMS_TO_TICKS(job->t2));
	}

	vib_params_free_memory(job);

	vTaskDelete(NULL);
}


void DD_vibro::init(uint16_t pin, uint8_t channel_){
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	ledcAttach(pin, 25000, 8);
	channel = pin;
#else
    ledcSetup(channel_, 25000, 8);
	ledcAttachPin(pin, channel_);
	ledc_ch = channel_;
#endif
	ledcWrite(ledc_ch, 0);
}

uint8_t DD_vibro::calc_strength(uint8_t strength_){
	strength = min(strength, (uint8_t) VIBRO_STRENGTH_LEVELS);
	return ((float) strength / VIBRO_STRENGTH_LEVELS) * strength_;
}

void DD_vibro::enable(uint8_t strength_){
	ledcWrite(ledc_ch, calc_strength(strength_));
}

void DD_vibro::disable(){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted){
		vTaskDelete(task_handler);
		task_handler = NULL;
		vib_params_free_memory((Vibro_task_param_t *) task_params);
	}

	ledcWrite(ledc_ch, 0);
}


void DD_vibro::pulse(uint16_t time, uint8_t strength_){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted)
		return;
	
	task_params = new Vibro_task_param_t();
	*(Vibro_task_param_t*) task_params = (Vibro_task_param_t){time, 0, 1, calc_strength(strength_), ledc_ch};
	
	xTaskCreatePinnedToCore(
		vibro_task,
		"vib",
		DD_STACK_SIZE_VIBRO,
		task_params,
		DD_TASK_PRIORITY_VIBRO,
		&task_handler,
		DIFFERENT_CORE
	);
}

void DD_vibro::multipulse(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted)
		return;
	
	task_params = new Vibro_task_param_t();
	*(Vibro_task_param_t*) task_params = (Vibro_task_param_t){time_enabled, time_disabled, repeat_times, calc_strength(strength_), ledc_ch};

	xTaskCreatePinnedToCore(
		vibro_task,
		"vib",
		DD_STACK_SIZE_VIBRO,
		task_params,
		DD_TASK_PRIORITY_VIBRO,
		&task_handler,
		DIFFERENT_CORE
	);
}