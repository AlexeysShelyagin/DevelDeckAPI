#include "vibro.h"

bool vib_task_stop = false;

struct Period_task_param_t{
	uint16_t t1, t2;
	uint8_t n;
	uint8_t strength_;
	uint8_t channel;
};



void clean_periodic_params(Period_task_param_t *params){
	if(params == nullptr)
		return;

	delete params;
	params = nullptr;
}

void vib_periodic_task(void *params){
	Period_task_param_t *job = (Period_task_param_t *) params;

	for(uint8_t i = 0; i < job->n; i++){
		ledcWrite(job->channel, job->strength_);
		vTaskDelay(pdMS_TO_TICKS(job->t1));
		ledcWrite(job->channel, 0);
		vTaskDelay(pdMS_TO_TICKS(job->t2));
	}

	clean_periodic_params(job);

	vTaskDelete(NULL);
}


void DD_vibro::init(uint16_t pin, uint8_t channel_){
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	ledcAttach(pin, 25000, 8);
	channel = pin;
#else
    ledcSetup(channel_, 25000, 8);
	ledcAttachPin(pin, channel_);
	channel = channel_;
#endif
	ledcWrite(channel, 0);
}

uint8_t DD_vibro::calc_strength(uint8_t strength_){
	strength = min(strength, (uint8_t) VIBRO_STRENGTH_LEVELS);
	return ((float) strength / VIBRO_STRENGTH_LEVELS) * strength_;
}

void DD_vibro::enable(uint8_t strength_){
	ledcWrite(channel, calc_strength(strength_));
}

void DD_vibro::disable(){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted){
		vTaskDelete(task_handler);
		task_handler = NULL;
		clean_periodic_params((Period_task_param_t *) task_params);
	}

	ledcWrite(channel, 0);
}


void DD_vibro::enable_for_time(uint16_t time, uint8_t strength_){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted)
		return;
	
	task_params = new Period_task_param_t();
	*(Period_task_param_t*) task_params = (Period_task_param_t){time, 0, 1, calc_strength(strength_), channel};
	
	xTaskCreatePinnedToCore(
		vib_periodic_task,
		"vib",
		VIBRO_STACK_SIZE,
		task_params,
		VIBRO_TASK_PRIORITY,
		&task_handler,
		DIFFERENT_CORE
	);
}

void DD_vibro::enable_periodic(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted)
		return;
	
	task_params = new Period_task_param_t();
	*(Period_task_param_t*) task_params = (Period_task_param_t){time_enabled, time_disabled, repeat_times, calc_strength(strength_), channel};

	xTaskCreatePinnedToCore(
		vib_periodic_task,
		"vib",
		VIBRO_STACK_SIZE,
		task_params,
		VIBRO_TASK_PRIORITY,
		&task_handler,
		DIFFERENT_CORE
	);
}