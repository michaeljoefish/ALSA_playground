#include <stdio.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <alsa/control_plugin.h>

int main(){
	void** hints = NULL;
	int result = snd_device_name_hint(-1, "ctl", &hints);
	if (result != 0) {printf("Failed to get hints. Error Code: %d\n", result); return result;}
	int i = 0;
	char* hint_name = NULL;
	char* hint_desc = NULL;
	char* hint_iod = NULL;
	while (hints[i] != NULL){
		printf("%d\n", i);
		hint_name = snd_device_name_get_hint(hints[i], "NAME");
		hint_desc = snd_device_name_get_hint(hints[i], "DESC");
		hint_iod = snd_device_name_get_hint(hints[i], "IOD");
		if (hint_name != NULL) printf("NAME: %s\n", hint_name);
		if (hint_desc != NULL) printf("DESC: %s\n", hint_desc);
		if (hint_iod != NULL) printf("IOD: %s\n", hint_iod);
		i++;
	}

	snd_device_name_free_hint(hints);

	snd_ctl_t* ctl_handle = NULL;
	result = snd_ctl_open(&ctl_handle, "hw:CARD=AudioPCI", SND_CTL_NONBLOCK);

	if (result != 0){printf("Failed to open control interface. Error code: %d\n", result); return result;}
	
	snd_ctl_close(ctl_handle);


	return 0;
}

