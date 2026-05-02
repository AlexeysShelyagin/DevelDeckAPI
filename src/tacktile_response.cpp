#include "tacktile_response.h"

// ========================== BUZZER ===========================



struct Buzzer_sequence_t{
	uint32_t size;
	uint8_t channel;
	uint8_t *volume;
	uint16_t *data;
	bool clear_data;
};

void clean_buzz_task_params(Buzzer_sequence_t *params){
	if(params == nullptr)
		return;
	
	if(params->clear_data)
		delete params->data;
	delete params;
	params = nullptr;
}

void play_tone_seq_task(void *params){
	Buzzer_sequence_t *seq = (Buzzer_sequence_t *) params;

	for(uint32_t i = 0; i < seq->size; i++){
		if(seq->data[i * 2] != 0){
			ledcChangeFrequency(seq->channel, seq->data[i * 2], 8);
			ledcWrite(seq->channel, *seq->volume);
		}
		else
			ledcWrite(seq->channel, 0);
		
		vTaskDelay(pdMS_TO_TICKS(seq->data[i * 2 + 1]));
	}
	ledcWrite(seq->channel, 0);

	clean_buzz_task_params(seq);

	vTaskDelete(NULL);
}


void Gamepad_buzzer::init(uint16_t pin, uint8_t channel_){
#if ESP_ARDUINO_VERSION_MAJOR >= 3
	ledcAttach(pin, 100, 8);
	channel = pin;
#else
    ledcSetup(channel_, 100, 8);
	ledcAttachPin(pin, channel_);
	channel = channel_;
#endif
	ledcWrite(channel_, 0);

	change_volume(DEFAULT_BUZZER_VOLUME);
}

void Gamepad_buzzer::play_tone(uint16_t freq){
	ledcChangeFrequency(channel, freq, 8);
	ledcWrite(channel, volume);
}

void Gamepad_buzzer::stop(){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted){
		vTaskDelete(task_handler);
		task_handler = NULL;
		clean_buzz_task_params((Buzzer_sequence_t *) task_params);
	}

	ledcWrite(channel, 0);
}

void Gamepad_buzzer::change_volume(uint8_t level){
	volume_level = level;
	if(level >= BUZZER_VOLUME_LEVELS)
		volume = 100;
	else
		volume = level;
}

uint8_t Gamepad_buzzer::get_volume(){
	return volume_level;
}

void Gamepad_buzzer::play_for_time(uint16_t freq, uint16_t time){
	uint16_t seq_data[2] = {freq, time};
	play_sequence(seq_data, 1);
}

void Gamepad_buzzer::play_sequence(std::vector < Buzzer_element_t > &sequence){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted)
		return;
	
	Buzzer_sequence_t *seq = new Buzzer_sequence_t();
	seq->size = sequence.size();
	seq->channel = channel;
	seq->volume = &volume;
	seq->clear_data = true;
	seq->data = new uint16_t[seq->size * 2];

	for(uint32_t i = 0; i < seq->size; i++){
		seq->data[i*2] = sequence[i].freq;
		seq->data[i*2 + 1] = sequence[i].timing;
	}

	task_params = seq;
	xTaskCreatePinnedToCore(
		play_tone_seq_task,
		"buzz",
		BUZZER_STACK_SIZE,
		seq,
		BUZZER_TASK_PRIORITY,
		&task_handler,
		DIFFERENT_CORE
	);
}

void Gamepad_buzzer::play_sequence(uint16_t *data, uint32_t size, bool nocopy){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted)
		return;
	
	Buzzer_sequence_t *seq = new Buzzer_sequence_t();
	seq->size = size;
	seq->channel = channel;
	seq->volume = &volume;
	
	if(esp_ptr_in_drom(data) || nocopy){
		seq->data = data;
		seq->clear_data = false;
	}
	else{
		seq->data = new uint16_t[size*2];
		memcpy(seq->data, data, size*2 * sizeof(uint16_t));
		seq->clear_data = true;
	}

	task_params = seq;
	xTaskCreatePinnedToCore(
		play_tone_seq_task,
		"buzz",
		BUZZER_STACK_SIZE,
		seq,
		BUZZER_TASK_PRIORITY,
		&task_handler,
		DIFFERENT_CORE
	);
}




// ===================== VIBRO =======================================

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


void Gamepad_vibro::init(uint16_t pin, uint8_t channel_){
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

uint8_t Gamepad_vibro::calc_strength(uint8_t strength_){
	strength = min(strength, (uint8_t) VIBRO_STRENGTH_LEVELS);
	return ((float) strength / VIBRO_STRENGTH_LEVELS) * strength_;
}

void Gamepad_vibro::enable(uint8_t strength_){
	ledcWrite(channel, calc_strength(strength_));
}

void Gamepad_vibro::disable(){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted){
		vTaskDelete(task_handler);
		task_handler = NULL;
		clean_periodic_params((Period_task_param_t *) task_params);
	}

	ledcWrite(channel, 0);
}


void Gamepad_vibro::enable_for_time(uint16_t time, uint8_t strength_){
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

void Gamepad_vibro::enable_periodic(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_){
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