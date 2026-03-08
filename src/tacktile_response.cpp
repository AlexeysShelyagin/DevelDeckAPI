#include "tacktile_response.h"


struct Buzzer_sequence_t{
	uint32_t size;
	uint8_t channel;
	uint8_t *volume;
	uint16_t *data;
};

void play_tone_seq_task(void *params){
	Buzzer_sequence_t *seq = (Buzzer_sequence_t *) params;

	for(uint32_t i = 0; i < seq -> size; i++){
		ledcChangeFrequency(seq -> channel, seq -> data[i * 2], 8);
		ledcWrite(seq -> channel, *seq -> volume);

		vTaskDelay(pdMS_TO_TICKS(seq -> data[i * 2 + 1]));
	}
	ledcWrite(seq -> channel, 0);

	delete seq;

	vTaskDelete(NULL);
}


void Gamepad_buzzer::init(uint16_t pin, uint8_t channel_){
	channel = channel_;
    ledcSetup(channel_, 100, 8);
	ledcAttachPin(pin, channel_);
	ledcWrite(channel_, 0);

	change_volume(DEFAULT_BUZZER_VOLUME);
}

void Gamepad_buzzer::play_tone(uint16_t freq){
	ledcChangeFrequency(channel, freq, 8);
	ledcWrite(channel, volume);
}

void Gamepad_buzzer::stop(){
	if(buzz_task_handler != NULL && eTaskGetState(buzz_task_handler) == eBlocked)
		vTaskDelete(buzz_task_handler);
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

void Gamepad_buzzer::play_sequence(std::vector < Buzzer_element_t > sequence){
	if(buzz_task_handler != NULL && eTaskGetState(buzz_task_handler) == eBlocked)
		return;
	
	Buzzer_sequence_t *seq = new Buzzer_sequence_t();
	seq -> size = sequence.size();
	seq -> channel = channel;
	seq -> volume = &volume;
	seq -> data = new uint16_t[seq -> size * 2];

	for(uint32_t i = 0; i < seq -> size; i++){
		seq -> data[i * 2] = sequence[i].freq;
		seq -> data[i * 2 + 1] = sequence[i].timing;
	}

	xTaskCreatePinnedToCore(
		play_tone_seq_task,
		"buzz",
		1024,
		seq,
		2,
		&buzz_task_handler,
		xPortGetCoreID()
	);
}

void Gamepad_buzzer::play_sequence(uint16_t *data, uint32_t size){
	if(buzz_task_handler != NULL && eTaskGetState(buzz_task_handler) == eBlocked)
		return;
	
	Buzzer_sequence_t *seq = new Buzzer_sequence_t();
	seq -> size = size;
	seq -> channel = channel;
	seq -> volume = &volume;
	seq -> data = new uint16_t[seq -> size * 2];

	for(uint32_t i = 0; i < seq -> size * 2; i++){
		seq -> data[i] = data[i];
	}

	xTaskCreatePinnedToCore(
		play_tone_seq_task,
		"buzz",
		1024,
		seq,
		2,
		&buzz_task_handler,
		xPortGetCoreID()
	);
}




struct Period_task_param_t{
	uint16_t t1, t2;
	uint8_t n;
	uint8_t strength_;
	uint8_t channel;
};

void vib_periodic_task(void *parameters){
	Period_task_param_t *params = (Period_task_param_t *) parameters;

	for(uint8_t i = 0; i < params -> n; i++){
		ledcWrite(params -> channel, params -> strength_);
		vTaskDelay(pdMS_TO_TICKS(params -> t1));
		ledcWrite(params -> channel, 0);
		vTaskDelay(pdMS_TO_TICKS(params -> t2));
	}

	delete params;

	vTaskDelete(NULL);
}


void Gamepad_vibro::init(uint16_t pin, uint8_t channel_){
	channel = channel_;
    ledcSetup(channel, 25000, 8);
	ledcAttachPin(pin, channel);
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
	ledcWrite(channel, 0);
}


void Gamepad_vibro::enable_for_time(uint16_t time, uint8_t strength_){
	if(vib_task_handler != NULL && eTaskGetState(vib_task_handler) == eBlocked)
		return;
	
	Period_task_param_t *params = new Period_task_param_t();
	*params = (Period_task_param_t){time, 0, 1, calc_strength(strength_), channel};
	xTaskCreatePinnedToCore(
		vib_periodic_task,
		"vib",
		1024,
		params,
		2,
		&vib_task_handler,
		xPortGetCoreID()
	);
}

void Gamepad_vibro::enable_periodic(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_){
	if(vib_task_handler != NULL && eTaskGetState(vib_task_handler) == eBlocked)
		return;

	
	Period_task_param_t *params = new Period_task_param_t();
	*params = (Period_task_param_t){time_enabled, time_disabled, repeat_times, calc_strength(strength_), channel};
	xTaskCreatePinnedToCore(
		vib_periodic_task,
		"vib",
		1024,
		params,
		2,
		&vib_task_handler,
		xPortGetCoreID()
	);
}