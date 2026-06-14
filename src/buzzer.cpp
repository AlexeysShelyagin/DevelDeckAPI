#include "buzzer.h"

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


void DD_buzzer::init(uint16_t pin, uint8_t channel_){
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

void DD_buzzer::play_tone(uint16_t freq){
	ledcChangeFrequency(channel, freq, 8);
	ledcWrite(channel, volume);
}

void DD_buzzer::stop(){
	if(task_handler != NULL && eTaskGetState(task_handler) != eDeleted){
		vTaskDelete(task_handler);
		task_handler = NULL;
		clean_buzz_task_params((Buzzer_sequence_t *) task_params);
	}

	ledcWrite(channel, 0);
}

void DD_buzzer::change_volume(uint8_t level){
	volume_level = level;
	if(level >= BUZZER_VOLUME_LEVELS)
		volume = 100;
	else
		volume = level;
}

uint8_t DD_buzzer::get_volume(){
	return volume_level;
}

void DD_buzzer::play_for_time(uint16_t freq, uint16_t time){
	uint16_t seq_data[2] = {freq, time};
	play_sequence(seq_data, 1);
}

void DD_buzzer::play_sequence(std::vector < Buzzer_element_t > &sequence){
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

void DD_buzzer::play_sequence(uint16_t *data, uint32_t size, bool nocopy){
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